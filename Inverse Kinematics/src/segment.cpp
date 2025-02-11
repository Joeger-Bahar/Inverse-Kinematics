#include "segment.hpp"

#include <iostream>

Segment::Segment(float length, float width, float angle, Segment* parent, Segment* child)
	: length(length), width(width), angle(angle), parent(parent), child(child)
{

}

void Segment::AssignChild(Segment* child)
{
	this->child = child;
}

void Segment::AssignParent(Segment parent)
{
	this->parent = &parent;
}

void Segment::Render()
{
	Renderer::DrawRectRot(a.x, a.y, width, length, 255, 0, 255, 255, angle);
	if (child != nullptr)
	{
		child->Render();
	}
}

void Segment::Update()
{
	glm::vec2 target = glm::vec2(parent->b.x, parent->b.y);

	// Angle from target to base
	glm::vec2 direction = glm::vec2(target - a);
	angle = glm::degrees(glm::atan(-direction.y, -direction.x));

	// Normalize direction to length
	direction = glm::normalize(direction) * length;
	// Reverse direction
	direction *= -1;
	// Add direction to the target to get the new point
	a = target + direction;
	b = glm::vec2(a.x + width * glm::cos(glm::radians(angle)), a.y + width * glm::sin(glm::radians(angle)));
	if (child != nullptr)
	{
		child->Update();
	}
}

void Segment::AssignParent(Segment* parent)
{
	this->parent = parent;
}

void Segment::AssignChild(Segment child)
{
	this->child = &child;
}

void BaseSegment::Update(const int mouseX, const int mouseY)
{
	glm::vec2 mouse = glm::vec2(mouseX, mouseY);

	// Angle from mouse to base
	glm::vec2 direction = glm::vec2(mouse - a);
	angle = glm::degrees(glm::atan(-direction.y, -direction.x));

	// Normalize direction to length
	direction = glm::normalize(direction) * length;
	// Reverse direction
	direction *= -1;
	// Add direction to the mouse to get the new point
	a = mouse + direction;
	// Asign b using sin and cos of the angle
	b = glm::vec2(a.x + width * glm::cos(glm::radians(angle)), a.y + width * glm::sin(glm::radians(angle)));
	if (child != nullptr)
	{
		child->Update();
	}
}

void BaseSegment::Render()
{
	Renderer::DrawRectRot(a.x, a.y, width, length, 255, 255, 255, 255, angle);
	if (child != nullptr)
	{
		child->Render();
	}
}
