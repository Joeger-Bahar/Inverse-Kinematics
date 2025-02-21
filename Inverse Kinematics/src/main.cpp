#include <functional>

#include "renderer.hpp"
#include "arm.hpp"

int main()
{
	Renderer renderer("Inverse Kinematics", 1000, 700);
	// BaseX, BaseY, SegmentCount, SegmentLength, SegmentWidth
	Arm arm(500, 350, 5, 10, 100);
	// What the actual fuck
	renderer.OnMove(std::bind(&Arm::SetTarget, &arm, std::placeholders::_1, std::placeholders::_2));

	while (Renderer::running)
	{
		arm.Update();
		arm.Render();
		renderer.Update();
		renderer.Render();
	}
	return 0;
}