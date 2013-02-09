#version 130
uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;
uniform mat3 normal_matrix;
uniform sampler2D 
// 0 = phong, 1 = flat, 2 = wireframe, 3 = hidden_line
uniform int render_mode;

in  vec3 position;
in  vec3 normal;

flat out vec3 color;
smooth out vec3 v;
smooth out vec3 l;

smooth out vec3 normal_smooth;
flat out vec3 normal_flat;

out int render_mode_out;

void main() {
	vec4 pos = modelview_matrix * vec4(position, 1.0);
	v = normalize(-pos.xyz);
	l = normalize(vec3(200.0f, 200.0f, 200.0f) - pos.xyz);
	gl_Position = projection_matrix * pos;
	color = vec3(0.5f, 0.5f, 1.0f);

	if(render_mode == 0)
	{
		normal_smooth = normal_matrix*normal;
		color = vec3(0.5f, 0.0f, 0.0f);
	}
	else if(render_mode == 1)
	{
		normal_flat = normal_matrix*normal;
		color = vec3(0.5f, 0.0f, 0.0f);
	}
}