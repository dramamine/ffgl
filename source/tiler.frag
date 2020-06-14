#version 410 core
uniform sampler2D InputTexture;
uniform int VTiles;
uniform int HTiles;
uniform float TileSize;
uniform vec2 TileOffset;
uniform float VertScroll;
uniform int Frames;
uniform float HueScaling;
uniform float HueOffset;
uniform bool TilesOnly;
in vec2 uv;

out vec4 fragColor;
// precision mediump float;

#define VERT_SCROLL 0.
#define HUE_SCALING 1.
#define HUE_OFFSET 0.

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
    // y of .51 becomes .02
    // float ypos_in_section = mod(time_adjusted_uv.y, (TileSize + TileOffset.y)); 
    // .51 / .50 = 1.02
    // int y_section = int(trunc(time_adjusted_uv.y / (TileSize + TileOffset.y)));
    int y_section;
	float ypos_in_section = modf(time_adjusted_uv.y / (TileSize + TileOffset.y), y_section);
	

    if (xpos_in_section > TileOffset.x && ypos_in_section > TileOffset.y && x_section < HTiles && y_section < VTiles) {
      
      // adapt from range 0.1-0.25 to 0-1
      // @TODO clamp
      float mx = (xpos_in_section - TileOffset.x) / TileSize;
      float my = mod((ypos_in_section - TileOffset.y) / TileSize, 1);
      
		int tilenumber = y_section + x_section * VTiles;
		hs = 6.25 * HueOffset + HueScaling * 2.1 * float(tilenumber);
        
		new_uv = vec2(mx, my);
		color = texture(InputTexture, new_uv);
		color = vec4(hueShift(color.rgb, hs), color.a);

		// debug
        color = vec4(0,1,0,1);

    }
    fragColor = color;
}