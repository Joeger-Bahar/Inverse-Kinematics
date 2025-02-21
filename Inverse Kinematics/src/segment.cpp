#include "segment.hpp"

#include <iostream>

#include <SDL2_gfx/SDL2_gfxPrimitives.h>

Segment::Segment(int length, int width, float angle, SDL_Color color, Segment* parent, Segment* child)
	: length(length), width(width), angle(angle), parent(parent), child(child), color(color)
{}

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
	thickLineRGBA(Renderer::renderer, a.x, a.y, b.x, b.y, (uint8_t)width,
		color.r, color.g, color.b, color.a);

	if (child != nullptr)
	{
		child->Render();
	}
}

void Segment::ReverseK(const int mouseX, const int mouseY)
{
	glm::vec2 target;

	if (!parent) // First segment, mouse coords should be passed
		target = glm::vec2(mouseX, mouseY);
	else // Mouse coords will be -1
		target = parent->a;

	// Angle from target to base
	glm::vec2 direction = glm::vec2(target - a);
	angle = glm::degrees(glm::atan(direction.y, direction.x));

	// Move end to target
	b = target;

	// Calculate the base position relative to the end
	relationDir = glm::normalize(glm::vec2(glm::cos(glm::radians(angle)), glm::sin(glm::radians(angle))))
		* (float)length;
	a = b - relationDir;

	if (child != nullptr)
	{
		child->ReverseK();
	}
}

void Segment::ForwardK()
{
	if (child == nullptr) // First one
	{
		a = glm::vec2(Renderer::screenWidth / 2, Renderer::screenHeight / 2);
		// Move end to position relative to base
		b = a + relationDir;
	}
	else
	{
		a = child->b;
		// Samesies
		b = a + relationDir;
	}
	if (parent != nullptr)
	{
		parent->ForwardK();
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

void BaseSegment::ForwardK()
{
	a = glm::vec2(Renderer::mouseX, Renderer::mouseY);
	b = a + relationDir;
}
