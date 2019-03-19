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
"#version 450 core										\n"
"layout (location = 0)in vec3 vPosition;				\n"
"layout (location = 1)in vec3 vColor;					\n"
"out vec2 pos;											\n"
"void main()											\n"
"{														\n"
"	gl_Position = vec4(vPosition, 1);					\n"
"	pos = vPosition.xy / 2 + vec2(0.5, 0.5);			\n"
"}														\n"
;
char const* fragmentSource =
"#version 450 core										\n"
"in vec2 pos;											\n"
"out vec4 fColor;										\n"
"uniform sampler2D smp;									\n"
"void main()											\n"
"{														\n"
"	fColor = texture(smp, pos);							\n"
"}														\n"
;
char const* computeSource =
"#version 450 core											\n"
"layout(local_size_x = 32, local_size_y = 32)in;			\n"
"layout(binding = 0, rgba32f)uniform image2D image;			\n"
"															\n"
"void main()												\n"
"{															\n"
"	imageStore(image, ivec2(gl_GlobalInvocationID.xy),		\n"
"		vec4(0, gl_GlobalInvocationID.xy, 0) / 32.0f);		\n"
"}															\n"
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

	GLFWwindow* window = glfwCreateWindow(1280, 1280, "Triangle", NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, keyCallback);

	glfwSwapInterval(1);
	glewInit();

	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint computeShader;
	GLuint renderer;
	GLuint computer;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	computeShader = glCreateShader(GL_COMPUTE_SHADER);
	renderer = glCreateProgram();
	computer = glCreateProgram();

	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glShaderSource(computeShader, 1, &computeSource, NULL);

	glCompileShader(vertexShader);
	glCompileShader(fragmentShader);
	glCompileShader(computeShader);

	char log[1024];
	GLint success(1);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 1024, NULL, log);
		::printf("%s\n", log);
	}
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 1024, NULL, log);
		::printf("%s\n", log);
	}
	glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(computeShader, 1024, NULL, log);
		::printf("%s\n", log);
	}

	glAttachShader(renderer, vertexShader);
	glAttachShader(renderer, fragmentShader);
	glAttachShader(computer, computeShader);

	glLinkProgram(renderer);
	glLinkProgram(computer);

	glGetShaderiv(renderer, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(renderer, 1024, NULL, log);
	}
	glGetShaderiv(computer, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(computer, 1024, NULL, log);
	}


	glViewport(0, 0, 1280, 1280);

	GLuint triangle;
	GLuint vao;




	GLuint texture;
	//GLuint textureBuffer;
	//glGenBuffers(1, &textureBuffer);
	//glBindBuffer(GL_TEXTURE_BUFFER, textureBuffer);
	//glBufferData(GL_TEXTURE_BUFFER, 16384, NULL, GL_DYNAMIC_DRAW);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, 32, 32);
	glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	/*glBindTexture(GL_TEXTURE_2D, 0);*/
	glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);


	glUseProgram(computer);
	glDispatchCompute(1, 1, 1);




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