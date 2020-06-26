#include "RadialMask.h"
using namespace ffglex;

static CFFGLPluginInfo PluginInfo(
	PluginFactory< RadialMask >,// Create method
	"DGRM",                      // Plugin unique ID of maximum length 4.
	"Radial Mask",            // Plugin name
	2,                           // API major version number
	1,                           // API minor version number
	1,                           // Plugin major version number
	0,                           // Plugin minor version number
	FF_EFFECT,                   // Plugin type
	"Mask along polar coords",  // Plugin description
	"by domegod (marten@metal-heart.org)"      // About
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
#define PI 3.14159265358979323844
uniform sampler2D InputTexture;
uniform float Angle;
uniform float Range;

in vec2 uv;

out vec4 fragColor;

void main() {
  float angle = PI * (Angle - 180.) / 180.; // [ -pi, pi ]
  float range = PI * Range; // [ 0, pi ]
	vec2 p = uv;
  vec2 normCoord = 2 * (uv - 0.5); // -1 to 1

  float theta = atan(normCoord.y, normCoord.x);

  // range
  float shortest_diff = min(    
    abs(angle - (theta - 2*PI)), 
    min(
      abs(angle - theta),
      abs(angle - (theta + 2*PI))
    )
  );
  
  if (shortest_diff < range) {
    fragColor = texture(InputTexture, uv);
  } else {
    fragColor = vec4(0,0,0,0);
  }

  return;
}

)";

RadialMask::RadialMask()
{
	// Input properties
	SetMinInputs( 1 );
	SetMaxInputs( 1 );

	//We declare that this plugin has a Brightness parameter which is a RGB param.
	//The name here must match the one you declared in your fragment shader.
	AddParam( angle = ffglqs::ParamRange::Create( "Angle", 0.0f, { 0, 360 } ) );
	AddParam( range = ffglqs::Param::Create( "Range", 0.25f ) );
}
RadialMask::~RadialMask()
{
}

FFResult RadialMask::InitGL( const FFGLViewportStruct* vp )
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
FFResult RadialMask::ProcessOpenGL( ProcessOpenGLStruct* pGL )
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
	shader.Set( "Angle", angle->GetValue() );
	shader.Set( "Range", range->GetValue() );

	//The input texture's dimension might change each frame and so might the content area.
	//We're adopting the texture's maxUV using a uniform because that way we dont have to update our vertex buffer each frame.
	FFGLTexCoords maxCoords = GetMaxGLTexCoords( *pGL->inputTextures[ 0 ] );
	shader.Set( "MaxUV", maxCoords.s, maxCoords.t );

	//This takes care of sending all the parameter that the plugin registered to the shader.
	SendParams( shader );

	quad.Draw();

	return FF_SUCCESS;
}
FFResult RadialMask::DeInitGL()
{
	shader.FreeGLResources();
	quad.Release();

	return FF_SUCCESS;
}
