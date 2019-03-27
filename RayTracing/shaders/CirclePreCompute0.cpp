#version 460 core
layout(local_size_x = 1024)in;
struct Color
{
	vec3 r;
	vec3 t;
	vec3 d;
	vec3 g;
	float n;
};
struct Circle
{
	vec4 plane;
	vec4 sphere;
	vec3 e1;
	vec3 e2;
	uint tex;
	Color color;
};
layout(std140, binding = 3)uniform GeometryNum
{
	uint planeNum;
	uint triangleNum;
	uint sphereNum;
	uint circleNum;
};
layout(std430, binding = 4)buffer Circles
{
	Circle circles[];
};
void main()
{
	if (gl_GlobalInvocationID.x < circleNum)
	{
		vec3 n = normalize(circles[gl_GlobalInvocationID.x].plane.xyz);
		circles[gl_GlobalInvocationID.x].plane =
			vec4(n, -dot(n, circles[gl_GlobalInvocationID.x].sphere.xyz));
		circles[gl_GlobalInvocationID.x].e1 =
			normalize(circles[gl_GlobalInvocationID.x].e1);
		circles[gl_GlobalInvocationID.x].e2 =
			cross(n, circles[gl_GlobalInvocationID.x].e1);
	}
}