#include "EzRadialCloner.h"
using namespace ffglex;
using namespace ffglqs;

static CFFGLPluginInfo PluginInfo(
	PluginFactory< EzRadialCloner >,// Create method
	"DG02",                      // Plugin unique ID of maximum length 4.
	"EZ Radial Cloner",          // Plugin name
	2,                           // API major version number
	1,                           // API minor version number
	1,                           // Plugin major version number
	0,                           // Plugin minor version number
	FF_EFFECT,                   // Plugin type
	"Take a wedge and repeat it a couple times.",  // Plugin description
	"Effects Plugin by Domegod (marten@metal-heart.org)"      // About
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

static const char _fragmentShaderCode[] = R"(#version 410 core
uniform sampler2D InputTexture;
uniform float Angle;
uniform float Area;
uniform float Clones;
uniform float Mirror;

in vec2 uv;
out vec4 fragColor;

#define PI 3.14159265358979323844

void main() {
  float clones = 1. + floor(20. * Clones);
  float angle = Angle * 2.*PI;
  bool mirror = Mirror > 0.5;
  // i.e. the target area to clone
  float area = Area * 2.*PI;

  // keep area from flippin'
  if (mirror) {
    area = 2.*PI - area;
  }

  vec2 normCoord = uv - vec2(0.5, 0.5);
  fragColor = vec4(0.,0.,0.,1.);
  
  // Convert Cartesian to Polar coords
  float r = length(normCoord);
  float theta = atan(normCoord.y, normCoord.x);
  
  // change theta
  theta = mod(theta + angle, 2.*PI / clones) - angle + area;
  if (mirror) {
    theta = (2.*PI / clones) - theta;
  }
  // Convert Polar back to Cartesian coords
  normCoord.x = r * cos(theta);
  normCoord.y = r * sin(theta);
  // Shift origin back to bottom-left (taking offset into account)
  vec2 new_uv = normCoord + vec2(0.5, 0.5);
  fragColor = texture(InputTexture, new_uv);
    
  return;
}

)";

EzRadialCloner::EzRadialCloner()
{
	// Input properties
	SetMinInputs( 4 );
	SetMaxInputs( 4 );

	AddParam( Param::Create( "Angle" ) );
	AddParam( Param::Create( "Area" ) );
	AddParam( Param::Create( "Clones" ) );
	AddParam( Param::Create( "Mirror" ) );

}
EzRadialCloner::~EzRadialCloner()
{
}

FFResult EzRadialCloner::InitGL( const FFGLViewportStruct* vp )
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
FFResult EzRadialCloner::ProcessOpenGL( ProcessOpenGLStruct* pGL )
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

	//This takes care of sending all the parameter that the plugin registered to the shader.
	SendParams( shader );

	quad.Draw();

	return FF_SUCCESS;
}
FFResult EzRadialCloner::DeInitGL()
{
	shader.FreeGLResources();
	quad.Release();

	return FF_SUCCESS;
}
