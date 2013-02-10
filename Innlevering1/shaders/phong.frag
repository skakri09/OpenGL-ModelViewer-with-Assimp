#version 130

smooth in vec3 normal_smooth;
smooth in vec3 v;
smooth in vec3 l;

out vec4 out_color;

void main() {
    vec3 h = normalize(v+l);
	vec3 n = normalize(normal_smooth);

	float diff = max(0.1f, dot(n, l));

    float spec = pow(max(0.0f, dot(n, h)), 500.0f);

    out_color = diff*vec4(1.0f, 0.6f, 0.1f, 1.0f) + vec4(spec);
}