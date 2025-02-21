#include <functional>

#include "renderer.hpp"
#include "arm.hpp"

int main()
{
	Renderer renderer("Inverse Kinematics", 1000, 700);
	// BaseX, BaseY, SegmentCount, SegmentLength, SegmentWidth
	Arm arm(500, 350, 5, 100, 10);

	while (Renderer::running)
	{
		arm.Update();
		arm.Render();
		renderer.Update();
		renderer.Render();
	}
	return 0;
}