#version 450 core
layout(local_size_x = 32, local_size_y = 32)in;


struct Ray
{
	vec4 p0;
	vec3 n;
	float t;
};



layout(std140, binding = 0)uniform Size
{
	uvec2 size;
};
layout(std140, row_major, binding = 1)uniform Trans
{
	mat3 trans;
	float z0;
	vec4 r0;
};
layout(std430, binding = 2)buffer FrameBuffer
{
	vec4 frame[];
};


Ray rayAlloctor()
{
	vec3 n0 = vec3(gl_GlobalInvocationID.xy - (size.xy >> 1), z0);
	return Ray(r0, normalize(trans * n0), -1);
}


void main()
{
	frame[size.x * gl_GlobalInvocationID.y + gl_GlobalInvocationID.x] =
		vec4(0, atan(gl_GlobalInvocationID.x / 32.0), atan(gl_GlobalInvocationID.y / 32.0), 0);
}
