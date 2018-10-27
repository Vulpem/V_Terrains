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

void ModuleEditor::Start()
{
	//ImGui_ImplSdlGL3_NewFrame(App->m_window->GetWindow());

	App->m_renderer3D->FindViewPort(0)->m_active = false;

	m_singleViewportIndex = App->m_renderer3D->AddViewPort(float2(0, 0), float2(100, 100), App->m_camera->GetDefaultCam());
	m_multipleViewportsIndex[0] = App->m_renderer3D->AddViewPort(float2(0, 0), float2(100, 100), App->m_camera->GetDefaultCam());
	m_multipleViewportsIndex[1] = App->m_renderer3D->AddViewPort(float2(0, 0), float2(100, 100), App->m_camera->GetTopCam());
	m_fullScreenViewportIndex = App->m_renderer3D->AddViewPort(float2(0, 0), float2(100, 100), App->m_camera->GetDefaultCam());

	OnScreenResize(App->m_window->GetWindowSize().x, App->m_window->GetWindowSize().y);
	SwitchViewPorts();
}

// Called every draw update
UpdateStatus ModuleEditor::PreUpdate()
{
	ImGui_ImplSdlGL3_NewFrame(App->m_window->GetWindow());
	if (Time.PlayMode != Play::Play)
	{

		ImGuiIO IO = ImGui::GetIO();
		App->m_input->m_ignoreMouse = IO.WantCaptureMouse;
		App->m_input->m_ignoreKeyboard = (IO.WantCaptureKeyboard || IO.WantTextInput);
	}
	else
	{
		App->m_input->m_ignoreKeyboard = false;
		App->m_input->m_ignoreMouse = false;
	}
	return UpdateStatus::Continue;
}

UpdateStatus ModuleEditor::Update()
{
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
			m_multipleViewports = !m_multipleViewports;
			SwitchViewPorts();
		}
	}
	return UpdateStatus::Continue;
}

UpdateStatus ModuleEditor::PostUpdate()
{
	if (Time.PlayMode != Play::Play)
	{
		if (m_isTestWindowOpen)
		{
			ImGui::ShowTestWindow();
		}

		MenuBar();
		Editor();
		//Console();
		//PlayButtons();
		Outliner();
		AttributeWindow();
		//SaveLoadPopups();
	}
	return UpdateStatus::Continue;
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
void ModuleEditor::CleanUp()
{
	ImGui_ImplSdlGL3_Shutdown();
}


// ---- Each ViewPort UI -------------------------------------------------------------------
void ModuleEditor::Render(const ViewPort & port) const
{
	if (Time.PlayMode != Play::Play)
	{
		if (port.m_withUI)
		{
			//Here we put the UI we'll draw for each viewport, since Render is called one time for each port that's m_active
			ViewPortUI(port);

			if (false && m_show0Plane)
			{
				P_Plane p(0, 0, 0, 1);
				p.m_axis = true;
				p.Render();
			}
		}
	}
}



void ModuleEditor::OnScreenResize(int width, int heigth)
{
	m_screenW = width;
	m_screenH = heigth;
	m_viewPortMax.x = m_screenW-330;
	m_viewPortMax.y = m_screenH;
	m_viewPortMin.x = 350;
	m_viewPortMin.y = 20;

	//Setting the single ViewPort data
	ViewPort* port = App->m_renderer3D->FindViewPort(m_singleViewportIndex);
	port->m_pos = m_viewPortMin;
	port->m_size.x = m_viewPortMax.x - m_viewPortMin.x;
	port->m_size.y = m_viewPortMax.y - m_viewPortMin.y;

	//Setting the multiple ViewPort data
	float2 m_size((m_viewPortMax.x - m_viewPortMin.x), (m_viewPortMax.y - m_viewPortMin.y) / 2);
	port = App->m_renderer3D->FindViewPort(m_multipleViewportsIndex[0]);
	port->m_pos = m_viewPortMin;
	port->m_size = m_size;

	port = App->m_renderer3D->FindViewPort(m_multipleViewportsIndex[1]);
	port->m_pos = m_viewPortMin;
	port->m_pos.y += m_size.y;
	port->m_size = m_size;

	port = App->m_renderer3D->FindViewPort(m_fullScreenViewportIndex);
	port->m_size.x = m_screenW;
	port->m_size.y = m_screenH;
}

void ModuleEditor::HandleInput(SDL_Event* event)
{
	ImGui_ImplSdlGL3_ProcessEvent(event);
}


// ---- UI with IMGUI ViewPort UI -------------------------------------------------------------------

void ModuleEditor::MenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::Checkbox("Multiple Views", &m_multipleViewports))
			{
				SwitchViewPorts();
			}
			ImGui::Checkbox("ImGui TestBox", &m_isTestWindowOpen);
			ImGui::Checkbox("InGame Plane", &m_show0Plane);
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
}

