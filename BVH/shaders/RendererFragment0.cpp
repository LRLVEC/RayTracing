#version 450 core
#define RayTraceDepth 8
#define Pi 3.14159265359
#define offset 0.005
#define minColor 0.01
#define originSamples 8
#define bit(a, b)			((a & (1 << uint(b))) != 0)
#define setBit(a, b)		(a |= (1 << uint(b)))
#define clearBit(a, b)		(a &= ~(1 << uint(b)))

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
struct Bound
{
	vec3 min;
	int leftChild;
	vec3 max;
	int rightChild;
};
struct BVHNode
{
	Bound bound;
	uint father;
	uint axis;
	uint geometry;
	uint geometryNum;
	vec4 blank;
};
struct HitInfo
{
	vec3 n;
	float t;
	vec2 uv;
	bool hit;
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
	vec3 decayFactor;
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
layout(std430, binding = 8)buffer DecayOrigin
{
	vec3 decayOrigins[originSamples];
	vec3 decayOrigin;
};
layout(std430, binding = 9)buffer BVH
{
	BVHNode bvh[];
};


Ray rayAlloctor()
{
	return Ray(vec4(r0, 1), normalize(trans * vec3(2 * gl_FragCoord.xy - size, z0)), -1);
}
float getPlaneT(Ray ray, vec4 plane)
{
	return -dot(plane, ray.p0) / dot(plane.xyz, ray.n);
}
vec2 getTriangleUV(vec3 pos, uint num)
{

	vec3 d = pos - triangles[num].p1;
	return vec2(dot(d, triangles[num].k1), dot(d, triangles[num].k2));
}
bool triangleTest(vec2 uv)
{
	return  all(greaterThanEqual(uv, vec2(0, 0))) && (uv.x + uv.y <= 1);
}
bool judgeHitBox(Ray ray, Bound bound)
{
	if (all(lessThanEqual(bound.min, ray.p0.xyz)) && all(lessThanEqual(ray.p0.xyz, bound.max)))
		return true;
	vec3 tmin = (bound.min - ray.p0.xyz) / ray.n;
	vec3 tmax = (bound.max - ray.p0.xyz) / ray.n;
	vec3 mintt = min(tmin, tmax);
	tmax = max(tmin, tmax);
	float maxt = max(mintt.x, max(mintt.y, mintt.z));
	if (maxt > 0)
	{
		float mint = min(tmax.x, min(tmax.y, tmax.z));
		return (maxt <= mint) && (ray.t < 0 || maxt <= ray.t);
	}
	else
		return false;
}
bool judgeHit(Ray ray)
{
	uint now = 0;
	uint bvhStack = 0;
	uint bvhState;
	int bvhSP = -1;
	for (;;)
	{
		if (judgeHitBox(ray, bvh[now].bound))
		{
			if (bvh[now].geometry != 0)
			{
				uint n = bvh[now].geometryNum;
				switch (bvh[now].geometry)
				{
					/*	for (n = 0; n < planeNum; ++n)
							{
								float tt = getPlaneT(ray, planes[n].plane);
								if (tt > 0 && (tt < ray.t || ray.t < 0))
								{
									ray.t = tt;
									vec3 p1 = ray.p0.xyz + ray.n * ray.t;
									tempColor = planes[n].color;
									tempColor.g = ((int(4.2 * p1.x) + int(4.2 * p1.y)) % 2u) * tempColor.g;
									tempN = planes[n].plane.xyz;
								}
						}*/
					case 2:
					{
						float tt = getPlaneT(ray, triangles[n].plane);
						if (tt > 0 && tt < ray.t)
						{
							vec2 uv = getTriangleUV(ray.p0.xyz + ray.n * tt, n);
							if (triangleTest(uv))return true;
						}
						break;
					}
					case 3:
					{
						vec3 d = spheres[n].sphere.xyz - ray.p0.xyz;
						float s = spheres[n].sphere.w - dot(cross(d, ray.n), cross(d, ray.n));
						if (s >= 0)
						{
							s = sqrt(s);
							float k = dot(d, ray.n);
							float tt = -1;
							if (k + s > 0)tt = k + s;
							if (k - s > 0)tt = k - s;
							if (tt > 0 && tt < ray.t)return true;
						}
						break;
					}
					case 4:
					{
						float tt = getPlaneT(ray, circles[n].plane);
						if (tt > 0 && tt < ray.t)
						{
							vec3 d = ray.p0.xyz + ray.n * tt - circles[n].sphere.xyz;
							if (dot(d, d) <= circles[n].sphere.w)return true;
						}
						break;
					}
					case 5:
					{
						float nn0 = dot(ray.n, cylinders[n].n);
						float cnn02 = 1 - nn0 * nn0;
						if (cnn02 != 0)
						{
							vec3 d = ray.p0.xyz - cylinders[n].c;
							float nd = dot(cylinders[n].n, d);
							vec3 j = d - nd * cylinders[n].n;
							float n0j = dot(ray.n, j);
							float k = n0j * n0j + cnn02 * (cylinders[n].r2 - dot(j, j));
							if (k > 0)
							{
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
								if (tt > 0 && tt < ray.t)return true;
							}
						}
						break;
					}
					case 6:
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
									tt = ttt;
							}
							if (tt > 0 && tt < ray.t)return true;
						}
						break;
					}
				}
			}
			if (bvh[now].bound.leftChild != 0)
			{
				setBit(bvhStack, ++bvhSP);
				if (ray.n[bvh[now].axis] >= 0 || bvh[now].bound.rightChild == 0)
				{
					++now;
					clearBit(bvhState, bvhSP);
				}
				else
				{
					now = bvh[now].bound.rightChild;
					setBit(bvhState, bvhSP);
				}
				continue;
			}
		}
		while (bvhSP >= 0)
		{
			now = bvh[now].father;
			if (bit(bvhStack, bvhSP))
			{
				if (bit(bvhState, bvhSP))
				{
					clearBit(bvhStack, bvhSP);
					++now;
					break;
				}
				else if (bvh[now].bound.rightChild != 0)
				{
					clearBit(bvhStack, bvhSP);
					now = bvh[now].bound.rightChild;
					break;
				}
			}
			--bvhSP;
		}
		if (bvhSP < 0)break;
	}
	return false;
}

