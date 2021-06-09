#version 410 core
out vec4 color;
in vec2 vertex_tex;
uniform float opacity;
uniform sampler2D tex;

void main()
{
vec4 tcol = texture(tex, vertex_tex) *  vec4(opacity);
if(tcol.r < 0.1) discard;
color = tcol;
}
