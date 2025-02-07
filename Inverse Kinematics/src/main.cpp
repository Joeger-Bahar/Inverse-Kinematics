#include "renderer.hpp"


int main()
{
	Renderer renderer("Inverse Kinematics", 800, 600);

	while (Renderer::running)
	{
		renderer.Update();
		renderer.Render();
	}
	return 0;
}