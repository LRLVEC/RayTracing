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
				glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
				glClear(GL_COLOR_BUFFER_BIT);
				glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			}
		};
		struct TracerInit :Computers
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


			RayTracing::Transform* transform;
			TrianglePre trianglePre;
			CirclePre circlePre;
			DecayOriginCalc decayOriginCalc;
			TracerInit(SourceManager* _sm, RayTracing::FrameScale* _frameScale, RayTracing::Model* _model, RayTracing::Transform* _transform)
				:
				transform(_transform),
				trianglePre(_sm, _model),
				circlePre(_sm, _model),
				decayOriginCalc(_sm)
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
				model->spheres.data.spheres[0].sphere[1] += sin(t);
				model->spheres.data.spheres[1].sphere[1] += sin(t);
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
				//pointLightRun();
				sphereRun();
				model->dataInit();
			}
		};

		SourceManager sm;
		bool sizeChanged;
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
		STL stl;
		Texture texture;
		TextureCube cube;
		TextureConfig<TextureStorage3D>textureConfig;
		TracerInit tracerInit;
		Renderer renderer;
		Movement movement;

		RayTrace()
			:
			sm(),
			sizeChanged(true),
			frameScale(),
<<<<<<< HEAD
			transform({ {60.0},{0.4,0.9,0.1},{2},{0,0,10},700.0 }),
=======
			transform({ {60.0},{0.1,0.9,0.1},{0.5},{0,0,10},700.0 }),
>>>>>>> b5e640255209d2a0eca8a5a9b56d4d53c5014d5d
			model({ {ShaderStorageBuffer,0},{1,2},{3},{4},{5},{6},{7},{3},{9} }),
			frameSizeBuffer(&frameScale),
			transBuffer(&transform.bufferData),
			decayOriginBuffer(&decayOriginData),
			frameSizeUniform(&frameSizeBuffer, UniformBuffer, 0),
			transUniform(&transBuffer, UniformBuffer, 1),
			decayOriginStorage(&decayOriginBuffer, ShaderStorageBuffer, 8),
			testBMP("resources\\Haja1.bmp"),
			cubeData("resources\\vendetta\\"),
<<<<<<< HEAD
			stl(sm.folder.find("resources/¿ËÀ³ÒòÆ¿_1.stl").readSTL()),
=======
			stl(sm.folder.find("resources/Stanford_bunny_simplified.stl").readSTL()),
