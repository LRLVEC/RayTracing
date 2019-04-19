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
}
layout(std140, binding = 2)uniform ParameterBuffer
{
	float dt;
	uint num;
};
void main()
{
	uint a = num >> 1;
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
