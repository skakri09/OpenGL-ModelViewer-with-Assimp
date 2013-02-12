#version 130
/*
*	A super simple shader I use when doing hidden line wireframe rendering.
*	This program takes the color that openGL uses as clearing color as a uniform
*	and draws the model, without any lighing or other effects, in that color with a 
*	slightly darker tone. The model can then be drawn again in wireframe mode on top 
*	of the model now disguised in the background color.
*/

uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;
uniform vec4 rendering_color;

in  vec3 position;

void main() {
	vec4 pos = modelview_matrix * vec4(position, 1.0);
	
	gl_Position = projection_matrix * pos;

}