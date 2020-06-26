#version 410 core
precision mediump float;

uniform sampler2D InputTexture;
uniform sampler2D OldTexture;
uniform float Wildness;
uniform int Frame;
in vec2 uv;
uniform vec2 MaxUV;
uniform float Threshold;
uniform bool Vertical;
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

  if (Vertical) {
    // we differentiate every 1/2 pixel on the horizontal axis, will be -1 or 1
    float vp = mod(floor(st.y * MaxUV.y), 2.0) * 2. - 1.;
    mod
    vec2 dir = vec2(0, 1);
    dir*= fParity * vp;
    dir/= MaxUV.xy;

    // we sort
    vec4 curr = texture(InputTexture, st);

    float gCurr = gscale(curr.rgb);
    
    // we prevent the sort from happening on the borders
    if (st.y + dir.y < 0.0 || st.y + dir.y > 1.0) {
      fragColor = curr;
      return;
    }

    vec4 comp;
    float gComp;
    
    for (int i = int(floor(Wildness)); i > 0; i--) {
      comp = texture(InputTexture, st + i * dir);
      gComp = gscale(comp.rgb);      
      if (dir.y < 0.0) {
        if (gCurr > Threshold && gComp > gCurr) { // (gCurr > Threshold && 
          fragColor = comp;
          return;
        }
      } else {
        if (gComp > Threshold && gCurr >= gComp) {
          fragColor = comp;
          return;
        }
      }
    }
    fragColor = curr;
  } else {
      
    // we differentiate every 1/2 pixel on the horizontal axis, will be -1 or 1
    float vp = mod(floor(st.x * MaxUV.x), 2.0) * 2. - 1.;
    
    
    vec2 dir = vec2(1, 0);
    dir*= fParity * vp;
    dir/= MaxUV.xy;

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

      if (dir.x < 0.0) {
        if (gCurr > Threshold && gComp > gCurr) {
          fragColor = comp;
          return;
        }
      } else {
        if (gComp > Threshold && gCurr >= gComp) {
          fragColor = comp;
          return;
        }
      }
    }
    fragColor = curr;
  }
  
}
