#define FRAME_RANGE_PIXELS 8.

float czm_luminance(vec3 rgb)
{
    // Algorithm from Chapter 10 of Graphics Shaders.
    const vec3 W = vec3(0.2125, 0.7154, 0.0721);
    return dot(rgb, W);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = fragCoord/iResolution.xy;
    if (iFrame < 10) {
      fragColor = texture(iChannel0, uv);
      return;
    }
    // the frame number parity, -1 is odd 1 is even
	  //float fParity = mod(float(iFrame), 2.) * 2. - 1.;
    
    vec2 dir = vec2(1, 0);
    //dir*= fParity;
	  dir/= iResolution.xy;
    
    // my position in the frame
    float mypos = floor(mod(fragCoord.x*iResolution.x, FRAME_RANGE_PIXELS));
    vec4 me = texture(iChannel1, uv);
    float me_lum = czm_luminance(me.rgb);
    vec4 comp;
    float comp_lum;
    
    if (mypos < FRAME_RANGE_PIXELS/2.) {
      // look to the right for lighter pixels
      for (float i = FRAME_RANGE_PIXELS-1.; i > FRAME_RANGE_PIXELS/2.; i--) {
        if (uv.x + (dir * (i - mypos)).x > 1.0) continue;
        comp = texture(iChannel1, uv + dir * (i - mypos));
        comp_lum = czm_luminance(comp.rgb);
        if (comp_lum > me_lum) {
          fragColor = comp;
          return;
        }
      }
      
    } else {
      // look to the left for darker pixels
      for (float i = 0.; i < FRAME_RANGE_PIXELS/2.; i++) {
        if (uv.x + (dir * (i - mypos)).x < 0.0) continue;
        comp = texture(iChannel1, uv + dir * (i - mypos));
        comp_lum = czm_luminance(comp.rgb);
        if (comp_lum < me_lum) {
          fragColor = comp;
          return;
        }
      }
    }
    
    // Output to screen
    fragColor = me;
    return;
}
