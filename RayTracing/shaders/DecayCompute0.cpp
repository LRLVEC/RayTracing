#version 450 core
layout(local_size_x = 1)in;

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

layout(std140, row_major, binding = 1)uniform Trans
{
	mat3 trans;
	vec3 r0;
	float z0;
};
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
layout(std430, binding = 8)buffer DecayOrigin
{
	vec3 decayOrigin;
};


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
void main()
{
	Ray ray;
	ray.p0 = vec4(10000, 10000, 10000, 1);
	vec3 dr = r0 - ray.p0.xyz;
	float l = length(dr);
	ray.n = normalize(dr);
	decayOrigin = vec3(0);
	while (true)
	{
		float t = -1;
		uint n = 0;
		vec3 decayNow = vec3(0);
		for (n = 0; n < triangleNum; ++n)
		{
			float tt = getPlaneT(ray, triangles[n].plane);
			if (tt > 0 && (tt < t || t < 0) && l > tt)
			{
				vec2 uv = getTriangleUV(ray.p0.xyz + ray.n * tt, n);
				if (triangleTest(uv))
				{
					t = tt;
					decayNow = -triangles[n].color.decayFactor * sign(dot(ray.n, triangles[n].plane.xyz));
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
				if (tt > 0 && (tt < t || t < 0) && l > tt)
				{
					t = tt;
					decayNow = -spheres[n].color.decayFactor * sign(dot(ray.n, (ray.p0.xyz + t * ray.n - spheres[n].sphere.xyz) / sqrt(spheres[n].sphere.w)));
				}
			}
		}
		for (n = 0; n < circleNum; ++n)
		{
			float tt = getPlaneT(ray, circles[n].plane);
			if (tt > 0 && (tt < t || t < 0) && l > tt)
			{
				vec3 d = ray.p0.xyz + ray.n * tt - circles[n].sphere.xyz;
				if (dot(d, d) <= circles[n].sphere.w)
				{
					t = tt;
					decayNow = -circles[n].color.decayFactor * sign(dot(ray.n, circles[n].plane.xyz));
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
			if (tt > 0 && (tt < t || t < 0) && l > tt)
			{
				t = tt;
				decayNow = -cylinders[n].color.decayFactor * sign(dot(ray.n, normalize(d + ray.n * t - cylinders[n].n * v)));
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
				if (ttt > 0 && (ttt < tt || (tt < 0)) && l > ttt)
				{
					float r2t = d2 + ttt * ttt + 2 * dn * ttt;
					float k = dn0 + nn0 * ttt;
					if (r2t <= cones[n].l2 && k > 0)
					{
						tt = ttt;
					}
				}
				if (tt > 0 && (tt < t || t < 0) && l > tt)
				{
					t = tt;
					decayNow = -cones[n].color.decayFactor * sign(dot(ray.n, normalize(d + ray.n * t - cones[n].n * sqrt(r2 / cones[n].c2))));
				}
			}
		}
		decayOrigin += decayNow;
		if (t < 0)break;
		else
		{
			t += 0.001;
			ray.p0 += vec4(ray.n * t, 0);
			l -= t;
		}
	}
}