vec4 rayTrace(Ray ray)
{
	Stack stack[RayTraceDepth];
	int sp = -1;
	int depth = 0;
	vec3 ratioNow = vec3(1);
	vec3 answer = vec3(0);
	vec3 tempN;
	vec2 tempUV;
	uvec2 hitObj;
	vec3 decayNow = decayOrigin;
	for (;;)
	{
		ray.t = -1;
		uint now = 0;
		uint bvhStack = 0;
		uint bvhState;
		int bvhSP = -1;
		for (;;)
		{
			if (judgeHitBox(ray, bvh[now].bound))
			{
				uint geometry = bvh[now].geometry;
				if (geometry != 0)
				{
					uint n = bvh[now].geometryNum;
					switch (geometry)
					{
						/*	for (n = 0; n < planeNum; ++n)
								{
									float tt = getPlaneT(ray, planes[n].plane);
									if (tt > 0 && (tt < ray.t || ray.t < 0))
									{
										ray.t = tt;
										vec3 p1 = ray.p0.xyz + ray.n * ray.t;
										tempColor = planes[n].color;
										tempColor.g = ((int(4.2 * p1.x) + int(4.2 * p1.y)) % 2u) * tempColor.g;
										tempN = planes[n].plane.xyz;
									}
							}*/
						case 2:
						{
							float tt = getPlaneT(ray, triangles[n].plane);
							if (tt > 0 && (tt < ray.t || ray.t < 0))
							{
								vec2 uv = getTriangleUV(ray.p0.xyz + ray.n * tt, n);
								if (triangleTest(uv))
								{
									ray.t = tt;
									hitObj = uvec2(geometry, n);
									tempN = triangles[n].plane.xyz;
									tempUV = (1 - uv.x - uv.y) * triangles[n].uv1 + uv.x * triangles[n].uv2 + uv.y * triangles[n].uv3;
								}
							}
							break;
						}
						case 3:
						{
							vec3 d = spheres[n].sphere.xyz - ray.p0.xyz;
							vec3 c = cross(d, ray.n);
							float s = spheres[n].sphere.w - dot(c, c);
							if (s >= 0)
							{
								s = sqrt(s);
								float k = dot(d, ray.n);
								float tt = -1;
								if (k + s > 0)tt = k + s;
								if (k - s > 0)tt = k - s;
								if (tt > 0 && (tt < ray.t || ray.t < 0))
								{
									ray.t = tt;
									hitObj = uvec2(geometry, n);
									tempN = (ray.p0.xyz + ray.t * ray.n - spheres[n].sphere.xyz) / sqrt(spheres[n].sphere.w);
									float ne1 = dot(tempN, spheres[n].e1);
									vec3 nxy = normalize(tempN - ne1 * spheres[n].e1);
									float u =
										dot(nxy, cross(spheres[n].e1, spheres[n].e2)) >= 0 ?
										acos(dot(spheres[n].e2, nxy)) / (2 * Pi) :
										1 - acos(dot(spheres[n].e2, nxy)) / (2 * Pi);
									tempUV = vec2(u, 1 - acos(ne1) / Pi);
								}
							}
							break;
						}
						case 4:
						{
							float tt = getPlaneT(ray, circles[n].plane);
							if (tt > 0 && (tt < ray.t || ray.t < 0))
							{
								vec3 d = ray.p0.xyz + ray.n * tt - circles[n].sphere.xyz;
								if (dot(d, d) <= circles[n].sphere.w)
								{
									ray.t = tt;
									hitObj = uvec2(geometry, n);
									tempN = circles[n].plane.xyz;
									vec3 e2 = cross(tempN, circles[n].e1);
									tempUV = (vec2(1) + vec2(dot(circles[n].e1, d), dot(e2, d)) / sqrt(circles[n].sphere.w)) / 2;
								}
							}
							break;
						}
						case 5:
						{
							float nn0 = dot(ray.n, cylinders[n].n);
							float cnn02 = 1 - nn0 * nn0;
							if (cnn02 != 0)
							{
								vec3 d = ray.p0.xyz - cylinders[n].c;
								float nd = dot(cylinders[n].n, d);
								vec3 j = d - nd * cylinders[n].n;
								float n0j = dot(ray.n, j);
								float k = n0j * n0j + cnn02 * (cylinders[n].r2 - dot(j, j));
								if (k > 0)
								{
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
									if (tt > 0 && (tt < ray.t || ray.t < 0))
									{
										ray.t = tt;
										hitObj = uvec2(geometry, n);
										tempN = normalize(d + ray.n * ray.t - cylinders[n].n * v);
										vec3 e2 = cross(cylinders[n].n, cylinders[n].e1);
										float u =
											dot(tempN, e2) >= 0 ?
											acos(dot(cylinders[n].e1, tempN)) / (2 * Pi) :
											1 - acos(dot(cylinders[n].e1, tempN)) / (2 * Pi);
										tempUV = vec2(u, v / cylinders[n].l);
									}
								}
							}
							break;
						}
						case 6:
						{
							vec3 d = ray.p0.xyz - cones[n].c;
							float nn0 = dot(ray.n, cones[n].n);
							float dn = dot(d, cones[n].n);
							vec3 j = dn * cones[n].n - cones[n].c2 * d;
							float c = dot(d, j);
							float b = dot(ray.n, j);
							float a = nn0 * nn0 - cones[n].c2;
							float s = b * b - a * c;
							if (s > 0)
							{
								s = sqrt(s);
								float tt = -1;
								float r2;
								tt = (s - b) / a;
								float d2 = dot(d, d);
								float dn0 = dot(d, ray.n);
								if (tt > 0)
								{
									r2 = d2 + tt * tt + 2 * dn0 * tt;
									float k = dn + nn0 * tt;
									if (r2 > cones[n].l2 || k < 0)tt = -1;
								}
								float ttt = (-b - s) / a;
								if (ttt > 0 && (ttt < tt || (tt < 0)))
								{
									float r2t = d2 + ttt * ttt + 2 * dn0 * ttt;
									float k = dn + nn0 * ttt;
									if (r2t <= cones[n].l2 && k > 0)
									{
										tt = ttt;
										r2 = r2t;
									}
								}
								if (tt > 0 && (tt < ray.t || ray.t < 0))
								{
									ray.t = tt;
									hitObj = uvec2(geometry, n);
									tempN = ((d + ray.n * ray.t) * sqrt(cones[n].c2 / r2) - cones[n].n) /
										sqrt(1 - cones[n].c2);
									vec3 nxy = normalize(d + ray.n * ray.t - cones[n].n * sqrt(r2 * cones[n].c2));
									float u =
										dot(nxy, cross(cones[n].n, cones[n].e1)) >= 0 ?
										acos(dot(cones[n].e1, nxy)) / (2 * Pi) :
										1 - acos(dot(cones[n].e1, nxy)) / (2 * Pi);
									tempUV = vec2(u, 1 - sqrt(r2 / cones[n].l2));
								}
							}
							break;
						}
					}
				}
				if (bvh[now].bound.leftChild != 0)
				{
					setBit(bvhStack, ++bvhSP);
					if (ray.n[bvh[now].axis] >= 0 || bvh[now].bound.rightChild == 0)
					{
						++now;
						clearBit(bvhState, bvhSP);
					}
					else
					{
						now = bvh[now].bound.rightChild;
						setBit(bvhState, bvhSP);
					}
					continue;
				}
			}
			while (bvhSP >= 0)
			{
				now = bvh[now].father;
				if (bit(bvhStack, bvhSP))
				{
					if (bit(bvhState, bvhSP))
					{
						clearBit(bvhStack, bvhSP);
						++now;
						break;
					}
					else if (bvh[now].bound.rightChild != 0)
					{
						clearBit(bvhStack, bvhSP);
						now = bvh[now].bound.rightChild;
						break;
					}
				}
				--bvhSP;
			}
			if (bvhSP < 0)break;
		}
		Color tempColor;
		tempColor.texG = -1;
		if (hitObj.x != 0)
		{
			switch (hitObj.x)
			{
				case 2:tempColor = triangles[hitObj.y].color; break;
				case 3:tempColor = spheres[hitObj.y].color; break;
				case 4:tempColor = circles[hitObj.y].color; break;
				case 5:tempColor = cylinders[hitObj.y].color; break;
				case 6:tempColor = cones[hitObj.y].color; break;
			}
		}
		if (tempColor.texG >= 0)
			tempColor.g *= texture(texSmp, vec3(tempUV, tempColor.texG)).xyz;
		if (ray.t < 0)
		{
			tempColor.g = texture(cubeSmp, ray.n).xyz;
			ray.t = 0;
		}
		ratioNow *= exp(decayNow * ray.t);
		answer += tempColor.g * ratioNow;
		if (ray.t > 0 && depth < RayTraceDepth)
		{
			if (tempColor.texT >= 0)
				tempColor.t *= texture(texSmp, vec3(tempUV, tempColor.texT)).xyz;
			tempColor.t *= ratioNow;
			float cosi1 = dot(ray.n, tempN);
			if (any(greaterThanEqual(tempColor.t, vec3(minColor))))
			{
				if (cosi1 > 0) tempColor.n = 1 / tempColor.n;
				float sini1 = sqrt(1 - cosi1 * cosi1);
				float sini2 = sini1 / tempColor.n;
				if (sini2 < 1)
				{
					float cosi2 = sqrt(1 - sini2 * sini2);
					if (sini2 <= 0.01)
					{
						float nadd1 = 1 / (tempColor.n + 1);
						tempColor.r *= pow((tempColor.n - 1) * nadd1, 2);
						tempColor.t *= pow(2 * nadd1 * pow(tempColor.n, 2), 2);
					}
					else
					{
						float cosadd = abs(cosi1) * cosi2;
						float sinadd = sini1 * cosi2;
						float cosminus = cosadd + sini1 * sini2;
						float sinminus = sinadd - abs(cosi1) * sini2;
						cosadd = 2 * cosadd - cosminus;
						float ahh = 1 / pow(cosminus, 2);
						sinadd = 2 * sinadd - sinminus;
						tempColor.r *= (1 + pow(cosadd, 2) * ahh) * pow(sinminus / sinadd, 2) / 2;
						tempColor.t *= abs(cosi2 * cosi1 * pow(2 * sini2 * tempColor.n * tempColor.n / sinadd, 2) * (1 + ahh) / 2);
					}
					if (any(greaterThanEqual(tempColor.t, vec3(minColor))))
					{
						stack[++sp].decayFactor = decayNow - sign(cosi1) * tempColor.decayFactor;
						stack[sp].p0 = ray.p0 + vec4((ray.t + offset) * ray.n, 0);
						stack[sp].n = (ray.n + (tempColor.n * sign(cosi1) * cosi2 - cosi1) * tempN) / tempColor.n;
						stack[sp].ratio = tempColor.t;
						stack[sp].depth = depth + 1;
					}
				}
				else
				{
					tempColor.r = vec3(1);
				}
			}
			ray.p0 += vec4((ray.t - offset) * ray.n, 0);
			uint n = 0;
			for (; n < pointLightNum; ++n)
			{
				vec3 dn = pointLights[n].p - ray.p0.xyz;
				float tt = dot(dn, dn);
				dn = normalize(dn);
				if (!judgeHit(Ray(ray.p0, dn, sqrt(tt))))
				{
					if (tempColor.texD >= 0)
						tempColor.d *= texture(texSmp, vec3(tempUV, tempColor.texD)).xyz;
					answer += max(-20 * sign(dot(ray.n, tempN)) * dot(tempN, dn) / tt, 0) * pointLights[n].color * tempColor.d * ratioNow;
				}
			}
			if (tempColor.texR >= 0)
				tempColor.r *= texture(texSmp, vec3(tempUV, tempColor.texR)).xyz;
			ratioNow *= tempColor.r;
			if (any(greaterThanEqual(ratioNow, vec3(minColor))))
			{
				ray.n -= 2 * cosi1 * tempN;
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
			decayNow = stack[sp].decayFactor;
			--sp;
		}
	}
	return vec4(answer, 1);
}

void main()
{
	gl_FragColor = rayTrace(rayAlloctor());
}