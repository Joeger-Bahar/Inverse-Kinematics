#include "arm.hpp"

#include <algorithm>
#include <iostream>
#include <cmath>

static glm::vec2 Lerp(const glm::vec2& start, const glm::vec2& end, float t);

Arm::Arm(int baseX, int baseY, int segCount, int segLength, int segWidth)
	: m_BaseX(baseX), m_BaseY(baseY), m_SegCount(segCount), m_SegLength(segLength), m_SegWidth(segWidth)
{
	// Reduce some ambiguity
	const float defaultAngle = 0.;

	const size_t handIndex = Renderer::LoadTexture("assets/hand.png");
	const size_t armIndex = Renderer::LoadTexture("assets/arm.png");

	m_Segments.resize(m_SegCount);
	m_BaseSeg = new BaseSegment(segLength, 50, defaultAngle, 10, armIndex);
	m_Segments[0] = m_BaseSeg;

	for (int i = 1; i < m_SegCount; i++)
	{
		// Cycle color
		bool isEven = i % 2 == 0;
		SDL_Color color = { 255 - (i * 25), 0 + (i + 25), 255, 255};
		const size_t segIndex = i == m_SegCount - 1 ? handIndex : armIndex;
		const int width = (segIndex == handIndex ? 100 : 50); // Hand needs to be wider than arm

		m_Segments[i] = new Segment(segLength, width,
			defaultAngle, static_cast<uint8_t>(10 - (i / ((m_SegCount / 10) + 1))), segIndex, color);

		m_Segments[i]->AssignParent(m_Segments[i - 1]);
		m_Segments[i - 1]->AssignChild(m_Segments[i]);
	}
}

Arm::~Arm()
{
	for (Segment* seg : m_Segments)
	{
		delete seg;
	}
}

void Arm::Update()
{
	// Point to mouse
	m_Segments.back()->ReverseK(Renderer::mouseX, Renderer::mouseY);
	// Nail to point
	m_BaseSeg->ForwardK();
}

void Arm::Render()
{
	m_BaseSeg->Render();
}