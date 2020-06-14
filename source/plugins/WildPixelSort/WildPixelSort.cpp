#include "WildPixelSort.h"
using namespace ffglex;

static CFFGLPluginInfo PluginInfo(
	PluginFactory< WildPixelSort >,// Create method
	"DGWP",                    // Plugin unique ID of maximum length 4.
	"AA Wild Pixelsort",               // Plugin name
	2,                         // API major version number
	1,                         // API minor version number
	1,                         // Plugin major version number
	0,                         // Plugin minor version number
	FF_EFFECT,                 // Plugin type
	"Wild pixel sorter",            // Plugin description
	""                         // About
);

static const char _vertexShaderCode[] = R"(#version 410 core
layout( location = 0 ) in vec4 vPosition;
layout( location = 1 ) in vec2 vUV;
out vec2 uv;
void main()
{
	gl_Position = vPosition;
	uv = vUV;
}
)";

static const char _fragmentShaderCode[] = R"(
#version 410 core
precision mediump float;

uniform sampler2D InputTexture;
uniform sampler2D OldTexture;
uniform vec2 Resolution;
uniform float Wildness;
uniform int Frame;
in vec2 uv;
uniform vec2 MaxUV;
uniform float Threshold;
out vec4 fragColor;

// grayscale average of the colors
float gscale (vec3 c) { return (c.r+c.g+c.b)/3.; }

void main()
{
  vec2 st = uv * MaxUV;
  vec4 color = texture( OldTexture, st);

  fragColor = color;
  if (Wildness < 1.) return;
  
  // the frame number parity, -1 is odd 1 is even
  float fParity = mod(float(Frame), 2.) * 2. - 1.;
  
  // we differentiate every 1/2 pixel on the horizontal axis, will be -1 or 1
  float vp = mod(floor(st.x * Resolution.x), 2.0) * 2. - 1.;
  
  vec2 dir = vec2(1, 0);
  dir*= fParity * vp;
  dir/= Resolution.xy;

  // we sort
  vec4 curr = texture(InputTexture, st);

  float gCurr = gscale(curr.rgb);
  
  // we prevent the sort from happening on the borders
  if (st.x + dir.x < 0.0 || st.x + dir.x > 1.0) {
    fragColor = curr;
    return;
  }

  vec4 comp;
  float gComp;
  
  for (int i = int(floor(Wildness)); i > 0; i--) {
    comp = texture(InputTexture, st + i * dir);
    gComp = gscale(comp.rgb);      
    if (gComp <= Threshold) continue;

    if (dir.x < 0.0) {
      if (gComp > gCurr) {
        fragColor = comp;
        return;
      }
    } else {
      if (gCurr > gComp) {
        fragColor = comp;
        return;
      }
    }
  }
  fragColor = curr;
}

)";

WildPixelSort::WildPixelSort() :
	texture( nullptr )
{
	// Input properties
	SetMinInputs( 1 );
	SetMaxInputs( 1 );

	AddParam( th = ffglqs::Param::Create( "Threshold", 0.2f ) );
	AddParam( wildness = ffglqs::ParamRange::Create( "Wildness", 0.0f, { 0, 8 } ) );
}
WildPixelSort::~WildPixelSort()
{
}

FFResult WildPixelSort::InitGL( const FFGLViewportStruct* vp )
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
FFResult WildPixelSort::ProcessOpenGL( ProcessOpenGLStruct* pGL )
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

	auto inputTex = pGL->inputTextures[ 0 ];

	if( !texture || wildness->GetValue() >= 1.0f )
	{
		auto textureBinding = Scoped2DTextureBinding( pGL->inputTextures[ 0 ]->Handle );

		shader.Set( "InputTexture", 0 );
		shader.Set( "OldTexture", 0 );

		//The input texture's dimension might change each frame and so might the content area.
		//We're adopting the texture's maxUV using a uniform because that way we dont have to update our vertex buffer each frame.
		FFGLTexCoords maxCoords = GetMaxGLTexCoords( *pGL->inputTextures[ 0 ] );
		shader.Set( "MaxUV", maxCoords.s, maxCoords.t );
		shader.Set( "Wildness", 0.0f );

		quad.Draw();
	}

	if( wildness->GetValue() >= 1.0f )
	{
		if( !frozen )
		{
			frozen = true;
			glDeleteTextures( 0, &handle );

			glGenTextures( 1, &handle );
			Scoped2DTextureBinding textureBinding( handle );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, inputTex->HardwareWidth, inputTex->HardwareHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );

			delete texture;
			texture         = new FFGLTextureStruct( *inputTex );
			texture->Handle = handle;

			glCopyTexSubImage2D( GL_TEXTURE_2D,                         //target
								 0,                                     //level
								 0,                                     //xoffset
								 0,                                     //yoffset
								 0,                                     //x
								 0,                                     //y
								 pGL->inputTextures[ 0 ]->HardwareWidth,//width
								 pGL->inputTextures[ 0 ]->HardwareHeight//height
			);
		}
	}

	if( texture )
	{
		shader.Set( "Frame", int( m_frame ) );
		if( wildness->GetValue() < 1.0f )
		{
			Scoped2DTextureBinding textureBinding( pGL->inputTextures[ 0 ]->Handle );
			shader.Set( "InputTexture", 0 );
			shader.Set( "OldTexture", 0 );

			FFGLTexCoords maxCoords = GetMaxGLTexCoords( *texture );
			shader.Set( "MaxUV", maxCoords.s, maxCoords.t );
			shader.Set( "Wildness", 0.0f );
			shader.Set( "Resolution", texture->Width, texture->Height );

			quad.Draw();
			// not sure if necessary
			glCopyTexSubImage2D( GL_TEXTURE_2D,         //target
								 0,                     //level
								 0,                     //xoffset
								 0,                     //yoffset
								 0,                     //x
								 0,                     //y
								 texture->HardwareWidth,//width
								 texture->HardwareHeight//height
			);
			if( frozen )
			{
				frozen = false;
				delete texture;
				texture         = new FFGLTextureStruct( *inputTex );
				texture->Handle = handle;
			}
		}
		else
		{
			Scoped2DTextureBinding textureBinding0( pGL->inputTextures[ 0 ]->Handle );
			shader.Set( "InputTexture", 0 );
			Scoped2DTextureBinding textureBinding1( texture->Handle );
			shader.Set( "OldTexture", 1 );
			FFGLTexCoords maxCoords = GetMaxGLTexCoords( *texture );
			shader.Set( "MaxUV", maxCoords.s, maxCoords.t );
			shader.Set( "Wildness", wildness->GetValue() );
			shader.Set( "Resolution", texture->Width, texture->Height );
			shader.Set( "Threshold", th->GetValue() );

			quad.Draw();

			glCopyTexSubImage2D( GL_TEXTURE_2D,         //target
								 0,                     //level
								 0,                     //xoffset
								 0,                     //yoffset
								 0,                     //x
								 0,                     //y
								 texture->HardwareWidth,//width
								 texture->HardwareHeight//height
			);
		}
	}
	m_frame++;
	return FF_SUCCESS;
}
FFResult WildPixelSort::DeInitGL()
{
	shader.FreeGLResources();
	quad.Release();

	return FF_SUCCESS;
}