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
#include "ModuleTerrainTests.h"
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
	LOG("Init editor gui with imgui lib version %s", ImGui::GetVersion());
	//Linking ImGUI and the m_window
	ImGui_ImplSdlGL3_Init(App->m_window->GetWindow());

	return true;
}

void ModuleEditor::Start()
{
	//ImGui_ImplSdlGL3_NewFrame(App->m_window->GetWindow());

	//Initializing the strings used to test the editor
	strcpy(m_toImport, "");

	m_selectedGameObject = nullptr;

	App->m_renderer3D->FindViewPort(0)->m_active = false;

	m_singleViewportID = App->m_renderer3D->AddViewPort(float2(0, 0), float2(100, 100), App->m_camera->GetDefaultCam());
	m_multipleViewportsIDs[0] = App->m_renderer3D->AddViewPort(float2(0, 0), float2(100, 100), App->m_camera->GetDefaultCam());
	m_multipleViewportsIDs[1] = App->m_renderer3D->AddViewPort(float2(0, 0), float2(100, 100), App->m_camera->GetTopCam());
	m_multipleViewportsIDs[2] = App->m_renderer3D->AddViewPort(float2(0, 0), float2(100, 100), App->m_camera->GetRightCam());
	m_multipleViewportsIDs[3] = App->m_renderer3D->AddViewPort(float2(0, 0), float2(100, 100), App->m_camera->GetFrontCam());

	OnScreenResize(App->m_window->GetWindowSize().x, App->m_window->GetWindowSize().y);
	SwitchViewPorts();

	strcpy(m_sceneName, "");
}

// Called every draw update
UpdateStatus ModuleEditor::PreUpdate()
{
	ImGui_ImplSdlGL3_NewFrame(App->m_window->GetWindow());

	ImGuiIO IO = ImGui::GetIO();
	App->m_input->m_ignoreMouse = IO.WantCaptureMouse;

	if (IO.WantCaptureKeyboard || IO.WantTextInput)
	{
		App->m_input->m_ignoreKeyboard = true;
	}
	else
	{
		App->m_input->m_ignoreKeyboard = false;
	}

	return UpdateStatus::Continue;
}

UpdateStatus ModuleEditor::Update()
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
		m_displayMultipleViews = !m_displayMultipleViews;
		SwitchViewPorts();
	}

	return UpdateStatus::Continue;
}

UpdateStatus ModuleEditor::PostUpdate()
{
	if (m_isTestWindowOpen)
	{
		ImGui::ShowTestWindow();
	}

	UpdateStatus ret = MenuBar();
	switch (m_multiWindowDisplay)
	{
	default: { Outliner(); break; }
	case 1: { App->m_terrain->DrawUI(); break; }
	case 2: { Editor(); break; }
	}
	Console();
	PlayButtons();
	AttributeWindow();
	SaveLoadPopups();

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

	return ret;
}

// Called before quitting
void ModuleEditor::CleanUp()
{
	ClearConsole();
	ImGui_ImplSdlGL3_Shutdown();
}

void ModuleEditor::OnPlay()
{
	SwitchViewPorts();
}

void ModuleEditor::OnStop()
{
	SwitchViewPorts();
}

// ---- Each ViewPort UI -------------------------------------------------------------------
void ModuleEditor::Render(const ViewPort & port) const
{
	if (port.m_withUI)
	{
		//Here we put the UI we'll draw for each viewport, since Render is called one time for each port that's active
		ViewPortUI(port);

		App->m_renderer3D->DrawLine(m_selectionRay.a, m_selectionRay.b, float4(1.0f, 1.0f, 1.0f, 1.0f));
		
		App->m_renderer3D->DrawLocator(m_selectedRayPos, float4(0.75f, 0.75f, 0.75f, 1));
		App->m_renderer3D->DrawLine(m_selectedRayPos, m_selectedRayPos + m_selectRayNormal * 2, float4(1, 1, 0, 1));

		if (m_show0Plane)
		{
			P_Plane p(0, 0, 0, 1);
			p.m_axis = true;
			p.Render();
		}
	}
}



