#include "renderer.hpp"

#include <iostream>

std::vector<SDL_Texture*> Renderer::m_LoadedTextures;
std::vector<SDL_Rect> Renderer::m_LoadedTexturesRects;
SDL_Renderer* Renderer::renderer = nullptr;
SDL_Window* Renderer::window = nullptr;
bool Renderer::running = true;

Renderer::Renderer(const char* title, int width, int height)
{
	this->Init(title, width, height);
}

void Renderer::Init(const char* title, int width, int height)
{
	SDL_Init(SDL_INIT_EVERYTHING);
	window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
}

void Renderer::Clear()
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
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
	m_LoadedTextures.push_back(texture);
	m_LoadedTexturesRects.push_back({ 0, 0, surface->w, surface->h });
	SDL_FreeSurface(surface);

	return m_LoadedTextures.size() - 1;
}

const size_t Renderer::LoadTexture(const char* path, int* width, int* height)
{
	SDL_Surface* surface = IMG_Load(path);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	m_LoadedTextures.push_back(texture);
	m_LoadedTexturesRects.push_back({ 0, 0, surface->w, surface->h });
	SDL_FreeSurface(surface);

	SDL_Rect rect;
	SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
	*width = rect.w;
	*height = rect.h;

	return m_LoadedTextures.size() - 1;
}

void Renderer::DrawRect(SDL_Rect* rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	SDL_SetRenderDrawColor(renderer, r, g, b, a);
	SDL_RenderFillRect(renderer, rect);
}

void Renderer::DrawRectRot(SDL_Rect* rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a, double rot)
{
	// Create a temporary texture with alpha support
	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, rect->w, rect->h);
	if (!texture) return;

	// Set the texture as the rendering target
	SDL_SetRenderTarget(renderer, texture);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0); // Transparent background
	SDL_RenderClear(renderer);

	// Draw the filled rectangle on the texture
	SDL_Rect tempRect = { 0, 0, rect->w, rect->h };
	SDL_SetRenderDrawColor(renderer, r, g, b, a);
	SDL_RenderFillRect(renderer, &tempRect);

	// Reset the rendering target to the default
	SDL_SetRenderTarget(renderer, nullptr);

	// Define destination rectangle for rendering the rotated texture
	SDL_FRect dstRect = { static_cast<float>(rect->x), static_cast<float>(rect->y), static_cast<float>(rect->w), static_cast<float>(rect->h) };

	// Render the texture with rotation
	SDL_RenderCopyExF(renderer, texture, nullptr, &dstRect, rot, nullptr, SDL_FLIP_NONE);

	// Clean up
	SDL_DestroyTexture(texture);
}

void Renderer::DrawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	SDL_SetRenderDrawColor(renderer, r, g, b, a);
	SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
}

void Renderer::OnClick(std::function<void(int, int)> func)
{
	m_MouseClickCallback = func;
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

void Renderer::Update()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			running = false;

		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE)
				running = false;

		case SDL_MOUSEMOTION:
			mouseX = event.motion.x;
			mouseY = event.motion.y;
			m_MouseClickCallback(mouseX, mouseY);

		case SDL_MOUSEBUTTONDOWN:
			uint32_t mouseState = SDL_GetMouseState(&mouseX, &mouseY);
			if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT))
			{
				m_MouseClickCallback(mouseX, mouseY);
			}
		}
	}
}

Renderer::~Renderer()
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}