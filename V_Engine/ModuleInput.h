#ifndef __MODULE_INPUT__
#define __MODULE_INPUT__

#include "Module.h"
#include "Globals.h"

#define MAX_MOUSE_BUTTONS 5

enum KEY_STATE
{
	KEY_IDLE = 0,
	KEY_DOWN,
	KEY_REPEAT,
	KEY_UP
};

class ModuleInput : public Module
{
public:
	
	ModuleInput(Application* app, bool start_enabled = true);
	~ModuleInput();

	bool Init() override;
	UpdateStatus PreUpdate() override;
	void CleanUp() override;

	KEY_STATE GetKey(int id) const { return m_keyboardStates[id]; }
	KEY_STATE GetMouseButton(int id) const { return m_mouseButtons[id]; }

	int GetMouseX() const { return m_mouseX; }
	int GetMouseY() const { return m_mouseY; }
	int GetMouseZ() const { return m_mouseZ; }

	int GetMouseXMotion() const { return m_mouseMotionX; }
	int GetMouseYMotion() const { return m_mouseMotionY; }
public:
	bool m_captureMouse = false;
	bool m_ignoreMouse = false;
	bool m_ignoreKeyboard = false;

	char m_lastDroppedFile[1024];
	bool m_fileWasDropped;

private:
	bool CaptureMouse(SDL_Event& e);

	KEY_STATE* m_keyboardStates;
	KEY_STATE m_mouseButtons[MAX_MOUSE_BUTTONS];
	int m_mouseX;
	int m_mouseY;
	int m_mouseZ;
	int m_mouseMotionX;
	int m_mouseMotionY;
};

#endif