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

char const* vertexSource =//顶点着色器
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
char const* fragmentSource =//片元着色器
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
	glewExperimental = GL_TRUE;//如果不写程序会崩溃？
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//使用opengl核心模式
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);//主版本号
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);//副版本号
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	GLFWwindow* window = glfwCreateWindow(400, 400, "Square", NULL, NULL);
	glfwMakeContextCurrent(window);//设置窗口的上下文
	glfwSetKeyCallback(window, keyCallback);//设置回调函数，按下esc窗口关闭

	glfwSwapInterval(1);//缓存刷新时间秒？
	glewInit();

	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint renderer;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);//创建顶点着色器和片元着色器
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	renderer = glCreateProgram();

	glShaderSource(vertexShader, 1, &vertexSource, NULL);//着色器对象关联源代码
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);

	glCompileShader(vertexShader);//把着色器源代码编译成目标代码
	glCompileShader(fragmentShader);

	char log[1024];
	GLint success(1);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);//验证是否编译通过
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
	}//如果不通过，返回诊断信息
	glAttachShader(renderer, vertexShader);
	glAttachShader(renderer, fragmentShader);//把着色器链接到程序里
	glLinkProgram(renderer);
	glGetShaderiv(renderer, GL_LINK_STATUS, &success);//链接着色器程序
	if (!success)
	{
		glGetProgramInfoLog(renderer, 2014, NULL, log);
	}


	glViewport(0, 0, 400, 400);

	GLuint square;
	GLuint vao;


	glCreateBuffers(1, &square);//第一个参数表示创建的缓存数量
	glBindBuffer(GL_ARRAY_BUFFER, square);//第一个参数表示顶点数据数组还是索引数据数组
	float squ[4][2][3] =
	{
		{
			{0.5f,-0.5f,0},
			{0.5,0,1},
		},
		{
			{-0.5f,-0.5f,0},
			{1,1,1},
		},
		{
			{-0.5f,0.5f,0},
			{0,0.5,1},
		},
		{
			{0.5f,0.5f,0},
			{0.5,1,0}

		}
	};
	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), NULL);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);//开启顶点属性数组
	glUseProgram(renderer);
	glBindBuffer(GL_ARRAY_BUFFER, square);
	glBufferData(GL_ARRAY_BUFFER, sizeof(squ), squ, GL_STATIC_DRAW);
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		glfwSwapBuffers(window);
	}
	glfwDestroyWindow(window);
	glfwTerminate();
}