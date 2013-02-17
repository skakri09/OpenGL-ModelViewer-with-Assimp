#version 130

uniform sampler2D tex;
uniform vec4 color;

in vec2 uv_coords;
out vec4 res_Color;

void main() {
	res_Color = vec4(1, 1, 1, texture2D(tex, uv_coords).r) * color;
}