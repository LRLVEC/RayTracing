#include <cstdio>
#include <GL/_OpenGL.h>
#include <GL/_Window.h>
#include <_Math.h>
#include <_Time.h>
#include <random>

namespace OpenGL
{
	struct NBody :OpenGL
	{
		struct Particles
		{
			struct Particle
			{
				Math::vec3<float>position;
				float mass;
				Math::vec4<float>velocity;
			};
			Vector<Particle>particles;
			void randomGalaxy(unsigned int n)
			{
				std::mt19937 mt;
				std::uniform_real_distribution<float>randReal(0, 1);
				while (n--)
				{
					float theta(2 * Math::Pi * randReal(mt));
					float r(randReal(mt));
					particles.pushBack(
						{
							{r * cos(theta),r * sin(theta),2.0f * randReal(mt) - 1.0f},
							randReal(mt),
							{-(r + 0.5f) * sin(theta),(r + 0.5f) * cos(theta),2.0f * randReal(mt) - 1.0f},
						});
				}
			}
		};
		struct ParticlesData :Buffer::Data
		{
			Particles* particles;
			ParticlesData(Particles* _particles)
				:
				Data(DynamicDraw),
				particles(_particles)
			{
			}
			virtual void* pointer()override
			{
				return particles->particles.data;
			}
			virtual unsigned int size()override
			{
				return sizeof(Particles::Particle)* (particles->particles.length);
			}
		};

		struct Renderer :Program
		{
			Buffer transBuffer;
			BufferConfig transUniform;
			BufferConfig particlesArray;
			VertexAttrib positions;
			VertexAttrib velocities;


			Renderer(SourceManager* _sm, Buffer* _particlesBuffer, Transform* _trans)
				:
				Program(_sm, "Renderer", Vector<VertexAttrib*>{&positions, & velocities}),
				transBuffer(&_trans->bufferData),
				transUniform(&transBuffer, UniformBuffer, 0),
				particlesArray(_particlesBuffer, ArrayBuffer),
				positions(&particlesArray, 0, VertexAttrib::three, VertexAttrib::Float, false, sizeof(Particles::Particle), 0, 0),
				velocities(&particlesArray, 1, VertexAttrib::three, VertexAttrib::Float, false, sizeof(Particles::Particle), 16, 0)
			{
				init();
			}
			virtual void initBufferData()override
			{
			}
			virtual void run()override
			{
				glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glDrawArrays(GL_POINTS, 0, particlesArray.buffer->data->size() / sizeof(Particles::Particle));
			}

		};
		struct Computer :Program
		{
			struct ParameterData : Buffer::Data
			{
				struct Parameter
				{
					float dt;
				};
				Parameter parameter;
				ParameterData(Parameter const& _parameter)
					:
					parameter(_parameter)
				{

				}
				virtual unsigned int size()override
				{
					return sizeof(Parameter);
				}
				virtual void* pointer()override
				{
					return &parameter;
				}
			};
			ParameterData parameterData;
			Buffer parameterBuffer;
			BufferConfig parameterUniform;
			BufferConfig particlesShader;


			Computer(SourceManager* _sm, Buffer* _particlesBuffer)
				:
				Program(_sm, "Computer"),
				parameterData({ 0.001f }),
				parameterBuffer(&parameterData),
				parameterUniform(&parameterBuffer, UniformBuffer, 2),
				particlesShader(_particlesBuffer, ShaderStorageBuffer, 1)
			{
				init();
			}
			virtual void initBufferData()override
			{
			}
			virtual void run()override
			{
				//particlesShader.bind();
				glDispatchCompute(particlesShader.buffer->data->size() / 1024, 1, 1);
			}
		};

		SourceManager sm;
		Particles particles;
		ParticlesData particlesData;
		Buffer particlesBuffer;
		Transform trans;
		Renderer renderer;
		Computer computer;

		NBody(unsigned int _groups)
			:
			sm(),
			particles(),
			particlesData(&particles),
			particlesBuffer(&particlesData),
			trans({ {80.0,0.01,20},{0.05,0.8,0.01},{0.05},500.0 }),
			renderer(&sm, &particlesBuffer, &trans),
			computer(&sm, &particlesBuffer)
		{
			particles.randomGalaxy(1024 * _groups);

		}
		virtual void init(FrameScale const& _size)override
		{
			glViewport(0, 0, _size.w, _size.h);
			glPointSize(1);
			glEnable(GL_DEPTH_TEST);
			trans.init(_size);
			renderer.transUniform.dataInit();
			renderer.particlesArray.dataInit();
			computer.parameterUniform.dataInit();
		}
		virtual void run()override
		{
			renderer.use();
			trans.operate();
			if (trans.updated)
			{
				renderer.transUniform.refreshData();
				trans.updated = false;
			}
			renderer.run();
			computer.use();
			computer.run();
		}
		virtual void frameSize(int _w, int _h) override
		{
			trans.resize(_w, _h);
			glViewport(0, 0, _w, _h);
		}
		virtual void framePos(int, int) override
		{
		}
		virtual void frameFocus(int) override
		{
		}
		virtual void mouseButton(int _button, int _action, int _mods) override
		{
			switch (_button)
			{
				case GLFW_MOUSE_BUTTON_LEFT:trans.mouse.refreshButton(0, _action); break;
				case GLFW_MOUSE_BUTTON_MIDDLE:trans.mouse.refreshButton(1, _action); break;
				case GLFW_MOUSE_BUTTON_RIGHT:trans.mouse.refreshButton(2, _action); break;
			}
		}
		virtual void mousePos(double _x, double _y) override
		{
			trans.mouse.refreshPos(_x, _y);
		}
		virtual void mouseScroll(double _x, double _y)override
		{
			if (_y != 0.0)
				trans.scroll.refresh(_y);
		}
		virtual void key(GLFWwindow * _window, int _key, int _scancode, int _action, int _mods) override
		{
			switch (_key)
			{
				case GLFW_KEY_ESCAPE:
					if (_action == GLFW_PRESS)
						glfwSetWindowShouldClose(_window, true);
					break;
				case GLFW_KEY_A:trans.key.refresh(0, _action); break;
				case GLFW_KEY_D:trans.key.refresh(1, _action); break;
				case GLFW_KEY_W:trans.key.refresh(2, _action); break;
				case GLFW_KEY_S:trans.key.refresh(3, _action); break;
			}
		}
	};
}







int main()
{
	OpenGL::OpenGLInit init(4, 5);
	Window::Window::Data winParameters
	{
		"NBody",
		{
			{800,800},
			true,false
		}
	};
	Window::WindowManager wm(winParameters);
	OpenGL::NBody test(40);
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