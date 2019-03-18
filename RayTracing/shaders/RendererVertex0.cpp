#version 450 core
layout(location = 0)in vec2 position;
layout(std140, binding = 0)uniform size
{
	uvec2 size;
};
out uvec2 pos;
void main()
{
	gl_Position = vec4(position, 0, 1);
	pos = (uvec2(position + vec2(1, 1)) * size) / 2u;
}
