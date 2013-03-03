#version 130
uniform sampler2D diffuseMap_texture;
uniform sampler2DShadow shadowMap_texture;

smooth in vec3 normal_smooth;
smooth in vec3 v;
smooth in vec3 l;
smooth in vec2 textCoord0;

smooth in vec4 shadow_coord;

out vec4 out_color;

void main() {
    vec3 h = normalize(v+l);
	vec3 n = normalize(normal_smooth);
	
	float diff = max(0.1f, dot(n, l));

	float shadeFactor = textureProj(shadowMap_texture, shadow_coord);

	shadeFactor = shadeFactor * 0.25 + 0.75;

	vec4 diffuse = texture2D(diffuseMap_texture, textCoord0)*vec4(diff);
	float spec = pow(max(0.0f, dot(n, h)), 500.0f);
	
	out_color = vec4((diffuse + spec) * shadeFactor);
    
	//out_color = diff*vec4(1.0f, 0.6f, 0.1f, 1.0f) + vec4(spec)+vec4(textCoord0, 0,0);
}