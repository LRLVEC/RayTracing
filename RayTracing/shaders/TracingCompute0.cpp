#version 450 core
layout(local_size_x = 32, local_size_y = 32)in;
#define RayTraceDepth 50

struct Ray
{
	vec4 p0;
	vec3 n;
	float t;
};
struct Color
{
	vec3 r;//反射率
	vec3 t;//透射率
	vec3 g;//自发光
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
//vec2 getCircleUV(vec3 pos, uint num)
//{
//	vec3 d = pos - circles[num];//
//}
bool triangleTest(vec2 uv)
{
	if (all(greaterThanEqual(uv, vec2(0, 0))) && (uv.x + uv.y <= 1.0f))
		return true;
	return false;
}


vec4 rayTrace(Ray ray)
{
	uint n = 0;
	int depth = RayTraceDepth;
	float t;
	vec3 ratioR = vec3(1);
	vec3 answer = vec3(0);
	vec3 tempColor;
	vec3 tempRatioR;
	vec3 tempN;
	while (bool(--depth))
	{
		t = -1;
		tempColor = vec3(0);// , 0.6, 0.8);
		for (n = 0; n < planeNum; ++n)
		{
			float tt = getPlaneT(ray, planes[n].plane);
			if (tt > 0 && (tt < t || t < 0))
			{
				t = tt;
				vec3 p1 = ray.p0.xyz + ray.n * t;
				tempColor = (uint(int(p1.x) + int(p1.y)) % 2u) * planes[n].color.g;
				tempRatioR = planes[n].color.r;//反射率
				tempN = planes[n].plane.xyz;//反射平面的法向
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
					//tempColor = (uint(int(uv.x * 10) + int(uv.y * 10)) % 2u) * triangles[n].color.g;
					tempColor = triangles[n].color.g;
					tempRatioR = triangles[n].color.r;
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
					tempColor = spheres[n].color.g;
					tempRatioR = spheres[n].color.r;
					tempN = normalize(ray.p0.xyz + t * ray.n - spheres[n].sphere.xyz);
				}
			}
		}
		/*for (n = 0; n < circleNum; ++n)
		{
			float tt = getPlaneT(ray, circles[n].plane);
			if (tt > 0 && (tt < t || t < 0))
			{
				vec3 p1 = ray.p0.xyz + ray.n * t;
				if ()
					t = tt;
				tempColor = (uint(int(p1.x) + int(p1.y)) % 2u) * planes[n].color.g;
				tempRatioR = planes[n].color.r;
				tempN = planes[n].plane.xyz;
			}
		}*/
		answer += ratioR * tempColor;
		if (t > 0)
		{
			ratioR *= tempRatioR;
			ray.p0 += vec4((t - 0.0001) * ray.n, 0);
			ray.n = reflect(ray.n, tempN);
		}
		else break;
	}
	return vec4(answer, 1);
}

void main()
{
	vec4 color = rayTrace(rayAlloctor());
	imageStore(image, ivec2(gl_GlobalInvocationID.xy), color);
}