void ModuleEditor::Editor()
{

		ImGui::SetNextWindowPos(ImVec2(0, ((m_screenH - 20) / 4) * 3));
		ImGui::SetNextWindowSize(ImVec2(350, (m_screenH - 20)/4 + 20));

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
			ImGui::DragFloat("##camSpeed", &App->m_camera->m_camSpeed, 0.1f);
			ImGui::Text("Sprint speed multiplier");
			ImGui::DragFloat("##camsprint", &App->m_camera->m_camSprintMultiplier, 0.1f);

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
		App->m_renderer3D->FindViewPort(m_fullScreenViewportIndex)->m_active = true;
		App->m_renderer3D->FindViewPort(m_singleViewportIndex)->m_active = false;
		for (int n = 0; n < 2; n++)
		{
			App->m_renderer3D->FindViewPort(m_multipleViewportsIndex[n])->m_active = false;
		}
	}
	else
	{
		App->m_renderer3D->FindViewPort(m_fullScreenViewportIndex)->m_active = false;
		App->m_renderer3D->FindViewPort(m_singleViewportIndex)->m_active = !m_multipleViewports;
		for (int n = 0; n < 2; n++)
		{
			App->m_renderer3D->FindViewPort(m_multipleViewportsIndex[n])->m_active = m_multipleViewports;
		}
	}
}

void ModuleEditor::UnselectGameObject(GameObject * go)
{
	if (m_selectedGameObject == go)
	{
		go->Unselect();
		m_selectedGameObject = nullptr;
	}
}

void ModuleEditor::ViewPortUI(const ViewPort& port) const
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
				ImGui::Text(port.m_camera->object->m_name);
				ImGui::Separator();
				ImGui::NewLine();
				if (ImGui::MenuItem("Switch view type"))
				{
					App->m_renderer3D->FindViewPort(port.m_ID)->m_camera->SwitchViewType();
				}
				//if (ImGui::Button("Select m_active m_camera"))
				//{
				//	SelectGameObject(port.m_camera->object);
				//}
				ImGui::EndMenu();
			}
			ImGui::Separator();
			std::multimap<ComponentType, Component*>::iterator comp = App->m_goManager->components.find(ComponentType::camera);
			for (; comp != App->m_goManager->components.end() && comp->first == ComponentType::camera; comp++)
			{
				Camera* cam = (Camera*)&*comp->second;
				if (ImGui::MenuItem(cam->object->m_name))
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
	ImGui::SetNextWindowPos(ImVec2(m_screenW - 330, 20.0f));
	ImGui::SetNextWindowSize(ImVec2(330, (m_screenH - 20) / 3 * 2));

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

	ImGui::Begin("Attribute Editor", 0, flags);
	if (m_selectedGameObject)
	{
		m_selectedGameObject->DrawOnEditor();
		ImGui::Separator();
		if (m_selectedGameObject->HasComponent(ComponentType::transform))
		{
			if (ImGui::Button("Look at"))
			{
				float3 toLook = m_selectedGameObject->GetTransform()->GetGlobalPos();
				App->m_camera->LookAt(float3(toLook.x, toLook.y, toLook.z));
			}
			ImGui::NewLine();
			ImGui::Text("Danger Zone:");
			if (ImGui::Button("Delete##DeleteGO"))
			{
				App->m_goManager->DeleteGameObject(m_selectedGameObject);
				m_selectedGameObject = nullptr;
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
			m_selectionRay = port->m_camera->GetFrustum()->UnProjectLineSegment(portPos.x, -portPos.y);

			GameObject* out_go = NULL;

			if (App->m_goManager->RayCast(m_selectionRay, &out_go, &m_selectedRayPos, &m_selectRayNormal, false))
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
	if (m_selectedGameObject)
	{
		m_selectedGameObject->Unselect();
	}
	if (node)
	{
		node->Select(m_renderNormals);
	}
	m_selectedGameObject = node;
}

void ModuleEditor::Outliner()
{
	ImGui::SetNextWindowPos(ImVec2(m_screenW - 330.f, (m_screenH - 20)/3.f*2+20));
	ImGui::SetNextWindowSize(ImVec2(330.f, (m_screenH - 20) / 3.f));

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

	ImGui::Begin("Outliner", 0, flags);

	std::vector<GameObject*>::const_iterator node = App->m_goManager->GetRoot()->m_childs.begin();
	while (node != App->m_goManager->GetRoot()->m_childs.end())
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
		if (m_selectedGameObject == node)
		{
			node_flags += ImGuiTreeNodeFlags_Selected;
		}
		if (node->m_childs.empty())
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

			std::vector<GameObject*>::iterator it = node->m_childs.begin();
			while (it != node->m_childs.end())
			{
				SceneTreeGameObject((*it));
				it++;
			}
			ImGui::TreePop();
		}
	}
}