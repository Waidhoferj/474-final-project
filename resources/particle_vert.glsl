#version 410 core

layout(location = 0) in vec3 pointPos;
layout(location = 1) in float pointScale;
uniform mat4 P;
uniform mat4 M;
uniform mat4 V;
uniform vec3 pointColor;

out float partScale;


void main()
{
	// Billboarding: set the upper 3x3 to be the identity matrix
	mat4 M0 = M;

	M0[0] = vec4(1.0, 0.0, 0.0, 0.0);
	M0[1] = vec4(0.0, 1.0, 0.0, 0.0);
	M0[2] = vec4(0.0, 0.0, 1.0, 0.0);

	gl_Position = P *V* M0 * vec4(pointPos.xyz, 1.0);
	gl_PointSize= pointScale;

	partScale = pointScale;
}