#version 450 core
in uvec2 pos;
out vec4 color;
layout(std140, binding = 0)uniform Size
{
	uvec2 size;
};
layout(std430, binding = 1)buffer FrameBuffer
{
	vec4 frame[];
};

void main()
{
	color = frame[pos.x + size.x * pos.y];
}