void ModuleEditor::OnScreenResize(int width, int heigth)
{
	m_screenW = width;
	m_screenH = heigth;
	m_viewPortMax.x = m_screenW - 330;
	m_viewPortMax.y = m_screenH - 200;
	m_viewPortMin.x = 300;
	m_viewPortMin.y = 20;

	//Setting the single ViewPort data
	ViewPort* port = App->m_renderer3D->FindViewPort(m_singleViewportID);
	port->m_pos = m_viewPortMin;
	port->m_size.x = m_viewPortMax.x - m_viewPortMin.x;
	port->m_size.y = m_viewPortMax.y - m_viewPortMin.y;

	//Setting the multiple ViewPort data
	float2 m_size((m_viewPortMax.x - m_viewPortMin.x) / 2, (m_viewPortMax.y - m_viewPortMin.y) / 2);
	port = App->m_renderer3D->FindViewPort(m_multipleViewportsIDs[0]);
	port->m_pos = m_viewPortMin;
	port->m_size = m_size;

	port = App->m_renderer3D->FindViewPort(m_multipleViewportsIDs[1]);
	port->m_pos = m_viewPortMin;
	port->m_pos.x += m_size.x;
	port->m_size = m_size;

	port = App->m_renderer3D->FindViewPort(m_multipleViewportsIDs[2]);
	port->m_pos = m_viewPortMin;
	port->m_pos.y += m_size.y;
	port->m_size = m_size;

	port = App->m_renderer3D->FindViewPort(m_multipleViewportsIDs[3]);
	port->m_pos = m_viewPortMin;
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
	m_buffer.appendf(m_input);
	m_scrollToBottom = true;
}

void ModuleEditor::ClearConsole()
{
	m_buffer.clear();
	m_scrollToBottom = true;
}

