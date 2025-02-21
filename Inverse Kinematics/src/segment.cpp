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

					// Default to -1
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

void BaseSegment::ReverseK(const int mouseX, const int mouseY)
{
	glm::vec2 mouse = glm::vec2(mouseX, mouseY);

	// Angle from mouse to base
	glm::vec2 direction = glm::vec2(mouse - a);
	angle = glm::degrees(glm::atan(direction.y, direction.x));

	// Normalize direction to length
	b = mouse;

	// Reversely calculate the base position
	// Normalize direction
	relationDir = glm::normalize(glm::vec2(glm::cos(glm::radians(angle)), glm::sin(glm::radians(angle))))
		* (float)length;
	a = b - relationDir;

	if (child != nullptr)
	{
		child->ReverseK();
	}
}

void BaseSegment::ForwardK()
{
	a = glm::vec2(Renderer::mouseX, Renderer::mouseY);
	b = a + relationDir;
}

void BaseSegment::Render()
{
	thickLineRGBA(Renderer::renderer, a.x, a.y, b.x, b.y, (uint8_t)width,
		color.r, color.g, color.b, color.a);
	if (child != nullptr)
	{
		child->Render();
	}
}
