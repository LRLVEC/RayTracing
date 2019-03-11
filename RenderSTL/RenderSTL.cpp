#include <cstdio>
#include <cstdlib>
#include <GL/_OpenGL.h>
#include <GL/_Window.h>
#include <_Math.h>
#include <_Time.h>
#include <_Array.h>
#include <_Pair.h>
#include <_STL.h>

namespace OpenGL
{
	struct RenderSTL :OpenGL
	{
		struct Renderer :Program
		{
			struct Light :Buffer::Data
			{
				Math::vec3<float>eularAngle;
				Math::vec4<float>normal;
				Light()
					:
					eularAngle({ 0, 1, 0 }),
					normal(Math::vec4<float>(Math::eulerAngle(eularAngle)))
				{
				}
				Light(Math::vec3<float>const& a)
					:
					normal(a)
				{
				}
				virtual void* pointer()override
				{
					return (void*)normal.data;
				}
				virtual unsigned int size()override
				{
					return sizeof(Math::vec4<float>);
				}
				void refresh()
				{
					normal = Math::eulerAngle(eularAngle);
				}
			};

			STL model;

			STLVertices modelPostions;
			STLNormals modelNormals;
			Transform trans;
			Light light;

			Buffer positionBuffer;
			Buffer transformBuffer;
			Buffer lightBuffer;
			Buffer normalBuffer;

			BufferConfig positionArray;
			BufferConfig transformUniform;
			BufferConfig lightUniform;
			BufferConfig normalShader;

			VertexAttrib positions;

			Renderer(SourceManager* _sourceManage)
				:
				Program(_sourceManage, "Triangle", Vector<VertexAttrib*>{&positions}),
				model(_sourceManage->folder.find("resources/star.stl").readSTL()),

				modelPostions(&model),
				modelNormals(&model),

				trans({ {80.0,0.1,200},{0.5,0.8,0.05},{2},500.0 }),
				light(),

				positionBuffer(&modelPostions),
				transformBuffer(&trans.bufferData),
				lightBuffer(&light),
				normalBuffer(&modelNormals),

				positionArray(&positionBuffer,ArrayBuffer),
				transformUniform(&transformBuffer, UniformBuffer, 0),
				lightUniform(&lightBuffer, UniformBuffer, 1),
				normalShader(&normalBuffer, ShaderStorageBuffer, 2),

				positions(&positionArray, 1, VertexAttrib::three, VertexAttrib::Float, false,
					sizeof(Math::vec3<float>), 0, 0)
			{
				//model.printInfo();
				model.removeUseless();
				model.getVerticesRepeated();
				model.getNormals();
				init();
			}
			void refreshBuffer()
			{
				trans.operate();
				if (trans.updated)
				{
					//trans.bufferData.ans.print();
					transformUniform.refreshData();
					trans.updated = false;
				}
				light.eularAngle[2] += 0.01;
				light.refresh();
				lightUniform.refreshData();
			}
			virtual void initBufferData()override
			{
			}
			virtual void run() override
			{
				glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glDrawArrays(GL_TRIANGLES, 0, 3 * model.triangles.length);
			}
			void resize(int _w, int _h)
			{
				glViewport(0, 0, _w, _h);
				trans.resize(_w, _h);
			}
		};

		SourceManager sm;
		Renderer renderer;

		RenderSTL();
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

	inline RenderSTL::RenderSTL()
		:
		sm(),
		renderer(&sm)
	{
	}
	inline void RenderSTL::init(FrameScale const& _size)
	{
		glViewport(0, 0, _size.w, _size.h);
		glEnable(GL_DEPTH_TEST);
		renderer.trans.init(_size);
		renderer.transformUniform.dataInit();
		renderer.lightUniform.dataInit();
		renderer.normalShader.dataInit();

		renderer.positionArray.dataInit();
	}
	inline void RenderSTL::run()
	{
		/*
		glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);*/
		renderer.use();
		renderer.refreshBuffer();
		renderer.run();
	}
	inline void RenderSTL::frameSize(int _w, int _h)
	{
		renderer.resize(_w, _h);
	}
	inline void RenderSTL::framePos(int, int)
	{
	}
	inline void RenderSTL::frameFocus(int)
	{
	}
	inline void RenderSTL::mouseButton(int _button, int _action, int _mods)
	{
		switch (_button)
		{
			case GLFW_MOUSE_BUTTON_LEFT:renderer.trans.mouse.refreshButton(0, _action); break;
			case GLFW_MOUSE_BUTTON_MIDDLE:renderer.trans.mouse.refreshButton(1, _action); break;
			case GLFW_MOUSE_BUTTON_RIGHT:renderer.trans.mouse.refreshButton(2, _action); break;
		}
	}
	inline void RenderSTL::mousePos(double _x, double _y)
	{
		renderer.trans.mouse.refreshPos(_x, _y);
	}
	inline void RenderSTL::mouseScroll(double _x, double _y)
	{
		if (_y != 0.0)
			renderer.trans.scroll.refresh(_y);
	}
	inline void RenderSTL::key(GLFWwindow * _window, int _key, int _scancode, int _action, int _mods)
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



}


int main()
{
	OpenGL::OpenGLInit init(4, 4);
	Window::Window::Data winParameters
	{
		"Ahh",
		{
			{800,800},
			true,false
		}
	};
	Window::WindowManager wm(winParameters);
	OpenGL::RenderSTL test;
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

