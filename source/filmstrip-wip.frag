#version 410 core
uniform sampler2D InputTexture;
uniform int VTiles;
uniform int HTiles;
uniform float TileSize; // as %age of width and height
uniform vec2 TileOffset;
uniform float VertScroll;
uniform int Frames;
uniform float HueScaling;
uniform float HueOffset;
uniform bool TilesOnly;
in vec2 uv;

out vec4 fragColor;
// precision mediump float;

#define UGH_MARGIN 0.01
#define HEIGHTFIX 0.945
#define LANE_WIDTH 0.04
#define FILMHOLE_PCT 0.5
#define FILMHOLE_SIZE 0.05

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
        x_section < HTiles && y_section < VTiles) {
      
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
    } else if (uv.x > TileOffset.x - LANE_WIDTH &&
               uv.x < TileOffset.x + TileSize + LANE_WIDTH) { 
    bool filmlane = uv.x < TileOffset.x || uv.x > TileOffset.x + TileSize;
	float distance_from_center = min(
        abs( uv.x - (TileOffset.x - LANE_WIDTH * 0.5) ),
        abs( uv.x - (TileOffset.x + TileSize + LANE_WIDTH * 0.5) )
    );
    float filmhole = mod( ypos_in_section, FILMHOLE_SIZE);
    if (distance_from_center > 0.01 || !(filmlane && filmhole > FILMHOLE_SIZE * FILMHOLE_PCT)) {   
        fragColor = vec4(0,0,0,1); // @TODO transparency
        return;
    }
        
    }
    fragColor = color;
}
