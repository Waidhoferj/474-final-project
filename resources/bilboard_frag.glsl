#version 410 core
out vec4 color;
in vec2 vertex_tex;
uniform vec3 campos;
uniform sampler2D tex;

void main()
{
vec4 tcol = texture(tex, vertex_tex);
if(tcol.w < 0.1) discard;
color = tcol;
}
