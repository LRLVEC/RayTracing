#include <cstdio>
#include <GL/_OpenGL.h>
#include <GL/_Window.h>
#include <_Math.h>
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
				Program(_sm, "Renderer"),
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
			struct Tracing:Program
			{
				Tracing(SourceManager* _sm)
				:
					Program(_sm,"Tracing")
				{

				}
				virtual void initBufferData()override
				{

				}
				virtual void run()override
				{

				}
			};

			virtual void initBufferData()
			{

			}
			virtual void run()
			{

			}
		};


		SourceManager sm;

		RayTracing::FrameSize frameSize;
		RayTracing::FrameData frameData;
		Buffer frameSizeBuffer;
		Buffer frameDataBuffer;


		Renderer renderer;
		//RayTracer rayTracer;



		virtual void init(FrameScale const&) override
		{

		}
		virtual void run() override;
		virtual void frameSize(int, int) override;
		virtual void framePos(int, int) override;
		virtual void frameFocus(int) override;
		virtual void mouseButton(int, int, int) override;
		virtual void mousePos(double, double) override;
		virtual void mouseScroll(double, double) override;
		virtual void key(GLFWwindow*, int, int, int, int) override;
	};
}



int main()
{
	::printf("Hello World!");
}
