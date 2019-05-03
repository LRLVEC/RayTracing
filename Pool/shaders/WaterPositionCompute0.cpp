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
	water[index].z -= dt * (water[index].v + water[index].a / 2);
	water[index].v += water[index].a;
	water[index].v *= 0.999985;
}