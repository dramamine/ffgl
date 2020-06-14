#version 410 core
#define THRESHOLD 0.2
precision mediump float;

uniform sampler2D InputTexture;
uniform sampler2D OldTexture;
uniform vec2 Resolution;
uniform bool DoGol;
uniform int Frame;
in vec2 uv;
uniform vec2 MaxUV;
out vec4 fragColor;

// grayscale average of the colors
float gscale (vec3 c) { return (c.r+c.g+c.b)/3.; }


void main()
{
	vec2 st = uv * MaxUV;
	vec4 color = texture( OldTexture, uv);

	fragColor = color;
	if (DoGol)
	{
		// the frame number parity, -1 is odd 1 is even
		float fParity = mod(float(Frame), 2.) * 2. - 1.;
    
	    // we differentiate every 1/2 pixel on the horizontal axis, will be -1 or 1
	    //float vp = mod(floor(uv.x * Resolution.x), 2.0) * 2. - 1.;
	    float vp = mod(floor(uv.x * Resolution.x), 2.0) * 2. - 1.;
    
		vec2 dir = vec2(1, 0);
	    dir*= fParity * vp;
		dir/= Resolution.xy;

		// we sort
		vec4 curr = texture(InputTexture, uv);
		vec4 comp = texture(InputTexture, uv);
		// vec4 comp = texture(InputTexture, uv + dir);
	
		float gCurr = gscale(curr.rgb);
		float gComp = gscale(comp.rgb);
	
	
		// we prevent the sort from happening on the borders
		if (uv.x + dir.x < 0.0 || uv.x + dir.x > 1.0) {
			fragColor = curr;
			return;
		}
	
		// the direction of the displacement defines the order of the comparaison 
		if (dir.x < 0.0) {
			if (gCurr > THRESHOLD && gComp > gCurr) {
				fragColor = comp;
			} else {
				fragColor = curr;
			}
		} 
		else {
			if (gComp > THRESHOLD && gCurr >= gComp) {
				fragColor = comp;
			} else {
				fragColor = curr;
			}
		}
	}
	
	
}
