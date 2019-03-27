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
			struct TrianglePre :Program
			{
				RayTracing::Model* model;
				TrianglePre(SourceManager* _sm, RayTracing::Model* _model)
					:
					Program(_sm, "TrianglePre"),
					model(_model)
				{
					init();
				}
				virtual void initBufferData()override
				{
				}
				virtual void run()override
				{
					glDispatchCompute((model->geometryNum.data.num.triangleNum + 1023) / 1024, 1, 1);
				}
			};
			struct CirclePre :Program
			{
				RayTracing::Model* model;
				CirclePre(SourceManager* _sm, RayTracing::Model* _model)
					:
					Program(_sm, "CirclePre"),
					model(_model)
				{
					init();
				}
				virtual void initBufferData()override
				{
				}
				virtual void run()override
				{
					glDispatchCompute((model->geometryNum.data.num.circleNum + 1023) / 1024, 1, 1);
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

			TrianglePre trianglePre;
			CirclePre circlePre;
			Tracing tracing;
			RayTracer(SourceManager* _sm, RayTracing::FrameScale* _frameScale, RayTracing::Model* _model)
				:
				trianglePre(_sm, _model),
				circlePre(_sm, _model),
				tracing(_sm, _frameScale)
			{
			}
			virtual void initBufferData()
			{
			}
			virtual void run()
			{
				if (!trianglePre.model->triangles.GPUUpToDate)
				{
					trianglePre.use();
					trianglePre.run();
					trianglePre.model->triangles.GPUUpToDate = true;
				}
				if (!circlePre.model->circles.GPUUpToDate)
				{
					circlePre.use();
					circlePre.run();
					circlePre.model->circles.GPUUpToDate = true;
				}
				tracing.use();
				tracing.run();
			}
		};
		struct Movement
		{
			RayTracing::Model* model;

			void sphereRun()
			{

			}
			void run()
			{

			}
		};

		SourceManager sm;
		RayTracing::FrameScale frameScale;
		RayTracing::Transform transform;
		RayTracing::Model model;
		Buffer frameSizeBuffer;
		Buffer transBuffer;
		BufferConfig frameSizeUniform;
		BufferConfig transUniform;
		GLuint texture;
		Renderer renderer;
		RayTracer rayTracer;

		RayTrace(Math::vec2<unsigned int>const& _scale)
			:
			sm(),
			frameScale(_scale),
			transform({ {40.0,_scale.data[1]},{0.02,0.9,0.01},{0.1},{0,0,10},500.0 }),
			model({ {ShaderStorageBuffer,0}, {1,2}, {3}, {4},{5},{6}, {3} }),
			frameSizeBuffer(&frameScale),
			transBuffer(&transform.bufferData),
			frameSizeUniform(&frameSizeBuffer, UniformBuffer, 0),
			transUniform(&transBuffer, UniformBuffer, 1),
			renderer(&sm),
			rayTracer(&sm, &frameScale, &model)
		{
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, _scale.data[0], _scale.data[1]);
			glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glBindImageTexture(2, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);


			/*model.planes.data.planes.pushBack
			(
				{
					{0,0,1,0},
					{{0,0,0},{0,0,0},{0.1,1,1},1}
				}
			);*/
			/*		model.triangles.trianglesOrigin.trianglesOrigin.pushBack
					(
						{
							{{0,0,5},{1,0,5},{1,1,5}},
							{{0.4,0.4,0.4},{0,0,0},{0.7,0,0},1}
						}
					);

<<<<<<< HEAD
			RayTracing::Model::Color borderColor
			{ {0.1,0.7,0.5},{0,0,0},{0,0,0},1 };
			model.triangles.trianglesOrigin.trianglesOrigin +=
			{
				{
					{ {2, 2, 0}, { 2,-2, 0 }, { 2,-2, 4 }},
						borderColor
				},
				{
					{{2,2,0},{2,-2,4},{2,2,4}},
					borderColor
				},
				{
					{{-2,2,0},{-2,-2,0},{-2,-2,4}},
					borderColor
				},
				{
					{{-2,2,0},{-2,-2,4},{-2,2,4}},
					borderColor
				},
				{
					{ {2, 2, 0}, { -2,2, 0 }, { -2,2, 4 }},
						borderColor
				},
				{
					{{2,2,0},{-2,2,4},{2,2,4}},
					borderColor
				},
				{
					{{2,-2,0},{-2,-2,0},{-2,-2,4}},
					borderColor
				},
				{
					{{2,-2,0},{-2,-2,4},{2,-2,4}},
					borderColor
				}
			};
=======

					model.triangles.trianglesOrigin.trianglesOrigin.pushBack
					(
						{
							{{1,1,1},{1,-1,1},{1,-1,3}},
							{{0.8,0.8,0.8},{0,0,0},{0,0,0},1}
						}
					);
					model.triangles.trianglesOrigin.trianglesOrigin.pushBack
					(
						{
							{{1,1,1},{1,-1,3},{1,1,3}},
							{{0.8,0.8,0.8},{0,0,0},{0,0,0},1}
						}
					);
					model.triangles.trianglesOrigin.trianglesOrigin.pushBack
					(
						{
							{{-1,1,1},{-1,-1,1},{-1,-1,3}},
							{{0.8,0.8,0.8},{0,0,0},{0,0,0},1}
						}
					);
					model.triangles.trianglesOrigin.trianglesOrigin.pushBack
					(
						{
							{{-1,1,1},{-1,-1,3},{-1,1,3}},
							{{0.8,0.8,0.8},{0,0,0},{0,0,0},1}
						}
					);*/

			model.spheres.data.spheres.pushBack
			(
				{
					{0,0,2.1,4},
					{0},
					{0},
					{{0,0.2,0.2},{0,0.5,0.5},{0,1,1},{0,0,0},1.1}
				}
			);
			model.circles.data.circles +=
			{
				{
					{ 0, 0, 1, 0 },
					{ 0,0,0 },
						250,
					{ 1,1,0 },
					{ 0,0,0 },
					{ {0.1,0.1,0.1},{0,0,0},{0.6,0.6,0.6},{0.4,0.4,0.4},1 }
				}
			};
			model.addCylinder
			(
				{
					{0, 5, 1.2},
					1,
					{ 0,0,1 },
					5,
					{ 0 },
					{ 0 },
					{{0.1,0,0.1},{0.5,0,0.5},{1,0,1},{0,0,0},1.1 }
				}
			);
			model.addCylinder
			(
				{
					{3, 3, 0.01},
					1,
					{ 0,0,1 },
					0.5,
					{ 0 },
					{ 0 },
					{{0.1,0.1,0.1},{0.6,0.6,0},{1,1,0},{0,0,0},1.6 }
				}
			);
			model.pointLights.data.pointLights +=
			{
				/*{
					{200, 200, 200},
					{ 0,0,100 }
				},*/
				{
					{3,3,3},
					{ 0,0,7 }
				}
			};

			model.planes.numChanged = true;
			model.triangles.numChanged = true;
			model.spheres.numChanged = true;
			model.circles.numChanged = true;
			model.cylinders.numChanged = true;
			model.pointLights.numChanged = true;
		}
		virtual void init(FrameScale const& _size) override
		{
			glViewport(0, 0, _size.w, _size.h);
			transform.init(_size);
			renderer.viewArray.dataInit();
			frameSizeUniform.dataInit();
			transUniform.dataInit();
			model.dataInit();
			//GLint s(0);
			//glGetActiveUniformBlockiv(rayTracer.tracing.program, 1, GL_UNIFORM_BLOCK_DATA_SIZE, &s);
			//::printf("Uniform block size: %d\n", s);
		}
		virtual void run() override
		{
			transform.operate();
			if (transform.updated)
			{
				transUniform.refreshData();
				transform.updated = false;
			}
			rayTracer.run();
			//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			renderer.use();
			renderer.run();
		}
		virtual void frameSize(int _w, int _h) override
		{

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
			case GLFW_MOUSE_BUTTON_LEFT:transform.mouse.refreshButton(0, _action); break;
			case GLFW_MOUSE_BUTTON_MIDDLE:transform.mouse.refreshButton(1, _action); break;
			case GLFW_MOUSE_BUTTON_RIGHT:transform.mouse.refreshButton(2, _action); break;
			}
		}
		virtual void mousePos(double _x, double _y) override
		{
			transform.mouse.refreshPos(_x, _y);
		}
		virtual void mouseScroll(double _x, double _y) override
		{
			if (_y != 0.0)
				transform.scroll.refresh(_y);
		}
		virtual void key(GLFWwindow * _window, int _key, int _scancode, int _action, int _mods) override
		{
			switch (_key)
			{
			case GLFW_KEY_ESCAPE:
				if (_action == GLFW_PRESS)
					glfwSetWindowShouldClose(_window, true);
				break;
			case GLFW_KEY_A:transform.key.refresh(0, _action); break;
			case GLFW_KEY_D:transform.key.refresh(1, _action); break;
			case GLFW_KEY_W:transform.key.refresh(2, _action); break;
			case GLFW_KEY_S:transform.key.refresh(3, _action); break;
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
			{1024,1024},
			false,false,
		}
	};
	Window::WindowManager wm(winPara);
	OpenGL::RayTrace test({ 1024,1024 });
	wm.init(0, &test);
	glfwSwapInterval(0);
	FPS fps;
	fps.refresh();
	//int temp(0);
	//glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &temp);
	//::printf("%d\n", temp);
	while (!wm.close())
	{
		wm.pullEvents();
		wm.render();
		wm.swapBuffers();
		fps.refresh();
		fps.printFPS(1);
	}
	return 0;
}
