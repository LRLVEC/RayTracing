#version 450 core
layout(location = 0) in vec3 position;
//layout(location = 1) in vec3 color;
layout(std140, row_major, binding = 0)uniform transBuffer
{
	mat4 trans;
};
out vec4 in_color;
void main()
{
	gl_Position = trans * vec4(position, 1);
	in_color = vec4(0, 0.7, 0.7, 1);
}