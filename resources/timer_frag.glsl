#version 410 core
out vec4 color;
in vec2 vertex_tex;
uniform int timer;
uniform vec3 campos;

uniform sampler2D tex;

void main()
{
int t = timer;
vec2 tex_coord = vertex_tex;
if(t < 0) t = 0;
int tens = t / 10;
int ones = t % 10;
vec2 offsets = vec2(tens, ones) * vec2(0.1);
float width = 0.1;
// Tens
if(tex_coord.x > 0.5){
    float w = 1.0 - (tex_coord.x - 0.5) * 2.0;
    tex_coord.x = w * width + offsets.x;
} else {
    //Ones
    float w = 1.0 - tex_coord.x * 2.0;
    tex_coord.x = w * width + offsets.y;
}

vec4 tcol = texture(tex, tex_coord);
if(tcol.w < 0.1) discard;
color = tcol;
}
