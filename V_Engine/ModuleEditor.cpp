#include "Globals.h"
#include "Application.h"
#include "ModuleEditor.h"

#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleAudio.h"
#include "ModuleRenderer3D.h"
#include "ModuleCamera3D.h"
#include "ModulePhysics3D.h"
#include "ModuleGOmanager.h"
#include "ModuleResourceManager.h"
#include "Timers.h"

#include "AllComponents.h"
#include "AllResources.h"

#include "imGUI\imgui_impl_sdl_gl3.h"

#include "OpenGL.h"


ModuleEditor::ModuleEditor(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	moduleName = "ModuleEditor";
}

// Destructor
ModuleEditor::~ModuleEditor()
{

}

// Called before render is available
bool ModuleEditor::Init()
{
	bool ret = true;
LOG("Init editor gui with imgui lib version %s", ImGui::GetVersion());
//Linking ImGUI and the window
ImGui_ImplSdlGL3_Init(App->m_window->GetWindow());

return ret;
}

bool ModuleEditor::Start()
{
	//ImGui_ImplSdlGL3_NewFrame(App->m_window->GetWindow());

	App->m_renderer3D->FindViewPort(0)->m_active = false;

	singleViewPort = App->m_renderer3D->AddViewPort(float2(0, 0), float2(100, 100), App->m_camera->GetDefaultCam());
	multipleViewPorts[0] = App->m_renderer3D->AddViewPort(float2(0, 0), float2(100, 100), App->m_camera->GetDefaultCam());
	multipleViewPorts[1] = App->m_renderer3D->AddViewPort(float2(0, 0), float2(100, 100), App->m_camera->GetTopCam());
	fullScreenViewPort = App->m_renderer3D->AddViewPort(float2(0, 0), float2(100, 100), App->m_camera->GetDefaultCam());

	OnScreenResize(App->m_window->GetWindowSize().x, App->m_window->GetWindowSize().y);
	SwitchViewPorts();

	return true;
}

// Called every draw update
UpdateStatus ModuleEditor::PreUpdate()
{
	UpdateStatus ret = UPDATE_CONTINUE;
	ImGui_ImplSdlGL3_NewFrame(App->m_window->GetWindow());
	if (Time.PlayMode != Play::Play)
	{

		ImGuiIO IO = ImGui::GetIO();
		App->m_input->ignoreMouse = IO.WantCaptureMouse;
		App->m_input->ignoreKeyboard = (IO.WantCaptureKeyboard || IO.WantTextInput);
	}
	else
	{
		App->m_input->ignoreKeyboard = false;
		App->m_input->ignoreMouse = false;
	}
	return ret;
}

