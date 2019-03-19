#include <cstdio>
#include <GL/_OpenGL.h>
#include <GL/_Window.h>
#include <_Math.h>
#include <_Time.h>
#include <RayTracing/_RayTracing.h>

namespace OpenGL
{
	struct RayTrace :OpenGL
	{
		struct Renderer :Program
		{
			RayTracing::View view;
			Buffer viewBuffer;
			BufferConfig viewArray;
			VertexAttrib position;

			//BufferConfig frameStorage;

			Renderer(SourceManager* _sm)
				:
				Program(_sm, "Renderer", Vector<VertexAttrib*>{&position}),
				view(),
				viewBuffer(&view),
				viewArray(&viewBuffer, ArrayBuffer),
				position(&viewArray, 0, VertexAttrib::two,
					VertexAttrib::Float, false, sizeof(Math::vec2<float>), 0, 0)
			{
				init();
			}
			virtual void initBufferData()
			{
			}
			virtual void run()override
			{
				glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
				glClear(GL_COLOR_BUFFER_BIT);
				glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			}
		};
		struct RayTracer :Computers
		{
			struct Preprocessor :Program
			{
				Preprocessor(SourceManager* _sm)
					:
					Program(_sm, "Preprocessor")
				{

				}
				virtual void initBufferData()override
				{

				}
				virtual void run()override
				{

				}
			};
			struct Tracing :Program
			{
				RayTracing::FrameScale* frameScale;
				Tracing(SourceManager* _sm, RayTracing::FrameScale* _frameScale)
					:
					Program(_sm, "Tracing"),
					frameScale(_frameScale)
				{
					init();
				}
				virtual void initBufferData()override
				{
				}
				virtual void run()override
				{
					glDispatchCompute(frameScale->scale.data[0] / 32, frameScale->scale.data[1] / 32, 1);
				}
			};

			Tracing tracing;
			RayTracer(SourceManager* _sm, RayTracing::FrameScale* _frameScale)
				:
				tracing(_sm, _frameScale)
			{
			}
			virtual void initBufferData()
			{
			}
			virtual void run()
			{
				tracing.use();
				tracing.run();
			}
		};

		SourceManager sm;
		RayTracing::FrameScale frameScale;
		RayTracing::FrameData frameData;
		Buffer frameSizeBuffer;
		Buffer frameDataBuffer;
		BufferConfig frameSizeUniform;
		BufferConfig frameDataStorage;
		Renderer renderer;
		RayTracer rayTracer;

		RayTrace(Math::vec2<unsigned int>const& _scale)
			:
			sm(),
			frameScale(_scale),
			frameData(&frameScale),
			frameSizeBuffer(&frameScale),
			frameDataBuffer(&frameData),
			frameSizeUniform(&frameSizeBuffer, UniformBuffer, 0),
			frameDataStorage(&frameDataBuffer, ShaderStorageBuffer, 1),
			renderer(&sm),
			rayTracer(&sm, &frameScale)
		{
		}

		virtual void init(FrameScale const& _size) override
		{
			glViewport(0, 0, _size.w, _size.h);
			renderer.viewArray.dataInit();
			frameSizeUniform.dataInit();
			frameDataStorage.dataInit();
		}
		virtual void run() override
		{
			rayTracer.run();
			renderer.use();
			renderer.run();
		}
		virtual void frameSize(int, int) override
		{
		}
		virtual void framePos(int, int) override
		{
		}
		virtual void frameFocus(int) override
		{
		}
		virtual void mouseButton(int, int, int) override
		{

		}
		virtual void mousePos(double, double) override
		{

		}
		virtual void mouseScroll(double, double) override
		{

		}
		virtual void key(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods) override
		{
			switch (_key)
			{
			case GLFW_KEY_ESCAPE:
				if (_action == GLFW_PRESS)
					glfwSetWindowShouldClose(_window, true);
				break;
				/*case GLFW_KEY_A:trans.key.refresh(0, _action); break;
				case GLFW_KEY_D:trans.key.refresh(1, _action); break;
				case GLFW_KEY_W:trans.key.refresh(2, _action); break;
				case GLFW_KEY_S:trans.key.refresh(3, _action); break;*/
			}
		}
	};
}

int main()
{
	OpenGL::OpenGLInit init(4, 5);
	Window::Window::Data winPara
	{
		"RayTracing",
		{
			{640,640},
			false,false,
		}
	};
	Window::WindowManager wm(winPara);
	OpenGL::RayTrace test({ 640,640 });
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
