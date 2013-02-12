#version 130

/*
*	The worlds most simple shader! Used when we render our scene as a wireframe.
*	Thus we only need to position our vertices correctly.
*	The fragment shader outputs a constant color for the wireframe
*/


out vec4 out_color;

void main() {

    out_color = vec4(1.0f, 0.6f, 0.1f, 1.0f);
}