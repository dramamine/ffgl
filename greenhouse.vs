
/*{
    "CREDIT": "",
    "DESCRIPTION": "",
    "CATEGORIES": [ "effect" ],
    "INPUTS": [
        {
            "TYPE": "image",
            "NAME": "VideoInput",
            "LABEL": "Video"
        }
    ]
}*/

/*
ISF Reference
=========================

1) Valid inputs:
    - "event"
    - "bool"
    - "long"
    - "float"
    - "point2D"
    - "color"
    - "image"
    - "audio"
    - "audioFFT"

2) Functions:
    - IMG_NORM_PIXEL() -> get a pixel from input with normalized coordinates
    - IMG_PIXEL() -> get a pixel from input with screen space coordinates

3) Predefined variables:
    - RENDERSIZE (resolution of the shader)
    - TIME (run time)
    - gl_FragCoord.xy (screen space coordinates of current fragment)
    - isf_FragNormCoord.xy (normalized coordinates)

To learn more see:
https://github.com/mrRay/ISF_Spec/
*/
#define PI 3.14159265358979323844
float offset = 0.323;

vec2 center = vec2(0.5, 0.5);
vec2 o1 = vec2(604,1024-639)/1024;
vec2 o2 = vec2(667,1024-466)/1024;

vec2 o3 = vec2(712,1024-794)/1024;
vec2 o4 = vec2(797,1024-608)/1024;
vec2 o5 = vec2(846,1024-409)/1024;

vec2 o6 = vec2(806,1024-925)/1024;
vec2 o7 = vec2(913,1024-763)/1024;

vec2 o8 = vec2(983,1024-553)/1024;
vec2 o9 = vec2(1000,1024-359)/1024;

vec2 tri1[3] = vec2[](o1, o2, center);
vec2 tri2[3] = vec2[](o3, o4, o1);
vec2 tri3[3] = vec2[](o1, o4, o2);
vec2 tri4[3] = vec2[](o4, o5, o2);

vec2 tri5[3] = vec2[](o6, o7, o3);
vec2 tri6[3] = vec2[](o3, o7, o4);
vec2 tri7[3] = vec2[](o7, o8, o4);
vec2 tri8[3] = vec2[](o4, o8, o5);
vec2 tri9[3] = vec2[](o8, o9, o5);


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

bool point_inside_trigon(vec2 s, vec2[3] t) {
	return point_inside_trigon(s, t[0], t[1], t[2]);
}

vec2[3] find_trigon(float r, vec2 c) {
	if (r < length(o2-center)) {
		if (point_inside_trigon(c, tri1)) {
					gl_FragColor = vec4(0,1,0,1);
          return tri1;
				}
			else if (point_inside_trigon(c, tri3)) {
					gl_FragColor = vec4(1,1,0,1);
          return tri3;
				}
		} else if (r < length(o4-center)) {
			if (point_inside_trigon(c, tri2)) {
					gl_FragColor = vec4(0,1,0,1);
          return tri2;
				}
			else if (point_inside_trigon(c, tri3)) {
					gl_FragColor = vec4(1,1,0,1);
          return tri3;
				}
			else if (point_inside_trigon(c, tri4)) {
					gl_FragColor = vec4(0,0.5,1,1);
          return tri4;
				}
		}  else if (r < length(o5-center)) {
			if (point_inside_trigon(c, tri2)) {
					gl_FragColor = vec4(0,1,0,1);
          return tri2;
				}
			else if (point_inside_trigon(c, tri4)) {
					gl_FragColor = vec4(0,0.5,1,1);
          return tri4;
				}
			else if (point_inside_trigon(c, tri6)) {
					gl_FragColor = vec4(1,0.5,0,1);
          return tri6;
				}			
			else if (point_inside_trigon(c, tri7)) {
					gl_FragColor = vec4(0.5,0.5,0,1);
          return tri7;
				}			
			else if (point_inside_trigon(c, tri8)) {
					gl_FragColor = vec4(1,0.5,0,1);
          return tri8;
				}			
		}  else {
			if (point_inside_trigon(c, tri5)) {
					gl_FragColor = vec4(0,1,0,1);
          return tri5;
				}
			else if (point_inside_trigon(c, tri6)) {
					gl_FragColor = vec4(1,0.5,0,1);
          return tri6;
				}			
			else if (point_inside_trigon(c, tri7)) {
					gl_FragColor = vec4(0.5,0.5,0,1);
          return tri7;
				}			
			else if (point_inside_trigon(c, tri8)) {
					gl_FragColor = vec4(1,0.5,0,1);
          return tri8;
				}		
			else if (point_inside_trigon(c, tri9)) {
					gl_FragColor = vec4(0.5,0.5,0,1);
          return tri9;
				}				
		}
}

float dist(vec2 pt1, vec2 pt2, vec2 pt3)
{
   vec2 v1 = pt2 - pt1;
   vec2 v2 = pt1 - pt3;
   vec2 v3 = vec2(v1.y,-v1.x);
   return abs(dot(v2,normalize(v3)));
}

void main() {
	gl_FragColor = vec4(isf_FragNormCoord.x, 0.0 , isf_FragNormCoord.y, 1.0);
	vec2 normCoord = isf_FragNormCoord - vec2(0.5, 0.5);
	float r = length(normCoord);
    float theta = mod( atan(normCoord.y, normCoord.x), 2 * PI);
	
	float segment = mod( round( (theta+offset) / (.4 * PI)), 5);
	gl_FragColor = vec4(segment / 5,0,0,1);
	
	theta -= segment * (.4 * PI);
	vec2 c = vec2(r * cos(theta), r * sin(theta)) + vec2(0.5, 0.5);

	vec2 mytriangle[3] = find_trigon(r, c);
	float mytriangle_height = dist(mytriangle[0], mytriangle[1], mytriangle[2]);
	
	float normalized_y = dist(mytriangle[0], mytriangle[1], c) / mytriangle_height;
	// gl_FragColor = vec4(normalized_y, 0, 0, 1);
	
	// find the vector that's p0 -> p1 rotated 90 degrees
	vec2 x2y2 = vec2(
	    mytriangle[0].x - (mytriangle[1].y - mytriangle[0].y),
	    mytriangle[0].y + (mytriangle[1].x - mytriangle[0].x)
	);
	float normalized_x = dist(mytriangle[0], x2y2, c) / distance(mytriangle[0], mytriangle[1]);
	// gl_FragColor = vec4(normalized_x, 0, 0, 1);
	gl_FragColor = IMG_NORM_PIXEL(VideoInput, vec2(normalized_x, normalized_y));
	
	
}/*{
    "CREDIT": "",
    "DESCRIPTION": "",
    "CATEGORIES": [ "generator" ],
    "INPUTS": [
        {
			"TYPE": "float",
            "NAME": "MR",
            "LABEL": "Mod Red",
            "DEFAULT": 1
        }
	,
	    {
			"TYPE": "float",
            "NAME": "MB",
            "LABEL": "Mod Blue",
            "DEFAULT": 1	
        }
    ]
}*/
