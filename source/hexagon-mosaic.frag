#version 410 core

uniform float Width;
uniform float Blending;
uniform float XOffset;
uniform float YOffset;
uniform sampler2D InputTexture;
in vec2 uv;
uniform vec2 MaxUV;
out vec4 fragColor;

  // hexagon width
  float w = Width/10.;
  float r = w * sqrt(3)/2.0;
  int sampling = int(clamp((1 + floor(7*Blending)), 1, 7));

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

// for a given point, walk down and left to find an orientation point for the hexagon.
// orientation points are the lower left corners of the hexagons.
vec2 find_op(vec2 p) {
  vec2 center = MaxUV/2.0 
    - vec2(w/2.0, r) 
    + vec2(XOffset*(3*w)/MaxUV.x,YOffset*(2*r)/MaxUV.y);

	float lane_width = 3 * w / 2.0;
	int which_lane = int(floor((p.x - center.x) / lane_width));
	float offset = (which_lane % 2) * r;
	float stripe_height = 2*r;
	int which_stripe = int(floor((p.y - center.y - offset) / stripe_height));
	return vec2(which_lane * lane_width + center.x, which_stripe * stripe_height + offset + center.y);
}

// what color should this hexagon be? (by orientation point)
vec3 op_to_color(vec2 p) {
  vec2 offsets[7];
  offsets[0] = vec2(0.5*w,w); // center
  offsets[1] = vec2(0.25*w, 0.5*w); // bottom left...etc.
  offsets[2] = vec2(0.75*w, 1.5*w);
  offsets[3] = vec2(0.75*w, 0.5*w);
  offsets[4] = vec2(0.25*w, 1.5*w);
  offsets[5] = vec2(w, w);
  offsets[6] = vec2(0, w);

  vec3 acc = vec3(0,0,0);
  for (int i = 0; i < sampling; i++) {
    acc += texture(InputTexture, clamp(p + offsets[i], vec2(0,0), vec2(1,1))).rgb;
  }
  return acc /= sampling;
}

void main() {
	vec2 p = uv;
  if (w < 0.001) {
    fragColor = texture(InputTexture, uv);
    return;
  };

  // the lower left corner of the hexagon I'm in
  vec2 op = find_op(p);
  // @TODO could just check y coordinate
  bool in_square = (p.x >= op.x && p.x < op.x+w &&
    p.y >= op.y && p.y < op.y+2*r);

  bool inside_rightedge = false;
  if (!in_square) {
    inside_rightedge = point_inside_trigon(
      p,
      op + vec2(w,0.0),
      op + vec2(3*w/2.0,w*sqrt(3)/2.0),
      op + vec2(w,w*sqrt(3))
    );

    if (!inside_rightedge) {
      bool above = p.y > op.y + r;
      op += vec2(1.5*w, 2*r*int(above)-r);
    }
  }

  vec3 color = op_to_color(op);
  fragColor = vec4(color, 1.0);
}
