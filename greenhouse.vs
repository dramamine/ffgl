
/*{
  "CREDIT": "domegod",
  "DESCRIPTION": "Translate a triangular image to each individual triangle",
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
float offset=.323;

vec2 center=vec2(519,1024-515)/1024;
vec2 corner=vec2(0,0);
vec2 o1=vec2(605,1024-614)/1024;
vec2 o2=vec2(648,1024-491)/1024;

vec2 o3=vec2(712,1024-794)/1024;
vec2 o4=vec2(797,1024-608)/1024;
vec2 o5=vec2(846,1024-409)/1024;

vec2 o6=vec2(806,1024-925)/1024;
vec2 o7=vec2(913,1024-763)/1024;

vec2 o8=vec2(983,1024-553)/1024;
vec2 o9=vec2(1000,1024-359)/1024;

vec2 tri1[3]=vec2[](
vec2(605,1024-614)/1024,
vec2(648,1024-490)/1024,
vec2(519,1024-515)/1024
);
vec2 tri2[3]=vec2[](
vec2(711,1024-768)/1024,
vec2(774,1024-622)/1024,
vec2(616,1024-641)/1024
);
vec2 tri3[3]=vec2[](
vec2(616,1024-629)/1024,
vec2(770,1024-602)/1024,
vec2(669,1024-485)/1024
);
vec2 tri4[3]=vec2[](
vec2(790,1024-582)/1024,
vec2(838,1024-431)/1024,
vec2(683,1024-467)/1024
);
vec2 tri5[3]=vec2[](
vec2(808,1024-913)/1024,
vec2(872,1024-793)/1024,
vec2(735,1024-798)/1024
);
vec2 tri6[3]=vec2[](
vec2(735,1024-777)/1024,
vec2(896,1024-755)/1024,
vec2(798,1024-626)/1024
);
vec2 tri7[3]=vec2[](
vec2(912,1024-733)/1024,
vec2(965,1024-580)/1024,
vec2(805,1024-610)/1024
);
vec2 tri8[3]=vec2[](
vec2(812,1024-588)/1024,
vec2(965,1024-547)/1024,
vec2(853,1024-436)/1024
);
vec2 tri9[3]=vec2[](
vec2(976,1024-510)/1024,
vec2(996,1024-370)/1024,
vec2(864,1024-423)/1024
);

// vec2 tri2[3] = vec2[](o3, o4, o1);
// vec2 tri3[3] = vec2[](o1, o4, o2);
// vec2 tri4[3] = vec2[](o4, o5, o2);

// vec2 tri5[3] = vec2[](o6, o7, o3);
// vec2 tri6[3] = vec2[](o3, o7, o4);
// vec2 tri7[3] = vec2[](o7, o8, o4);
// vec2 tri8[3] = vec2[](o4, o8, o5);
// vec2 tri9[3] = vec2[](o8, o9, o5);
vec2 outside[3]=vec2[](corner,corner,corner);

// from stackoverflow
bool point_inside_trigon(vec2 s,vec2 a,vec2 b,vec2 c)
{
float as_x=s.x-a.x;
float as_y=s.y-a.y;
bool s_ab=(b.x-a.x)*as_y-(b.y-a.y)*as_x>0;
if((c.x-a.x)*as_y-(c.y-a.y)*as_x>0==s_ab)return false;
if((c.x-b.x)*(s.y-b.y)-(c.y-b.y)*(s.x-b.x)>0!=s_ab)return false;
return true;
}

bool point_inside_trigon(vec2 s,vec2[3]t){
return point_inside_trigon(s,t[0],t[1],t[2]);
}

vec2[3]find_trigon(float r,vec2 c){
if(r<length(o2-center)){
if(point_inside_trigon(c,tri1)){
return tri1;
}
else if(point_inside_trigon(c,tri3)){
return tri3;
}
}else if(r<length(o4-center)){
if(point_inside_trigon(c,tri2)){
return tri2;
}
else if(point_inside_trigon(c,tri3)){
return tri3;
}
else if(point_inside_trigon(c,tri4)){
return tri4;
}
}else if(r<length(o5-center)){
if(point_inside_trigon(c,tri2)){
return tri2;
}
else if(point_inside_trigon(c,tri4)){
return tri4;
}
else if(point_inside_trigon(c,tri6)){
return tri6;
}
else if(point_inside_trigon(c,tri7)){
return tri7;
}
else if(point_inside_trigon(c,tri8)){
return tri8;
}
}else{
if(point_inside_trigon(c,tri5)){
return tri5;
}
else if(point_inside_trigon(c,tri6)){
return tri6;
}
else if(point_inside_trigon(c,tri7)){
return tri7;
}
else if(point_inside_trigon(c,tri8)){
return tri8;
}
else if(point_inside_trigon(c,tri9)){
return tri9;
}
}
return outside;
}

float dist(vec2 pt1,vec2 pt2,vec2 pt3)
{
vec2 v1=pt2-pt1;
vec2 v2=pt1-pt3;
vec2 v3=vec2(v1.y,-v1.x);
return abs(dot(v2,normalize(v3)));
}

void main(){
// normalize relative to center
vec2 normCoord=isf_FragNormCoord-vec2(.5,.5);

// find polar coords compared to center
float r=length(normCoord);
float theta=mod(atan(normCoord.y,normCoord.x),2*PI);

// which of the 5 sections of the dome are we in?
float segment=mod(round((theta+offset)/(.4*PI)),5);

// rotate to treat as one section
theta-=segment*(.4*PI);
// calculate new coordinate - c is basically our original normCoord but
// all other sections have been rotated into a single section.
vec2 c=vec2(r*cos(theta),r*sin(theta))+vec2(.5,.5);

// which triangle are we in?
vec2 tri[3]=find_trigon(r,c);
if(tri[0].x==0){
gl_FragColor=vec4(0,0,0,1);
return;
}

float normalized_y=dist(tri[0],tri[1],c)/dist(tri[0],tri[1],tri[2]);

// find the vector that's p0 -> p1 rotated 90 degrees
vec2 p=vec2(
tri[0].x-(tri[1].y-tri[0].y),
tri[0].y+(tri[1].x-tri[0].x)
);
float normalized_x=dist(tri[0],p,c)/distance(tri[0],tri[1]);

gl_FragColor=IMG_NORM_PIXEL(VideoInput,vec2(normalized_x,normalized_y+.16));
}