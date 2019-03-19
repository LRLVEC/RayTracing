#version 450 core
layout(local_size_x = 32, local_size_y = 32)in;
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
	frame[size.x * gl_GlobalInvocationID.y + gl_GlobalInvocationID.x] =
		vec4(0, atan(gl_GlobalInvocationID.x / 640.0), atan(gl_GlobalInvocationID.y / 640.0), 0);
}