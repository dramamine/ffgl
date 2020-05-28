#version 410 core
uniform sampler2D InputTexture;
uniform vec3 Brightness;
uniform float Redness;

in vec2 uv;

layout (location = 0) out vec4 fragColor;

void main()
{
	vec4 color = texture( InputTexture, uv );
    color.r = Redness
	fragColor = color;
}
