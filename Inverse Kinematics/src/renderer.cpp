#include "renderer.hpp"

#include <SDL2/SDL_opengl.h>

#include <iostream>

std::vector<SDL_Texture*> Renderer::loadedTextures;
std::vector<SDL_Rect> Renderer::loadedTexturesRects;
SDL_Renderer* Renderer::renderer = nullptr;
SDL_Window* Renderer::window = nullptr;
int Renderer::mouseX = 0;
int Renderer::mouseY = 0;
int Renderer::prevMouseX = 0;
int Renderer::prevMouseY = 0;
int Renderer::screenWidth = 0;
int Renderer::screenHeight = 0;
bool Renderer::running = true;
bool Renderer::renderTextures = true;

Renderer::Renderer(const char* title, int width, int height)
	: runningSegCount(0)
{
	this->Init(title, width, height);
}

void Renderer::Init(const char* title, int width, int height)
{
	SDL_Init(SDL_INIT_EVERYTHING);
	window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP);
	renderer = SDL_CreateRenderer(window, SDL_VIDEO_RENDER_OGL, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	SDL_GetRendererOutputSize(renderer, &Renderer::screenWidth, &Renderer::screenHeight);

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
}

void Renderer::Clear()
{
	SDL_SetRenderDrawColor(renderer, 9, 9, 9, 255);
	SDL_RenderClear(renderer);
}

void Renderer::Present()
{
	SDL_RenderPresent(renderer);
}

const size_t Renderer::LoadTexture(const char* path)
{
	SDL_Surface* surface = IMG_Load(path);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	loadedTextures.push_back(texture);
	loadedTexturesRects.push_back({ 0, 0, surface->w, surface->h });
	SDL_FreeSurface(surface);

	return loadedTextures.size() - 1;
}

const size_t Renderer::LoadTexture(const char* path, int* width, int* height)
{
	SDL_Surface* surface = IMG_Load(path);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	loadedTextures.push_back(texture);
	loadedTexturesRects.push_back({ 0, 0, surface->w, surface->h });
	SDL_FreeSurface(surface);

	SDL_Rect rect;
	SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
	*width = rect.w;
	*height = rect.h;

	return loadedTextures.size() - 1;
}

void Renderer::OnClick(std::function<void(int, int)> func)
{
	m_MouseClickCallback = func;
}

void Renderer::OnMove(std::function<void(int, int)> func)
{
	m_MouseMoveCallback = func;
}

void Renderer::DrawTexture(SDL_Texture* texture, SDL_Rect* srcRect, SDL_Rect* dstRect)
{
	SDL_RenderCopy(renderer, texture, srcRect, dstRect);
}

void Renderer::DrawTextureRot(SDL_Texture* texture, SDL_Rect* srcRect, SDL_Rect* dstRect, double rot)
{
	SDL_RenderCopyEx(renderer, texture, srcRect, dstRect, rot, nullptr, SDL_FLIP_NONE);
}

void Renderer::Render()
{
	this->Present();
	this->Clear();
}

int Renderer::Update()
{
	int returnSegCount = 0;

	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			running = false;
			break;

		case SDL_KEYDOWN:
			switch (event.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				running = false;
				break;

			// Number keys
			// This is a switch case OR statement
			case SDLK_0:
			case SDLK_1:
			case SDLK_2:
			case SDLK_3:
			case SDLK_4:
			case SDLK_5:
			case SDLK_6:
			case SDLK_7:
			case SDLK_8:
			case SDLK_9:
				runningSegCount *= 10;
				// Offset from 0 value
				runningSegCount += (event.key.keysym.sym - 48);
				break;

			case SDLK_BACKSPACE:
				runningSegCount /= 10; // Will truncate the last digit
				break;
			case SDLK_RETURN:
				returnSegCount = runningSegCount;
				runningSegCount = 0;
				break;

			case SDLK_a:
				Renderer::renderTextures = !Renderer::renderTextures;
				break;

			}
			break;

		case SDL_MOUSEMOTION:
			prevMouseX = mouseX;
			prevMouseY = mouseY;
			mouseX = event.motion.x;
			mouseY = event.motion.y;
			if (m_MouseMoveCallback)
				m_MouseMoveCallback(mouseX, mouseY);
			break;

		case SDL_MOUSEBUTTONDOWN:
			if (!m_MouseClickCallback)
				break;
			uint32_t mouseState = SDL_GetMouseState(&mouseX, &mouseY);
			if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT))
			{
				m_MouseClickCallback(mouseX, mouseY);
			}
			break;
		}
	}

	return returnSegCount;
}

Renderer::~Renderer()
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}