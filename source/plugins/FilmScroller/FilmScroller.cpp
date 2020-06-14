#include "FilmScroller.h"
using namespace ffglex;
using namespace ffglqs;

static CFFGLPluginInfo PluginInfo(
	PluginFactory< FilmScroller >,// Create method
	"DGFS",                    // Plugin unique ID of maximum length 4.
	"Film Scroller",           // Plugin name
	2,                         // API major version number
	1,                         // API minor version number
	1,                         // Plugin major version number
	0,                         // Plugin minor version number
	FF_EFFECT,                 // Plugin type
	"Create and scroll tiles", // Plugin description
	"marten@metal-heart.org"   // About
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
uniform int VTiles;
uniform float TileSize; // as %age of width and height
uniform vec2 TileOffset;
uniform float VertScroll;
uniform float HueScaling;
uniform float HueOffset;
uniform bool TilesOnly;
uniform float FilmholePct;
uniform float FilmholeSize;
uniform float LaneWidth;
uniform float FilmAlpha;
in vec2 uv;

out vec4 fragColor;
// precision mediump float;

#define UGH_MARGIN 0.01
#define HEIGHTFIX 0.945

vec3 hueShift(vec3 color, float hue) {
    const vec3 k = vec3(0.57735, 0.57735, 0.57735);
    float cosAngle = cos(hue);
    return vec3(color * cosAngle + cross(k, color) * sin(hue) + k * dot(k, color) * (1.0 - cosAngle));
}

void main( )
{	
    vec2 new_uv = uv;
    vec2 time_adjusted_uv = vec2(uv.x, mod(uv.y + VertScroll, 1.));
    vec4 color = vec4(0,0,0,1);
	if (!TilesOnly) {
		color = texture(InputTexture, uv);
    }
    float hs = 0.;
    // determine tile ranges.
    // ex. 0.1 - 0.25, 0.35 - 0.5, 0.6-0.75 
    // int horizontal_section;
    // @TODO use modf from opengl4
    // float d = modf(uv.x / (TileSize.x + TileOffset.x), horizontal_section);
    float xpos_in_section = mod(uv.x, (TileSize + TileOffset.x)); // ex. 0 - 0.25
    int x_section = int(trunc(uv.x / (TileSize + TileOffset.x)));


	// ex. size .5 offset .0:
    // y of .51 becomes .1
    float ypos_in_section = mod(time_adjusted_uv.y, (TileSize + TileOffset.y)); 
    // .51 / .50 = 1.02
    int y_section = int(trunc(time_adjusted_uv.y / (TileSize + TileOffset.y)));

    if (xpos_in_section > TileOffset.x + UGH_MARGIN && 
        xpos_in_section < TileOffset.x + TileSize - UGH_MARGIN &&
        ypos_in_section > TileOffset.y + UGH_MARGIN && 
        ypos_in_section < TileOffset.y + HEIGHTFIX * TileSize - UGH_MARGIN && 
        x_section < 1 && y_section < VTiles) {
      
      // adapt from range 0.1-0.25 to 0-1
      // @TODO clamp
      float mx = (xpos_in_section - TileOffset.x) / TileSize;
      float my = (ypos_in_section - TileOffset.y) / TileSize;
      
		int tilenumber = y_section + x_section * VTiles;
		hs = 6.25 * HueOffset + HueScaling * 2.1 * float(tilenumber);
        
		new_uv = vec2(mx, my);
		color = texture(InputTexture, new_uv);
		color = vec4(hueShift(color.rgb, hs), color.a);

    // blackspace eligible
    } else if (uv.x > TileOffset.x - LaneWidth &&
               uv.x < TileOffset.x + TileSize + LaneWidth) { 
    bool filmlane = uv.x < TileOffset.x || uv.x > TileOffset.x + TileSize;
	float distance_from_center = min(
        abs( uv.x - (TileOffset.x - (LaneWidth - UGH_MARGIN) * 0.5) ),
        abs( uv.x - (TileOffset.x + TileSize + (LaneWidth - UGH_MARGIN) * 0.5) )
    );
    float filmhole = mod( uv.y + VertScroll, FilmholeSize);
    if (distance_from_center > 0.01 || !(filmlane && filmhole < FilmholeSize * FilmholePct)) {   
        fragColor = vec4(0,0,0,FilmAlpha);
        return;
    }
        
    }
    fragColor = color;
}


)";

FilmScroller::FilmScroller()
{
	// Input properties
	SetMinInputs( 1 );
	SetMaxInputs( 1 );

	AddParam( vtiles = ParamRange::Create( "Vertical Tiles", 3, { 1, 10 } ) );
	AddParam( tilex = Param::Create( "Size", 0.25f ) );
	AddParam( offsetx = Param::Create( "X Offset", 0.1f ) );
	AddParam( offsety = Param::Create( "Y Offset", 0.0f ) );
	AddParam( huescale = Param::Create( "Hue Scaling", 0.0f ) );
	AddParam( hueoffset = Param::Create( "Hue Offset", 0.0f ) );
	AddParam( filmholesize = Param::Create( "Filmhole Size", 0.1f ) );
	AddParam( filmholepct = Param::Create( "Filmhole Pct", 0.5f ) );
	AddParam( lanewidth = Param::Create( "Lane Width", 0.05f ) );
	AddParam( filmalpha = Param::Create( "Film Alpha", 1.0f ) );
	AddParam( scroll = Param::Create( "Vertical Scroll", 0.0f ) );
	AddParam( tilesonly = ParamBool::Create( "Tiles Only", 0.0f ) );
}
FilmScroller::~FilmScroller()
{
}

FFResult FilmScroller::InitGL( const FFGLViewportStruct* vp )
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
FFResult FilmScroller::ProcessOpenGL( ProcessOpenGLStruct* pGL )
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

	shader.Set( "VTiles", int( vtiles->GetValue() ) );
	shader.Set( "TileSize", tilex->GetValue() );
	shader.Set( "TileOffset", offsetx->GetValue(), offsety->GetValue() );
	shader.Set( "VertScroll", scroll->GetValue() );
	shader.Set( "HueScaling", huescale->GetValue() );
	shader.Set( "HueOffset", hueoffset->GetValue() );
	shader.Set( "TilesOnly", tilesonly->GetValue() );
	shader.Set( "FilmholeSize", filmholesize->GetValue() );
	shader.Set( "FilmholePct", filmholepct->GetValue() );
	shader.Set( "FilmAlpha", filmalpha->GetValue() );
	shader.Set( "LaneWidth", lanewidth->GetValue() );

	//This takes care of sending all the parameter that the plugin registered to the shader.
	SendParams( shader );

	quad.Draw();

	return FF_SUCCESS;
}
FFResult FilmScroller::DeInitGL()
{
	shader.FreeGLResources();
	quad.Release();

	return FF_SUCCESS;
}
