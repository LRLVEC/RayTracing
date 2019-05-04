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
		STL box1;
		STL box2;
		STL box3;
		STL boxt1;
		STL boxt2;
		STL mirror;
		STL three1;
		STL three2;
		STL three3;
		STL threet1;
		STL threet2;
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
			transform({ {60.0},{0.09,0.9,0.01},{0.15},{0,0,30},700.0 }),
			model({ {ShaderStorageBuffer,0},{1,2},{3},{4},{5},{6},{7},{3},{9} }),
			frameSizeBuffer(&frameScale),
			transBuffer(&transform.bufferData),
			decayOriginBuffer(&decayOriginData),
			frameSizeUniform(&frameSizeBuffer, UniformBuffer, 0),
			transUniform(&transBuffer, UniformBuffer, 1),
			decayOriginStorage(&decayOriginBuffer, ShaderStorageBuffer, 8),
			testBMP("resources\\Haja1.bmp"),
			cubeData("resources\\room\\"),
			box1(sm.folder.find("resources/box/box1.stl").readSTL()),
			box2(sm.folder.find("resources/box/box2.stl").readSTL()),
			box3(sm.folder.find("resources/box/box3.stl").readSTL()),
			boxt1(sm.folder.find("resources/box/boxtransparent1.stl").readSTL()),
			boxt2(sm.folder.find("resources/box/boxtransparent2.stl").readSTL()),
			mirror(sm.folder.find("resources/mirror.stl").readSTL()),
			three1(sm.folder.find("resources/three/three1.stl").readSTL()),
			three2(sm.folder.find("resources/three/three2.stl").readSTL()),
			three3(sm.folder.find("resources/three/three3.stl").readSTL()),
			threet1(sm.folder.find("resources/three/three transparent1.stl").readSTL()),
			threet2(sm.folder.find("resources/three/three transparent2.stl").readSTL()),
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
			model.circles.data.circles +=
			{
				{
					{0, 0, 1, 0},
					{ 0,0,0 },
					5000,
					{ 1,0,0 },
					{
						0,-1,
						0,-1,
						1,-2,
						0.1,-1,
						0,
						1
					}
				}
			};
			model.spheres.data.spheres +=
			{
				{
					{0, 0, 10, 16},
					{ 0,0,1,0 },
					{ 1,0,0,0 },
					{
						1,-1,
						1,-1,
						0,-1,
						0,-1,
						{-0.15, -0.15, 0},
						1.5
					}
				},
				{
					{0, 0, 10, 16*0.95*0.95},
					{ 0,0,1,0 },
					{ 1,0,0,0 },
					{
						1,-1,
						1,-1,
						0,-1,
						0,-1,
						{0.15, 0.15, 0},
						1/1.5
					}
				},
				{
					{10, 0, 10, 16},
					{ 0,0,1,0 },
					{ 1,0,0,0 },
					{
						0.7,-1,
						0,-1,
						0,-1,
						0,-1,
						0,
						1
					}
				},
				{
					{20, 0, 10, 16},
					{ 0,0,1,0 },
					{ 1,0,0,0 },
					{
						0,-1,
						0,-1,
						1,-1,
						0,-1,
						0,
						1
					}
				},
				{
					{30, 0, 10, 16},
					{ 0,0,1,0 },
					{ 1,0,0,0 },
					{
						1,-1,
						1,-1,
						0,-1,
						0,-1,
						{-0.15, -0.15, 0},
						1.5
					}
				}
			};
			model.pointLights.data.pointLights +=
			{
				{
					{4000, 4000, 4000},
					{ 0,0,300 }
				},
				{
					{10, 10, 10},
					{ -10,20,20 }
				},
				{
					{10, 10, 10},
					{ 30,20,20 }
				},
			};
			model.addSTL
			(
				box1,
				{
					1,-1,
					1,-1,
					0,-1,
					0,-1,
					{0, 0, -0.03},
					1.5
				},
				box1.triangles.length
			);
			model.addSTL
			(
				box2,
				{
					0.7,-1,
					0,-1,
					0,-1,
					0,-1,
					0,
					1
				},
				box2.triangles.length
			);
			model.addSTL
			(
				box3,
				{
					0,-1,
					0,-1,
					1,-1,
					0,-1,
					0,
					1
				},
				box3.triangles.length
			);
			model.addSTL
			(
				boxt1,
				{
					1,-1,
					1,-1,
					0,-1,
					0,-1,
					{0, 0, -0.03},
					1.5
				},
				boxt1.triangles.length
			);
			model.addSTL
			(
				boxt2,
				{
					1,-1,
					1,-1,
					0,-1,
					0,-1,
					{0, 0, 0.03},
					1.0/1.5
				},
				boxt2.triangles.length
			);
			model.addSTL
			(
				mirror,
				{
					0.7,-1,
					0,-1,
					0,-1,
					0,-1,
					0,
					1
				},
				mirror.triangles.length
			);
			model.addCylinder
			(
				{
					{-15,35,1},
					1,
					{0,0,1},
					2,
					{1,0,0},
					{
						1,-1,
						1,-1,
						0,-1,
						0,-1,
						{0, 0, -0.1},
						1.5
					}
				}
			);
			model.addCylinder
			(
				{
					{-15,37.5,1},
					1,
					{0,0,1},
					2,
					{1,0,0},
					{
						0.7,-1,
						0,-1,
						0,-1,
						0,-1,
						0,
						1
					}
				}
			);
			model.addCylinder
			(
				{
					{-15,32.5,1},
					1,
					{0,0,1},
					2,
					{1,0,0},
					{
						0,-1,
						0,-1,
						1,-1,
						0,-1,
						0,
						1
					}
				}
			);
			model.addCylinder
			(
				{
					{-15,30,1},
					1,
					{0,0,1},
					2,
					{1,0,0},
					{
						1,-1,
						1,-1,
						0,-1,
						0,-1,
						{0, 0, -0.01},
						1.5
					}
				}
			);
			model.addCylinder
			(
				{
					{-15,30,1.1},
					1*0.95*0.95,
					{0,0,1},
					1.9,
					{1,0,0},
					{
						1,-1,
						1,-1,
						0,-1,
						0,-1,
						{0, 0, 0.01},
						1/1.5
					}
				}
			);
			model.addCone
			(
				{
					{-7,35,7},
					0.8,
					{0,0,-1},
					4,
					{1,0,0},
					{
						1,-1,
						1,-1,
						0,-1,
						0,-1,
						{-0.2, 0, -0.2},
						1.5
					}
				}
			);
			model.addCone
			(
				{
					{-7,37,7},
					0.8,
					{0,0,-1},
					4,
					{1,0,0},
					{
						0.7,-1,
						0,-1,
						0,-1,
						0,-1,
						0,
						1
					}
				}
			);
			model.addCone
			(
				{
					{-7,33,7},
					0.8,
					{0,0,-1},
					4,
					{1,0,0},
					{
						0,-1,
						0,-1,
						1,-1,
						0,-1,
						0,
						1
					}
				}
			);
			model.addCone
			(
				{
					{-7,31,7},
					0.8,
					{0,0,-1},
					4,
					{1,0,0},
					{
						1,-1,
						1,-1,
						0,-1,
						0,-1,
						{-0.2, 0, -0.2},
						1.5
					}
				}
			);
			model.addCone
			(
				{
					{-7,31,7-0.1},
					0.8,
					{0,0,-1},
					4-0.5,
					{1,0,0},
					{
						1,-1,
						1,-1,
						0,-1,
						0,-1,
						{0.2, 0, 0.2},
						1/1.5
					}
				}
			);

			model.addSTL
			(
				three1,
				{
					1,-1,
					1,-1,
					0,-1,
					0,-1,
					{0, 0, -0.03},
					1.5
				},
				three1.triangles.length
			);
			model.addSTL
			(
				three2,
				{
					0.7,-1,
					0,-1,
					0,-1,
					0,-1,
					0,
					1
				},
				three2.triangles.length
			);
			model.addSTL
			(
				three3,
				{
					0,-1,
					0,-1,
					1,-1,
					0,-1,
					0,
					1
				},
				three3.triangles.length
			);
			model.addSTL
			(
				threet1,
				{
					1,-1,
					1,-1,
					0,-1,
					0,-1,
					{0, 0, -0.1},
					1.3
				},
				threet1.triangles.length
			);
			model.addSTL
			(
				threet2,
				{
					1,-1,
					1,-1,
					0,-1,
					0,-1,
					{0, 0, 0.1},
					1.0/1.3
				},
				threet2.triangles.length
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
		"BVH",
		{
			{720,480},
			true, false,
		}
	};
	Window::WindowManager wm(winPara);
	OpenGL::RayTrace test;
	wm.init(0, &test);
	glfwSwapInterval(0);
	FPS fps;
	fps.refresh();
	::printf("FPS:\n");
	int n(0);
	while (!wm.close())
	{
		wm.pullEvents();
		wm.render();
		glFinish();
		wm.swapBuffers();
		fps.refresh();
		if (++n == 10)
		{
			::printf("\r%.2lf    ", fps.fps);
			n = 0;
		}
	}
	return 0;
}
