#version 330 core
precision highp float;


uniform sampler2D alphaTexture;

in vec4 partCol;

out vec4 outColor;


void main()
{
	float alpha = texture(alphaTexture, gl_PointCoord).r * partCol.w;
	if(alpha < .1) discard;

	outColor = vec4(partCol.xyz, alpha);
}