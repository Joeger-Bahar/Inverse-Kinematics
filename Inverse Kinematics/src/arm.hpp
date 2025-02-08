#pragma once

#include <SDL2/SDL.h>
#include <vector>

#include "renderer.hpp"

class Arm
{
public:
	Arm(int baseX, int baseY, int segCount, int segLength, int segWidth);
	~Arm();

	void Update();
	void Render();

	void SetTarget(int x, int y);

private:
	std::vector<SDL_Texture*> m_SegTextures;
	std::vector<SDL_Rect> m_SegRects;
	std::vector<float> m_SegAngles;

	int m_BaseX;
	int m_BaseY;
	int m_SegCount;
	int m_SegLength;
	int m_SegWidth;
};