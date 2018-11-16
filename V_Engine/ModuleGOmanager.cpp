#include "Globals.h"
#include "Application.h"

#include "ModuleGOmanager.h"

#include "ModuleInput.h"
#include "ModuleImporter.h"
#include "ModuleCamera3D.h"
#include "ModuleRenderer3D.h"
#include "ModuleFileSystem.h"
#include "imGUI\imgui.h"

#include "Mesh_RenderInfo.h"
#include "ViewPort.h"

#include "AllComponents.h"

#include <unordered_set>
#include <algorithm>


//------------------------- MODULE --------------------------------------------------------------------------------

ModuleGoManager::ModuleGoManager(Application* app, bool start_enabled) : Module(app, start_enabled), m_quadTree(float3(WORLD_WIDTH /-2,WORLD_HEIGHT/-2,WORLD_DEPTH/-2), float3(WORLD_WIDTH / 2, WORLD_HEIGHT / 2, WORLD_DEPTH / 2))
{
}

ModuleGoManager::~ModuleGoManager()
{

}

// Called before render is available
bool ModuleGoManager::Init()
{
	bool ret = true;

	CreateRootGameObject();
		
	return ret;
}

// Called every draw update
UpdateStatus ModuleGoManager::PreUpdate()
{
	TIMER_START("Components PreUpdate");
	std::multimap<ComponentType, Component*>::iterator comp = m_components.begin();
	for (; comp != m_components.end(); comp++)
	{
		std::multimap<ComponentType, Component*>::iterator nextIt = comp;
		nextIt--;
		if (comp->second->TryDeleteNow())
		{
			comp = nextIt;
		}
		else if (comp->second->GetOwner()->IsActive())
		{
			comp->second->PreUpdate();
		}
	}
	TIMER_READ_MS("Components PreUpdate");
	return UpdateStatus::Continue;
}

UpdateStatus ModuleGoManager::Update()
{
	if (App->m_input->m_fileWasDropped)
	{
		char droppedFile[1024];
		strcpy(droppedFile, App->m_input->m_lastDroppedFile);
		std::string file = App->m_importer->NormalizePath(droppedFile);
		LoadGO(file.data());
	}

	TIMER_START("Components Update");
	std::multimap<ComponentType, Component*>::iterator comp = m_components.begin();
	for (; comp != m_components.end(); comp++)
	{
		if (comp->second->GetOwner()->IsActive())
		{
			comp->second->Update();
		}
	}
	TIMER_READ_MS("Components Update");
	if (m_setting != nullptr)
	{
		if (ImGui::BeginPopupModal("##SetStaticChilds", &m_staticChildsPopUpIsOpen))
		{
			if (m_settingStatic)
			{
				ImGui::Text("Set childs to Static too?");
			}
			else
			{
				ImGui::Text("Set childs to non Static too?");
			}
			ImGui::NewLine();
			ImGui::SameLine(30);
			if (ImGui::Button("Yes##yesSetStatic"))
			{
				SetChildsStatic(m_settingStatic, m_setting);
				m_setting = nullptr;
			}
			ImGui::SameLine(150);
			if (ImGui::Button("No##NoSetStatic"))
			{
				SetStatic(m_settingStatic, m_setting);
				m_setting = nullptr;
			}
			ImGui::EndPopup();
		}
		if (m_staticChildsPopUpIsOpen == false)
		{
			ImGui::OpenPopup("##SetStaticChilds");
			m_staticChildsPopUpIsOpen = true;
		}
	}
	else
	{
		m_staticChildsPopUpIsOpen = false;
	}

	return UpdateStatus::Continue;
}

UpdateStatus ModuleGoManager::PostUpdate()
{
	TIMER_START("Components PostUpdate");
	std::multimap<ComponentType, Component*>::iterator comp = m_components.begin();
	for (; comp != m_components.end(); comp++)
	{
		if (comp->second->GetOwner()->IsActive())
		{
			comp->second->PostUpdate();
		}
	}

	TIMER_READ_MS("Components PostUpdate");

	TIMER_RESET_STORED("Cam culling longest");
	TIMER_RESET_STORED("GO render longest");

	bool worked = false;

	if (m_wantToSaveScene && worked == false)
	{
		worked = true;
		TIMER_START_PERF("Saving Scene");
		SaveSceneNow();
		m_wantToSaveScene = false;
		TIMER_READ_MS("Saving Scene");
	}

	if (m_wantToClearScene && worked == false)
	{
		worked = true;
		ClearSceneNow();
		m_wantToClearScene = false;
	}

	DeleteGOs();

	if (m_wantToLoadScene&& worked == false)
	{
		worked = true;
		TIMER_START_PERF("Loading Scene");
		LoadSceneNow();
		m_wantToLoadScene = false;
		TIMER_READ_MS("Loading Scene");
	}
	return UpdateStatus::Continue;
}

