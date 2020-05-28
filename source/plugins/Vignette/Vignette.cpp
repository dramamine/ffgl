#include "Vignette.h"
using namespace ffglex;

static CFFGLPluginInfo PluginInfo(
	PluginFactory< Vignette >,// Create method
	"VG01",                      // Plugin unique ID of maximum length 4.
	"Vignette DEMO",            // Plugin name
	2,                           // API major version number
	1,                           // API minor version number
	1,                           // Plugin major version number
	0,                           // Plugin minor version number
	FF_EFFECT,                   // Plugin type
	"Vignette effect",  // Plugin description
	"Resolume FFGL Example"      // About
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
uniform sampler2D InputTexture;
// rename to sensitivity
uniform float Sensitivity;

float sensitivity = (1 - Sensitivity) + 0.2;

in vec2 uv;
out vec4 fragColor;

void main( )
{
  float edginess = length(fwidth(texture2D(InputTexture, uv)));
  
  vec4 color;
  if (edginess > sensitivity) {
    color = vec4(1.,1.,1.,1.);
  } else {
    color = texture(InputTexture, uv);
  }

  fragColor = color;
}
)";

Vignette::Vignette()
{
	// Input properties
	SetMinInputs( 1 );
	SetMaxInputs( 1 );
	AddParam( ffglqs::Param::Create( "Sensitivity" ) );
}
Vignette::~Vignette()
{
}

FFResult Vignette::InitGL( const FFGLViewportStruct* vp )
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
FFResult Vignette::ProcessOpenGL( ProcessOpenGLStruct* pGL )
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
FFResult Vignette::DeInitGL()
{
	shader.FreeGLResources();
	quad.Release();

	return FF_SUCCESS;
}
