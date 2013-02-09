#version 130

flat in vec3 color;

smooth in vec3 normal_smooth;
flat in vec3 normal_flat;

smooth in vec3 v;
smooth in vec3 l;
uniform int render_mode;

out vec4 out_color;



void main() {
    vec3 h = normalize(v+l);
	vec3 n;
	if(render_mode == 0)
	{
		n = normalize(normal_smooth);
	}
	else if(render_mode == 1)
	{
		n = normalize(normal_flat);
	}
	else
	{
		n = normalize(normal_smooth);
	}
    
	float diff = max(0.1f, dot(n, l));
    float spec = pow(max(0.0f, dot(n, h)), 500.0f);
    out_color = diff*vec4(color, 1.0f) + vec4(spec);
}