void ModuleGoManager::Render(const ViewPort& port) const
{
	App->m_goManager->RenderGOs(port);
	if (port.m_renderQuadTree)
	{
		TIMER_START("QuadTree drawTime");
		m_quadTree.Draw();
		TIMER_READ_MS("QuadTree drawTime");
	}
}

// Called before quitting
void ModuleGoManager::CleanUp()
{
	if (m_root)
	{
		delete m_root;
	}
}


//Create an empty GameObject
GameObject * ModuleGoManager::CreateEmpty(const char* name)
{
	GameObject* empty = new GameObject();
	
	if (name != NULL && name != "")
	{
		empty->SetName(name);
	}

	AddGOtoRoot(empty);

	return empty;
}

//Create a gameobject with just a Camera attached to it
GameObject* ModuleGoManager::CreateCamera(const char* name)
{
	GameObject* m_camera = CreateEmpty(name);
	m_camera->CreateComponent(ComponentType::camera);
	return m_camera;
}

//Load a vGO avaliable in the resources
std::vector<GameObject*> ModuleGoManager::LoadGO(const char* fileName)
{
	GameObject* sceneRoot = App->m_importer->LoadVgo(fileName, "RootNode");
	std::vector<GameObject*> ret;
	std::vector<Transform*> sceneRootChilds = sceneRoot->GetTransform()->GetChilds();
	if (sceneRoot && sceneRootChilds.empty() == false)
	{
		std::vector<Transform*> childs = sceneRoot->GetTransform()->GetChilds();
		for (auto child : childs)
		{
			AddGOtoRoot(child->GetGameobject());
			ret.push_back(child->GetGameobject());
		}
		delete sceneRoot;
		LOG("Loaded %s", fileName);
	}
	else
	{
		LOG("Failed to load %s", fileName);
	}
	return ret;
}

void ModuleGoManager::DeleteGameObject(GameObject* toErase)
{
	m_toDelete.push_back(toErase);
}

void ModuleGoManager::DeleteComponent(Component * toErase)
{
	std::multimap<ComponentType, Component*>::iterator it = m_components.find(toErase->GetType());
	for (; it->first == toErase->GetType(); it++)
	{
		if (it->second == toErase)
		{
			toErase->GetOwner()->m_components.erase(std::find(toErase->GetOwner()->m_components.begin(), toErase->GetOwner()->m_components.end(), toErase));
			RELEASE(toErase);
			App->m_goManager->m_components.erase(it);
			return;
		}
	}
}


//Scene management
void ModuleGoManager::ClearSceneNow()
{
	for (auto child : m_root->GetTransform()->GetChilds())
	{
		if (child->m_hiddenOnOutliner == false)
		{
			m_toDelete.push_back(child->GetGameobject());
		}
	}
}

void ModuleGoManager::SaveSceneNow()
{
	// xml object were we will store all data
	pugi::xml_document data;
	pugi::xml_node root_node;
	pugi::xml_node Components_node;

	root_node = data.append_child("Scene");

	root_node.append_attribute("SceneName") = m_sceneName.data();

	//Saving GameObjects
	m_root->Save(root_node.append_child("GameObjects"));

	Components_node = root_node.append_child("Components");

	//Saving components
	std::multimap<ComponentType, Component*>::iterator comp = m_components.begin();
	for (; comp != m_components.end(); comp++)
	{
		//TODO "HiddenFromOutliner" should not affect which entitites are saved
		if (comp->second->GetOwner()->GetTransform()->m_hiddenOnOutliner == false)
		{
			comp->second->Save(Components_node.append_child("Component"));
		}
	}

	char path[524];
	sprintf(path, "Assets/Scenes/%s%s", m_sceneName.data(), SCENE_FORMAT);

	std::stringstream stream;
	data.save(stream);
	// we are done, so write data to disk
	App->m_fileSystem->Save(path, stream.str().c_str(), stream.str().length());
	LOG("Scene saved: %s", path);

	data.reset();
}

