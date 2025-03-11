#include <functional>

#include "renderer.hpp"
#include "arm.hpp"

int main()
{
	Renderer renderer("Inverse Kinematics", 1000, 700);
	// BaseX, BaseY, SegmentCount, SegmentLength, SegmentWidth
	Arm* arm = new Arm(500, 350, 7, 100, 10); // Pointer for reinit with different args

	while (Renderer::running)
	{
		arm->Update();
		arm->Render();
		if (int segCount = renderer.Update()) // Reinit arm with different num of segs
		{
			delete arm;
			arm = new Arm(500, 350, segCount, 700 / segCount, 10);
		}
		renderer.Render();
	}
	
	delete arm;

	return 0;
}