#include <cstdio>
#include <GL/_OpenGL.h>
#include <GL/_Window.h>
#include <GL/_NBody.h>
#include <_Math.h>
#include <_Time.h>


int main()
{
	OpenGL::OpenGLInit init(4, 5);
	Window::Window::Data winParameters
	{
		"NBody",
		{
			{800,800},
			false,false
		}
	};
	Window::WindowManager wm(winParameters);
	OpenGL::NBody test(20);
	wm.init(0, &test);
	glfwSwapInterval(0);
	FPS fps;
	fps.refresh();
	while (!wm.close())
	{
		wm.pullEvents();
		wm.render();
		wm.swapBuffers();
		//fps.refresh();
		//fps.printFPS(1);
	}
	return 0;
}