#include "Globals.h"
#include "Application.h"
#include "ModuleInput.h"
#include "ModuleRenderer3D.h"
#include "ModuleWindow.h"


#include "imGUI/imgui.h"
#include "Imgui/imgui_impl_sdl_gl3.h"

#define MAX_KEYS SDL_NUM_SCANCODES

ModuleInput::ModuleInput() : Module()
{
	m_keyboardStates = new KEY_STATE[MAX_KEYS + 1];
	memset(m_keyboardStates, KEY_IDLE, sizeof(KEY_STATE) * MAX_KEYS);
	memset(m_mouseButtons, KEY_IDLE, sizeof(KEY_STATE) * MAX_MOUSE_BUTTONS);
}

// Destructor
ModuleInput::~ModuleInput()
{
	delete[] m_keyboardStates;
}

// Called before render is available
bool ModuleInput::Init()
{
	LOG("Init SDL input event system");
	bool ret = true;
	SDL_Init(0);

	//SDL_ShowCursor(0);

	if(SDL_InitSubSystem(SDL_INIT_EVENTS) < 0)
	{
		LOG("SDL_EVENTS could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	m_lastDroppedFile[0] = '\0';
	m_fileWasDropped = false;

	return ret;
}

// Called every draw update
UpdateStatus ModuleInput::PreUpdate()
{
	SDL_PumpEvents();

	m_fileWasDropped = false;
	
	const Uint8* keys = SDL_GetKeyboardState(NULL);
	
	for(int i = 0; i < MAX_KEYS; ++i)
	{
		if(keys[i] == 1 && m_ignoreKeyboard == false)
		{
			if(m_keyboardStates[i] == KEY_IDLE)
				m_keyboardStates[i] = KEY_DOWN;
			else
				m_keyboardStates[i] = KEY_REPEAT;
		}
		else
		{
			if(m_keyboardStates[i] == KEY_REPEAT || m_keyboardStates[i] == KEY_DOWN)
				m_keyboardStates[i] = KEY_UP;
			else
				m_keyboardStates[i] = KEY_IDLE;
		}
	}

	Uint32 buttons = SDL_GetMouseState(&m_mouseX, &m_mouseY);

	m_mouseZ = 0;

	for(int i = 0; i < 5; ++i)
	{
		if(buttons & SDL_BUTTON(i) && m_ignoreMouse == false)
		{
			if(m_mouseButtons[i] == KEY_IDLE)
				m_mouseButtons[i] = KEY_DOWN;
			else
				m_mouseButtons[i] = KEY_REPEAT;
		}
		else
		{
			if(m_mouseButtons[i] == KEY_REPEAT || m_mouseButtons[i] == KEY_DOWN)
				m_mouseButtons[i] = KEY_UP;
			else
				m_mouseButtons[i] = KEY_IDLE;
		}
	}

	m_mouseMotionX = m_mouseMotionY = 0;

	SDL_Event e;
	while(SDL_PollEvent(&e))
	{
		if (m_ignoreMouse == false)
		{
			switch (e.type)
			{
				case SDL_EventType::SDL_MOUSEWHEEL:
				{
					m_mouseZ = e.wheel.y;
					break;
				}
				case SDL_EventType::SDL_MOUSEMOTION:
				{
					if (m_captureMouse)
					{
						if (CaptureMouse(e))
						{
							ImGui::GetIO().MousePos = ImVec2(-1, -1);
							ImGui::GetIO().MousePosPrev = ImVec2(-1, -1);
						}
					}
					m_mouseX = e.motion.x;
					m_mouseY = e.motion.y;

					m_mouseMotionX = e.motion.xrel;
					m_mouseMotionY = e.motion.yrel;
					break;
				}

				case SDL_EventType::SDL_DROPFILE:
				{
					strcpy_s(m_lastDroppedFile, e.drop.file);
					SDL_free(e.drop.file);
					m_fileWasDropped = true;
					LOG("Dropped %s", m_lastDroppedFile);
					break;
				}
				case SDL_EventType::SDL_QUIT:
				{
					return UpdateStatus::Stop;
				}
				case SDL_EventType::SDL_WINDOWEVENT:
				{
					if (e.window.event == SDL_WindowEventID::SDL_WINDOWEVENT_RESIZED)
					{
						App->OnScreenResize(e.window.data1, e.window.data2);
					}
				}
			}
		}
		ImGui_ImplSdlGL3_ProcessEvent(&e);
	}

	/*if (GetMouseButton(SDL_BUTTON_LEFT) == KEY_DOWN)
	{
		captureMouse = true;
	}
	else if (GetMouseButton(SDL_BUTTON_LEFT) == KEY_UP)
	{
		captureMouse = false;
	}*/		

	return UpdateStatus::Continue;
}

// Called before quitting
void ModuleInput::OnDisable()
{
	LOG("Quitting SDL input event subsystem");
	SDL_QuitSubSystem(SDL_INIT_EVENTS);
}

bool ModuleInput::CaptureMouse(SDL_Event& e)
{
	bool ret = false;
	float2 windowSize = App->m_window->GetWindowSize();
	if (m_mouseX + e.motion.xrel >= windowSize.x)
	{
		SDL_WarpMouseInWindow(App->m_window->GetWindow(), 1, e.motion.y);
		e.motion.xrel = 0;
		ret = true;
	}
	else if (m_mouseX + e.motion.xrel <= 0)
	{
		SDL_WarpMouseInWindow(App->m_window->GetWindow(), windowSize.x - 1, e.motion.y);
		e.motion.xrel = 0;
		ret = true;
	}

	if (m_mouseY + e.motion.yrel >= windowSize.y)
	{
		SDL_WarpMouseInWindow(App->m_window->GetWindow(), e.motion.x, 1);
		e.motion.yrel = 0;
		ret = true;
	}
	else if (m_mouseY + e.motion.yrel <= 0)
	{
		SDL_WarpMouseInWindow(App->m_window->GetWindow(), e.motion.x, windowSize.y - 1);
		e.motion.yrel = 0;
		ret = true;
	}

	return ret;
}