>>>>>>> b5e640255209d2a0eca8a5a9b56d4d53c5014d5d
			texture(&testBMP, 1),
			cube(&cubeData, 2, RGBA32f, 1, cubeData.bmp[0].header.width, cubeData.bmp[0].header.height),
			textureConfig(&texture, Texture2DArray, RGBA32f, 1, testBMP.bmp.header.width, testBMP.bmp.header.height, 1),
			tracerInit(&sm, &frameScale, &model, &transform),
			renderer(&sm),
			movement(&model)
		{
			textureConfig.dataRefresh(0, TextureInputBGRInt, TextureInputUByte, 0, 0, 0, testBMP.bmp.header.width, testBMP.bmp.header.height, 1);
			cube.dataInit(0, TextureInputBGRInt, TextureInputUByte);

			renderer.use();
			texture.bindUnit();
			cube.bindUnit();
			glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

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
<<<<<<< HEAD
			};
			model.spheres.data.spheres +=
			{
				{
					{0, -45, 0, 800},
=======
			};*/
			/*model.spheres.data.spheres +=
			{
				{
					{-10, -30, 10, 64},
>>>>>>> b5e640255209d2a0eca8a5a9b56d4d53c5014d5d
					{ 0,-1,0 },
					{ 1,0,0 },
					{
						1,-1,
						1,-1,
						0,-1,
						0,-1,
<<<<<<< HEAD
						{-0.03,0,-0.03},
						1.33
					}
=======
						{-0.5,0,-0.5},
						1.33
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
						{0.5,0,0.5},
						1 / 1.33
					}
>>>>>>> b5e640255209d2a0eca8a5a9b56d4d53c5014d5d
				}
			};*/
			/*for (int c0(0); c0 < 3; ++c0)
				for (int c1(0); c1 < 3; ++c1)
					for (int c2(0); c2 < 3; ++c2)
						model.spheres.data.spheres.pushBack
						({
<<<<<<< HEAD
							{-20 - 5.0f * c0, -5.0f * c1, -5.0f * c2, 4},
							{ 0,-1,0 },
							{ 1,0,0 },
							{
								0,-1,
=======
							{-10 - 5.0f * c0, -30.0f - 5.0f * c1, 10 - 5.0f * c2, 4},
							{ 0,-1,0 },
							{ 1,0,0 },
							{
								1,-1,
>>>>>>> b5e640255209d2a0eca8a5a9b56d4d53c5014d5d
								1,-1,
								0,-1,
								0,-1,
								{-0.5,0,-0.5},
								1.33
							}
							});*/
			model.circles.data.circles +=
			{
				{
<<<<<<< HEAD
					{ 0, -1, 0, 0 },
					{ 0,0,0 },
						4900,
=======
					{ 0, 0, -1, 0 },
					{ 0,0,-10 },
						2500,
>>>>>>> b5e640255209d2a0eca8a5a9b56d4d53c5014d5d
					{ 1,0,0 },
					{
						0,-1,
						0,-1,
<<<<<<< HEAD
						0.5,-1,
=======
						0.5,0,
>>>>>>> b5e640255209d2a0eca8a5a9b56d4d53c5014d5d
						0,-1,
						0,
						1.5
					}
				}
			};
			//model.cylinders.data.cylinders +=
			//{
			//	{
			//		{5, -20, -10},
			//			80,
			//		{ 1,0,0 },
			//			10,
			//		{ 1,0,0 },
			//		{
			//			1,-1,
			//			1,-1,
			//			{0,0,0},-1,
			//			0,0,
			//			{-0.1,0,-0.1},
			//			1.33
			//		}
			//	}
			//};
			//model.addCylinder
			//(
			//	{
			//		{5 , -20 , -10},
			//		80,
			//		{ 1,0,0 },
			//		10,
			//		{ 1,0,0 },
			//		{
			//			1,-1,
			//			1,-1,
			//			{0,0,0},-1,
			//			0,0,
			//			{-0.1,0,-0.1},
			//			1.33
			//		}
			//	}
			//);
			//model.addCylinder
			//(
			//	{
			//		{5.2 , -20 , -10},
			//		76,
			//		{ 1,0,0 },
			//		9.6,
			//		{ 1,0,0 },
			//		{
			//			1,-1,
			//			1,-1,
			//			{0,0,0},-1,
			//			0,0,
			//			{0.1,0,0.1},
			//			1 / 1.33
			//		}
			//	}
			//);
<<<<<<< HEAD
=======

>>>>>>> b5e640255209d2a0eca8a5a9b56d4d53c5014d5d
			//model.addCone
			//(
			//	{
			//		{10, -40, 10},0.75,
			//		{ 0,1,0 },100,
			//		{ 1,0,0 },
			//		{
			//			1,-1,
			//			1,-1,
			//			0,-1,
			//			0,-1,
			//			{0,-0.3,-0.3},
			//			1.33
			//		}
			//	}
			//);
			//model.addCone
			//(
			//	{
			//		{10, -39, 10},0.75,
			//		{ 0,1,0 },75,
			//		{ 1,0,0 },
			//		{
			//			1,-1,
			//			1,-1,
			//			0,-1,
			//			0,-1,
			//			{0,0.3,0.3},
			//			1 / 1.33
			//		}
			//	}
			//);
			model.pointLights.data.pointLights +=
			{
				{
<<<<<<< HEAD
					{4000, 4000, 4000},
					{ 0,-300,0 }
				},
					/*	{
							{3000, 3000, 3000},
							{ 400, 0,0 }
						},
						{
							{3000, 3000, 3000},
							{ -400, 0,0 }
						},
						{
							{3000, 3000, 3000},
							{ 0,0,400 }
						},
						{
							{3000,3000, 3000},
							{ 0,0,-400 }
						},*/
=======
					{8000, 8000, 8000},
					{ 0,0,400 }
				},
					/*{
						{100, 100, 100},
						{ -20,-40,20 }
					},
					{
						{100, 100, 100},
						{ 20,-40,-20 }
					}*/
>>>>>>> b5e640255209d2a0eca8a5a9b56d4d53c5014d5d
			};
			model.addSTL
			(
				stl,
				{
					1,-1,
<<<<<<< HEAD
					0,-1,
					0,-1,
					0,-1,
					0,//{ -0.03,0,-0.03 },
					1
=======
					1,-1,
					0,-1,
					0,-1,
					{-0.5,0,-0.5},
					1.5
>>>>>>> b5e640255209d2a0eca8a5a9b56d4d53c5014d5d
				},
				stl.triangles.length
			);
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
			frameScale.scale = { unsigned int(_size.w),unsigned int(_size.h) };
			renderer.viewArray.dataInit();
			frameSizeUniform.dataInit();
			sizeChanged = false;
			transUniform.dataInit();
			decayOriginStorage.dataInit();
			model.dataInit();
		}
		virtual void run() override
		{
			//movement.run();
			if (sizeChanged)
			{
				glViewport(0, 0, frameScale.scale.data[0], frameScale.scale.data[1]);
				frameSizeUniform.refreshData();
				sizeChanged = false;
			}
			transform.operate();
			if (transform.updated)
			{
				transUniform.refreshData();
				transform.updated = false;
			}
			tracerInit.run();
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
			renderer.use();
			renderer.run();
		}
		virtual void frameSize(int _w, int _h) override
		{
			frameScale.scale = { unsigned int(_w),unsigned int(_h) };
			transform.persp.y = _h;
			transform.persp.updated = true;
			sizeChanged = true;
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
		virtual void key(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods) override
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
			{480,480},
			true, false,
		}
	};
	Window::WindowManager wm(winPara);
	OpenGL::RayTrace test;
	wm.init(0, &test);
	glfwSwapInterval(0);
	FPS fps;
	fps.refresh();
	while (!wm.close())
	{
		wm.pullEvents();
		wm.render();
		wm.swapBuffers();
<<<<<<< HEAD
		//fps.refresh();
		//fps.printFPS(1);
=======
		fps.refresh();
		fps.printFPS(1);
>>>>>>> b5e640255209d2a0eca8a5a9b56d4d53c5014d5d
	}
	return 0;
}
