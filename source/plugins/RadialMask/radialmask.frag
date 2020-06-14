#version 410 core
// iMouse.x: Starting angle
// iMouse.y: Area - % of circle to show
#define PI 3.14159265358979323844
uniform sampler2D InputTexture;
uniform float Angle;
uniform float Range;

in vec2 uv;

out vec4 fragColor;

void main() {
  float angle = PI * (Angle - 180.) / 180.; // [ -pi, pi ]
  float range = PI * Range; // [ 0, pi ]
	vec2 p = uv;
  vec2 normCoord = 2 * (uv - 0.5); // -1 to 1

  float theta = atan(normCoord.y, normCoord.x);

  // range
  float shortest_diff = min(    
    abs(angle - (theta - 2*PI)), 
    min(
      abs(angle - theta),
      abs(angle - (theta + 2*PI))
    )
  );
  
  if (shortest_diff < range) {
    fragColor = texture(InputTexture, uv);
  } else {
    fragColor = vec4(0,0,0,0);
  }

  return;
}
