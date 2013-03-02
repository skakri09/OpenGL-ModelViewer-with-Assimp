#version 130

uniform sampler2D fbo_texture;

in vec2 ex_texcoord;
out vec4 res_Color;

float LinearizeDepth(float zoverw){
		float n = 0.2; // camera z near
		float f = 3.0; // camera z far
		return (2.0 * n) / (f + n - zoverw * (f - n));
	}

void main() {
    ///res_Color = (texture2D( fbo_texture, ex_texcoord.xy ) * 1000) - vec4(999);
	
	//float depth = texture2D(fbo_texture, ex_texcoord.xy).r;
	//depth = LinearizeDepth(depth);
	//res_Color = vec4(depth, depth, depth, 1.0f);
	
	res_Color = texture2D(fbo_texture, ex_texcoord.xy);

	//res_Color = res_Color / 2;
	//making the background color black with this crappy if check
	//if(res_Color.x < 0.01f && res_Color.y < 0.01f && res_Color.z > 0.49f && res_Color.z < 0.51f)
	//{
	//	res_Color = vec4(0);
	//}
}