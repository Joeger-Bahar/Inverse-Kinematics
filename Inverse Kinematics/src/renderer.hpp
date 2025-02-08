#pragma once

#include <SDL2/SDL.h>
#undef main
#include <SDL2/SDL_image.h>

#include <functional>
#include <vector>

class Renderer
{
public:
	Renderer(const char* title, int width, int height);

	void Init(const char* title, int width, int height);
	void Render();
	void Update();

	// Run function with 2 int parameters and pass the mouse position
	void OnClick(std::function<void(int, int)> func);

	static void DrawTexture(SDL_Texture* texture, SDL_Rect* srcRect, SDL_Rect* dstRect);
	static void DrawTextureRot(SDL_Texture* texture, SDL_Rect* srcRect, SDL_Rect* dstRect, double rot);
	// Return the index of the loaded texture
	static const size_t LoadTexture(const char* path);
	static const size_t LoadTexture(const char* path, int* width, int* height);

	static void DrawRect(SDL_Rect* rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
	static void DrawRectRot(SDL_Rect* rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a, double rot);
	static void DrawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	~Renderer();
public:
	static std::vector<SDL_Texture*> m_LoadedTextures;
	static std::vector<SDL_Rect> m_LoadedTexturesRects;
	static SDL_Renderer* renderer;
	static SDL_Window* window;
	static bool running;

private:
	void Clear();
	void Present();
private:
	std::function<void(int, int)> m_MouseClickCallback;
	int mouseX;
	int mouseY;
};