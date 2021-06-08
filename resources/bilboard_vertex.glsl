#version 410 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform int flip = 0;
out vec2 vertex_tex;
void main()
{
	vec4 pos = M * vec4(vertPos,1.0);
	gl_Position = P * V * pos;
	if(flip == 1) {
		vec2 flippedTex = vertTex;
		flippedTex.x = 1.0 - flippedTex.x;
		vertex_tex = flippedTex;
	} else {
		vertex_tex = vertTex;
	}
	
}
