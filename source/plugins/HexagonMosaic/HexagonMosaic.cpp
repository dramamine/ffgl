#include "HexagonMosaic.h"
using namespace ffglex;
using namespace ffglqs;

static CFFGLPluginInfo PluginInfo(
	PluginFactory< HexagonMosaic >,// Create method
	"DG03",                      // Plugin unique ID of maximum length 4.
	"Hexagon Mosaic",            // Plugin name
	2,                           // API major version number
	1,                           // API minor version number
	1,                           // Plugin major version number
	0,                           // Plugin minor version number
	FF_EFFECT,                   // Plugin type
	"Hexagon mosaic",  // Plugin description
	"Effect by domegod (marten@metal-heart.org)"      // About
);

static const char _vertexShaderCode[] = R"(#version 410 core
uniform vec2 MaxUV;

layout( location = 0 ) in vec4 vPosition;
layout( location = 1 ) in vec2 vUV;

out vec2 uv;

void main()
{
	gl_Position = vPosition;
	uv = vUV * MaxUV;
}
)";

static const char _fragmentShaderCode[] = R"(
#version 410 core

uniform float Width;
uniform float Blending;
uniform float XOffset;
uniform float YOffset;
uniform sampler2D InputTexture;
in vec2 uv;
uniform vec2 MaxUV;
out vec4 fragColor;

  // hexagon width
  float w = Width/10.;
  float r = w * sqrt(3)/2.0;
  int sampling = int(clamp((1 + floor(7*Blending)), 1, 7));

// from stackoverflow
bool point_inside_trigon(vec2 s, vec2 a, vec2 b, vec2 c)
{
    float as_x = s.x-a.x;
    float as_y = s.y-a.y;
    bool s_ab = (b.x-a.x)*as_y-(b.y-a.y)*as_x > 0;
    if((c.x-a.x)*as_y-(c.y-a.y)*as_x > 0 == s_ab) return false;
    if((c.x-b.x)*(s.y-b.y)-(c.y-b.y)*(s.x-b.x) > 0 != s_ab) return false;
    return true;
}

// for a given point, walk down and left to find an orientation point for the hexagon.
// orientation points are the lower left corners of the hexagons.
vec2 find_op(vec2 p) {
  vec2 center = MaxUV/2.0 
    - vec2(w/2.0, r) 
    + vec2(XOffset*(3*w)/MaxUV.x,YOffset*(2*r)/MaxUV.y);

	float lane_width = 3 * w / 2.0;
	int which_lane = int(floor((p.x - center.x) / lane_width));
	float offset = (which_lane % 2) * r;
	float stripe_height = 2*r;
	int which_stripe = int(floor((p.y - center.y - offset) / stripe_height));
	return vec2(which_lane * lane_width + center.x, which_stripe * stripe_height + offset + center.y);
}

// what color should this hexagon be? (by orientation point)
vec3 op_to_color(vec2 p) {
  vec2 offsets[7];
  offsets[0] = vec2(0.5*w,w); // center
  offsets[1] = vec2(0.25*w, 0.5*w); // bottom left...etc.
  offsets[2] = vec2(0.75*w, 1.5*w);
  offsets[3] = vec2(0.75*w, 0.5*w);
  offsets[4] = vec2(0.25*w, 1.5*w);
  offsets[5] = vec2(w, w);
  offsets[6] = vec2(0, w);

  vec3 acc = vec3(0,0,0);
  for (int i = 0; i < sampling; i++) {
    acc += texture(InputTexture, clamp(p + offsets[i], vec2(0,0), vec2(1,1))).rgb;
  }
  return acc /= sampling;
}

void main() {
	vec2 p = uv;
  if (w < 0.001) {
    fragColor = texture(InputTexture, uv);
    return;
  };

  // the lower left corner of the hexagon I'm in
  vec2 op = find_op(p);
  // @TODO could just check y coordinate
  bool in_square = (p.x >= op.x && p.x < op.x+w &&
    p.y >= op.y && p.y < op.y+2*r);

  bool inside_rightedge = false;
  if (!in_square) {
    inside_rightedge = point_inside_trigon(
      p,
      op + vec2(w,0.0),
      op + vec2(3*w/2.0,w*sqrt(3)/2.0),
      op + vec2(w,w*sqrt(3))
    );

    if (!inside_rightedge) {
      bool above = p.y > op.y + r;
      op += vec2(1.5*w, 2*r*int(above)-r);
    }
  }

  vec3 color = op_to_color(op);
  fragColor = vec4(color, 1.0);
}


)";

HexagonMosaic::HexagonMosaic()
{
	// Input properties
	SetMinInputs( 1 );
	SetMaxInputs( 1 );

	AddParam( width = ffglqs::Param::Create( "Width", 0.2f ) );
	AddParam( blending = ffglqs::Param::Create( "Blending", 0.0f ) );
	AddParam( xoffset = ffglqs::Param::Create( "XOffset", 0.f ) );
	AddParam( yoffset = ffglqs::Param::Create( "YOffset", 0.f ) );
}
HexagonMosaic::~HexagonMosaic()
{
}

FFResult HexagonMosaic::InitGL( const FFGLViewportStruct* vp )
{
	if( !shader.Compile( _vertexShaderCode, _fragmentShaderCode ) )
	{
		DeInitGL();
		return FF_FAIL;
	}
	if( !quad.Initialise() )
	{
		DeInitGL();
		return FF_FAIL;
	}

	//Use base-class init as success result so that it retains the viewport.
	return CFFGLPlugin::InitGL( vp );
}
FFResult HexagonMosaic::ProcessOpenGL( ProcessOpenGLStruct* pGL )
{
	if( pGL->numInputTextures < 1 )
		return FF_FAIL;

	if( pGL->inputTextures[ 0 ] == NULL )
		return FF_FAIL;

	//FFGL requires us to leave the context in a default state on return, so use this scoped binding to help us do that.
	ScopedShaderBinding shaderBinding( shader.GetGLID() );
	//The shader's sampler is always bound to sampler index 0 so that's where we need to bind the texture.
	//Again, we're using the scoped bindings to help us keep the context in a default state.
	ScopedSamplerActivation activateSampler( 0 );
	Scoped2DTextureBinding textureBinding( pGL->inputTextures[ 0 ]->Handle );

	shader.Set( "inputTexture", 0 );

	//The input texture's dimension might change each frame and so might the content area.
	//We're adopting the texture's maxUV using a uniform because that way we dont have to update our vertex buffer each frame.
	FFGLTexCoords maxCoords = GetMaxGLTexCoords( *pGL->inputTextures[ 0 ] );
	shader.Set( "MaxUV", maxCoords.s, maxCoords.t );

	shader.Set( "Width", width->GetValue() );
	shader.Set( "Blending", blending->GetValue() );
	shader.Set( "XOffset", xoffset->GetValue() );
	shader.Set( "YOffset", yoffset->GetValue() );

	//This takes care of sending all the parameter that the plugin registered to the shader.
	SendParams( shader );

	quad.Draw();

	return FF_SUCCESS;
}
FFResult HexagonMosaic::DeInitGL()
{
	shader.FreeGLResources();
	quad.Release();

	return FF_SUCCESS;
}
