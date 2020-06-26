// https://www.shadertoy.com/view/WtfcRl
// By Morgan McGuire @morgan3d, http://graphicscodex.com
// Reuse permitted under the BSD license.

// All noise functions are designed for values on integer scale.
// They are tuned to avoid visible periodicity for both positive and
// negative coordinates within a few orders of magnitude.

// For a single octave
//#define NOISE noise

// For multiple octaves
#define NOISE fbm
#define NUM_NOISE_OCTAVES 2

// Precision-adjusted variations of https://www.shadertoy.com/view/4djSRW
float hash(float p) { p = fract(p * 0.011); p *= p + 7.5; p *= p + p; return fract(p); }
float hash(vec2 p) {vec3 p3 = fract(vec3(p.xyx) * 0.13); p3 += dot(p3, p3.yzx + 3.333); return fract((p3.x + p3.y) * p3.z); }

float noise(float x) {
    float i = floor(x);
    float f = fract(x);
    float u = f * f * (3.0 - 2.0 * f);
    return mix(hash(i), hash(i + 1.0), u);
}


float noise(vec2 x) {
    vec2 i = floor(x);
    vec2 f = fract(x);

	// Four corners in 2D of a tile
	float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    // Simple 2D lerp using smoothstep envelope between the values.
	// return vec3(mix(mix(a, b, smoothstep(0.0, 1.0, f.x)),
	//			mix(c, d, smoothstep(0.0, 1.0, f.x)),
	//			smoothstep(0.0, 1.0, f.y)));

	// Same code, with the clamps in smoothstep and common subexpressions
	// optimized away.
    vec2 u = f * f * (3.0 - 2.0 * f);
	return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}


float noise(vec3 x) {
    const vec3 step = vec3(110, 241, 171);

    vec3 i = floor(x);
    vec3 f = fract(x);
 
    // For performance, compute the base input to a 1D hash from the integer part of the argument and the 
    // incremental change to the 1D based on the 3D -> 1D wrapping
    float n = dot(i, step);

    vec3 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(mix( hash(n + dot(step, vec3(0, 0, 0))), hash(n + dot(step, vec3(1, 0, 0))), u.x),
                   mix( hash(n + dot(step, vec3(0, 1, 0))), hash(n + dot(step, vec3(1, 1, 0))), u.x), u.y),
               mix(mix( hash(n + dot(step, vec3(0, 0, 1))), hash(n + dot(step, vec3(1, 0, 1))), u.x),
                   mix( hash(n + dot(step, vec3(0, 1, 1))), hash(n + dot(step, vec3(1, 1, 1))), u.x), u.y), u.z);
}


float fbm(float x) {
	float v = 0.0;
	float a = 0.5;
	float shift = float(100);
	for (int i = 0; i < NUM_NOISE_OCTAVES; ++i) {
		v += a * noise(x);
		x = x * 2.0 + shift;
		a *= 0.5;
	}
	return v;
}


// float fbm(vec2 x) {
// 	float v = 0.0;
// 	float a = 0.5;
// 	vec2 shift = vec2(100);
// 	// Rotate to reduce axial bias
//     mat2 rot = mat2(cos(0.5), sin(0.5), -sin(0.5), cos(0.50));
// 	for (int i = 0; i < NUM_NOISE_OCTAVES; ++i) {
// 		v += a * noise(x);
// 		x = rot * x * 2.0 + shift;
// 		a *= 0.5;
// 	}
// 	return v;
// }


// float fbm(vec3 x) {
// 	float v = 0.0;
// 	float a = 0.5;
// 	vec3 shift = vec3(100);
// 	for (int i = 0; i < NUM_NOISE_OCTAVES; ++i) {
// 		v += a * noise(x);
// 		x = x * 2.0 + shift;
// 		a *= 0.5;
// 	}
// 	return v;
// }


//////////////////////////////////////////////////////////////////////////////////////
// Visualization:
const float pi          = 3.1415926535;
float square(float x) { return x * x; }


void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
  vec2 uv = fragCoord/iResolution.xy;
	float v = 0.0;
  float coord = fragCoord.y * 0.06 + iTime * 1.0 - 10.0;
  float height = NOISE(coord);
  vec2 useCoord = clamp(uv+vec2(height/12.,0), 0.0, 1.0);
    if (useCoord.x > 1.0) {
        fragColor = vec4(0,0,0,1);
        return;
    }

  fragColor = texture(iChannel0, useCoord);

    // Visualize with a fun color map	
	// fragColor.rgb = pow(v, 0.35) * 1.3 * normalize(vec3(0.5, fragCoord.xy / iResolution.xy)) + vec3(v * 0.25);
}