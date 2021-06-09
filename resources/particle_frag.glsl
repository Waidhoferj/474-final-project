#version 330 core
precision highp float;


uniform sampler2D alphaTexture;
uniform vec3 pointColor = vec3(1,1,1);
in float partScale;

out vec4 outColor;


void main()
{
	float alpha = texture(alphaTexture, gl_PointCoord).r * partScale;
	if(alpha < .1) discard;

	outColor = vec4(pointColor, alpha);
}