#pragma once

#include <SDL2/SDL.h>
#undef main
#include <SDL2/SDL_image.h>

#include <vector>

class Renderer
{
public:
	Renderer(const char* title, int width, int height);

	void Init(const char* title, int width, int height);
	void LoadTexture(const char* path);
	void LoadTexture(const char* path, int* width, int* height);
	void DrawTexture(SDL_Texture* texture, SDL_Rect* srcRect, SDL_Rect* dstRect);
	void DrawTextureWithRot(SDL_Texture* texture, SDL_Rect* srcRect, SDL_Rect* dstRect, double rot);

	void Render();
	void Update();

	void EnableUpdateMousePos(bool enable);
	void AssignMousePos(int* x, int* y);

	~Renderer();
public:
	static SDL_Renderer* renderer;
	static SDL_Window* window;
	static bool running;

private:
	void Clear();
	void Present();
private:
	std::vector<SDL_Texture*> m_LoadedTextures;
	std::vector<SDL_Rect> m_LoadedTexturesRects;

	int* mouseX;
	int* mouseY;
	bool m_UpdateMousePos;
};