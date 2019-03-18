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

char const* vertexSource =//������ɫ��
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
char const* fragmentSource =//ƬԪ��ɫ��
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
	glewExperimental = GL_TRUE;//�����д����������
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//ʹ��opengl����ģʽ
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);//���汾��
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);//���汾��
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	GLFWwindow* window = glfwCreateWindow(400, 400, "Square", NULL, NULL);
	glfwMakeContextCurrent(window);//���ô��ڵ�������
	glfwSetKeyCallback(window, keyCallback);//���ûص�����������esc���ڹر�

	glfwSwapInterval(1);//����ˢ��ʱ���룿
	glewInit();

	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint renderer;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);//����������ɫ����ƬԪ��ɫ��
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	renderer = glCreateProgram();

	glShaderSource(vertexShader, 1, &vertexSource, NULL);//��ɫ���������Դ����
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);

	glCompileShader(vertexShader);//����ɫ��Դ��������Ŀ�����
	glCompileShader(fragmentShader);

	char log[1024];
	GLint success(1);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);//��֤�Ƿ����ͨ��
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
	}//�����ͨ�������������Ϣ
	glAttachShader(renderer, vertexShader);
	glAttachShader(renderer, fragmentShader);//����ɫ�����ӵ�������
	glLinkProgram(renderer);
	glGetShaderiv(renderer, GL_LINK_STATUS, &success);//������ɫ������
	if (!success)
	{
		glGetProgramInfoLog(renderer, 2014, NULL, log);
	}


	glViewport(0, 0, 400, 400);

	GLuint square;
	GLuint vao;


	glCreateBuffers(1, &square);//��һ��������ʾ�����Ļ�������
	glBindBuffer(GL_ARRAY_BUFFER, square);//��һ��������ʾ�����������黹��������������
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
	glEnableVertexAttribArray(1);//����������������
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