#version 450 core
layout(local_size_x = 32, local_size_y = 32)in;


struct Ray
{
	vec4 p0;
	vec3 n;
	float t;
};
struct Color
{
	vec3 r;
	vec3 g;
	vec3 b;
	float n;
};
struct Plane
{
	vec4 plane;
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
struct Sphere
{
	vec4 sphere;
	Color color;
};
struct Circle
{
	vec4 plane;
	vec4 sphere;
	Color color;
};





layout(std140, binding = 0)uniform Size
{
	uvec2 size;
};
layout(std140, row_major, binding = 1)uniform Trans
{
	mat3 trans;
	vec3 r0;
	float z0;
};
layout(binding = 2, rgba32f)uniform image2D image;
layout(std140, binding = 3)uniform GeometryNum
{
	uint planeNum;
	uint triangleNum;
	uint sphereNum;
	uint circleNum;
};
layout(std140, binding = 4)uniform Planes
{
	Plane planes[];
};

layout(std430, binding = 1)buffer Triangles
{
	TriangleGPU triangles[];
};
layout(std430, binding = 2)buffer Spheres
{
	Sphere spheres[];
};
layout(std430, binding = 3)buffer Circles
{
	Circle circles[];
};


Ray rayAlloctor()
{
	vec3 n0 = vec3(ivec2(gl_GlobalInvocationID.xy) - ivec2(size.xy / 2), z0);
	return Ray(vec4(r0, 1), normalize(trans * n0), -1);
}
float getPlaneT(Ray ray, vec4 para)
{
	return -dot(para, ray.p0) / dot(para.xyz, ray.n);
}
vec2 getTriangleUV(vec3 pos, uint num)
{

	vec3 d = pos - triangles[num].p1;
	return vec2
	(
		dot(d, triangles[num].k1),
		dot(d, triangles[num].k2)
	);
}
bool triangleTest(vec2 uv)
{
	if (all(greaterThanEqual(uv, vec2(0, 0))) && (uv.x + uv.y <= 1.0f))
		return true;
	return false;
}

vec4 rayTrace(Ray ray)
{
	float t = getPlaneT(ray, vec4(0, 0, 1, -1));
	if (t >= 0)
	{
		ray.p0 += vec4(ray.n * t, 0);
		return vec4(uint((int(ray.p0.x) + int(ray.p0.y)) % 2u) * vec3(0.7, 0.7, 0.7), 0);
	}
	return vec4(0, 0.6, 0.8, 0);
}


void main()
{
	vec4 color = rayTrace(rayAlloctor());
	imageStore(image, ivec2(gl_GlobalInvocationID.xy), color);
}
