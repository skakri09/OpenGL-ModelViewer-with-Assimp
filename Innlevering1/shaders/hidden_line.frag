#version 130



uniform vec4 rendering_color;
out vec4 out_color;

void main() {

	out_color = clamp(rendering_color-vec4(0.01f), 0.0f, 1.0f);
}