void ModuleGoManager::LoadSceneNow()
{
	std::map<uint64_t, GameObject*> UIDlib;

	char scenePath[526];
	sprintf(scenePath, "Assets/Scenes/%s%s", m_sceneName.data(), SCENE_FORMAT);


	char* buffer;
	uint size = App->m_fileSystem->Load(scenePath, &buffer);

	if (size > 0)
	{
		pugi::xml_document data;
		pugi::xml_node root;

		pugi::xml_parse_result result = data.load_buffer(buffer, size);
		RELEASE(buffer);

		if (result != NULL)
		{
			root = data.child("Scene");
			if (root)
			{
				for (pugi::xml_node GOs = root.child("GameObjects").first_child(); GOs != nullptr; GOs = GOs.next_sibling())
				{
					uint64_t parentUID = GOs.attribute("parent").as_ullong();
					std::string go_name = GOs.attribute("name").as_string();
					
					uint64_t UID = GOs.attribute("UID").as_ullong();

					GameObject* toAdd = new GameObject();
					toAdd->SetName(go_name.data());

					bool isStatic = GOs.attribute("Static").as_bool();
					toAdd->SetStatic(isStatic);

					bool isActive = GOs.attribute("Active").as_bool();
					toAdd->SetActive(isActive);

					toAdd->GetTransform()->LoadSpecifics(GOs.child("Transform"));

					std::map<uint64_t, GameObject*>::iterator parent = UIDlib.find(parentUID);
					if (parent != UIDlib.end())
					{
						toAdd->GetTransform()->SetParent(parent->second->GetTransform());
					}
					UIDlib.insert(std::pair<uint64_t, GameObject*>(UID, toAdd));
					if (UID != 0)
					{
						m_dynamicGO.push_back(toAdd);
					}
				}

				for (pugi::xml_node comp = root.child("Components").first_child(); comp != nullptr; comp = comp.next_sibling())
				{
					pugi::xml_node general = comp.child("General");
					std::string name = general.attribute("name").as_string();
					uint64_t UID = general.attribute("UID").as_ullong();
					ComponentType type = (ComponentType)general.attribute("type").as_int();
					int id = general.attribute("id").as_int();

					uint64_t GO = general.attribute("GO").as_ullong();

					bool enabled = general.attribute("enabled").as_bool();
					if (GO != 0)
					{
						std::map<uint64_t, GameObject*>::iterator go = UIDlib.find(GO);
						if (go != UIDlib.end())
						{
							Component* c = go->second->CreateComponent(type, "", true);
							if (c != nullptr)
							{
								c->LoadSpecifics(comp.child("Specific"));
							}
						}
					}
				}

				GameObject* sceneRoot = UIDlib.find(0)->second;
				std::vector<Transform*> childs = sceneRoot->GetTransform()->GetChilds();
				for (auto child : childs)
				{
					AddGOtoRoot(child->GetGameobject());
				}
				RELEASE(sceneRoot);

				LOG("Scene loaded: %s", m_sceneName.data());
			}
		}
	}

}


//Set a single GO to the passed Static value
void ModuleGoManager::SetStatic(bool Static, GameObject * GO)
{
	if (Static != GO->IsStatic())
	{
		GO->SetStatic(Static);
		if (Static)
		{
			if (GO->GetTransform()->GetParent() != nullptr)
			{
				SetStatic(true, GO->GetTransform()->GetParent()->GetGameobject());
			}
			App->m_goManager->m_quadTree.Add(GO);
			for (std::vector<GameObject*>::iterator it = App->m_goManager->m_dynamicGO.begin(); it != App->m_goManager->m_dynamicGO.end(); it++)
			{
				if ((*it) == GO)
				{
					m_dynamicGO.erase(it);
					break;
				}
			}
		}
		else
		{
			std::vector<Transform*> childs = GO->GetTransform()->GetChilds();
			for (auto child : childs)
			{
				SetStatic(false, child->GetGameobject());
			}
			m_quadTree.Remove(GO);
			m_dynamicGO.push_back(GO);
		}
	}
}

