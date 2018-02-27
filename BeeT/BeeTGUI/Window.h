#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "Module.h"
#include "../SDL2/include/SDL.h"

class Window : public Module
{
public:
	Window(const char* name);
	~Window();

	bool Init();
	bool CleanUp();

	void GetWindowSize(int& width, int& height)const;

public:
	SDL_Window* sdlWindow = nullptr;

private:
	int screenWidth = 800;
	int screenHeight = 600;
};
#endif