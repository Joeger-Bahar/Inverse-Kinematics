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
	void Update();
	void Render();

public:
	/* not yet */
	// I use the bottom-right and top-left coords in conjunct
	// with the width to obtain the center of the top and bottom 
	// to make chaining easier.
	glm::vec2 a; // bottom left
	glm::vec2 b; // top right
	float length, width;
	double angle;
	Segment* parent;
	Segment* child;
};

struct BaseSegment : public Segment
{
	using Segment::Segment;
	void Update(const int mouseX, const int mouseY);
	void Render();
	void AssignParent(Segment* child) = delete;
	void AssignParent(Segment child) = delete;
};