#version 130
uniform sampler2D diffuseMap_texture;

smooth in vec3 normal_smooth;
smooth in vec3 v;
smooth in vec3 l;
smooth in vec2 textCoord0;

out vec4 out_color;

void main() {
    vec3 h = normalize(v+l);
	vec3 n = normalize(normal_smooth);
	
	float diff = max(0.1f, dot(n, l));

	vec4 diffuse = texture2D(diffuseMap_texture, textCoord0)*vec4(diff);

    float spec = pow(max(0.0f, dot(n, h)), 500.0f);
	out_color = diffuse + spec;
    
	//out_color = diff*vec4(1.0f, 0.6f, 0.1f, 1.0f) + vec4(spec)+vec4(textCoord0, 0,0);
}