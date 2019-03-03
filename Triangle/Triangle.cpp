#include <cstdio>
#define GLEW_STATIC
#include <GL/GLEW/glew.h>
#include <GL/GLFW/glfw3.h>
#pragma comment(lib,"OpenGL32.lib")
#ifdef _DEBUG
#pragma comment(lib,"GL/glew32s.lib")
#pragma comment(lib,"GL/glfw3ddll.lib")
#else
#pragma comment(lib,"GL/glew32s.lib")
#pragma comment(lib,"GL/glfw3dll.lib")
#endif

char const* vertexSource =
"#version 450 core						\n"
"layout (location = 0)in vec3 vPosition;\n"
"layout (location = 1)in vec3 vColor;	\n"
"out vec4 vfColor;						\n"
"void main()							\n"
"{										\n"
"	gl_Position = vec4(vPosition, 1);	\n"
"	vfColor = vec4(vColor, 1);			\n"
"}										\n"
;
char const* fragmentSource =
"#version 450 core						\n"
"in vec4 vfColor;						\n"
"out vec4 fColor;						\n"
"void main()							\n"
"{										\n"
"	fColor = vfColor;					\n"
"}										\n"
;
void keyCallback(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods)
{

	switch (_key)
	{
		case GLFW_KEY_ESCAPE:
			if (_action == GLFW_PRESS)
				glfwSetWindowShouldClose(_window, true);
	}
}
int main()
{
	glfwInit();
	glewExperimental = GL_TRUE;
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

	GLFWwindow* window = glfwCreateWindow(200, 200, "Triangle", NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, keyCallback);

	glfwSwapInterval(1);
	glewInit();

	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint renderer;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	renderer = glCreateProgram();

	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);

	glCompileShader(vertexShader);
	glCompileShader(fragmentShader);

	char log[1024];
	GLint success(1);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 2014, NULL, log);
		::printf("%s\n", log);
	}
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 2014, NULL, log);
		::printf("%s\n", log);
	}
	glAttachShader(renderer, vertexShader);
	glAttachShader(renderer, fragmentShader);
	glLinkProgram(renderer);
	glGetShaderiv(renderer, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(renderer, 2014, NULL, log);
	}


	glViewport(0, 0, 200, 200);

	GLuint triangle;
	GLuint vao;


	glCreateBuffers(1, &triangle);
	glBindBuffer(GL_ARRAY_BUFFER, triangle);
	float tri[3][2][3] =
	{
		{
			{0.5f,-0.5f,0},
			{0,0,1},
		},
		{
			{-0.5f,-0.5f,0},
			{1,0,0},
		},
		{
			{0,0.5f,0},
			{0,1,0},
		},
	};
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), NULL);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glUseProgram(renderer);
	glBindBuffer(GL_ARRAY_BUFFER, triangle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(tri), tri, GL_STATIC_DRAW);
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glfwSwapBuffers(window);
	}
	glfwDestroyWindow(window);
	glfwTerminate();
}