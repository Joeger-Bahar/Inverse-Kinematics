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
	int Update();

	// Run function with 2 int parameters using the mouse position
	void OnClick(std::function<void(int, int)> func);
	void OnMove(std::function<void(int, int)> func);

	static void DrawTexture(SDL_Texture* texture, SDL_Rect* srcRect, SDL_Rect* dstRect);
	static void DrawTextureRot(SDL_Texture* texture, SDL_Rect* srcRect, SDL_Rect* dstRect, double rot);
	// Return the index of the loaded texture in the texture vector
	static const size_t LoadTexture(const char* path);
	static const size_t LoadTexture(const char* path, int* width, int* height);

	~Renderer();
public:
	// Globally accessable texture storage
	static std::vector<SDL_Texture*> loadedTextures;
	static std::vector<SDL_Rect> loadedTexturesRects;
	static SDL_Renderer* renderer;
	static SDL_Window* window;
	static int mouseX, mouseY;
	static int prevMouseX, prevMouseY;
	static int screenWidth;
	static int screenHeight;
	uint32_t runningSegCount; // Allows user to dynamically change segment count
	static bool running, renderTextures;

private:
	void Clear();
	void Present();
private:
	std::function<void(int, int)> m_MouseClickCallback;
	std::function<void(int, int)> m_MouseMoveCallback;
};