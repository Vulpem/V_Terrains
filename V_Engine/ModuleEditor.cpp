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

#include "Imgui/imgui_impl_sdl_gl3.h"
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
	//Linking ImGUI and the m_window
	ImGui_ImplSdlGL3_Init(App->m_window->GetWindow());

	return ret;
}

void ModuleEditor::Start()
{
	ImGui_ImplSdlGL3_NewFrame(App->m_window->GetWindow());

	//Initializing the strings used to test the editor
	strcpy(m_toImport, "");

	m_selectedGameObject = nullptr;

	App->m_renderer3D->FindViewPort(0)->m_active = false;

	m_singleViewport = App->m_renderer3D->AddViewPort(float2(0, 0), float2(100, 100), App->m_camera->GetDefaultCam());
	m_multipleViewports[0] = App->m_renderer3D->AddViewPort(float2(0, 0), float2(100, 100), App->m_camera->GetDefaultCam());
	m_multipleViewports[1] = App->m_renderer3D->AddViewPort(float2(0, 0), float2(100, 100), App->m_camera->GetTopCam());
	m_multipleViewports[2] = App->m_renderer3D->AddViewPort(float2(0, 0), float2(100, 100), App->m_camera->GetRightCam());
	m_multipleViewports[3] = App->m_renderer3D->AddViewPort(float2(0, 0), float2(100, 100), App->m_camera->GetFrontCam());

	OnScreenResize(App->m_window->GetWindowSize().x, App->m_window->GetWindowSize().y);
	SwitchViewPorts();

	strcpy(m_sceneName, "");
}

// Called every draw update
UpdateStatus ModuleEditor::PreUpdate()
{
	UpdateStatus ret = UpdateStatus::Continue;

	ImGui_ImplSdlGL3_NewFrame(App->m_window->GetWindow());

	ImGuiIO IO = ImGui::GetIO();
	App->m_input->ignoreMouse = IO.WantCaptureMouse;

	if (IO.WantCaptureKeyboard || IO.WantTextInput)
	{
		App->m_input->ignoreKeyboard = true;
	}
	else
	{
		App->m_input->ignoreKeyboard = false;
	}

	return ret;
}