//Set a GO and all his childs to the passed Static value
void ModuleGoManager::SetChildsStatic(bool Static, GameObject * GO)
{
	SetStatic(Static, GO);
	if (Static == true)
	{
		std::vector<Transform*> childs = GO->GetTransform()->GetChilds();
		for (auto child : childs)
		{
			SetChildsStatic(Static, child->GetGameobject());
		}
	}
}


/*Check if the ray collides with any GameObject
-return bool: wether if the ray collided with something or not
-OUT_Gameobject: gameobject the ray collided with. If there's none, nullptr is returned
-OUT_position: the position where the ray collided. If it didn't, it will return (-1,-1).
-OUT_normal: the direction of the normal of the surface where the ray collided. If it didn't, it will return (-1,-1).*/
bool ModuleGoManager::RayCast(const LineSegment & ray, GameObject** OUT_gameobject, float3 * OUT_position, float3* OUT_normal, bool onlyMeshes)
{
	TIMER_RESET_STORED("Raycast");
	TIMER_START("Raycast");
	bool collided = false;
	GameObject* out_go = NULL;
	float3 out_pos = float3::zero;
	float3 out_normal = float3::zero;

	//Obtaining all the AABB collisions, and sorting them by distance of the AABB
	std::vector<GameObject*> colls = App->m_goManager->FilterCollisions(ray);
	std::map<float, GameObject*> candidates;
	for (auto candidateGO : colls)
	{
		float distanceNear;
		float distanceFar;
		//The distance is normalized between [0,1] and is the relative position in the Segment the AABB collides
		if (candidateGO->GetOBB().Intersects(ray, distanceNear, distanceFar) == true)
		{
			candidates.insert(std::pair<float, GameObject*>(MIN(distanceNear, distanceFar), candidateGO));
		}
	}

	//Checking all the possible collisions in order
	for (std::map<float, GameObject*>::iterator check = candidates.begin(); check != candidates.end() && collided == false && candidates.empty() == false; check++)
	{
		float collisionDistance = floatMax;
		//One object may have more than a single mesh, so we'll check them one by one
		if (check->second->HasComponent<Mesh>())
		{
			std::vector<Mesh*> meshes;
			meshes.reserve(3);
			check->second->GetComponents<Mesh>(meshes);
			for (std::vector<Mesh*>::iterator m = meshes.begin(); m != meshes.end(); m++)
			{
				LineSegment transformedRay = ray;
				transformedRay.Transform(check->second->GetTransform()->GetGlobalTransform().InverseTransposed());
				//Generating the triangles the mes has, and checking them one by one
				const float3* vertices = (*m)->GetVertices();
				const uint* index = (*m)->GetIndices();
				for (int n = 0; n < (*m)->GetNumIndices(); n += 3)
				{
					Triangle tri(vertices[index[n]], vertices[index[n + 1]], vertices[index[n + 2]]);
					float3 intersectionPoint;
					float distance;
					//If the triangle we collided with is further away than a previous collision, we'll ignore it
					if (tri.Intersects(transformedRay, &distance, &intersectionPoint) == true)
					{
						if (distance < collisionDistance)
						{
							collided = true;
							collisionDistance = distance;
							out_go = check->second;
							out_pos = intersectionPoint;
							out_normal = tri.NormalCCW();
							LineSegment tmp(out_pos, out_pos + out_normal);
							tmp.Transform(check->second->GetTransform()->GetGlobalTransform().Transposed());
							out_pos = tmp.a;
							out_normal = tmp.b - tmp.a;
						}
					}
				}
			}
		}
		else if(onlyMeshes == false)
		{
			collided = true;
			out_go = check->second;
			out_normal = float3(0, 1, 0);
			out_pos = check->second->GetTransform()->GetGlobalPos();
		}
	}
	*OUT_gameobject = out_go;
	if (OUT_normal != NULL)
	{
		*OUT_normal = out_normal.Normalized();
	}
	if (OUT_position != NULL)
	{
		*OUT_position = out_pos;
	}
	TIMER_READ_MS("Raycast");
	return collided;
}


