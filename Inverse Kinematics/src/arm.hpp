#pragma once

#include <SDL2/SDL.h>
#include <vector>

#include "renderer.hpp"
#include "segment.hpp"

class Arm
{
public:
	Arm(int baseX, int baseY, int segCount, int segLength, int segWidth);
	~Arm();

	void Update();
	void Render();

	void SetTarget(int x, int y);

private:
	BaseSegment* m_BaseSeg;
	std::vector<Segment*> m_Segments;

	int m_BaseX;
	int m_BaseY;
	int m_SegCount;
	int m_SegLength;
	int m_SegWidth;
};