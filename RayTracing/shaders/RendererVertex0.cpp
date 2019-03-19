#version 450 core
layout(location = 0)in vec2 position;
layout(std140, binding = 0)uniform Size
{
	uvec2 size;
};
out vec2 pos;
void main()
{
	gl_Position = vec4(position, 0, 1);
	pos = ((position + vec2(1, 1)) * vec2(size)) / 2;
}
