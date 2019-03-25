#version 450 core
layout(local_size_x = 32, local_size_y = 32)in;
#define RayTraceDepth 6

struct Ray
{
	vec4 p0;
	vec3 n;
	float t;
};
struct Color
{
	vec3 r;
	vec3 t;
	vec3 g;
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
	vec3 k1;
	vec3 k2;
	Color color;
};
struct Sphere
{
	vec4 sphere;
	vec3 a;
	vec3 b;
	Color color;
};
struct Circle
{
	vec4 plane;
	vec4 sphere;
	vec3 e1;
	vec3 e2;
	Color color;
};
struct Cylinder
{
	vec4 c;//(c0, R^2)
	vec3 n;
	vec3 e1;
	vec3 e2;
	Color color;
};

struct Stack
{
	vec4 p0;
	vec3 n;
	int depth;
	vec3 ratio;
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

layout(std430, binding = 0)buffer Planes
{
	Plane planes[];
};
layout(std430, binding = 2)buffer Triangles
{
	TriangleGPU triangles[];
};
layout(std430, binding = 3)buffer Spheres
{
	Sphere spheres[];
};
layout(std430, binding = 4)buffer Circles
{
	Circle circles[];
};


Ray rayAlloctor()
{
	vec3 n0 = vec3(ivec2(gl_GlobalInvocationID.xy) - ivec2(size.xy / 2), z0);
	return Ray(vec4(r0, 1), normalize(trans * n0), -1);
}
float getPlaneT(Ray ray, vec4 plane)
{
	return -dot(plane, ray.p0) / dot(plane.xyz, ray.n);
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
	Stack stack[RayTraceDepth];
	int sp = -1;
	int depth = 0;
	float t;
	uint n = 0;
	vec3 ratioNow = vec3(1);
	vec3 answer = vec3(0);
	Color tempColor;
	vec3 tempN;
	while (true)
	{
		t = -1;
		tempColor.g = vec3(0);// , 0.6, 0.8);
		for (n = 0; n < planeNum; ++n)
		{
			float tt = getPlaneT(ray, planes[n].plane);
			if (tt > 0 && (tt < t || t < 0))
			{
				t = tt;
				vec3 p1 = ray.p0.xyz + ray.n * t;
				tempColor = planes[n].color;
				tempColor.g = ((int(4.2 * p1.x) + int(4.2 * p1.y)) % 2u) * tempColor.g;
				tempN = planes[n].plane.xyz;
			}
		}
		for (n = 0; n < triangleNum; ++n)
		{
			float tt = getPlaneT(ray, triangles[n].plane);
			if (tt > 0 && (tt < t || t < 0))
			{
				vec2 uv = getTriangleUV(ray.p0.xyz + ray.n * tt, n);
				if (triangleTest(uv))
				{
					t = tt;
					tempColor = planes[n].color;
					tempColor.g = (uint(int(uv.x * 10) + int(uv.y * 10)) % 2u) * triangles[n].color.g;
					tempN = triangles[n].plane.xyz;
				}
			}
		}
		for (n = 0; n < sphereNum; ++n)
		{
			vec3 d = spheres[n].sphere.xyz - ray.p0.xyz;
			float s = spheres[n].sphere.w - dot(cross(d, ray.n), cross(d, ray.n));
			if (s >= 0)
			{
				s = sqrt(s);
				float k = dot(d, ray.n);
				float tt = -1;
				if (k + s > 0)tt = k + s;
				if (k > s)tt = k - s;
				if (tt > 0 && (tt < t || t < 0))
				{
					t = tt;
					tempColor = spheres[n].color;
					tempN = normalize(ray.p0.xyz + t * ray.n - spheres[n].sphere.xyz);
				}
			}
		}
		for (n = 0; n < circleNum; ++n)
		{
			float tt = getPlaneT(ray, circles[n].plane);
			if (tt > 0 && (tt < t || t < 0))
			{
				vec3 d = ray.p0.xyz + ray.n * tt - circles[n].sphere.xyz;
				if (dot(d, d) <= circles[n].sphere.w)
				{
					t = tt;
					vec2 uv = vec2(dot(circles[n].e1, d), dot(circles[n].e2, d));
					tempColor = circles[n].color;
					tempColor.g *= uint(int(3 * uv.x) + int(3 * uv.y)) % 2u;
					tempN = circles[n].plane.xyz;
				}
			}
		}
		answer += tempColor.g * ratioNow;
		if (t > 0 && depth < RayTraceDepth)
		{
			tempColor.t *= ratioNow;
			if (any(greaterThanEqual(tempColor.t, vec3(0.05))))
			{
				float s = dot(ray.n, tempN);
				tempColor.n = pow(tempColor.n, -sign(s));
				float k = tempColor.n * tempColor.n + s * s - 1;
				if (k > 0)
				{
					++sp;
					s -= sign(s) * sqrt(k);
					stack[sp].p0 = ray.p0 + vec4((t + 0.0001) * ray.n, 0);
					stack[sp].n = (ray.n - tempN * s) / tempColor.n;
					stack[sp].ratio = tempColor.t;
					stack[sp].depth = depth + 1;
				}
				else tempColor.r = vec3(1);
			}
			ratioNow *= tempColor.r;
			if (any(greaterThanEqual(ratioNow, vec3(0.05))))
			{
				ray.p0 += vec4((t - 0.0001) * ray.n, 0);
				ray.n -= (2 * dot(ray.n, tempN)) * tempN;
				++depth;
				continue;
			}
		}
		if (sp < 0)
			return vec4(answer, 1);
		else
		{
			depth = stack[sp].depth;
			ray.p0 = stack[sp].p0;
			ray.n = stack[sp].n;
			ratioNow = stack[sp].ratio;
			--sp;
		}
	}
	return vec4(answer, 1);
}

void main()
{
	vec4 color = rayTrace(rayAlloctor());
	imageStore(image, ivec2(gl_GlobalInvocationID.xy), color);
}