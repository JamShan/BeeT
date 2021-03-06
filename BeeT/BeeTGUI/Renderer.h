#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "Module.h"
#include "../SDL2/include/SDL.h"

class Renderer : public Module
{
public:
	Renderer(const char* name);
	~Renderer();

	bool Init();
	bool CleanUp();

	bool PreUpdate();
	bool PostUpdate();

private:
	bool CheckGLError()const;

	void InitImGuiStyle();

private:
	SDL_GLContext context;
};
#endif
