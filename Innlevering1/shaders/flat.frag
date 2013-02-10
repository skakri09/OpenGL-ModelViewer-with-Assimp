#version 130

flat in vec3 color;
flat in vec3 normal_flat;

out vec4 out_color;

void main() {
    vec3 h = normalize(v+l);
	vec3 n = normalize(normal_flat);
	float diff = max(0.1f, dot(n, l));
    float spec = pow(max(0.0f, dot(n, h)), 500.0f);

    out_color = diff*vec4(color, 1.0f) + vec4(spec);
}