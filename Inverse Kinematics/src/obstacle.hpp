#pragma once

#include <glm/glm.hpp>
#include <SDL2_gfx/SDL2_gfxPrimitives.h>
#include <SDL2/SDL.h>

#include "renderer.hpp"

struct Obstacle
{
    glm::vec2 position;
    float radius;
    SDL_Color color;

    Obstacle(glm::vec2 pos, float r, SDL_Color col)
        : position(pos), radius(r), color(col) {
    }

    void Render()
    {
        filledCircleRGBA(Renderer::renderer, position.x, position.y, radius, color.r, color.g, color.b, color.a);
    }
};