void ModuleEditor::SceneTreeGameObject(GameObject* node)
{
	if (node->HiddenFromOutliner() == false || m_displayHiddenOutlinerGameobjects == true)
	{
		ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		if (m_selectedGameObject == node)
		{
			node_flags += ImGuiTreeNodeFlags_Selected;
		}
		if (node->GetTransform()->GetChilds().empty())
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

			std::vector<Transform*> childs = node->GetTransform()->GetChilds();
			for (auto child : childs)
			{
				SceneTreeGameObject(child->GetGameobject());
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
				m_wantNewScene = true;
			}
			if (ImGui::MenuItem("Save Scene##SaveMenuBar"))
			{
				m_wantToSaveScene = true;
			}
			if (ImGui::MenuItem("Load Scene##LoadMenuBar"))
			{
				m_wantToLoadScene = true;
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
				ImGui::DragInt("##autoRefreshDelay", &App->m_resourceManager->m_refreshInterval, 1.0f, 1, 600);
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
			if (ImGui::Checkbox("Multiple Views", &m_displayMultipleViews))
			{
				SwitchViewPorts();
			}
			//ImGui::Checkbox("Edit default shaders", &m_openShaderEditor);
			ImGui::Checkbox("ImGui TestBox", &m_isTestWindowOpen);
			ImGui::Checkbox("InGame Plane", &m_show0Plane);
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
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;

	if (ImGui::Begin("PlayButtons", 0, ImVec2(500, 300), 0.8f, flags))
	{
		ImGui::SetWindowPos(ImVec2(0.0f, 20.0f));
		ImGui::SetWindowSize(ImVec2(300.0f, 30.0f));
		if (Time.PlayMode == PlayMode::Stop)
		{
			if (ImGui::Button("Play##PlayButton"))
			{
				Time.PlayMode = PlayMode::DebugPlay;
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
				Time.PlayMode = PlayMode::Stop;
				Time.Pause = false;
				Time.gdt = 0.0f;
				App->m_goManager->LoadScene("temp");
			}
		}
		ImGui::SameLine();
		if (ImGui::BeginMenu("Window displayed:"))
		{
			if (ImGui::MenuItem("Outliner")) { m_multiWindowDisplay = 0; }
			if (ImGui::MenuItem("Terrain configuration")) { m_multiWindowDisplay = 1; }
			if (ImGui::MenuItem("Editor configuration")) { m_multiWindowDisplay = 2; }
			ImGui::EndMenu();
		}

		ImGui::End();
	}
}

void ModuleEditor::Editor()
{
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

	if (ImGui::Begin("Editor", 0, ImVec2(500, 300), 0.8f, flags))
	{
		ImGui::SetWindowPos(ImVec2(0.0f, 50.0f));
		ImGui::SetWindowSize(ImVec2(300.0f, m_screenH - 250.0f));

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

		if (ImGui::CollapsingHeader("Input"))
		{
			ImGui::LabelText("label", "MouseX: %i", App->m_input->GetMouseX());
			ImGui::LabelText("label", "MouseY: %i", App->m_input->GetMouseY());
		}

		if (ImGui::CollapsingHeader("Camera##CameraModule"))
		{
			ImGui::Text("Position");
			ImGui::Text("Camera speed");
			ImGui::DragFloat("##camSpeed", &App->m_camera->m_camSpeed, 0.1f);
			ImGui::Text("Sprint speed multiplier");
			ImGui::DragFloat("##camsprint", &App->m_camera->m_camSprintMultiplier, 0.1f);

		}

		if (ImGui::CollapsingHeader("Render"))
		{
			ImGui::Text("Global light direction");
			ImGui::DragFloat3("##GlobalLightDirection", App->m_renderer3D->m_sunDirection.ptr(), 0.1f, -1.0f, 1.0f);

			ImGui::Text("Ambient light intensity");
			ImGui::DragFloat("##GlobalLightDirection", &App->m_renderer3D->m_ambientLight.x, 0.1f, -1.0f, 1.0f);

			if (ImGui::TreeNode("Lights"))
			{
				for (int nLight = 0; nLight < MAX_LIGHTS; nLight++)
				{
					char lightName[46];
					sprintf(lightName, "Light %i", nLight);
					bool on = App->m_renderer3D->m_lights[nLight].m_on;
					ImGui::Checkbox(lightName, &on);

					if (on != App->m_renderer3D->m_lights[nLight].m_on)
					{
						App->m_renderer3D->m_lights[nLight].Active(on);
					}
					if (App->m_renderer3D->m_lights[nLight].m_on == true)
					{

						sprintf(lightName, "Expand##Light_%i", nLight);
						ImGui::SameLine();
						if (ImGui::TreeNode(lightName))
						{
							char tmp[46];
							sprintf(tmp, "X##light_%i", nLight);
							ImGui::DragFloat(tmp, &App->m_renderer3D->m_lights[nLight].m_position.x, 1.0f);
							sprintf(tmp, "Y##light_%i", nLight);
							ImGui::DragFloat(tmp, &App->m_renderer3D->m_lights[nLight].m_position.y, 1.0f);
							sprintf(tmp, "Z##light_%i", nLight);
							ImGui::DragFloat(tmp, &App->m_renderer3D->m_lights[nLight].m_position.z, 1.0f);
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
			if (res.size() > 0)
			{
				std::vector<Resource*>::const_iterator it = res.begin();
				ComponentType lastType = ComponentType::none;
				char name[256];
				for (; it != res.end(); it++)
				{
					if (lastType != (*it)->GetType())
					{
						lastType = (*it)->GetType();
						ImGui::Separator();
						switch (lastType)
						{
						case (ComponentType::GO):
						{
							ImGui::Text("GameObjects:"); break;
						}
						case (ComponentType::material):
						{
							ImGui::Text("Materials:"); break;
						}
						case (ComponentType::mesh):
						{
							ImGui::Text("Meshes:"); break;
						}
						case (ComponentType::texture):
						{
							ImGui::Text("Textures:"); break;
						}
						}
					}
					sprintf(name, "%s", (*it)->m_name.data());
					if (ImGui::TreeNode(name))
					{
						ImGui::Text("N references: %u", (*it)->m_numReferences);
						ImGui::TreePop();
					}
				}
			}
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
					ImGui::Text("%*s: %*0.3f ms", 25, it->first.data(), 5, it->second);
				}
			}
			else
			{
				ImGui::Text("No timers initialized");
			}
		}
		ImGui::End();
	}
}

void ModuleEditor::Console()
{
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

	if (ImGui::Begin("Console", 0, ImVec2(m_screenW - 330.0f, 200.0f), 0.8f, flags))
	{
		ImGui::SetWindowPos(ImVec2(0.0f, m_screenH - 200.0f));
		ImGui::SetWindowSize(ImVec2(m_screenW - 330.0f, 200.0f));
		ImGui::PushStyleColor(ImGuiCol(0), ImVec4(0.6f, 0.6f, 1.0f, 1.0f));

		ImGui::TextUnformatted(m_buffer.begin());
		ImGui::PopStyleColor();

		if (m_scrollToBottom)
			ImGui::SetScrollHere(1.0f);

		m_scrollToBottom = false;

		ImGui::End();
	}
}

void ModuleEditor::Outliner()
{
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

	if (ImGui::Begin("Outliner", 0, ImVec2(500, 300), 0.8f, flags))
	{
		ImGui::SetWindowPos(ImVec2(0.0f, 50.0f));
		ImGui::SetWindowSize(ImVec2(300.0f, m_screenH - 250.0f));
		ImGui::Checkbox("Show hidden objects", &m_displayHiddenOutlinerGameobjects);
		for (auto node : App->m_goManager->GetRoot()->GetTransform()->GetChilds())
		{
			SceneTreeGameObject(node->GetGameobject());
		}
		ImGui::End();
	}
}

void ModuleEditor::AttributeWindow()
{
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

	if (ImGui::Begin("Attribute Editor", 0, flags))
	{
		ImGui::SetWindowPos(ImVec2(m_screenW - 330, 20.0f));
		ImGui::SetWindowSize(ImVec2(330, m_screenH - 20));
		if (m_selectedGameObject)
		{
			m_selectedGameObject->DrawOnEditor();
			ImGui::Separator();
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
		ImGui::End();
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


void ModuleEditor::SwitchViewPorts()
{
	App->m_renderer3D->FindViewPort(m_singleViewportID)->m_active = !m_displayMultipleViews;
	for (int n = 0; n < 4; n++)
	{
		App->m_renderer3D->FindViewPort(m_multipleViewportsIDs[n])->m_active = m_displayMultipleViews;
	}
}

void ModuleEditor::ViewPortUI(const ViewPort & port) const
{
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize;

	char tmp[256];
	sprintf(tmp, "ViewPortMenu##%i", port.m_ID);

	ImGui::Begin(tmp, 0, flags);
	if (ImGui::BeginMenuBar())
	{
		ImGui::SetWindowPos(ImVec2(port.m_pos.x, port.m_pos.y));
		ImGui::SetWindowSize(ImVec2(port.m_size.x, 0));
		sprintf(tmp, "Display##ViewPort%i", port.m_ID);
		if (ImGui::BeginMenu(tmp))
		{
			ViewPort* editPort = App->m_renderer3D->FindViewPort(port.m_ID);
			ImGui::Checkbox("Wired", &editPort->m_useOnlyWires);
			ImGui::Checkbox("Lightning", &editPort->m_useLighting);
			ImGui::Checkbox("Render Terrain Heightmap", &editPort->m_renderHeightMap);
			ImGui::Checkbox("Single sided faces", &editPort->m_useSingleSidedFaces);
			ImGui::Checkbox("Render Terrain", &editPort->m_renderTerrain);
			ImGui::Checkbox("Render Terrain Collision", &editPort->m_renderTerrainCollisions);
			ImGui::Checkbox("Render Chunk Borders", &editPort->m_renderChunkBorders);
			ImGui::Checkbox("Render Bounding boxes", &editPort->m_renderBoundingBoxes);

			ImGui::EndMenu();
		}
		sprintf(tmp, "Camera##ViewPort%i", port.m_ID);
		if (ImGui::BeginMenu(tmp))
		{
			if (ImGui::BeginMenu("Current Camera"))
			{
				ImGui::Text("Name:");
				ImGui::Text(port.m_camera->GetOwner()->m_name);
				ImGui::Separator();
				ImGui::NewLine();
				if (ImGui::MenuItem("Switch view type"))
				{
					App->m_renderer3D->FindViewPort(port.m_ID)->m_camera->SwitchViewType();
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Link new camera"))
			{
				std::multimap<ComponentType, Component*>::iterator comp = App->m_goManager->components.find(ComponentType::camera);
				for (; comp != App->m_goManager->components.end() && comp->first == ComponentType::camera; comp++)
				{
					Camera* cam = (Camera*)&*comp->second;
					if (ImGui::MenuItem(cam->GetOwner()->m_name))
					{
						App->m_renderer3D->FindViewPort(port.m_ID)->m_camera = cam;
						int a = 0;
					}
				}
				ImGui::EndMenu();
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
	ImGui::SetWindowSize(ImVec2(300, 120));
	if (ImGui::BeginPopupModal("New scene"))
	{
		m_selectedGameObject = nullptr;
		bool close = false;
		ImGui::Text("Save current scene?");
		if (ImGui::Button("Yes##saveCurrentButton"))
		{
			m_wantToSaveScene = true;
			m_clearAfterSave = true;
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


	ImGui::SetWindowSize(ImVec2(300, 120));
	if (ImGui::BeginPopupModal("Save scene"))
	{
		bool close = false;
		ImGui::Text("Scene name:");
		ImGui::InputText("##saveSceneInputText", m_sceneName, 256);
		if (ImGui::Button("Save##saveButton") && m_sceneName[0] != '\0')
		{
			App->m_goManager->SaveScene(m_sceneName);
			close = true;
			if (m_clearAfterSave)
			{
				App->m_goManager->ClearScene();
			}
		}
		ImGui::SameLine();
		if (close || ImGui::Button("Cancel##cancelSaveScene"))
		{
			strcpy(m_sceneName, "");
			m_clearAfterSave = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	ImGui::SetWindowSize(ImVec2(300, 120));
	if (ImGui::BeginPopupModal("Load Scene"))
	{
		m_selectedGameObject = nullptr;
		ImGui::Text("Scene name:");
		ImGui::InputText("##saveSceneInputText", m_sceneName, 256);
		bool close = false;
		if (ImGui::Button("Load##loadButton") && m_sceneName[0] != '\0')
		{
			App->m_goManager->LoadScene(m_sceneName);
			close = true;
		}
		ImGui::SameLine();
		if (close || ImGui::Button("Cancel##cancelLoadScene"))
		{
			strcpy(m_sceneName, "");
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if (m_wantNewScene)
	{
		ImGui::OpenPopup("New scene");
		m_wantNewScene = false;
	}
	if (m_wantToSaveScene)
	{
		ImGui::OpenPopup("Save scene");
		m_wantToSaveScene = false;
	}
	if (m_wantToLoadScene)
	{
		ImGui::OpenPopup("Load Scene");
		m_wantToLoadScene = false;
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