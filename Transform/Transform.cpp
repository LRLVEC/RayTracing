#include <cstdio>
#include <cstdlib>
#include <GL/_OpenGL.h>
#include <GL/_Window.h>
#include <_Math.h>
#include <_Time.h>
#include <_Array.h>
#include <_Pair.h>

namespace OpenGL
{
	struct GLMathTest :OpenGL
	{
		struct Renderer :Program
		{
			struct TriangleData :Buffer::Data
			{
				using Position = Math::vec3<float>;
				using Color = Math::vec3<float>;
				struct Vertex
				{
					Position pos;
					Color color;
				};
				using Triangle = Array<Vertex, 3>;

				Vector<Triangle> triangles;
				TriangleData();
				virtual void* pointer()override
				{
					return (void*)triangles.data;
				}
				virtual unsigned int size()override
				{
					return sizeof(Triangle)* triangles.length;
				}
			};

			TriangleData triangles;

			Transform trans;
			Buffer buffer;
			Buffer transformBuffer;

			BufferConfig bufferArray;
			BufferConfig transformUnifrom;

			VertexAttrib positions;
			VertexAttrib colors;

			Renderer(SourceManager* _sourceManage)
				:
				Program(_sourceManage, "Triangle", Vector<VertexAttrib*>{&positions, & colors}),
				triangles(),
				trans({ {60.0,0.1,100},{0.05,0.8,0.05},{0.03},500.0 }),
				buffer(&triangles),
				transformBuffer(&trans.bufferData),
				bufferArray(&buffer, ArrayBuffer),
				transformUnifrom(&transformBuffer, UniformBuffer, 0),
				positions(&bufferArray, 0, VertexAttrib::three,
					VertexAttrib::Float, false, sizeof(TriangleData::Vertex), 0, 0),
				colors(&bufferArray, 1, VertexAttrib::three,
					VertexAttrib::Float, false, sizeof(TriangleData::Vertex), sizeof(TriangleData::Position), 0)
			{
				init();
			}
			void refreshBuffer()
			{
				trans.operate();
				if (trans.updated)
				{
					transformUnifrom.refreshData();
					trans.updated = false;
				}
			}
			virtual void initBufferData()override
			{
			}
			virtual void run() override
			{
				glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
				glClear(GL_COLOR_BUFFER_BIT);
				glDrawArrays(GL_TRIANGLES, 0, 3);
			}
			void resize(int _w, int _h)
			{
				trans.resize(_w, _h);
				glViewport(0, 0, _w, _h);
			}
		};

		SourceManager sm;
		Renderer renderer;

		GLMathTest();
		virtual void init(FrameScale const&) override;
		virtual void run() override;
		virtual void frameSize(int, int) override;
		virtual void framePos(int, int) override;
		virtual void frameFocus(int) override;
		virtual void mouseButton(int, int, int) override;
		virtual void mousePos(double, double) override;
		virtual void mouseScroll(double, double) override;
		virtual void key(GLFWwindow*, int, int, int, int) override;
	};

	inline GLMathTest::GLMathTest()
		:
		sm(),
		renderer(&sm)
	{
	}
	inline void GLMathTest::init(FrameScale const& _size)
	{
		glViewport(0, 0, _size.w, _size.h);
		renderer.trans.init(_size);
		renderer.transformUnifrom.dataInit();
		renderer.bufferArray.dataInit();
	}
	inline void GLMathTest::run()
	{
		/*
		glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);*/
		renderer.use();
		renderer.refreshBuffer();
		renderer.run();
	}
	inline void GLMathTest::frameSize(int _w, int _h)
	{
		renderer.resize(_w, _h);
	}
	inline void GLMathTest::framePos(int, int)
	{
	}
	inline void GLMathTest::frameFocus(int)
	{
	}
	inline void GLMathTest::mouseButton(int _button, int _action, int _mods)
	{
		switch (_button)
		{
			case GLFW_MOUSE_BUTTON_LEFT:renderer.trans.mouse.refreshButton(0, _action); break;
			case GLFW_MOUSE_BUTTON_MIDDLE:renderer.trans.mouse.refreshButton(1, _action); break;
			case GLFW_MOUSE_BUTTON_RIGHT:renderer.trans.mouse.refreshButton(2, _action); break;
		}
	}
	inline void GLMathTest::mousePos(double _x, double _y)
	{
		renderer.trans.mouse.refreshPos(_x, _y);
	}
	inline void GLMathTest::mouseScroll(double _x, double _y)
	{
		if (_y != 0.0)
			renderer.trans.scroll.refresh(_y);
	}
	inline void GLMathTest::key(GLFWwindow * _window, int _key, int _scancode, int _action, int _mods)
	{
		switch (_key)
		{
			case GLFW_KEY_ESCAPE:
				if (_action == GLFW_PRESS)
					glfwSetWindowShouldClose(_window, true);
				break;
			case GLFW_KEY_A:renderer.trans.key.refresh(0, _action); break;
			case GLFW_KEY_D:renderer.trans.key.refresh(1, _action); break;
			case GLFW_KEY_W:renderer.trans.key.refresh(2, _action); break;
			case GLFW_KEY_S:renderer.trans.key.refresh(3, _action); break;
		}
	}

	GLMathTest::Renderer::TriangleData::TriangleData()
		:
		Data(StaticDraw),
		triangles({ {{0.5f,-0.5f,-1.0},{1.0f,0,0}},{{-0.5f,-0.5f,-1.0},{0,1.0f,0}},{{0,0.5f,-1.0},{0,0,1.0f}} })
	{
	}
}


int main()
{
	//This is just a comment...
	//lrlvec's change
	OpenGL::OpenGLInit init(4, 5);
	Window::Window::Data winParameters
	{
		"Ahh",
		{
			{400,400},
			true,false
		}
	};
	Window::WindowManager wm(winParameters);
	OpenGL::GLMathTest test;
	wm.init(0, &test);
	glfwSwapInterval(1);
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

