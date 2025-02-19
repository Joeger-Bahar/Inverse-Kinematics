#include "segment.hpp"

#include <iostream>

#include <SDL2_gfx/SDL2_gfxPrimitives.h>

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
	//Renderer::DrawRectRot(a.x, a.y, width, length, 255, 0, 255, 255, angle);
	thickLineColor(Renderer::renderer, a.x, a.y, b.x, b.y, static_cast<Uint8>(10), 0xFFFFFFFF);
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
	direction = glm::normalize(direction) * length;
	// Reverse direction
	//direction *= -1;
	// Add direction to the mouse to get the new point
	//a = mouse + direction;
	//a = glm::vec2(500, 300);
	// Set end of line to mouse
	b = mouse;

	// Reversely calculate the base position
	// Normalize direction
	glm::vec2 aDirection = glm::normalize(glm::vec2(glm::cos(glm::radians(angle)), glm::sin(glm::radians(angle))));
	a = b - aDirection * width;

	if (child != nullptr)
	{
		child->ReverseK();
	}
	else // If this is the last segment
	{
		// Set the end of the line to the center of the screen
		a = glm::vec2(Renderer::screenWidth / 2, Renderer::screenHeight / 2);
		// Set the base position to the end of the line minus the width
		glm::vec2 bDirection = glm::normalize(glm::vec2(glm::cos(glm::radians(angle)), glm::sin(glm::radians(angle)));
		b = a + bDirection * length;
		parent->ForwardK();
	}
}

void Segment::ForwardK()
{
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
	direction = glm::normalize(direction) * length;
	// Reverse direction
	//direction *= -1;
	// Add direction to the mouse to get the new point
	//a = mouse + direction;
	//a = glm::vec2(500, 300);
	// Set end of line to mouse
	b = mouse;

	// Reversely calculate the base position
	// Normalize direction
	glm::vec2 bDirection = glm::normalize(glm::vec2(glm::cos(glm::radians(angle)), glm::sin(glm::radians(angle))));
	a = b - bDirection * width;

	if (child != nullptr)
	{
		child->ReverseK();
	}
}

void BaseSegment::ForwardK()
{
}

void BaseSegment::Render()
{
	//Renderer::DrawRectRot(a.x, a.y, width, length, 255, 255, 255, 255, angle);
	thickLineColor(Renderer::renderer, a.x, a.y, b.x, b.y, static_cast<Uint8>(10), 0xFFFFFFFF);
	if (child != nullptr)
	{
		child->Render();
	}
}
