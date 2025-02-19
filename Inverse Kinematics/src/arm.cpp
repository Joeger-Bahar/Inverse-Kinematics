#include "arm.hpp"

#include <algorithm>
#include <iostream>
#include <cmath>

Arm::Arm(int baseX, int baseY, int segCount, int segWidth, int segLength)
	: m_BaseX(baseX), m_BaseY(baseY), m_SegCount(segCount), m_SegWidth(segWidth), m_SegLength(segLength)
{
	m_Segments.resize(m_SegCount);
	m_BaseSeg = new BaseSegment(segWidth, segLength, 0.f);
	m_Segments[0] = m_BaseSeg;

	//const size_t handIndex = Renderer::LoadTexture("assets/hand.png");
	//m_SegTextures[segCount - 1] = Renderer::m_LoadedTextures[handIndex];

	//const size_t armIndex = Renderer::LoadTexture("assets/arm.png");
	for (int i = 1; i < m_SegCount; i++)
	{
		//m_SegTextures[i] = Renderer::m_LoadedTextures[armIndex];
		m_Segments[i] = new Segment(segWidth, segLength, 0.f);
		m_Segments[i]->AssignParent(m_Segments[i - 1]);
		m_Segments[i - 1]->AssignChild(m_Segments[i]);
	}
}

Arm::~Arm()
{
}

void Arm::Update()
{
	m_BaseSeg->ReverseK(Renderer::mouseX, Renderer::mouseY);
	m_Segments.back()->ForwardK();
}

void Arm::Render()
{
	m_BaseSeg->Render();
}

void Arm::SetTarget(int x, int y)
{
}
