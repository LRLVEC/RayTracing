#include <cstdio>
#include <GL/_OpenGL.h>
#include <GL/_Window.h>
#include <_Math.h>
#include <RayTracing/_RayTracing.h>

namespace OpenGL
{
	struct RayTracing :OpenGL
	{
		struct Renderer :Program
		{
			Renderer(SourceManager*_sm)
				:
				Program(_sm,"Renderer")
			{
			}
			virtual void initBufferData()
			{

			}
			virtual void run()override
			{

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
				virtual void initBufferData()
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
		Renderer renderer;
		RayTracer rayTracer;



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
