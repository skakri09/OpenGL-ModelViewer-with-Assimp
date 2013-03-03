#version 130
uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;
uniform mat4 model_matrix;
uniform mat3 normal_matrix;

uniform mat4 shadow_matrix;

in vec3 position;
in vec3 normal;
in vec2 textureCoord0;

smooth out vec3 v;
smooth out vec3 l;
smooth out vec3 normal_smooth;
smooth out vec2 textCoord0;
smooth out vec4 shadow_coord;

void main() {
	vec4 cam_pos = modelview_matrix * vec4(position, 1.0);

	v = normalize(-cam_pos .xyz);

	l = normalize(vec3(1.0f, 1.0f, 1.0f) - cam_pos .xyz);
	
	gl_Position = projection_matrix * cam_pos;

	normal_smooth = normal_matrix*normal;
	textCoord0 = textureCoord0;

	shadow_coord = shadow_matrix * model_matrix * vec4(position, 1.0);
}