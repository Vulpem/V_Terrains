#include <stdlib.h>
#include "Application.h"
#include "Globals.h"

#include "SDL/include/SDL.h"
#pragma comment( lib, "SDL/lib/win32/SDL2.lib" )
#pragma comment( lib, "SDL/lib/win32/SDL2main.lib" )

enum class MainStates
{
	Creation,
	Start,
	Update,
	Finish,
	Exit
};

Application* App = nullptr;

int main(int argc, char ** argv)
{
	LOG("Starting game '%s'...", TITLE);

	int main_return = EXIT_FAILURE;
	MainStates state = MainStates::Creation;

	while (state != MainStates::Exit)
	{
		switch (state)
		{
		case MainStates::Creation:

			LOG("-------------- Application Creation --------------");
			App = new Application();
			state = MainStates::Start;
			break;

		case MainStates::Start:

			LOG("-------------- Application Init --------------");
			if (App->Init() == false)
			{
				LOG("Application Init exits with ERROR");
				state = MainStates::Exit;
			}
			else
			{
				state = MainStates::Update;
				LOG("\n\n-------------- Application Update --------------\n\n");
			}

			break;

		case MainStates::Update:
		{
			UpdateStatus update_return = App->Update();
			if (update_return == UpdateStatus::Error)
			{
				LOG("Application Update exits with ERROR");
				state = MainStates::Exit;
			}
			if (update_return == UpdateStatus::Stop)
				state = MainStates::Finish;
		}
			break;

		case MainStates::Finish:

			LOG("-------------- Application CleanUp --------------");
			App->CleanUp();
			main_return = EXIT_SUCCESS;
			state = MainStates::Exit;
			break;

		}
	}

	delete App;
	App = nullptr;
	LOG("Exiting game '%s'...\n", TITLE);
	return main_return;
}