#include "Globals.h"
#include "Application.h"
#include "ModuleWindow.h"
#include "Math.h"

ModuleWindow::ModuleWindow() : Module()
{
	m_window = nullptr;
	m_screenSurface = nullptr;
}

ModuleWindow::~ModuleWindow()
{
}

// Called before render is available
bool ModuleWindow::Init()
{
	LOG("Init SDL window & surface");
	bool ret = true;

	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		LOG("SDL_VIDEO could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
	{
		//Create window
		int width = SCREEN_WIDTH;
		int height = SCREEN_HEIGHT;
		Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

		//Use OpenGL 2.1
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

		if(WIN_FULLSCREEN == true)
		{
			flags |= SDL_WINDOW_FULLSCREEN;
		}

		if (WIN_MAXIMIZED)
		{
			flags |= SDL_WINDOW_MAXIMIZED;
		}

		if(WIN_RESIZABLE == true)
		{
			flags |= SDL_WINDOW_RESIZABLE;
		}

		if(WIN_BORDERLESS == true)
		{
			flags |= SDL_WINDOW_BORDERLESS;
		}

		if(WIN_FULLSCREEN_DESKTOP == true)
		{
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		}

		m_window = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);

		if(m_window == nullptr)
		{
			LOG("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			ret = false;
		}
		else
		{
			//Get window surface
			m_screenSurface = SDL_GetWindowSurface(m_window);
			int w, h;
			SDL_GetWindowSize(m_window, &w, &h);
			OnScreenResize(w,h);
		}
	}

	return ret;
}

// Called before quitting
void ModuleWindow::OnDisable()
{
	LOG("Destroying SDL window and quitting all SDL systems");

	//Destroy window
	if(m_window != nullptr)
	{
		SDL_DestroyWindow(m_window);
	}

	//Quit SDL subsystems
	SDL_Quit();
}

void ModuleWindow::SetTitle(const char* title)
{
	SDL_SetWindowTitle(m_window, title);
}

void ModuleWindow::OnScreenResize(int width, int heigth)
{
	m_windowWidth = width;
	m_windowHeigth = heigth;
}

float2 ModuleWindow::GetWindowSize()
{
	return float2(m_windowWidth, m_windowHeigth);
}