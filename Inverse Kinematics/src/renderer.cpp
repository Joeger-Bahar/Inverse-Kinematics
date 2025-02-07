#include "renderer.hpp"

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

void Renderer::LoadTexture(const char* path)
{
	SDL_Surface* surface = IMG_Load(path);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	m_LoadedTextures.push_back(texture);
	m_LoadedTexturesRects.push_back({ 0, 0, surface->w, surface->h });
	SDL_FreeSurface(surface);
}

void Renderer::LoadTexture(const char* path, int* width, int* height)
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

void Renderer::DrawTexture(SDL_Texture* texture, SDL_Rect* srcRect, SDL_Rect* dstRect)
{
	SDL_RenderCopy(renderer, texture, srcRect, dstRect);
}

void Renderer::DrawTextureWithRot(SDL_Texture* texture, SDL_Rect* srcRect, SDL_Rect* dstRect, double rot)
{
	SDL_RenderCopyEx(renderer, texture, srcRect, dstRect, rot, nullptr, SDL_FLIP_NONE);
}

void Renderer::Render()
{
	this->Clear();
	this->Present();
}

void Renderer::Update()
{
	while (running)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
				running = false;
			if (m_UpdateMousePos)
			{
				SDL_GetMouseState(mouseX, mouseY);
			}
		}
	}
}

void Renderer::EnableUpdateMousePos(bool enable)
{
	m_UpdateMousePos = enable;
}

void Renderer::AssignMousePos(int* x, int* y)
{
	mouseX = x;
	mouseY = y;
}

Renderer::~Renderer()
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}