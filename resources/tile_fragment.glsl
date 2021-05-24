#version 410 core
out vec4 color;
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;
uniform vec3 campos;
uniform vec2 tile_offset;
uniform vec2 prev_offset;
uniform float t;

uniform sampler2D tex;
uniform sampler2D tex2;

void main()
{
    
vec2 panel_coord = vertex_tex;
panel_coord.x /= 4;
panel_coord.y /= 4;
vec4 tcol = texture(tex, panel_coord + tile_offset);
vec4 prev_tcol = texture(tex, panel_coord + prev_offset);
vec4 blended_color = tcol * t + prev_tcol * (1 - t);
if(blended_color.x < 0.1) discard;
color = blended_color;
}
