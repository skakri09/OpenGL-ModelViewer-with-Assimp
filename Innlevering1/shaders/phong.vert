#version 130
uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;
uniform mat3 normal_matrix;

in  vec3 position;
in  vec3 normal;

smooth out vec3 v;
smooth out vec3 l;
smooth out vec3 normal_smooth;

void main() {
	vec4 pos = modelview_matrix * vec4(position, 1.0);
	v = normalize(-pos.xyz);
	l = normalize(vec3(200.0f, 200.0f, 200.0f) - pos.xyz);
	
	gl_Position = projection_matrix * pos;

	normal_smooth = normal_matrix*normal;
}