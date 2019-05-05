#include <cstdio>
#include <GL/_OpenGL.h>
#include <GL/_Window.h>
#include <_Math.h>
#include <_Time.h>
#include <RayTracing/_RayTracing.h>
#include <GL/_Texture.h>
#include <_STL.h>
#include <_BMP.h>
#include <random>

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
			virtual void initBufferData()override
			{
			}
			virtual void run()override
			{
				glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
				glClear(GL_COLOR_BUFFER_BIT);
				glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			}
		};
		struct WaterSimulation :Computers
		{
			struct Water
			{
				struct WaterAttribs :Buffer::Data
				{
					struct Parameters
					{
						float k;
						float dt;
						unsigned int groupSizeX;
						unsigned int groupSizeY;
						unsigned int groupNumX;
						unsigned int groupNumY;
						float z0;
						float dx;
						float dzMax;
					};
					Parameters para;
					RayTracing::Model::Color color;

					WaterAttribs(Parameters const& _para)
						:
						Data(StaticDraw),
						para(_para)
					{
					}
					virtual void* pointer()override
					{
						return &para;
					}
					virtual unsigned int size()override
					{
						return 16;
					}
				};
				struct WaterData :Buffer::Data
				{
					struct WaterPoint
					{
						float z;
						float v;
						float a;
						float blank;
						Math::vec4<float> n;
					};
					Vector<WaterPoint>waterPoints;

					WaterData(WaterAttribs* attribs)
						:
						Data(DynamicDraw)
					{
						std::mt19937 mt;
						std::uniform_real_distribution<float>
							randReal(-attribs->para.dzMax / 2, attribs->para.dzMax / 2);
						float cX((attribs->para.groupNumX * attribs->para.groupSizeX - 1) / 2);
						float cY((attribs->para.groupNumY * attribs->para.groupSizeY - 1) / 2);
						for (int c0(0); c0 < attribs->para.groupNumY * attribs->para.groupSizeY; ++c0)
							for (int c1(0); c1 < attribs->para.groupNumX * attribs->para.groupSizeX; ++c1)
								waterPoints.pushBack
								(
									{
										attribs->para.z0 + attribs->para.dzMax * 0.5f *// (1 - float(pow(c1 - cX,2) + pow(c0 - cY,2)) / (cX * cX)),
										sinf(2 * Math::Pi * c1 / cX) * sinf(2 * Math::Pi * c0 / cY),
									//cosf(Math::Pi + 4 * (pow(c1 - cX,2) + pow(c0 - cY,2)) / ((cX * cX + cY * cY))),
									0,
									0,
									0,
									0
									}
						);
					}
					virtual void* pointer()override
					{
						return waterPoints.data;
					}
					virtual unsigned int size()override
					{
						return waterPoints.length * sizeof(WaterPoint);
					}
				};
				struct Info
				{
					int attribIndex;
					int dataIndex;
				};
				//make sure that the water triangles are always at the head of the 
				//triangles...
				WaterAttribs attribs;
				Buffer attribsBuffer;
				BufferConfig attribsConfig;
				WaterData data;
				Buffer dataBuffer;
				BufferConfig dataConfig;
				Water(Info const& _info, WaterAttribs::Parameters const& _para)
					:
					attribs(_para),
					attribsBuffer(&attribs),
					attribsConfig(&attribsBuffer, UniformBuffer, _info.attribIndex),
					data(&attribs),
					dataBuffer(&data),
					dataConfig(&dataBuffer, ShaderStorageBuffer, _info.dataIndex)
				{
				}
				void dataInit()
				{
					attribsConfig.dataInit();
					dataConfig.dataInit();
				}
			};
			struct AccelerationCalc :Program
			{
				Water::WaterAttribs* attribs;
				AccelerationCalc(SourceManager* _sm, Water::WaterAttribs* _attribs)
					:
					Program(_sm, "WaterAcceleration"),
					attribs(_attribs)
				{
					init();
				}
				virtual void initBufferData()override
				{
				}
				virtual void run()override
				{
					glDispatchCompute(attribs->para.groupNumX, attribs->para.groupNumY, 1);
				}
			};
			struct PositionCalc :Program
			{
				Water::WaterAttribs* attribs;
				PositionCalc(SourceManager* _sm, Water::WaterAttribs* _attribs)
					:
					Program(_sm, "WaterPosition"),
					attribs(_attribs)
				{
					init();
				}
				virtual void initBufferData()override
				{
				}
				virtual void run()override
				{
					glDispatchCompute(attribs->para.groupNumX, attribs->para.groupNumY, 1);
				}
			};
			struct PositionSync :Program
			{
				Water::WaterAttribs* attribs;
				PositionSync(SourceManager* _sm, Water::WaterAttribs* _attribs)
					:
					Program(_sm, "WaterPositionSync"),
					attribs(_attribs)
				{
					init();
				}
				virtual void initBufferData()override
				{
				}
				virtual void run()override
				{
					glDispatchCompute(attribs->para.groupNumX, attribs->para.groupNumY, 1);
				}
			};
			struct NormanCalc :Program
			{
				Water::WaterAttribs* attribs;
				NormanCalc(SourceManager* _sm, Water::WaterAttribs* _attribs)
					:
					Program(_sm, "WaterNormal"),
					attribs(_attribs)
				{
					init();
				}
				virtual void initBufferData()override
				{
				}
				virtual void run()override
				{
					glDispatchCompute(attribs->para.groupNumX, attribs->para.groupNumY, 1);
				}
			};
			Water water;
			AccelerationCalc accelerationCalc;
			PositionCalc positionCalc;
			PositionSync positionSync;
			NormanCalc normalCalc;
			unsigned int n;
			bool normalUpToDate;

			WaterSimulation(SourceManager* _sm, Water::Info const& _info, Water::WaterAttribs::Parameters const& _para, unsigned int _n)
				:
				water(_info, _para),
				accelerationCalc(_sm, &water.attribs),
				positionCalc(_sm, &water.attribs),
				positionSync(_sm, &water.attribs),
				normalCalc(_sm, &water.attribs),
				n(_n),
				normalUpToDate(false)
			{
			}
			virtual void initBufferData()override
			{
			}
			virtual void run()override
			{
				for (int c0(0); c0 < n; ++c0)
				{
					accelerationCalc.use();
					accelerationCalc.run();
					positionCalc.use();
					positionCalc.run();
				}
				positionSync.use();
				positionSync.run();
				normalUpToDate = false;
			}
			void dataInit()
			{
				water.dataInit();
			}
			void initModel(RayTracing::Model& _model, float x0, float y0, RayTracing::Model::Color color)
			{
				Vector<RayTracing::Model::Triangles::TriangleOriginData::TriangleOrigin>temp;
				int nx(water.attribs.para.groupNumX * water.attribs.para.groupSizeX);
				for (int c0(0); c0 < water.attribs.para.groupNumY * water.attribs.para.groupSizeY - 1; ++c0)
				{
					for (int c1(0); c1 < nx - 1; ++c1)
						temp.pushBack
						({
							{
								{
									x0 + water.attribs.para.dx * c1,
									y0 + water.attribs.para.dx * c0,
									water.attribs.para.z0 - water.attribs.para.dzMax
								},
								{
									x0 + water.attribs.para.dx * (c1 + 1),
									y0 + water.attribs.para.dx * c0,
									water.attribs.para.z0 - water.attribs.para.dzMax
								},
								{
									x0 + water.attribs.para.dx * c1,
									y0 + water.attribs.para.dx * (c0 + 1),
									water.attribs.para.z0 + water.attribs.para.dzMax
								}
							},
							0,0,0,
							{c1 + c0 * nx,c1 + 1 + c0 * nx,c1 + (c0 + 1) * nx},
							color
							});
					for (int c1(0); c1 < water.attribs.para.groupNumX * water.attribs.para.groupSizeX - 1; ++c1)
						temp.pushBack
						({
							{
								{
									x0 + water.attribs.para.dx * (c1 + 1),
									y0 + water.attribs.para.dx * (c0 + 1),
									water.attribs.para.z0 - water.attribs.para.dzMax
								},
								{
									x0 + water.attribs.para.dx * c1,
									y0 + water.attribs.para.dx * (c0 + 1),
									water.attribs.para.z0 - water.attribs.para.dzMax
								},
								{
									x0 + water.attribs.para.dx * (c1 + 1),
									y0 + water.attribs.para.dx * c0 ,
									water.attribs.para.z0 + water.attribs.para.dzMax
								}
							},
							0,0,0,
							{c1 + 1 + (c0 + 1) * nx,c1 + (c0 + 1) * nx,c1 + 1 + c0 * nx},
							color
							});
				}
				if (_model.triangles.trianglesOrigin.trianglesOrigin.length)
				{
					temp += _model.triangles.trianglesOrigin.trianglesOrigin;
					_model.triangles.trianglesOrigin.trianglesOrigin = temp;
				}
				else
					_model.triangles.trianglesOrigin.trianglesOrigin = temp;
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
			virtual void initBufferData()override
			{
			}
			virtual void run()override
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

		SourceManager sm;
		bool sizeChanged;
		bool paused;
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
		BMPData poolBMP;
		BMPData wallBMP;
		BMPData ceilingBMP;
		BMPData floorBMP;
		BMPCubeData cubeData;
		STL threePyramid;
		Texture textures;
		TextureCube cube;
		TextureConfig<TextureStorage3D>textureConfig;
		TracerInit tracerInit;
		Renderer renderer;
		WaterSimulation waterSim;

		RayTrace()
			:
			sm(),
			sizeChanged(true),
			paused(true),
			frameScale(),
			transform({ {60.0},{0.002,0.9,0.001},{0.01},{0,0,1},700.0 }),
			model({ {ShaderStorageBuffer,0},{1,2},{3},{4},{5},{6},{7},{3},{9} }),
			frameSizeBuffer(&frameScale),
			transBuffer(&transform.bufferData),
			decayOriginBuffer(&decayOriginData),
			frameSizeUniform(&frameSizeBuffer, UniformBuffer, 0),
			transUniform(&transBuffer, UniformBuffer, 1),
			decayOriginStorage(&decayOriginBuffer, ShaderStorageBuffer, 8),
			poolBMP("resources/pool.bmp"),
			wallBMP("resources/brick.bmp"),
			ceilingBMP("resources/ceiling.bmp"),
			floorBMP("resources/floor.bmp"),
			cubeData("resources/room/"),
			threePyramid(sm.folder.find("resources/pool.stl").readSTL()),
			textures(&poolBMP, 1),
			cube(&cubeData, 2, RGBA32f, 1, cubeData.bmp[0].header.width, cubeData.bmp[0].header.height),
			textureConfig(&textures, Texture2DArray, RGBA32f, 1, poolBMP.bmp.header.width, poolBMP.bmp.header.height, 4),
			tracerInit(&sm, &frameScale, &model, &transform),
			renderer(&sm),
			waterSim(&sm, { 4,10 }, { 0.0005,0.05,8,8,6,6,0.4,(1.0 - 0.01) / (8 * 6 - 1),0.15 }, 200)
		{
			textureConfig.dataRefresh(0, TextureInputBGRInt, TextureInputUByte, 0, 0, 0, poolBMP.bmp.header.width, poolBMP.bmp.header.height, 1);
			textures.data = &wallBMP;
			textureConfig.dataRefresh(0, TextureInputBGRInt, TextureInputUByte, 0, 0, 1, wallBMP.bmp.header.width, wallBMP.bmp.header.height, 1);
			textures.data = &ceilingBMP;
			textureConfig.dataRefresh(0, TextureInputBGRInt, TextureInputUByte, 0, 0, 2, ceilingBMP.bmp.header.width, ceilingBMP.bmp.header.height, 1);
			textures.data = &floorBMP;
			textureConfig.dataRefresh(0, TextureInputBGRInt, TextureInputUByte, 0, 0, 3, floorBMP.bmp.header.width, floorBMP.bmp.header.height, 1);

			cube.dataInit(0, TextureInputBGRInt, TextureInputUByte);
			renderer.use();
			textures.bindUnit();
			cube.bindUnit();
			glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
			model.pointLights.data.pointLights +=
			{
				{
					{0.25, 0.25, 0.25},
					{ 0,0,2.6 }
				}
			};
			waterSim.initModel(model, -0.5 + 0.005, -0.5 + 0.005,
				{
					1,-1,
					1,-1,
					0,-1,
					0,-1,
					-1,
					1.5
				});
			unsigned int k(model.triangles.trianglesOrigin.trianglesOrigin.length);
			float glow = 0.6;
			model.addSTL
			(
				threePyramid,
				{
					0,-1,
					0,-1,
					1,-1,
					0,-1,
					0,//{ -0.03,0,-0.03 },
					1
				},
				threePyramid.triangles.length
			);
			for (int c0(0); c0 < 4; ++c0)
			{
				model.triangles.trianglesOrigin.trianglesOrigin[k + 2 * c0].uv1 = { 5,0 };
				model.triangles.trianglesOrigin.trianglesOrigin[k + 2 * c0].uv2 = { 5,1.5 };
				model.triangles.trianglesOrigin.trianglesOrigin[k + 2 * c0].uv3 = { 0,0 };
				model.triangles.trianglesOrigin.trianglesOrigin[k + 2 * c0].color.d = 0;
				model.triangles.trianglesOrigin.trianglesOrigin[k + 2 * c0].color.g = glow;
				model.triangles.trianglesOrigin.trianglesOrigin[k + 2 * c0].color.texG = 0;
				model.triangles.trianglesOrigin.trianglesOrigin[k + 2 * c0].color.n = 1.33;

				model.triangles.trianglesOrigin.trianglesOrigin[k + 2 * c0 + 1].uv1 = { 0,1.5 };
				model.triangles.trianglesOrigin.trianglesOrigin[k + 2 * c0 + 1].uv2 = { 0,0 };
				model.triangles.trianglesOrigin.trianglesOrigin[k + 2 * c0 + 1].uv3 = { 5,1.5 };
				model.triangles.trianglesOrigin.trianglesOrigin[k + 2 * c0 + 1].color.d = 0;
				model.triangles.trianglesOrigin.trianglesOrigin[k + 2 * c0 + 1].color.g = glow;
				model.triangles.trianglesOrigin.trianglesOrigin[k + 2 * c0 + 1].color.texG = 0;
				model.triangles.trianglesOrigin.trianglesOrigin[k + 2 * c0 + 1].color.n = 1.33;
			}
			for (int c0(8); c0 < 16; ++c0)
			{
				model.triangles.trianglesOrigin.trianglesOrigin[k + c0].color.d = 1;
				model.triangles.trianglesOrigin.trianglesOrigin[k + c0].color.g = 0;
				model.triangles.trianglesOrigin.trianglesOrigin[k + c0].color.texD = 3;
				model.triangles.trianglesOrigin.trianglesOrigin[k + c0].color.n = 1;
			}

			{
				model.triangles.trianglesOrigin.trianglesOrigin[k + 16].uv1 = { 5,0 };
				model.triangles.trianglesOrigin.trianglesOrigin[k + 16].uv2 = { 0,0 };
				model.triangles.trianglesOrigin.trianglesOrigin[k + 16].uv3 = { 5,5 };
				model.triangles.trianglesOrigin.trianglesOrigin[k + 16].color.g = glow;
				model.triangles.trianglesOrigin.trianglesOrigin[k + 16].color.texG = 0;
				model.triangles.trianglesOrigin.trianglesOrigin[k + 16].color.decayFactor = -1;
				model.triangles.trianglesOrigin.trianglesOrigin[k + 16].color.n = 1.33;

				model.triangles.trianglesOrigin.trianglesOrigin[k + 17].uv1 = { 0,5 };
				model.triangles.trianglesOrigin.trianglesOrigin[k + 17].uv2 = { 5,5 };
				model.triangles.trianglesOrigin.trianglesOrigin[k + 17].uv3 = { 0,0 };
				model.triangles.trianglesOrigin.trianglesOrigin[k + 17].color.g = glow;
				model.triangles.trianglesOrigin.trianglesOrigin[k + 17].color.texG = 0;
				model.triangles.trianglesOrigin.trianglesOrigin[k + 17].color.decayFactor = -1;
				model.triangles.trianglesOrigin.trianglesOrigin[k + 17].color.n = 1.33;
			}
			for (int c0(9); c0 < 13; ++c0)
			{
				model.triangles.trianglesOrigin.trianglesOrigin[k + 2 * c0].uv1 = { 1,0 };
				model.triangles.trianglesOrigin.trianglesOrigin[k + 2 * c0].uv2 = { 1,1 };
				model.triangles.trianglesOrigin.trianglesOrigin[k + 2 * c0].uv3 = { 0,0 };
				model.triangles.trianglesOrigin.trianglesOrigin[k + 2 * c0].color.d = 1;
				model.triangles.trianglesOrigin.trianglesOrigin[k + 2 * c0].color.g = 0;
				model.triangles.trianglesOrigin.trianglesOrigin[k + 2 * c0].color.texD = 1;
				model.triangles.trianglesOrigin.trianglesOrigin[k + 2 * c0].color.n = 1;

				model.triangles.trianglesOrigin.trianglesOrigin[k + 2 * c0 + 1].uv1 = { 0,1 };
				model.triangles.trianglesOrigin.trianglesOrigin[k + 2 * c0 + 1].uv2 = { 0,0 };
				model.triangles.trianglesOrigin.trianglesOrigin[k + 2 * c0 + 1].uv3 = { 1,1 };
				model.triangles.trianglesOrigin.trianglesOrigin[k + 2 * c0 + 1].color.d = 1;
				model.triangles.trianglesOrigin.trianglesOrigin[k + 2 * c0 + 1].color.g = 0;
				model.triangles.trianglesOrigin.trianglesOrigin[k + 2 * c0 + 1].color.texD = 1;
				model.triangles.trianglesOrigin.trianglesOrigin[k + 2 * c0 + 1].color.n = 1;
			}
			{
				model.triangles.trianglesOrigin.trianglesOrigin[k + 26].uv1 = { 1,1 };
				model.triangles.trianglesOrigin.trianglesOrigin[k + 26].uv2 = { 0,0 };
				model.triangles.trianglesOrigin.trianglesOrigin[k + 26].uv3 = { 1,0 };
				model.triangles.trianglesOrigin.trianglesOrigin[k + 26].color.d = 1;
				model.triangles.trianglesOrigin.trianglesOrigin[k + 26].color.g = 0;
				model.triangles.trianglesOrigin.trianglesOrigin[k + 26].color.texD = 2;
				model.triangles.trianglesOrigin.trianglesOrigin[k + 26].color.n = 1;

				model.triangles.trianglesOrigin.trianglesOrigin[k + 27].uv1 = { 1,1 };
				model.triangles.trianglesOrigin.trianglesOrigin[k + 27].uv2 = { 0,1 };
				model.triangles.trianglesOrigin.trianglesOrigin[k + 27].uv3 = { 0,0 };
				model.triangles.trianglesOrigin.trianglesOrigin[k + 27].color.d = 1;
				model.triangles.trianglesOrigin.trianglesOrigin[k + 27].color.g = 0;
				model.triangles.trianglesOrigin.trianglesOrigin[k + 27].color.texD = 2;
				model.triangles.trianglesOrigin.trianglesOrigin[k + 27].color.n = 1;
			}
			/*stl.triangles.traverse
			([](STL::Triangle const& a)
				{
					a.print();
					return true;
				});
			model.spheres.data.spheres +=
			{
				{
					{0, 0, 0.4, 0.01},
					{ 0,0,1 },
					{ 1,0,0 },
					{
						0.8,-1,
						0,-1,
						0.1,-1,
						0,-1,
						0,
						1
					}
				}
			};*/
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
			waterSim.dataInit();
			waterSim.positionSync.use();
			waterSim.positionSync.run();
		}
		virtual void run() override
		{
			if (!paused)
			{
				waterSim.run();
				tracerInit.trianglePre.model->triangles.GPUUpToDate = false;
			}
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
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);//如果不加就会出现水面撕裂-.-
			tracerInit.run();
			if (!waterSim.normalUpToDate)
			{
				waterSim.normalCalc.use();
				waterSim.normalCalc.run();
				waterSim.normalUpToDate = true;
			}
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
				case GLFW_KEY_P: if (_action == GLFW_PRESS)paused = !paused;
			}
		}
	};
}

int main()
{
	OpenGL::OpenGLInit init(4, 5);
	Window::Window::Data winPara
	{
		"Pool",
		{
			{600,400},
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
		unsigned int t = (6 * fps.dt) / 1000000;
		if (t > 6000 || t == 0)t = 200;
		test.waterSim.n = t;
		if (++n == 10)
		{
			::printf("\r%.2lf    ", fps.fps);
			n = 0;
		}
		//fps.printFPS(1);
	}
	return 0;
}
