#pragma once

#include <glm/glm.hpp>

#include "renderer.hpp"

struct Segment
{
	Segment(int length, int width, float angle, SDL_Color color = { 255, 255, 255, 255 },
		Segment* parent = nullptr, Segment* child = nullptr);
	void AssignChild(Segment* child);
	void AssignChild(Segment child);
	void AssignParent(Segment* parent);
	void AssignParent(Segment parent);
	void ReverseK(const int mouseX = -1, const int mouseY = -1);
	void ForwardK();
	void Render();

public:
	Segment* parent;
	Segment* child;
	glm::vec2 a; // base
	glm::vec2 b; // top
	glm::vec2 relationDir; // Cached matematical direction from a-b or b-a
	double angle;
	SDL_Color color;
	uint8_t length, width;
};

// Actually the segment that follows the mouse
struct BaseSegment : public Segment
{
	using Segment::Segment;
	void ForwardK();
	void AssignParent(Segment* child) = delete;
	void AssignParent(Segment child) = delete;
};