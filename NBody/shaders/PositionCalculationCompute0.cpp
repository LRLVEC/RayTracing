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
	//vec3 dv = vec3(0);
	//uint c0 = (gl_GlobalInvocationID.x - 1) * (gl_GlobalInvocationID.x - 2);
	//uint s0 = c0 + gl_GlobalInvocationID.x;
	//for (; c0 < s0; ++c0)
	//	dv += a[c0];
	particles[gl_GlobalInvocationID.x].position +=
		dt * particles[gl_GlobalInvocationID.x].velocity;
	if (particles[gl_GlobalInvocationID.x].position.x < -1.0 ||
		particles[gl_GlobalInvocationID.x].position.x > 1.0)
		particles[gl_GlobalInvocationID.x].velocity.x *= -1.0;
	if (particles[gl_GlobalInvocationID.x].position.y < -1.0 ||
		particles[gl_GlobalInvocationID.x].position.y > 1.0)
		particles[gl_GlobalInvocationID.x].velocity.y *= -1.0;
	if (particles[gl_GlobalInvocationID.x].position.z < -1.0 ||
		particles[gl_GlobalInvocationID.x].position.z > 1.0)
		particles[gl_GlobalInvocationID.x].velocity.z *= -1.0;
}