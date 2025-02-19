#pragma once

#include <glm/glm.hpp>

#include "renderer.hpp"

struct Segment
{
	Segment(float length, float width, float angle, Segment* parent = nullptr, Segment* child = nullptr);
	void AssignChild(Segment* child);
	void AssignChild(Segment child);
	void AssignParent(Segment* parent);
	void AssignParent(Segment parent);
	void ReverseK();
	void ForwardK();
	void Render();

public:
	glm::vec2 a; // base
	glm::vec2 b; // top
	float length, width;
	double angle;
	Segment* parent;
	Segment* child;
};

// Actually the segment that follows the mouse
struct BaseSegment : public Segment
{
	using Segment::Segment;
	void ReverseK(const int mouseX, const int mouseY);
	void ForwardK();
	void Render();
	void AssignParent(Segment* child) = delete;
	void AssignParent(Segment child) = delete;
};