Mesh_RenderInfo ModuleGoManager::GetMeshData(Mesh * getFrom) const
{
	Mesh_RenderInfo ret = getFrom->GetMeshInfo();

	ret.m_transform = getFrom->GetOwner()->GetTransform()->GetGlobalTransform();

	if (getFrom->GetOwner()->HasComponent<Material>())
	{
		Material* mat = getFrom->GetOwner()->GetComponent<Material>();
		if (mat->MarkedForDeletion() == false)
		{
			ret.m_meshColor = mat->GetColor();
			ret.m_textureBuffer = mat->GetTexture(getFrom->m_textureIndex);
			ret.m_alphaType = mat->GetAlphaType();
			ret.m_alphaTest = mat->GetAlphaTest();
			ret.m_blendType = mat->GetBlendType();
			ret.m_shader = mat->GetShader();
		}
	}
	else
	{
		ret.m_meshColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	if (ret.m_shader.m_program == -1)
	{
		ret.m_shader = App->m_resourceManager->GetDefaultShader();
	}
	return ret;
}

void ModuleGoManager::RenderGOs(const ViewPort & port)  const
{
		//Call the Draw function of all the components, so they do what they need to
		for (auto component : m_components)
		{
			if (component.second->GetOwner()->IsActive())
			{
				component.second->Draw(port);
				if (component.second->GetOwner()->HasComponent<Billboard>())
				{
					Transform* camTransform = port.m_camera->GetOwner()->GetTransform();
					component.second->GetOwner()->GetComponent<Billboard>()->UpdateNow(camTransform->GetGlobalPos(), camTransform->Up());
				}
			}
		}
		TIMER_START("Cam culling longest");

		std::vector<GameObject*> toRender;
		bool aCamHadCulling = false;
		//Finding all the cameras that have culling on, and collecting all the GOs we need to render
		std::vector<Camera*> cameras = GetComponentsByType<Camera>(ComponentType::camera);
		for(auto camera : cameras)
		{
			if (camera->HasCulling())
			{
				aCamHadCulling = true;
				//If a m_camera has ortographiv view, we'll need to test culling against an AABB instead of against it frustum
				if (camera->GetFrustum()->type == FrustumType::PerspectiveFrustum)
				{
					toRender = FilterCollisions(*camera->GetFrustum());
				}
				else
				{
					toRender = FilterCollisions(camera->GetFrustum()->MinimalEnclosingAABB());
				}
			}
		}

		//If no cameras had culling active, we'll cull from the Current Active m_camera
		if (aCamHadCulling == false)
		{
			std::vector<GameObject*> GOs;
			if (port.m_camera->GetFrustum()->type == FrustumType::PerspectiveFrustum)
			{
				toRender = FilterCollisions(*port.m_camera->GetFrustum());
			}
			else
			{
				toRender = FilterCollisions(port.m_camera->GetFrustum()->MinimalEnclosingAABB());
			}
		}
		TIMER_READ_MS_MAX("Cam culling longest");

	TIMER_START("GO render longest");
	TIMER_RESET_STORED("Mesh slowest");
	for(const GameObject* go : toRender)
	{
		//TODO fix linker issue of Transform Draw
		//go->GetTransform()->Draw(port);
		if (go->HasComponent<Mesh>())
		{
			std::vector<Mesh*> meshes;
			meshes.reserve(3);
			go->GetComponents<Mesh>(meshes);
			if (meshes.empty() == false)
			{
				for(auto mesh : meshes)
				{
					if (mesh->IsEnabled() && mesh->MarkedForDeletion() == false)
					{
						TIMER_START("Mesh slowest");
						Mesh_RenderInfo info = GetMeshData(mesh);
						if (port.m_useOnlyWires)
						{
							info.m_drawFilled = false;
							info.m_drawWired = true;
						}
						App->m_renderer3D->DrawMesh(info);
						TIMER_READ_MS_MAX("Mesh slowest");
					}
				}
			}
		}
	}
	TIMER_READ_MS_MAX("GO render longest");
}

void ModuleGoManager::AddGOtoRoot(GameObject * GO)
{
	GO->GetTransform()->SetParent(m_root->GetTransform());
}

void ModuleGoManager::CreateRootGameObject()
{
	if (m_root == nullptr)
	{
		LOG("Creating root node for scene");
		m_root = new GameObject();
		m_root->SetName("Root");
		m_root->GetTransform()->SetParent(nullptr);
	}
	else
	{
		LOG("Can't create a second root node.");
	}
}

void ModuleGoManager::DeleteGOs()
{
	for (auto goToDelete : m_toDelete)
	{
		LOG("Erasing GO %s", goToDelete->GetName());
		delete goToDelete;
	}
	m_toDelete.clear();
}
