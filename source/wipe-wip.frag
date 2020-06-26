#define SAMPLE_RATE 50
#define MODE 1.

int direction = mod(MODE, 2.) == 0. ? 1 : -1;
int d2 = MODE >= 2. ? -1 : 1;

// grayscale average of the colors
float gscale (vec3 c) { return (c.r+c.g+c.b)/3.; }

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = fragCoord/iResolution.xy;
	fragColor = texture(iChannel0, uv);
    float origg = gscale(fragColor.rgb);
    float THRESHOLD = 0.25*iMouse.x/iResolution.x;
    float DISTANCE = iMouse.y/iResolution.y;
   
    
    for (int i = 0; i < SAMPLE_RATE; i++) {
        vec4 comp = texture(iChannel0, uv+float(i)*vec2(float(d2)*DISTANCE/float(SAMPLE_RATE),0.));
        float compg = gscale(comp.rgb);
        
        if (float(direction)*(origg-compg)>THRESHOLD) {
            fragColor = comp;
            return;
        }
            
    }
    return;
    
    // Time varying pixel color
    //vec3 col = 0.5 + 0.5*cos(iTime+uv.xyx+vec3(0,2,4));

    // Output to screen
    // fragColor = vec4(col,1.0);
}