#include "arm.hpp"

#include <algorithm>
#include <iostream>
#include <cmath>

Arm::Arm(int baseX, int baseY, int segCount, int segWidth, int segLength)
	: m_BaseX(baseX), m_BaseY(baseY), m_SegCount(segCount), m_SegWidth(segWidth), m_SegLength(segLength),
	m_SegAngles(segCount, 0.0f)
{
	m_SegRects.resize(segCount);
	m_SegTextures.resize(segCount);
	m_SegRects[0] = { baseX, baseY, segWidth, segLength };

	//const size_t handIndex = Renderer::LoadTexture("assets/hand.png");
	//m_SegTextures[segCount - 1] = Renderer::m_LoadedTextures[handIndex];

	//const size_t armIndex = Renderer::LoadTexture("assets/arm.png");
	for (int i = 1; i < segCount; i++)
	{
		//m_SegTextures[i] = Renderer::m_LoadedTextures[armIndex];
		m_SegRects[i] = { m_SegRects[i - 1].x, m_SegRects[i - 1].y - segLength, segWidth, segLength};
	}
}

Arm::~Arm()
{
}

void Arm::Update()
{
	m_SegRects[0].x = m_BaseX;
	m_SegRects[0].y = m_BaseY;
	float dAngle = m_SegAngles.back();
	std::cout << "dAngle: " << dAngle << '\n';
	float anglePerSeg = dAngle / (m_SegCount);
	m_SegAngles[0] = anglePerSeg;
	//std::cout << "Angle per segment: " << anglePerSeg << '\n';

	// Update the position of each segment based on the previous segment
	for (int i = 1; i < m_SegRects.size(); i++)
	{
		m_SegAngles[i] = std::min(i + 1, static_cast<int>(m_SegRects.size())) * anglePerSeg;

		// Calculate the new position based on the previous segment's position and angle
		m_SegRects[i].x = m_SegRects[i - 1].x + m_SegLength * sin(m_SegAngles[i - 1] * M_PI / 180.0f);
		m_SegRects[i].y = m_SegRects[i - 1].y - m_SegLength * cos(m_SegAngles[i - 1] * M_PI / 180.0f);
	}
}

void Arm::Render()
{
	Renderer::DrawRectRot(&m_SegRects[0], 255, 255, 255, 255, m_SegAngles[0]);
	for (size_t i = 1; i < m_SegRects.size(); i++)
	{
		//Renderer::DrawTexture(m_SegTextures[i], nullptr, &m_SegRects[i]);
		Renderer::DrawRectRot(&m_SegRects[i], 255 * (i % 2), 255 * ((i + 1) % 2), 255, 255, m_SegAngles[i]);
	}
}

void Arm::SetTarget(int x, int y)
{
	m_SegRects.back().x = x;
	m_SegRects.back().y = y;
	// Calculate the angle of the last segment in relation to the base coords
	const float dx = m_SegRects.back().x - m_BaseX;
	const float dy = m_SegRects.back().y - m_BaseY;
	float angle = atan2(dy, dx) * 180.0f / M_PI;

	// Adjust the angle to have 90 degrees as 0
	angle = angle + 90.f;

	m_SegAngles.back() = angle;
}
