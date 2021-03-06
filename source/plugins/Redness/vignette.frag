#version 410 core
uniform sampler2D InputTexture;
uniform float Strength;

in vec2 uv;
out vec4 fragColor;

void main()
{
  // relative strength, 0 - 0.7
  float horizon = (1 - Strength) * sqrt(0.5);
	vec4 color = texture( InputTexture, uv );
  vec2 center_distance = vec2(
    abs(uv.x - 0.5),
    abs(uv.y - 0.5)
  );
  // how far is this pixel from the center?
  // corner pixel will be 0.7
  float r = sqrt(pow(center_distance.x, 2.) 
    + pow(center_distance.y, 2.));
  float dist = r - horizon;
  if (dist > 0.) {
    fragColor = mix(color, vec4(0,0,0,1), dist*1.42);
  } else {
  	fragColor = color;
  }
}
