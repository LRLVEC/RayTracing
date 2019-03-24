#version 450 core
in vec2 pos;
out vec4 color;
layout(std140, binding = 0)uniform Size
{
	uvec2 size;
};
uniform sampler2D smp;
void main()
{
	color = texture(smp, pos);
}