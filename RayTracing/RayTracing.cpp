#include <cstdio>
#include <GL/_OpenGL.h>
#include <GL/_Window.h>
#include <_Math.h>
#include <_Time.h>
#include <RayTracing/_RayTracing.h>
#include <GL/_Texture.h>
#include <_STL.h>
#include <_BMP.h>

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
				//glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
				//glClear(GL_COLOR_BUFFER_BIT);
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
			struct DecayOriginCalc :Computers
			{
				struct DecayOriginPre :Program
				{
					DecayOriginPre(SourceManager* _sm)
						:
						Program(_sm, "DecayOriginPre")
					{
						init();
					}
					virtual void initBufferData()override
					{
					}
					virtual void run()override
					{
						glDispatchCompute(8, 1, 1);
					}
				};
				struct DecayOrigin :Program
				{
					DecayOrigin(SourceManager* _sm)
						:
						Program(_sm, "DecayOrigin")
					{
						init();
					}
					virtual void initBufferData()override
					{
					}
					virtual void run()override
					{
						glDispatchCompute(1, 1, 1);
					}
				};

				DecayOriginPre decayOriginPre;
				DecayOrigin decayOrigin;

				DecayOriginCalc(SourceManager* _sm)
					:
					decayOriginPre(_sm),
					decayOrigin(_sm)
				{
				}
				virtual void initBufferData()override
				{
				}
				virtual void run()override
				{
					glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
					decayOriginPre.use();
					decayOriginPre.run();
					glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
					decayOrigin.use();
					decayOrigin.run();
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

			RayTracing::Transform* transform;
			TrianglePre trianglePre;
			CirclePre circlePre;
			DecayOriginCalc decayOriginCalc;
			Tracing tracing;
			RayTracer(SourceManager* _sm, RayTracing::FrameScale* _frameScale, RayTracing::Model* _model, RayTracing::Transform* _transform)
				:
				transform(_transform),
				trianglePre(_sm, _model),
				circlePre(_sm, _model),
				decayOriginCalc(_sm),
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
				}
				if (!circlePre.model->circles.GPUUpToDate)
				{
					circlePre.use();
					circlePre.run();
				}
				if (transform->moved || trianglePre.model->moved)
				{
					decayOriginCalc.run();
				}
				trianglePre.model->upToDate();
				glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
				tracing.use();
				tracing.run();
			}
		};
		struct Movement
		{
			RayTracing::Model* model;
			float t;

			Movement(RayTracing::Model* _model)
				:
				model(_model),
				t(0)
			{
			}
			void sphereRun()
			{
				model->spheres.data.spheres[0].sphere[2] += sin(t) * 0.2;
				model->spheres.upToDate = false;
			}
			void pointLightRun()
			{
				model->pointLights.data.pointLights[0].p[0] = 3 * sin(t);
				model->pointLights.data.pointLights[0].p[2] += 0.2 * sin(t);
				model->pointLights.upToDate = false;
			}
			void run()
			{
				t += 0.1;
				pointLightRun();
				sphereRun();
				model->dataInit();
			}
		};

		SourceManager sm;
		RayTracing::FrameScale frameScale;
		RayTracing::Transform transform;
		RayTracing::DecayOriginData decayOriginData;
		RayTracing::Model model;
		Buffer frameSizeBuffer;
		Buffer transBuffer;
		Buffer decayOriginBuffer;
		BufferConfig frameSizeUniform;
		BufferConfig transUniform;
		BufferConfig decayOriginStorage;
		BMPData testBMP;
		BMPCubeData cubeData;
		Texture image;
		Texture texture;
		TextureCube cube;
		TextureConfig<TextureStorage2D>imageConfig;
		TextureConfig<TextureStorage3D>textureConfig;
		Renderer renderer;
		RayTracer rayTracer;
		Movement movement;

		RayTrace(Math::vec2<unsigned int>const& _scale)
			:
			sm(),
			frameScale(_scale),
			transform({ {60.0,_scale.data[1]},{0.1,0.9,0.1},{0.5},{0,0,10},700.0 }),
			model({ {ShaderStorageBuffer,0}, {1,2}, {3}, {4},{5},{6},{7},{3} }),
			frameSizeBuffer(&frameScale),
			transBuffer(&transform.bufferData),
			decayOriginBuffer(&decayOriginData),
			frameSizeUniform(&frameSizeBuffer, UniformBuffer, 0),
			transUniform(&transBuffer, UniformBuffer, 1),
			decayOriginStorage(&decayOriginBuffer, ShaderStorageBuffer, 8),
			testBMP("resources\\Haja1.bmp"),
			cubeData("resources\\vendetta\\"),
			image(nullptr, 0),
			texture(&testBMP, 1),
			cube(&cubeData, 2, RGBA32f, 1, cubeData.bmp[0].header.width, cubeData.bmp[0].header.height),
			imageConfig(&image, Texture2D, RGBA32f, 1, _scale.data[0], _scale.data[1]),
			textureConfig(&texture, Texture2DArray, RGBA32f, 1, testBMP.bmp.header.width, testBMP.bmp.header.height, 1),
			//cubeConfig(&cube,TextureCubeMap,RGBA32f,1,testBMP.bmp.header.width,testBMP.bmp.header.height,)
			renderer(&sm),
			rayTracer(&sm, &frameScale, &model, &transform),
			movement(&model)
		{
			imageConfig.parameteri(TextureParameter::TextureMinFilter, TextureParameter::MinFilter_Linear);
			imageConfig.parameteri(TextureParameter::TextureMagFilter, TextureParameter::MagFilter_Linear);
			glBindImageTexture(2, image.texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

			//testBMP.bmp.printInfo();

			//GLenum bgra[4]{ GL_BLUE,GL_GREEN,GL_RED,GL_ALPHA };
			//glTextureParameteriv(texture.texture, GL_TEXTURE_SWIZZLE_RGBA, (GLint*)bgra);
			//glTextureParameteri(texture.texture, GL_TEXTURE_SWIZZLE_A, GL_ONE);
			textureConfig.dataRefresh(0, TextureInputBGRInt, TextureInputUByte, 0, 0, 0, testBMP.bmp.header.width, testBMP.bmp.header.height, 1);
			cube.dataInit(0, TextureInputBGRInt, TextureInputUByte);
			/*unsigned char* gg((unsigned char*)::malloc(3 * 256 * 256));
			for (int c0(0); c0 < 3 * 256 * 256; ++c0)
				gg[c0] = 128;
			glTextureSubImage3D(texture.texture, 0, 0, 0, 0, 256, 256, 1, TextureInputBGRInt, TextureInputUByte, gg);
			::free(gg);*/

			renderer.use();
			image.bindUnit();
			rayTracer.tracing.use();
			image.bindUnit();
			texture.bindUnit();
			cube.bindUnit();
			glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

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


			/*model.triangles.trianglesOrigin.trianglesOrigin +=
			{
				{
					{
						{20, -5, 20 },
						{ 30,-5,20 },
						{ 20,-5,10 }
					},
						{ 0,0 },
						{ 1,0 },
						{ 0,1 },
					{
						0, -1,
						0, -1,
						0.5, 0,
						0,-1,
						0,
						1
					}
				},
				{
					{
						{30, -5, 10},
						{ 20,-5,10},
						{ 30,-5,20 }
					},
					{ 1,1 },
					{ 0,1 },
					{ 1,0 },
					{
						0, -1,
						0, -1,
						0.5, 0,
						0,-1,
						0,
						1
					}
				},
			};*/
			model.spheres.data.spheres +=
			{
				{
					{-10, -30, 10, 64},
					{ 0,-1,0 },
					{ 1,0,0 },
					{
						1,-1,
						1,-1,
						0,-1,
						0,-1,
						{-1,-1,0},
						1.8
					}
				},
				{
					{-10, -30, 10, 60},
					{ 0,-1,0 },
					{ 1,0,0 },
					{
						{0,0,0},-1,
						{1,1,1},-1,
						0,-1,
						0,-1,
						{1,1,0},
						1 / 1.8
					}
				}
			};
			model.circles.data.circles +=
			{
				{
					{ 0, -1, 0, 0 },
					{ 0,0,0 },
						2500,
					{ 1,0,0 },
					{
						0,-1,
						0,-1,
						0.5,0,
						0,-1,
						0,
						1.5
					}
				}
			};
			model.addCylinder
			(
				{
					{5 , -20 , -10},
					80,
					{ 1,0,0 },
					10,
					{ 1,0,0 },
					{
						1,-1,
						1,-1,
						{0,0,0},-1,
						0,0,
						{-0.1,0,-0.1},
						1.6
					}
				}
			);
			model.addCylinder
			(
				{
					{5.2 , -20 , -10},
					76,
					{ 1,0,0 },
					9.6,
					{ 1,0,0 },
					{
						1,-1,
						1,-1,
						{0,0,0},-1,
						0,0,
						{0.1,0,0.1},
						1 / 1.6
					}
				}
			);

			model.addCone
			(
				{
					{10, -40, 10},0.75,
					{ 0,1,0 },100,
					{ 1,0,0 },
					{
						1,-1,
						1,-1,
						0,-1,
						0,-1,
						{0,-0.3,-0.3},
						1.6
					}
				}
			);
			model.addCone
			(
				{
					{10, -39.5, 10},0.75,
					{ 0,1,0 },85,
					{ 1,0,0 },
					{
						1,-1,
						1,-1,
						0,-1,
						0,-1,
						{0,0.3,0.3},
						1 / 1.6
					}
				}
			);
			model.pointLights.data.pointLights +=
			{
				{
					{600, 600, 600},
					{ 0,-100,0 }
				},
				{
					{100, 100, 100},
					{ -20,-40,20 }
				},
				{
					{100, 100, 100},
					{ 20,-40,-20 }
				}
			};

			model.planes.numChanged = true;
			model.triangles.numChanged = true;
			model.spheres.numChanged = true;
			model.circles.numChanged = true;
			model.cylinders.numChanged = true;
			model.cones.numChanged = true;
			model.pointLights.numChanged = true;
		}
		virtual void init(FrameScale const& _size) override
		{
			glViewport(0, 0, _size.w, _size.h);
			transform.init(_size);
			renderer.viewArray.dataInit();
			frameSizeUniform.dataInit();
			transUniform.dataInit();
			decayOriginStorage.dataInit();
			model.dataInit();
			//GLint s(0);
			//glGetActiveUniformBlockiv(rayTracer.tracing.program, 1, GL_UNIFORM_BLOCK_DATA_SIZE, &s);
			//::printf("Uniform block size: %d\n", s);
		}
		virtual void run() override
		{
			//movement.run();
			transform.operate();
			if (transform.updated)
			{
				transUniform.refreshData();
				transform.updated = false;
			}
			rayTracer.run();
			//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
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
		//fps.refresh();
		//fps.printFPS(1);
	}
	return 0;
}
