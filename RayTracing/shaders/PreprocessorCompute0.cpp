#version 450 core
layout(local_size_x = 1)in;

struct Color
{
	vec3 r;
	vec3 g;
	vec3 b;
	float n;
};
struct Triangle
{
	vec3 p1;
	vec3 p2;
	vec3 p3;
	Color color;
};
struct TriangleGPU
{
	vec4 plane;
	vec3 p1;
	vec3 e1;
	vec3 e2;
	vec3 k1;
	vec3 k2;
	Color color;
};

layout(std430, binding = 0)buffer TriangleOrigin
{
	Triangle trianglesOrigin[];
};
layout(std430, binding = 1)buffer Triangles
{
	TriangleGPU triangles[];
};

void main()
{
	triangles[gl_GlobalInvocationID.x].color =
		trianglesOrigin[gl_GlobalInvocationID.x].color;
	triangles[gl_GlobalInvocationID.x].p1 =
		trianglesOrigin[gl_GlobalInvocationID.x].p1;
	triangles[gl_GlobalInvocationID.x].e1 =
		trianglesOrigin[gl_GlobalInvocationID.x].p2 -
		trianglesOrigin[gl_GlobalInvocationID.x].p1;
	triangles[gl_GlobalInvocationID.x].e2 =
		trianglesOrigin[gl_GlobalInvocationID.x].p3 -
		trianglesOrigin[gl_GlobalInvocationID.x].p1;
	vec3 n = cross
	(
		triangles[gl_GlobalInvocationID.x].e1,
		triangles[gl_GlobalInvocationID.x].e2
	);
	float s = dot(n, n);
	n = normalize(n);
	triangles[gl_GlobalInvocationID.x].plane =
		vec4(n, -dot(n, triangles[gl_GlobalInvocationID.x].p1));
	float d = dot
	(
		triangles[gl_GlobalInvocationID.x].e1,
		triangles[gl_GlobalInvocationID.x].e2
	);
	triangles[gl_GlobalInvocationID.x].k1 =
		(
			dot
			(
				triangles[gl_GlobalInvocationID.x].e2,
				triangles[gl_GlobalInvocationID.x].e2
			) * triangles[gl_GlobalInvocationID.x].e1 -
			d * triangles[gl_GlobalInvocationID.x].e2
			) / s;
	triangles[gl_GlobalInvocationID.x].k2 =
		(
			dot
			(
				triangles[gl_GlobalInvocationID.x].e1,
				triangles[gl_GlobalInvocationID.x].e1
			) * triangles[gl_GlobalInvocationID.x].e2 -
			d * triangles[gl_GlobalInvocationID.x].e1
			) / s;

}