UpdateStatus ModuleEditor::Update()
{
	UpdateStatus ret = UPDATE_CONTINUE;

	if (App->m_input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_DOWN || App->m_input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_DOWN)
	{
		ViewPort* port = App->m_renderer3D->HoveringViewPort();
		if (port != nullptr)
		{
			App->m_camera->SetMovingCamera(port->camera);
		}
	}

	SelectByViewPort();

	if (App->m_input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
	{
		m_multipleViews = !m_multipleViews;
		SwitchViewPorts();
	}

	return ret;
}

UpdateStatus ModuleEditor::PostUpdate()
{
	UpdateStatus ret = UPDATE_CONTINUE;

	if (m_isTestWindowOpen)
	{
		ImGui::ShowTestWindow(&m_isTestWindowOpen);
	}

	ret = MenuBar();
	Editor();
	Console();
	PlayButtons();
	Outliner();
	AttributeWindow();
	SaveLoadPopups();

	return ret;
}

// Called before quitting
void ModuleEditor::CleanUp()
{
	ClearConsole();

	ImGui_ImplSdlGL3_Shutdown();

	return true;
}


// ---- Each ViewPort UI -------------------------------------------------------------------
void ModuleEditor::Render(const ViewPort & port) const
{
	if (port.withUI)
	{
		//Here we put the UI we'll draw for each viewport, since Render is called one time for each port that's active
		ViewPortUI(port);

		App->m_renderer3D->DrawLine(selectRay.a, selectRay.b, float4(1.0f, 1.0f, 1.0f, 1.0f));

		App->m_renderer3D->DrawLocator(m_out_pos, float4(0.75f, 0.75f, 0.75f, 1));
		App->m_renderer3D->DrawLine(m_out_pos, m_out_pos + m_out_normal * 2, float4(1, 1, 0, 1));

		if (m_showPlane)
		{
			P_Plane p(0, 0, 0, 1);
			p.axis = true;
			p.Render();
		}
	}
}



void ModuleEditor::OnScreenResize(int width, int heigth)
{
	screenW = width;
	screenH = heigth;
	viewPortMax.x = screenW - 330;
	viewPortMax.y = screenH - 200;
	viewPortMin.x = 300;
	viewPortMin.y = 20;

	//Setting the single ViewPort data
	ViewPort* port = App->m_renderer3D->FindViewPort(m_singleViewport);
	port->m_pos = viewPortMin;
	port->m_size.x = viewPortMax.x - viewPortMin.x;
	port->m_size.y = viewPortMax.y - viewPortMin.y;

	//Setting the multiple ViewPort data
	float2 m_size((viewPortMax.x - viewPortMin.x) / 2, (viewPortMax.y - viewPortMin.y) / 2);
	port = App->m_renderer3D->FindViewPort(m_multipleViewports[0]);
	port->m_pos = viewPortMin;
	port->m_size = m_size;

	port = App->m_renderer3D->FindViewPort(m_multipleViewports[1]);
	port->m_pos = viewPortMin;
	port->m_pos.x += m_size.x;
	port->m_size = m_size;

	port = App->m_renderer3D->FindViewPort(m_multipleViewports[2]);
	port->m_pos = viewPortMin;
	port->m_pos.y += m_size.y;
	port->m_size = m_size;

	port = App->m_renderer3D->FindViewPort(m_multipleViewports[3]);
	port->m_pos = viewPortMin;
	port->m_pos.x += m_size.x;
	port->m_pos.y += m_size.y;
	port->m_size = m_size;
}

void ModuleEditor::HandleInput(SDL_Event* event)
{
	ImGui_ImplSdlGL3_ProcessEvent(event);
}

void ModuleEditor::Log(const char* m_input)
{
	m_buffer.append(m_input);
	m_scrollToBottom = true;
}

void ModuleEditor::ClearConsole()
{
	m_buffer.clear();
	m_scrollToBottom = true;
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



// ---- UI with IMGUI ViewPort UI -------------------------------------------------------------------

UpdateStatus ModuleEditor::MenuBar()
{
	UpdateStatus ret = UpdateStatus::Continue;

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New Scene##NewMenuBar"))
			{
				m_wantNew = true;
			}
			if (ImGui::MenuItem("Save Scene##SaveMenuBar"))
			{
				m_wantToSave = true;
			}
			if (ImGui::MenuItem("Load Scene##LoadMenuBar"))
			{
				m_wantToLoad = true;
			}
			if (ImGui::MenuItem("ClearConsole"))
			{
				ClearConsole();
			}
			if (ImGui::MenuItem("Quit"))
			{
				ret = UpdateStatus::Stop;
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Import"))
		{
			ImGui::Checkbox("Auto Refresh", &App->m_resourceManager->m_autoRefresh);
			if (App->m_resourceManager->m_autoRefresh)
			{
				ImGui::Text("Auto Refresh delay(seconds):");
				ImGui::DragInt("##autoRefreshDelay", &App->m_resourceManager->m_refreshDelay, 1.0f, 1, 600);
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Refresh Assets"))
			{
				App->m_resourceManager->Refresh();
			}

			ImGui::NewLine();
			ImGui::NewLine();
			ImGui::Separator();
			if (ImGui::MenuItem("Reimport All Assets"))
			{
				App->m_resourceManager->ReimportAll();
			}
			ImGui::Separator();
			ImGui::Text("It may take some time.\nAlso, it may cause\nsome problems if\nthere are already\nassets loaded.\nRecommended to use refresh\nwhen possible");
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::Checkbox("Multiple Views", &m_multipleViews))
			{
				SwitchViewPorts();
			}
			ImGui::Checkbox("Edit default shaders", &m_openShaderEditor);
			ImGui::Checkbox("ImGui TestBox", &m_isTestWindowOpen);
			ImGui::Checkbox("InGame Plane", &m_showPlane);
			ImGui::Checkbox("QuadTree", &App->m_goManager->drawQuadTree);
			if (ImGui::Checkbox("Render Normals", &m_renderNormals))
			{
				SelectGameObject(m_selectedGameObject);
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Create"))
		{
			if (ImGui::MenuItem("Empty##CreateEmpty") == true)
			{
				App->m_goManager->CreateEmpty();
			}
			if (ImGui::MenuItem("Camera##CreateEmptyCam") == true)
			{
				App->m_goManager->CreateCamera();
			}
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

void ModuleEditor::PlayButtons()
{
	ImGui::SetNextWindowPos(ImVec2(0.0f, 20.0f));
	ImGui::SetNextWindowSize(ImVec2(300.0f, 30.0f));

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;

	ImGui::Begin("PlayButtons", 0, ImVec2(500, 300), 0.8f, flags);

	if (Time.PlayMode == false)
	{
		if (ImGui::Button("Play##PlayButton"))
		{
			Time.PlayMode = true;
			Time.GameRuntime = 0.0f;
			App->m_goManager->SaveScene("temp");
		}
	}
	else
	{
		if (ImGui::Button("Pause##PauseButton"))
		{
			Time.Pause = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Stop##StopButton"))
		{
			Time.PlayMode = false;
			Time.Pause = false;
			Time.gdt = 0.0f;
			App->m_goManager->LoadScene("temp");
		}
	}
	ImGui::End();
}

void ModuleEditor::Editor()
{
	ImGui::SetNextWindowPos(ImVec2(screenW - 330, 20 + (screenH - 20) / 2));
	ImGui::SetNextWindowSize(ImVec2(330, (screenH - 20) / 2));

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

	ImGui::Begin("Editor", 0, ImVec2(500, 300), 0.8f, flags);

	if (ImGui::CollapsingHeader("Application"))
	{
		ImGui::Text("Time since startup: %f", Time.AppRuntime);
		ImGui::Text("Game Time: %f", Time.GameRuntime);

		ImGui::InputInt("Max Framerate:", &App->maxFPS, 15);
		char tmp[256];
		sprintf(tmp, "Framerate: %i", int(App->framerate[EDITOR_FRAME_SAMPLES - 1]));
		ImGui::PlotHistogram("##Framerate:", App->framerate, EDITOR_FRAME_SAMPLES - 1, 0, tmp, 0.0f, 100.0f, ImVec2(310, 100));

		char tmp2[256];
		sprintf(tmp2, "Ms: %i", int(App->ms_frame[EDITOR_FRAME_SAMPLES - 1] * 1000));
		ImGui::PlotHistogram("##ms", App->ms_frame, EDITOR_FRAME_SAMPLES - 1, 0, tmp2, 0.0f, 0.07f, ImVec2(310, 100));
	}

	if (ImGui::CollapsingHeader("Input"))
	{
		ImGui::LabelText("label", "MouseX: %i", App->m_input->GetMouseX());
		ImGui::LabelText("label", "MouseY: %i", App->m_input->GetMouseY());
	}

	if (ImGui::CollapsingHeader("Camera##CameraModule"))
	{
		ImGui::Text("Position");
		ImGui::Text("Camera speed");
		ImGui::DragFloat("##camSpeed", &App->m_camera->camSpeed, 0.1f);
		ImGui::Text("Sprint speed multiplier");
		ImGui::DragFloat("##camsprint", &App->m_camera->camSprintMultiplier, 0.1f);

	}

	if (ImGui::CollapsingHeader("Render"))
	{
		ImGui::Text("Global light direction");
		ImGui::DragFloat3("##GlobalLightDirection", App->m_renderer3D->sunDirection.ptr(), 0.1f, -1.0f, 1.0f);

		ImGui::Text("Ambient light intensity");
		ImGui::DragFloat("##GlobalLightDirection", &App->m_renderer3D->ambientLight.x, 0.1f, -1.0f, 1.0f);

		if (ImGui::TreeNode("Lights"))
		{
			for (int nLight = 0; nLight < MAX_LIGHTS; nLight++)
			{
				char lightName[46];
				sprintf(lightName, "Light %i", nLight);
				bool on = App->m_renderer3D->lights[nLight].on;
				ImGui::Checkbox(lightName, &on);

				if (on != App->m_renderer3D->lights[nLight].on)
				{
					App->m_renderer3D->lights[nLight].Active(on);
				}
				if (App->m_renderer3D->lights[nLight].on == true)
				{

					sprintf(lightName, "Expand##Light_%i", nLight);
					ImGui::SameLine();
					if (ImGui::TreeNode(lightName))
					{
						char tmp[46];
						sprintf(tmp, "X##light_%i", nLight);
						ImGui::DragFloat(tmp, &App->m_renderer3D->lights[nLight].position.x, 1.0f);
						sprintf(tmp, "Y##light_%i", nLight);
						ImGui::DragFloat(tmp, &App->m_renderer3D->lights[nLight].position.y, 1.0f);
						sprintf(tmp, "Z##light_%i", nLight);
						ImGui::DragFloat(tmp, &App->m_renderer3D->lights[nLight].position.z, 1.0f);
						ImGui::TreePop();
					}
				}
			}
			ImGui::TreePop();
		}
	}

	if (ImGui::CollapsingHeader("Resource Manager"))
	{
		ImGui::Text("Loaded resources:");
		const std::vector<Resource*> res = App->m_resourceManager->ReadLoadedResources();
		if (res.m_size() > 0)
		{
			std::vector<Resource*>::const_iterator it = res.begin();
			Component::Type lastType = Component::Type::C_None;
			char name[256];
			for (; it != res.end(); it++)
			{
				if (lastType != (*it)->GetType())
				{
					lastType = (*it)->GetType();
					ImGui::Separator();
					switch (lastType)
					{
					case (Component::C_GO):
					{
						ImGui::Text("GameObjects:"); break;
					}
					case (Component::C_material):
					{
						ImGui::Text("Materials:"); break;
					}
					case (Component::C_mesh):
					{
						ImGui::Text("Meshes:"); break;
					}
					case (Component::C_Texture):
					{
						ImGui::Text("Textures:"); break;
					}
					}
				}
				sprintf(name, "%s", (*it)->name.data());
				if (ImGui::TreeNode(name))
				{
					ImGui::Text("N references: %u", (*it)->nReferences);
					ImGui::TreePop();
				}
			}
		}
	}

	if (ImGui::CollapsingHeader("Timers##ReadingTimers"))
	{
		std::vector<std::pair<std::string, float>> timers = App->timers->GetLastReads();
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
				ImGui::Text("%*s: %*0.3f ms", 25, it->first.data(), 5, it->second);
			}
		}
		else
		{
			ImGui::Text("No timers initialized");
		}
	}

	if (ImGui::CollapsingHeader("Tests"))
	{
		ImGui::InputText("##consoleTest", testConsoleInput, 60);
		ImGui::SameLine();
		if (ImGui::Button("TestConsole"))
		{
			LOG(testConsoleInput);
		}
	}
	ImGui::End();
}

void ModuleEditor::Console()
{
	ImGui::SetNextWindowPos(ImVec2(0.0f, screenH - 200.0f));
	ImGui::SetNextWindowSize(ImVec2(screenW - 330.0f, 200.0f));

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

	ImGui::Begin("Console", 0, ImVec2(500, 300), 0.8f, flags);

	ImColor col = ImColor(0.6f, 0.6f, 1.0f, 1.0f);
	ImGui::PushStyleColor(0, col);

	ImGui::TextUnformatted(m_buffer.begin());
	ImGui::PopStyleColor();

	if (m_scrollToBottom)
		ImGui::SetScrollHere(1.0f);

	m_scrollToBottom = false;

	ImGui::End();
}

void ModuleEditor::Outliner()
{
	ImGui::SetNextWindowPos(ImVec2(0.0f, 50.0f));
	ImGui::SetNextWindowSize(ImVec2(300.0f, screenH - 250.0f));

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

	ImGui::Begin("Outliner", 0, ImVec2(500, 300), 0.8f, flags);

	std::vector<GameObject*>::const_iterator node = App->m_goManager->GetRoot()->m_childs.begin();
	while (node != App->m_goManager->GetRoot()->m_childs.end())
	{
		SceneTreeGameObject((*node));
		node++;
	}

	ImGui::End();
}

void ModuleEditor::AttributeWindow()
{
	ImGui::SetNextWindowPos(ImVec2(screenW - 330, 20.0f));
	ImGui::SetNextWindowSize(ImVec2(330, (screenH - 20) / 2));

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

	ImGui::Begin("Attribute Editor", 0, flags);
	if (m_selectedGameObject)
	{
		m_selectedGameObject->DrawOnEditor();
		ImGui::Separator();
		if (m_selectedGameObject->HasComponent(Component::Type::C_transform))
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




void ModuleEditor::SwitchViewPorts()
{
	App->m_renderer3D->FindViewPort(m_singleViewport)->m_active = !m_multipleViews;
	for (int n = 0; n < 4; n++)
	{
		App->m_renderer3D->FindViewPort(m_multipleViewports[n])->m_active = m_multipleViews;
	}
}

void ModuleEditor::ViewPortUI(const ViewPort & port) const
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
			ImGui::Checkbox("Wired", &editPort->useOnlyWires);
			ImGui::Checkbox("Lightning", &editPort->useLighting);
			ImGui::Checkbox("Textured", &editPort->useMaterials);
			ImGui::Checkbox("Single sided faces", &editPort->useSingleSidedFaces);
			ImGui::EndMenu();
		}
		sprintf(tmp, "Camera##ViewPort%i", port.m_ID);
		if (ImGui::BeginMenu(tmp))
		{
			if (ImGui::BeginMenu("Current Camera"))
			{
				ImGui::Text("Name:");
				ImGui::Text(port.camera->object->name);
				ImGui::Separator();
				ImGui::NewLine();
				if (ImGui::MenuItem("Switch view type"))
				{
					App->m_renderer3D->FindViewPort(port.m_ID)->camera->SwitchViewType();
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
					App->m_renderer3D->FindViewPort(port.m_ID)->camera = cam;
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

bool ModuleEditor::SaveLoadPopups()
{
	ImGui::SetNextWindowSize(ImVec2(300, 120));
	if (ImGui::BeginPopupModal("New scene"))
	{
		m_selectedGameObject = nullptr;
		bool close = false;
		ImGui::Text("Save current scene?");
		if (ImGui::Button("Yes##saveCurrentButton"))
		{
			m_wantToSave = true;
			clearAfterSave = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("No##NotSaveCurrentButton"))
		{
			App->m_goManager->ClearScene();
			close = true;
		}
		ImGui::SameLine();
		if (close || ImGui::Button("Cancel##CancelSaveCurrentButton"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}


	ImGui::SetNextWindowSize(ImVec2(300, 120));
	if (ImGui::BeginPopupModal("Save scene"))
	{
		bool close = false;
		ImGui::Text("Scene name:");
		ImGui::InputText("##saveSceneInputText", sceneName, 256);
		if (ImGui::Button("Save##saveButton") && sceneName[0] != '\0')
		{
			App->m_goManager->SaveScene(sceneName);
			close = true;
			if (clearAfterSave)
			{
				App->m_goManager->ClearScene();
			}
		}
		ImGui::SameLine();
		if (close || ImGui::Button("Cancel##cancelSaveScene"))
		{
			strcpy(sceneName, "");
			clearAfterSave = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	ImGui::SetNextWindowSize(ImVec2(300, 120));
	if (ImGui::BeginPopupModal("Load Scene"))
	{
		m_selectedGameObject = nullptr;
		ImGui::Text("Scene name:");
		ImGui::InputText("##saveSceneInputText", sceneName, 256);
		bool close = false;
		if (ImGui::Button("Load##loadButton") && sceneName[0] != '\0')
		{
			App->m_goManager->LoadScene(sceneName);
			close = true;
		}
		ImGui::SameLine();
		if (close || ImGui::Button("Cancel##cancelLoadScene"))
		{
			strcpy(sceneName, "");
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if (m_wantNew)
	{
		ImGui::OpenPopup("New scene");
		m_wantNew = false;
	}
	if (m_wantToSave)
	{
		ImGui::OpenPopup("Save scene");
		m_wantToSave = false;
	}
	if (m_wantToLoad)
	{
		ImGui::OpenPopup("Load Scene");
		m_wantToLoad = false;
	}

	return false;
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
			selectRay = port->camera->GetFrustum()->UnProjectLineSegment(portPos.x, -portPos.y);

			GameObject* out_go = NULL;

			if (App->m_goManager->RayCast(selectRay, &out_go, &m_out_pos, &m_out_normal, false))
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