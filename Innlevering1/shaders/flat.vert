#version 130
uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;
uniform mat3 normal_matrix;

in  vec3 position;
in  vec3 normal;

flat out vec3 color;
flat out vec3 normal_flat;


void main() {
	vec4 pos = modelview_matrix * vec4(position, 1.0);
	v = normalize(-pos.xyz);
	l = normalize(vec3(200.0f, 200.0f, 200.0f) - pos.xyz);
	gl_Position = projection_matrix * pos;
	color = vec3(0.5f, 0.5f, 1.0f);

	normal_flat = normal_matrix*normal;
	color = vec3(0.5f, 0.0f, 0.0f);

}