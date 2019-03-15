#version 450 core
layout(local_size_x = 1024)in;

struct Particle
{
	vec3 position;
	float mass;
	vec3 velocity;
};
layout(std430, binding = 1)buffer ParticlesBuffer
{
	Particle particles[];
};
layout(std430, binding = 2)buffer Acceleration
{
	vec3 a[];
};
layout(std140, binding = 3)uniform ParameterBuffer
{
	float dt;
	float G;
	uint num;
};
void main()
{
	uint i = uint(sqrt(2.0 * gl_GlobalInvocationID.x)) + 1;
	uint j = gl_GlobalInvocationID.x - ((i - 1) * (i - 2) >> 1);
	vec3 dr = particles[j].position - particles[i].position;
	a[gl_GlobalInvocationID.x] = dr / (pow(length(dr), 3) + 0.001);
}