#include "segment.hpp"
#include <iostream>
#include <SDL2_gfx/SDL2_gfxPrimitives.h>

glm::vec2 Lerp(const glm::vec2& start, const glm::vec2& end, float t)
{
    return start + t * (end - start);
}

Segment::Segment(int length, int width, float angle, uint8_t thickness, size_t textureIndex, SDL_Color color, Segment* parent, Segment* child)
	: length(length), width(width), angle(angle), parent(parent), child(child), color(color), interpolationSpeed(0.08f),
      thickness(thickness), textureIndex(textureIndex)
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
                     // Wraparound
	if (textureIndex != (size_t)-1 && Renderer::renderTextures)
	{
		SDL_Rect srcRect = Renderer::loadedTexturesRects[textureIndex];
		SDL_Rect dstRect = { (int)a.x, (int)a.y, length, width };
		Renderer::DrawTextureRot(Renderer::loadedTextures[textureIndex], &srcRect, &dstRect, angle);
	}
	else
    {
        thickLineRGBA(Renderer::renderer, a.x, a.y, b.x, b.y, thickness,
            color.r, color.g, color.b, color.a);
    }
    if (child != nullptr)
    {
        child->Render();
    }
}

void Segment::ReverseK(const int mouseX, const int mouseY)
{
    glm::vec2 target;

    if (!child) // Last segment follows the mouse
    {
        float distance = glm::distance(glm::vec2(mouseX, mouseY), glm::vec2(Renderer::prevMouseX, Renderer::prevMouseY));

        interpolationSpeed = std::min(1.0f / (1.0f + distance), 0.1f);

        glm::vec2 mouse = glm::vec2(mouseX, mouseY);
        target = Lerp(b, mouse, interpolationSpeed);
    }
    else
    {
        target = child->a; // Target is the previous segment's base
    }

	// Direction from base to target
    glm::vec2 direction = target - a;
    float distToTarget = glm::length(direction);

    angle = glm::degrees(glm::atan(direction.y, direction.x));
    b = target;

    // Compute new base position while keeping segment within allowed length
    relationDir = glm::normalize(glm::vec2(glm::cos(glm::radians(angle)),
                                           glm::sin(glm::radians(angle)))) * (float)length;
    a = b - relationDir;

    if (parent != nullptr)
    {
        parent->ReverseK();
    }
}

void Segment::ForwardK()
{
    if (parent != nullptr)
    {
        glm::vec2 direction = a - parent->b;
        float currentDist = glm::length(direction);

		float dotProduct = glm::dot(glm::normalize(direction), glm::normalize(relationDir));

        // Adjust distance to maintain even spacing while allowing movement in [0, maxDistance]
        if (currentDist > maxDistance)
        {
            direction = glm::normalize(direction) * maxDistance;
        }
        else if (dotProduct < 0) // Prevent inversion
        {
			std::cout << "Inversion detected" << std::endl;
            direction = glm::vec2(0);
        }

        a = parent->b + direction; // Keep spacing between segments flexible
    }

    b = a + relationDir; // Ensure end remains consistent with direction

    if (child != nullptr)
    {
        child->ForwardK();
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
    a = glm::vec2(Renderer::screenWidth / 2, Renderer::screenHeight / 2);
    // Move end to position relative to base
    b = a + relationDir;

	if (child != nullptr)
	{
		child->ForwardK();
	}
}
