#include "Redness.h"
#include <iostream>
#include <fstream>

using namespace ffglex;
using namespace ffglqs;

std::string readFile( const char* filePath )
{
	std::string content;
	std::ifstream fileStream( filePath, std::ios::in );
	std::string line = "";
	while( !fileStream.eof() )
	{
		getline( fileStream, line );
		content.append( line + "\n" );
	}
	fileStream.close();
	return content;
}

static CFFGLPluginInfo PluginInfo(
	PluginFactory< Redness >,  // Create method
	"DG02",                    // Plugin unique ID of maximum length 4.
	"AA TEST Redness",         // Plugin name
	2,                         // API major version number
	1,                         // API minor version number
	1,                         // Plugin major version number
	0,                         // Plugin minor version number
	FF_EFFECT,                 // Plugin type
	"Add and Subtract colours",// Plugin description
	"Resolume FFGL Example"    // About
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
uniform vec3 Brightness;
uniform float Redness;

in vec2 uv;

out vec4 fragColor;

void main()
{
	vec4 color = texture( InputTexture, uv );
    color.r = Redness;
	fragColor = color;
}
)";

Redness::Redness()
{
	// Input properties
	SetMinInputs( 2 );
	SetMaxInputs( 2 );

	//We declare that this plugin has a Brightness parameter which is a RGB param.
	//The name here must match the one you declared in your fragment shader.
	AddRGBColorParam( "Brightness" );
	AddParam( Param::Create( "Redness" ) );
}
Redness::~Redness()
{
}

FFResult Redness::InitGL( const FFGLViewportStruct* vp )
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
FFResult Redness::ProcessOpenGL( ProcessOpenGLStruct* pGL )
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
FFResult Redness::DeInitGL()
{
	shader.FreeGLResources();
	quad.Release();

	return FF_SUCCESS;
}
