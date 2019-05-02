#version 450 core
layout(local_size_x = 8, local_size_y = 8)in;

struct Water
{
	float z;
	float v;
	float a;
	float blank;
};

layout(std140, binding = 4)uniform WaterParameters
{
	float k;
	float dt;
};
layout(std430, binding = 10)buffer Waters
{
	Water water[];
};

void main()
{
	//optimize: use shared memory
	float dv = 0;
	uint index = gl_GlobalInvocationID.x + gl_NumWorkGroups.x * 8 * gl_GlobalInvocationID.y;
	if (gl_GlobalInvocationID.x != 0 && gl_GlobalInvocationID.x != gl_NumWorkGroups.x * 8 - 1)
		dv += 2 * water[index].z - water[index - 1].z - water[index + 1].z;
	else if (gl_GlobalInvocationID.x != 0)
		dv += water[index].z - water[index - 1].z;
	else
		dv += water[index].z - water[index + 1].z;
	uint offset = gl_NumWorkGroups.y * 8;
	if (gl_GlobalInvocationID.y != 0 && gl_GlobalInvocationID.y != offset - 1)
		dv += 2 * water[index].z - water[index - offset].z - water[index + offset].z;
	else if (gl_GlobalInvocationID.y != 0)
		dv += water[index].z - water[index - offset].z;
	else
		dv += water[index].z - water[index + offset].z;
	water[index].a = k * dt * dv;
}