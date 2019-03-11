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
			//Renderer(SourceManager*_sm)
			//	:
			//	Program(_sm,"Renderer",)
			//{
			//}
			virtual void setBufferData()override
			{
			}
			virtual void run()override
			{

			}
			virtual void resize(int, int)override
			{

			}
		};
		struct Preprocessor :Program
		{
			Preprocessor(SourceManager* _sm)
				:
				Program(_sm, "Preprocessor")
			{

			}
			virtual void setBufferData()override
			{
			}
			virtual void run()override
			{

			}
			virtual void resize(int, int)override
			{

			}
		};
		struct RayTracer :Program
		{
			RayTracer(SourceManager* _sm)
				:
				Program(_sm, "RayTracer")
			{

			}
			virtual void setBufferData()override
			{
			}
			virtual void run()override
			{

			}
			virtual void resize(int, int)override
			{

			}
		};
		

		SourceManager sm;

		Renderer renderer;
		Preprocessor preprocessor;
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
