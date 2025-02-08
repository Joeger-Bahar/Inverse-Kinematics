#include <functional>

#include "renderer.hpp"
#include "arm.hpp"

int main()
{
	Renderer renderer("Inverse Kinematics", 1000, 700);
	// BaseX, BaseY, SegmentCount, SegmentWidth, SegmentLength
	Arm arm(500, 700 - 100, 5, 20, 100);
	// What the actual fuck
	renderer.OnClick(std::bind(&Arm::SetTarget, &arm, std::placeholders::_1, std::placeholders::_2));

	while (Renderer::running)
	{
		arm.Update();
		arm.Render();
		renderer.Update();
		renderer.Render();

	}
	return 0;
}