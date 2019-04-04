#version 450 core
layout(local_size_x = 32, local_size_y = 32)in;
#define RayTraceDepth 6
#define Pi 3.14159265359

struct Ray
{
	vec4 p0;
	vec3 n;
	float t;
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
	vec2 uv1;
	vec2 uv2;
	vec2 uv3;
	Color color;
};
struct Sphere
{
	vec4 sphere;
	vec3 e1;
	vec3 e2;
	Color color;
};
struct Circle
{
	vec4 plane;
	vec4 sphere;
	vec3 e1;
	int tex;
	Color color;
};
struct Cylinder
{
	vec3 c;
	float r2;
	vec3 n;
	float l;
	vec3 e1;
	Color color;
};
struct Cone
{
	vec3 c;
	float c2;
	vec3 n;
	float l2;
	vec3 e1;
	Color color;
};
struct PointLight
{
	vec3 color;
	vec3 p;
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
layout(binding = 0)uniform sampler2D smp;
layout(binding = 1)uniform sampler2DArray texSmp;
layout(binding = 2)uniform samplerCube cubeSmp;
layout(std140, binding = 3)uniform GeometryNum
{
	uint planeNum;
	uint triangleNum;
	uint sphereNum;
	uint circleNum;
	uint cylinderNum;
	uint coneNum;
	uint pointLightNum;
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
layout(std430, binding = 5)buffer Cylinders
{
	Cylinder cylinders[];
};
layout(std430, binding = 6)buffer Cones
{
	Cone cones[];
};
layout(std430, binding = 7)buffer PointLights
{
	PointLight pointLights[];
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
bool judgeHit(Ray ray)
{
	uint n;
	for (n = 0; n < planeNum; ++n)
	{
		float tt = getPlaneT(ray, planes[n].plane);
		if (tt > 0 && tt < ray.t)
			return true;
	}
	for (n = 0; n < triangleNum; ++n)
	{
		float tt = getPlaneT(ray, triangles[n].plane);
		if (tt > 0 && tt < ray.t)
			if (triangleTest(getTriangleUV(ray.p0.xyz + ray.n * tt, n)))
				return true;
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
			if (tt > 0 && tt < ray.t)
				return true;
		}
	}
	for (n = 0; n < circleNum; ++n)
	{
		float tt = getPlaneT(ray, circles[n].plane);
		if (tt > 0 && tt < ray.t)
		{
			vec3 d = ray.p0.xyz + ray.n * tt - circles[n].sphere.xyz;
			if (dot(d, d) <= circles[n].sphere.w)
			{
				return true;
			}
		}
	}
	for (n = 0; n < cylinderNum; ++n)
	{
		float nn0 = dot(ray.n, cylinders[n].n);
		float cnn02 = 1 - nn0 * nn0;
		if (cnn02 == 0)continue;
		vec3 d = ray.p0.xyz - cylinders[n].c;
		float nd = dot(cylinders[n].n, d);
		vec3 j = d - nd * cylinders[n].n;
		float n0j = dot(ray.n, j);
		float k = n0j * n0j + cnn02 * (cylinders[n].r2 - dot(j, j));
		if (k <= 0)continue;
		k = sqrt(k);
		float tt = -1;
		float u;
		if (k - n0j > 0)
		{
			tt = (k - n0j) / cnn02;
			u = nd + nn0 * tt;
			if (u > cylinders[n].l || u < 0)
				tt = -1;
		}
		if (k + n0j < 0)
		{
			tt = -(k + n0j) / cnn02;
			u = nd + nn0 * tt;
			if (u > cylinders[n].l || u < 0)
				tt = -1;
		}
		if (tt > 0 && tt < ray.t)
			return true;
	}
	for (n = 0; n < coneNum; ++n)
	{
		vec3 d = ray.p0.xyz - cones[n].c;
		float nn0 = dot(ray.n, cones[n].n);
		float dn0 = dot(d, cones[n].n);
		float dn = dot(d, ray.n);
		float d2 = dot(d, d);
		float a = cones[n].c2 - nn0 * nn0;
		float b = nn0 * dn0 - dn * cones[n].c2;
		float c = d2 * cones[n].c2 - dn0 * dn0;
		float s = b * b - a * c;
		if (s <= 0)continue;
		s = sqrt(s);
		float r2;
		float tt = (b + s) / a;
		if (tt > 0 && tt < ray.t)
		{
			r2 = d2 + tt * tt + 2 * dn * tt;
			float k = dn0 + nn0 * tt;
			if (r2 < cones[n].l2 && k >= 0)
				return true;
		}
		tt = (b - s) / a;
		if (tt > 0 && tt < ray.t)
		{
			r2 = d2 + tt * tt + 2 * dn * tt;
			float k = dn0 + nn0 * tt;
			if (r2 < cones[n].l2 && k >= 0)
				return true;
		}
	}
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
	tempColor.texG = -1;
	vec3 tempN;
	vec2 tempUV;
	vec3 tempDecay = vec3(1);
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
					tempColor = triangles[n].color;
					tempN = triangles[n].plane.xyz;
					tempUV = (1 - uv.x - uv.y) * triangles[n].uv1 + uv.x * triangles[n].uv2 + uv.y * triangles[n].uv3;
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
					tempN = (ray.p0.xyz + t * ray.n - spheres[n].sphere.xyz) / sqrt(spheres[n].sphere.w);
					float ne1 = dot(tempN, spheres[n].e1);
					vec3 nxy = normalize(tempN - ne1 * spheres[n].e1);
					float u =
						dot(nxy, cross(spheres[n].e1, spheres[n].e2)) >= 0 ?
						acos(dot(spheres[n].e2, nxy)) / (2 * Pi) :
						1 - acos(dot(spheres[n].e2, nxy)) / (2 * Pi);
					tempUV = vec2(u, 1 - acos(ne1) / Pi);
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
					tempColor = circles[n].color;
					tempN = circles[n].plane.xyz;
					vec3 e2 = cross(tempN, circles[n].e1);
					tempUV = (vec2(1) + vec2(dot(circles[n].e1, d), dot(e2, d)) / sqrt(circles[n].sphere.w)) / 2;
				}
			}
		}
		for (n = 0; n < cylinderNum; ++n)
		{
			float nn0 = dot(ray.n, cylinders[n].n);
			float cnn02 = 1 - nn0 * nn0;
			if (cnn02 == 0)continue;
			vec3 d = ray.p0.xyz - cylinders[n].c;
			float nd = dot(cylinders[n].n, d);
			vec3 j = d - nd * cylinders[n].n;
			float n0j = dot(ray.n, j);
			float k = n0j * n0j + cnn02 * (cylinders[n].r2 - dot(j, j));
			if (k <= 0)continue;
			k = sqrt(k);
			float tt = -1;
			float v;
			if (k - n0j > 0)
			{
				tt = (k - n0j) / cnn02;
				v = nd + nn0 * tt;
				if (v > cylinders[n].l || v < 0)
					tt = -1;
			}
			if (k + n0j < 0)
			{
				float ttt = -(k + n0j) / cnn02;
				float ut = nd + nn0 * ttt;
				if (ut <= cylinders[n].l && ut >= 0)
				{
					tt = ttt;
					v = ut;
				}
			}
			if (tt > 0 && (tt < t || t < 0))
			{
				t = tt;
				tempColor = cylinders[n].color;
				tempN = normalize(d + ray.n * t - cylinders[n].n * v);
				vec3 e2 = cross(cylinders[n].n, cylinders[n].e1);
				float u =
					dot(tempN, e2) >= 0 ?
					acos(dot(cylinders[n].e1, tempN)) / (2 * Pi) :
					1 - acos(dot(cylinders[n].e1, tempN)) / (2 * Pi);
				tempUV = vec2(u, v / cylinders[n].l);
			}
		}
		for (n = 0; n < coneNum; ++n)
		{
			vec3 d = ray.p0.xyz - cones[n].c;
			float nn0 = dot(ray.n, cones[n].n);
			float dn0 = dot(d, cones[n].n);
			float dn = dot(d, ray.n);
			float d2 = dot(d, d);
			float a = cones[n].c2 - nn0 * nn0;
			float b = nn0 * dn0 - dn * cones[n].c2;
			float c = d2 * cones[n].c2 - dn0 * dn0;
			float s = b * b - a * c;
			if (s > 0)
			{
				s = sqrt(s);
				float tt = -1;
				float r2;
				tt = (b + s) / a;
				if (tt > 0)
				{
					r2 = d2 + tt * tt + 2 * dn * tt;
					float k = dn0 + nn0 * tt;
					if (r2 > cones[n].l2 || k < 0)tt = -1;
				}
				float ttt = (b - s) / a;
				if (ttt > 0 && (ttt < tt || (tt < 0)))
				{
					float r2t = d2 + ttt * ttt + 2 * dn * ttt;
					float k = dn0 + nn0 * ttt;
					if (r2t <= cones[n].l2 && k > 0)
					{
						tt = ttt;
						r2 = r2t;
					}
				}
				if (tt > 0 && (tt < t || t < 0))
				{
					t = tt;
					tempColor = cones[n].color;
					tempN = normalize(d + ray.n * t - cones[n].n * sqrt(r2 / cones[n].c2));
					float ne1 = dot(tempN, cones[n].n);
					vec3 nxy = normalize(tempN - ne1 * cones[n].n);
					float u =
						dot(nxy, cross(cones[n].n, cones[n].e1)) >= 0 ?
						acos(dot(cones[n].e1, nxy)) / (2 * Pi) :
						1 - acos(dot(cones[n].e1, nxy)) / (2 * Pi);
					tempUV = vec2(u, 1 - sqrt(r2 / cones[n].l2));
				}
			}
		}
		if (tempColor.texG >= 0)
			tempColor.g *= texture(texSmp, vec3(tempUV, tempColor.texG)).xyz;
		if (t < 0)
		{
			tempColor.g = texture(cubeSmp, ray.n).xyz;
		}
		answer += tempColor.g * ratioNow;
		if (t > 0 && depth < RayTraceDepth)
		{
			if (tempColor.texT >= 0)
				tempColor.t *= texture(texSmp, vec3(tempUV, tempColor.texT)).xyz;
			tempColor.t *= ratioNow;
			if (any(greaterThanEqual(tempColor.t, vec3(0.05))))
			{
				float s = dot(ray.n, tempN);
				tempColor.n = s > 0 ? 1 / tempColor.n : tempColor.n;
				if (tempDecay.x != 1)
					tempColor.t *= exp(tempDecay * t);
				tempDecay = tempColor.decayFactor;
				float k = tempColor.n * tempColor.n + s * s - 1;
				if (k > 0)
				{
					++sp;
					s -= sign(s) * sqrt(k);
					stack[sp].p0 = ray.p0 + vec4((t + 0.003) * ray.n, 0);
					stack[sp].n = (ray.n - tempN * s) / tempColor.n;
					stack[sp].ratio = tempColor.t;
					stack[sp].depth = depth + 1;
				}
				else tempColor.r = vec3(1);
			}

			ray.p0 += vec4((t - 0.003) * ray.n, 0);
			for (n = 0; n < pointLightNum; ++n)
			{
				vec3 dn = pointLights[n].p - ray.p0.xyz;
				float tt = dot(dn, dn);
				float ttt = sqrt(tt);
				tt *= 0.05;
				dn = normalize(dn);
				if (!judgeHit(Ray(ray.p0, dn, ttt)))
				{
					if (tempColor.texD >= 0)
						tempColor.d *= texture(texSmp, vec3(tempUV, tempColor.texD)).xyz;
					answer += max(dot(tempN, dn) / tt, 0) * pointLights[n].color * tempColor.d * ratioNow;
				}
			}

			if (tempColor.texR >= 0)
				tempColor.r *= texture(texSmp, vec3(tempUV, tempColor.texR)).xyz;
			ratioNow *= tempColor.r;
			if (any(greaterThanEqual(ratioNow, vec3(0.05))))
			{
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