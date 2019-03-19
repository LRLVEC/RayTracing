#version 450 core
layout(local_size_x = 32, local_size_y = 32)in;


struct Ray
{
	vec4 p0;
	vec3 n;
	float t;
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
layout(std430, binding = 2)buffer FrameBuffer
{
	vec4 frame[];
};


Ray rayAlloctor()
{
	vec3 n0 = vec3(ivec2(gl_GlobalInvocationID.xy) - ivec2(size.xy / 2), z0);
	return Ray(vec4(r0, 1), normalize(trans * n0), -1);
}
float planeTest(Ray ray, vec4 para)
{
	return -dot(para, ray.p0) / dot(para.xyz, ray.n);
}
vec4 rayTrace(Ray ray)
{
	float t = planeTest(ray, vec4(0, 0, 1, 1));
	if (t >= 0)
	{
		ray.p0 += vec4(ray.n * t, 0);
		return vec4(0, uint((int(ray.p0.x) + int(ray.p0.y)) % 2u), 0, 0);
	}
	return vec4(0, 0, 1, 0);
}

void main()
{
	frame[size.x * gl_GlobalInvocationID.y + gl_GlobalInvocationID.x] = rayTrace(rayAlloctor());
}
