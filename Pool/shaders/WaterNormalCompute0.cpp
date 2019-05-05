#version 450 core
layout(local_size_x = 8, local_size_y = 8)in;

struct Water
{
	float z;
	float v;
	float a;
	float blank;
	vec3 n;
	float blank1;
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
struct TriangleGPU
{
	vec4 plane;
	vec3 p1;
	vec3 k1;
	vec3 k2;
	vec2 uv1;
	vec2 uv2;
	vec2 uv3;
	vec2 blank;
	ivec4 nIndices;
	Color color;
};
layout(std140, binding = 4)uniform WaterParameters
{
	float k;
	float dt;
};

layout(std430, binding = 2)buffer Triangles
{
	TriangleGPU triangles[];
};

layout(std430, binding = 10)buffer Waters
{
	Water water[];
};
void main()
{
	vec3 n = vec3(0);
	uint triangleIndex = gl_GlobalInvocationID.x +
		gl_GlobalInvocationID.y * 2 * (gl_NumWorkGroups.x * 8 - 1);
	if (gl_GlobalInvocationID.x != 0)
	{
		if (gl_GlobalInvocationID.y != 0)
			n += triangles[triangleIndex - gl_NumWorkGroups.x * 8].plane.xyz;
		if (gl_GlobalInvocationID.y != gl_NumWorkGroups.y * 8 - 1)
		{
			n += triangles[triangleIndex - 1].plane.xyz;
			n += triangles[triangleIndex + gl_NumWorkGroups.x * 8 - 2].plane.xyz;
		}
	}
	if (gl_GlobalInvocationID.x != gl_NumWorkGroups.x * 8 - 1)
	{
		if (gl_GlobalInvocationID.y != 0)
		{
			n += triangles[triangleIndex - 2 * (gl_NumWorkGroups.x * 8 - 1)].plane.xyz;
			n += triangles[triangleIndex - gl_NumWorkGroups.x * 8 + 1].plane.xyz;
		}
		if (gl_GlobalInvocationID.y != gl_NumWorkGroups.y * 8 - 1)
			n += triangles[triangleIndex].plane.xyz;
	}
	uint index = gl_GlobalInvocationID.x + gl_NumWorkGroups.x * 8 * gl_GlobalInvocationID.y;
	water[index].n = normalize(n);
}