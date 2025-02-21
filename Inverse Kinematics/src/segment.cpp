#include "segment.hpp"

#include <iostream>

#include <SDL2_gfx/SDL2_gfxPrimitives.h>

Segment::Segment(int length, int width, float angle, Segment* parent, Segment* child)
	: length(length), width(width), angle(angle), parent(parent), child(child), color({ 255, 255, 255, 255 })
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
	thickLineRGBA(Renderer::renderer, a.x, a.y, b.x, b.y, static_cast<Uint8>(10),
		color.r, color.g, color.b, color.a);

	if (child != nullptr)
	{
		child->Render();
	}
}

void Segment::ReverseK()
{
	glm::vec2 mouse = glm::vec2(parent->a.x, parent->a.y);

	// Angle from mouse to base
	glm::vec2 direction = glm::vec2(mouse - a);
	angle = glm::degrees(glm::atan(direction.y, direction.x));

	// Normalize direction to length
	b = mouse;

	// Reversely calculate the base position
	relationDir = glm::normalize(glm::vec2(glm::cos(glm::radians(angle)), glm::sin(glm::radians(angle))));
	a = b - relationDir * (float)width;

	if (child != nullptr)
	{
		child->ReverseK();
	}
}

void Segment::ForwardK()
{
	if (child == nullptr) // First one
	{
		// This is most likely redundant
		a = glm::vec2(Renderer::screenWidth / 2, Renderer::screenHeight / 2);
		relationDir = glm::normalize(glm::vec2(glm::cos(glm::radians(angle)), glm::sin(glm::radians(angle))));
		b = a + relationDir * (float)width;
	}
	else
	{
		// Samesies
		a = child->b;
		b = a + relationDir * (float)width;
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
	relationDir = glm::normalize(glm::vec2(glm::cos(glm::radians(angle)), glm::sin(glm::radians(angle))));
	a = b - relationDir * (float)width;

	if (child != nullptr)
	{
		child->ReverseK();
	}
}

void BaseSegment::ForwardK()
{
	a = glm::vec2(Renderer::mouseX, Renderer::mouseY);
	b = a + relationDir * (float)width;
}

void BaseSegment::Render()
{
	thickLineRGBA(Renderer::renderer, a.x, a.y, b.x, b.y, static_cast<Uint8>(10),
		color.r, color.g, color.b, color.a);
	if (child != nullptr)
	{
		child->Render();
	}
}