UpdateStatus ModuleEditor::Update()
{
	UpdateStatus ret = UPDATE_CONTINUE;
	if (Time.PlayMode != Play::Play)
	{
		if (App->m_input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_DOWN || App->m_input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_DOWN)
		{
			ViewPort* port = App->m_renderer3D->HoveringViewPort();
			if (port != nullptr)
			{
				App->m_camera->SetMovingCamera(port->m_camera);
			}
		}

		SelectByViewPort();

		if (App->m_input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
		{
			multipleViews = !multipleViews;
			SwitchViewPorts();
		}
	}
	return ret;
}

UpdateStatus ModuleEditor::PostUpdate()
{
	UpdateStatus ret = UPDATE_CONTINUE;
	if (Time.PlayMode != Play::Play)
	{
		if (IsOpenTestWindow)
		{
			ImGui::ShowTestWindow();
		}

		ret = MenuBar();
		Editor();
		//Console();
		//PlayButtons();
		Outliner();
		AttributeWindow();
		//SaveLoadPopups();
	}
	return ret;
}

void ModuleEditor::OnPlay()
{
	SwitchViewPorts();
}

void ModuleEditor::OnStop()
{
	SwitchViewPorts();
}

// Called before quitting
bool ModuleEditor::CleanUp()
{
	ImGui_ImplSdlGL3_Shutdown();

	return true;
}


// ---- Each ViewPort UI -------------------------------------------------------------------
void ModuleEditor::Render(const ViewPort & port)
{
	if (Time.PlayMode != Play::Play)
	{
		if (port.m_withUI)
		{
			//Here we put the UI we'll draw for each viewport, since Render is called one time for each port that's m_active
			ViewPortUI(port);

			if (false && showPlane)
			{
				P_Plane p(0, 0, 0, 1);
				p.axis = true;
				p.Render();
			}
		}
	}
}



void ModuleEditor::OnScreenResize(int width, int heigth)
{
	screenW = width;
	screenH = heigth;
	viewPortMax.x = screenW-330;
	viewPortMax.y = screenH;
	viewPortMin.x = 350;
	viewPortMin.y = 20;

	//Setting the single ViewPort data
	ViewPort* port = App->m_renderer3D->FindViewPort(singleViewPort);
	port->m_pos = viewPortMin;
	port->m_size.x = viewPortMax.x - viewPortMin.x;
	port->m_size.y = viewPortMax.y - viewPortMin.y;

	//Setting the multiple ViewPort data
	float2 m_size((viewPortMax.x - viewPortMin.x), (viewPortMax.y - viewPortMin.y) / 2);
	port = App->m_renderer3D->FindViewPort(multipleViewPorts[0]);
	port->m_pos = viewPortMin;
	port->m_size = m_size;

	port = App->m_renderer3D->FindViewPort(multipleViewPorts[1]);
	port->m_pos = viewPortMin;
	port->m_pos.y += m_size.y;
	port->m_size = m_size;

	port = App->m_renderer3D->FindViewPort(fullScreenViewPort);
	port->m_size.x = screenW;
	port->m_size.y = screenH;
}

void ModuleEditor::HandleInput(SDL_Event* event)
{
	ImGui_ImplSdlGL3_ProcessEvent(event);
}


// ---- UI with IMGUI ViewPort UI -------------------------------------------------------------------

UpdateStatus ModuleEditor::MenuBar()
{
	UpdateStatus ret = UPDATE_CONTINUE;

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::Checkbox("Multiple Views", &multipleViews))
			{
				SwitchViewPorts();
			}
			ImGui::Checkbox("ImGui TestBox", &IsOpenTestWindow);
			ImGui::Checkbox("InGame Plane", &showPlane);
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Documentation"))
		{
			if (ImGui::MenuItem("MathGeoLib"))
			{
				App->OpenBrowser("http://clb.demon.fi/MathGeoLib/nightly/reference.html");
			}
			if (ImGui::MenuItem("ImGui"))
			{
				App->OpenBrowser("https://github.com/ocornut/imgui");
			}
			if (ImGui::MenuItem("Bullet"))
			{
				App->OpenBrowser("http://bulletphysics.org/Bullet/BulletFull/annotated.html");
			}
			if (ImGui::MenuItem("SDL"))
			{
				App->OpenBrowser("https://wiki.libsdl.org/APIByCategory");
			}

			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
	return ret;
}

void ModuleEditor::Editor()
{

		ImGui::SetNextWindowPos(ImVec2(0, ((screenH - 20) / 4) * 3));
		ImGui::SetNextWindowSize(ImVec2(350, (screenH - 20)/4 + 20));

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

		ImGui::Begin("Editor", 0, flags);

		if (ImGui::CollapsingHeader("Application"))
		{
			ImGui::Text("Time since startup: %f", Time.AppRuntime);
			ImGui::Text("Game Time: %f", Time.GameRuntime);

			ImGui::InputInt("Max Framerate:", &App->m_maxFps, 15);
			char tmp[256];
			sprintf(tmp, "Framerate: %i", int(App->m_framerate[EDITOR_FRAME_SAMPLES - 1]));
			ImGui::PlotHistogram("##Framerate:", App->m_framerate, EDITOR_FRAME_SAMPLES - 1, 0, tmp, 0.0f, 100.0f, ImVec2(310, 100));

			char tmp2[256];
			sprintf(tmp2, "Ms: %i", int(App->m_msFrame[EDITOR_FRAME_SAMPLES - 1] * 1000));
			ImGui::PlotHistogram("##ms", App->m_msFrame, EDITOR_FRAME_SAMPLES - 1, 0, tmp2, 0.0f, 0.07f, ImVec2(310, 100));
		}
		
		if (ImGui::CollapsingHeader("Camera##CameraModule"))
		{
			ImGui::Text("Camera speed");
			ImGui::DragFloat("##camSpeed", &App->m_camera->camSpeed, 0.1f);
			ImGui::Text("Sprint speed multiplier");
			ImGui::DragFloat("##camsprint", &App->m_camera->camSprintMultiplier, 0.1f);

		}

		if (ImGui::CollapsingHeader("Timers##ReadingTimers"))
		{
			std::vector<std::pair<std::string, float>> timers = App->m_timers->GetLastReads();
			if (timers.empty() == false)
			{
				char lastLetter = '0';
				for (std::vector<std::pair<std::string, float>>::iterator it = timers.begin(); it != timers.end(); it++)
				{
					if (it->first.data()[0] != lastLetter)
					{
						lastLetter = it->first.data()[0];
						ImGui::Separator();
					}
					ImGui::Text("%*s: %*0.3f ms", 25, it->first.data(), 5,it->second);
				}
			}
			else
			{
				ImGui::Text("No timers initialized");
			}
		}
		ImGui::End();
}


void ModuleEditor::SwitchViewPorts()
{
	if (Time.PlayMode == Play::Play)
	{
		App->m_renderer3D->FindViewPort(fullScreenViewPort)->m_active = true;
		App->m_renderer3D->FindViewPort(singleViewPort)->m_active = false;
		for (int n = 0; n < 2; n++)
		{
			App->m_renderer3D->FindViewPort(multipleViewPorts[n])->m_active = false;
		}
	}
	else
	{
		App->m_renderer3D->FindViewPort(fullScreenViewPort)->m_active = false;
		App->m_renderer3D->FindViewPort(singleViewPort)->m_active = !multipleViews;
		for (int n = 0; n < 2; n++)
		{
			App->m_renderer3D->FindViewPort(multipleViewPorts[n])->m_active = multipleViews;
		}
	}
}

void ModuleEditor::UnselectGameObject(GameObject * go)
{
	if (selectedGameObject == go)
	{
		go->Unselect();
		selectedGameObject = nullptr;
	}
}

void ModuleEditor::ViewPortUI(const ViewPort& port)
{
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize;

	ImGui::SetNextWindowPos(ImVec2(port.m_pos.x, port.m_pos.y));
	ImGui::SetNextWindowSize(ImVec2(port.m_size.x, 0));

	char tmp[256];
	sprintf(tmp, "ViewPortMenu##%i", port.m_ID);

	ImGui::Begin(tmp, 0, flags);
	if (ImGui::BeginMenuBar())
	{
		sprintf(tmp, "Display##ViewPort%i", port.m_ID);
		if (ImGui::BeginMenu(tmp))
		{
			ViewPort* editPort = App->m_renderer3D->FindViewPort(port.m_ID);
			ImGui::Checkbox("Wired", &editPort->m_useOnlyWires);
			ImGui::Checkbox("Lightning", &editPort->m_useLighting);
			ImGui::Checkbox("Render Heightmap", &editPort->m_renderHeightMap);
			ImGui::Checkbox("Single sided faces", &editPort->m_useSingleSidedFaces);
			ImGui::Checkbox("Render Terrain", &editPort->m_renderTerrain);
			ImGui::Checkbox("Render Terrain collision", &editPort->m_renderTerrainCollisions);
			ImGui::Checkbox("Render chunk borders", &editPort->m_renderChunkBorders);
			ImGui::Checkbox("Render Bounding boxes", &editPort->m_renderBoundingBoxes);

			ImGui::EndMenu();
		}
		sprintf(tmp, "Camera##ViewPort%i", port.m_ID);
		if (ImGui::BeginMenu(tmp))
		{
			if (ImGui::BeginMenu("Current Camera"))
			{
				ImGui::Text("Name:");
				ImGui::Text(port.m_camera->object->name);
				ImGui::Separator();
				ImGui::NewLine();
				if (ImGui::MenuItem("Switch view type"))
				{
					App->m_renderer3D->FindViewPort(port.m_ID)->m_camera->SwitchViewType();
				}
				if (ImGui::Button("Select m_active m_camera"))
				{
					SelectGameObject(port.m_camera->object);
				}
				ImGui::EndMenu();
			}
			ImGui::Separator();
			std::multimap<Component::Type, Component*>::iterator comp = App->m_goManager->components.find(Component::Type::C_camera);
			for (; comp != App->m_goManager->components.end() && comp->first == Component::Type::C_camera; comp++)
			{
				Camera* cam = (Camera*)&*comp->second;
				if (ImGui::MenuItem(cam->object->name))
				{
					App->m_renderer3D->FindViewPort(port.m_ID)->m_camera = cam;
					int a = 0;
				}
			}
			ImGui::EndMenu();
		}
		sprintf(tmp, "Switch View Type:##%i", port.m_ID);

		ImGui::EndMenuBar();
	}
	ImGui::End();
}

void ModuleEditor::AttributeWindow()
{
	ImGui::SetNextWindowPos(ImVec2(screenW - 330, 20.0f));
	ImGui::SetNextWindowSize(ImVec2(330, (screenH - 20) / 3 * 2));

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

	ImGui::Begin("Attribute Editor", 0, flags);
	if (selectedGameObject)
	{
		selectedGameObject->DrawOnEditor();
		ImGui::Separator();
		if (selectedGameObject->HasComponent(Component::Type::C_transform))
		{
			if (ImGui::Button("Look at"))
			{
				float3 toLook = selectedGameObject->GetTransform()->GetGlobalPos();
				App->m_camera->LookAt(float3(toLook.x, toLook.y, toLook.z));
			}
			ImGui::NewLine();
			ImGui::Text("Danger Zone:");
			if (ImGui::Button("Delete##DeleteGO"))
			{
				App->m_goManager->DeleteGameObject(selectedGameObject);
				selectedGameObject = nullptr;
			}
		}
	}
	ImGui::End();
}

void ModuleEditor::SelectByViewPort()
{
	if (App->m_input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_DOWN)
	{
		ViewPort* port = nullptr;
		float2 portPos = App->m_renderer3D->ScreenToViewPort(float2(App->m_input->GetMouseX(), App->m_input->GetMouseY()), &port);
		//Checking the click was made on a port
		if (port != nullptr)
		{
			//Normalizing the mouse position in port to [-1,1]
			portPos.x = portPos.x / (port->m_size.x / 2) - 1;
			portPos.y = portPos.y / (port->m_size.y / 2) - 1;
			//Generating the LineSegment we'll check for collisions
			selectRay = port->m_camera->GetFrustum()->UnProjectLineSegment(portPos.x, -portPos.y);

			GameObject* out_go = NULL;

			if (App->m_goManager->RayCast(selectRay, &out_go, &out_pos, &out_normal, false))
			{
				SelectGameObject(out_go);
			}
			else
			{
				SelectGameObject(nullptr);
			}

		}
	}
}

void ModuleEditor::SelectGameObject(GameObject* node)
{
	if (selectedGameObject)
	{
		selectedGameObject->Unselect();
	}
	if (node)
	{
		node->Select(renderNormals);
	}
	selectedGameObject = node;
}

void ModuleEditor::Outliner()
{
	ImGui::SetNextWindowPos(ImVec2(screenW - 330.f, (screenH - 20)/3.f*2+20));
	ImGui::SetNextWindowSize(ImVec2(330.f, (screenH - 20) / 3.f));

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

	ImGui::Begin("Outliner", 0, flags);

	std::vector<GameObject*>::const_iterator node = App->m_goManager->GetRoot()->childs.begin();
	while (node != App->m_goManager->GetRoot()->childs.end())
	{
		SceneTreeGameObject((*node));
		node++;
	}

	ImGui::End();
}

void ModuleEditor::SceneTreeGameObject(GameObject* node)
{
	if (node->HiddenFromOutliner() == false)
	{
		ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		if (selectedGameObject == node)
		{
			node_flags += ImGuiTreeNodeFlags_Selected;
		}
		if (node->childs.empty())
		{
			node_flags += ImGuiTreeNodeFlags_Leaf;
		}
		char name[256];
		sprintf(name, "%s##%llu", node->GetName(), node->GetUID());

		if (ImGui::TreeNodeEx(name, node_flags))
		{
			if (ImGui::IsItemClicked())
			{
				SelectGameObject(node);
			}

			std::vector<GameObject*>::iterator it = node->childs.begin();
			while (it != node->childs.end())
			{
				SceneTreeGameObject((*it));
				it++;
			}
			ImGui::TreePop();
		}
	}
}