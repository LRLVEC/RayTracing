#version 450 core
layout(local_size_x = 8, local_size_y = 8)in;

struct Water
{
	float z;
	float v;
	float a;
	float blank;
};
struct Color
{
	vec3 r;
	int texR;
	vec3 t;
	int texT;
	vec3 d;
	int texD;
	vec3 g;
	int texG;
	vec3 decayFactor;
	float n;
};
struct Triangle
{
	vec3 p1;
	vec3 p2;
	vec3 p3;
	vec2 uv1;
	vec2 uv2;
	vec2 uv3;
	bool changed;
	Color color;
};
layout(std140, binding = 4)uniform WaterParameters
{
	float k;
	float dt;
};

layout(std430, binding = 1)buffer TriangleOrigin
{
	Triangle trianglesOrigin[];
};
layout(std430, binding = 10)buffer Waters
{
	Water water[];
};
void main()
{
	uint index = gl_GlobalInvocationID.x + gl_NumWorkGroups.x * 8 * gl_GlobalInvocationID.y;
	float z = water[index].z -= dt * (water[index].v + water[index].a / 2);
	water[index].v += water[index].a;
	water[index].v *= 0.9996;
	uint triangleIndex = gl_GlobalInvocationID.x +
		gl_GlobalInvocationID.y * 2 * (gl_NumWorkGroups.x * 8 - 1);
	if (gl_GlobalInvocationID.x != 0)
	{
		if (gl_GlobalInvocationID.y != 0)
			trianglesOrigin[triangleIndex - gl_NumWorkGroups.x * 8].p1.z = z;
		if (gl_GlobalInvocationID.y != gl_NumWorkGroups.y * 8 - 1)
		{
			trianglesOrigin[triangleIndex - 1].p2.z = z;
			trianglesOrigin[triangleIndex + gl_NumWorkGroups.x * 8 - 2].p3.z = z;
		}
	}
	if (gl_GlobalInvocationID.x != gl_NumWorkGroups.x * 8 - 1)
	{
		if (gl_GlobalInvocationID.y != 0)
		{
			trianglesOrigin[triangleIndex - 2 * (gl_NumWorkGroups.x * 8 - 1)].p3.z = z;
			trianglesOrigin[triangleIndex - gl_NumWorkGroups.x * 8 + 1].p2.z = z;
		}
		if (gl_GlobalInvocationID.y != gl_NumWorkGroups.y * 8 - 1)
			trianglesOrigin[triangleIndex].p1.z = z;
	}
}