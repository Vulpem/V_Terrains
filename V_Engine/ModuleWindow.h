#ifndef __ModuleWindow_H__
#define __ModuleWindow_H__

#include "Module.h"
#include "SDL/include/SDL.h"

class Application;

class ModuleWindow : public Module
{
public:

	ModuleWindow();
	~ModuleWindow();

	bool Init() override;
	void OnDisable() override;

	void SetTitle(const char* title);

	SDL_Window* GetWindow() { return m_window; }
	void OnScreenResize(int width, int heigth) override;
	float2 GetWindowSize();

private:
	//The window we'll be rendering to
	SDL_Window* m_window;

	//The surface contained by the window
	SDL_Surface* m_screenSurface;

	int m_windowWidth = 100;
	int m_windowHeigth = 100;
};

#endif // __ModuleWindow_H__