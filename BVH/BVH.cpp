#include <GL/_OpenGL.h>
#include <GL/_Window.h>
#include <RayTracing/_RayTracing.h>
#include <_Time.h>


namespace OpenGL
{
	struct BVHRayTracing :OpenGL
	{
		struct Renderer :Program
		{
			RayTracing::View view;
			Buffer viewBuffer;
			BufferConfig viewArray;
			VertexAttrib position;

			Renderer(SourceManager* _sm)
				:
				Program(_sm, "Renderer", { &position }),
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

		};
	};
}


int main()
{
	::printf("Hello World!");
}