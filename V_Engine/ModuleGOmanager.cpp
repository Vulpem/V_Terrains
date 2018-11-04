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

ModuleGoManager::ModuleGoManager(Application* app, bool start_enabled) : Module(app, start_enabled), quadTree(float3(WORLD_WIDTH /-2,WORLD_HEIGHT/-2,WORLD_DEPTH/-2), float3(WORLD_WIDTH / 2, WORLD_HEIGHT / 2, WORLD_DEPTH / 2))
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
	std::multimap<ComponentType, Component*>::iterator comp = components.begin();
	for (; comp != components.end(); comp++)
	{
		std::multimap<ComponentType, Component*>::iterator nextIt = comp;
		nextIt--;
		if (comp->second->TryDeleteNow())
		{
			comp = nextIt;
		}
		else if (comp->second->object->IsActive())
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
	std::multimap<ComponentType, Component*>::iterator comp = components.begin();
	for (; comp != components.end(); comp++)
	{
		if (comp->second->object->IsActive())
		{
			comp->second->Update();
		}
	}
	TIMER_READ_MS("Components Update");
	if (setting != nullptr)
	{
		if (ImGui::BeginPopupModal("##SetStaticChilds", &StaticChildsPopUpIsOpen))
		{
			if (settingStatic)
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
				SetChildsStatic(settingStatic, setting);
				setting = nullptr;
			}
			ImGui::SameLine(150);
			if (ImGui::Button("No##NoSetStatic"))
			{
				SetStatic(settingStatic, setting);
				setting = nullptr;
			}
			ImGui::EndPopup();
		}
		if (StaticChildsPopUpIsOpen == false)
		{
			ImGui::OpenPopup("##SetStaticChilds");
			StaticChildsPopUpIsOpen = true;
		}
	}
	else
	{
		StaticChildsPopUpIsOpen = false;
	}

	return UpdateStatus::Continue;
}

UpdateStatus ModuleGoManager::PostUpdate()
{
	TIMER_START("Components PostUpdate");
	std::multimap<ComponentType, Component*>::iterator comp = components.begin();
	for (; comp != components.end(); comp++)
	{
		if (comp->second->object->IsActive())
		{
			comp->second->PostUpdate();
		}
	}

	TIMER_READ_MS("Components PostUpdate");

	TIMER_RESET_STORED("Cam culling longest");
	TIMER_RESET_STORED("GO render longest");

	bool worked = false;

	if (wantToSaveScene && worked == false)
	{
		worked = true;
		TIMER_START_PERF("Saving Scene");
		SaveSceneNow();
		wantToSaveScene = false;
		TIMER_READ_MS("Saving Scene");
	}

	if (wantToClearScene && worked == false)
	{
		worked = true;
		ClearSceneNow();
		wantToClearScene = false;
	}

	DeleteGOs();

	if (wantToLoadScene&& worked == false)
	{
		worked = true;
		TIMER_START_PERF("Loading Scene");
		LoadSceneNow();
		wantToLoadScene = false;
		TIMER_READ_MS("Loading Scene");
	}
	return UpdateStatus::Continue;
}

void ModuleGoManager::Render(const ViewPort& port) const
{
	App->m_goManager->RenderGOs(port);
	if (drawQuadTree)
	{
		TIMER_START("QuadTree drawTime");
		quadTree.Draw();
		TIMER_READ_MS("QuadTree drawTime");
	}
}

// Called before quitting
void ModuleGoManager::CleanUp()
{
	if (root)
	{
		delete root;
	}
}


//Creating/deleting GOs

GameObject * ModuleGoManager::CreateEmpty(const char* name)
{
	GameObject* empty = new GameObject();

	empty->AddComponent(ComponentType::transform);
	
	if (name != NULL && name != "")
	{
		empty->SetName(name);
	}

	AddGOtoRoot(empty);

	return empty;
}

GameObject* ModuleGoManager::CreateCamera(const char* name)
{
	GameObject* m_camera = CreateEmpty(name);
	m_camera->AddComponent(ComponentType::camera);
	return m_camera;
}

GameObject * ModuleGoManager::DuplicateGO(GameObject * toCopy)
{
	GameObject* ret = new GameObject();

	return nullptr;
}

std::vector<GameObject*> ModuleGoManager::LoadGO(const char* fileName)
{
	GameObject* sceneRoot = App->m_importer->LoadVgo(fileName, "RootNode");
	std::vector<GameObject*> ret;
	if (sceneRoot && sceneRoot->m_childs.empty() == false)
	{
		for (std::vector<GameObject*>::iterator childs = sceneRoot->m_childs.begin(); childs != sceneRoot->m_childs.end(); childs++)
		{
			AddGOtoRoot(*childs);
			ret.push_back((*childs));
		}
		//Deleting a Gameobject will also delete and clear all his childs. In this special case we don't want that
		sceneRoot->m_childs.clear();
		delete sceneRoot;
		LOG("Loaded %s", fileName);
	}
	else
	{
		LOG("Failed to load %s", fileName);
	}
	return ret;
}

bool ModuleGoManager::DeleteGameObject(GameObject* toErase)
{
	if (toErase)
	{
		toDelete.push(toErase);
		return true;
	}
	return false;
	
}


//Scene management

void ModuleGoManager::ClearSceneNow()
{
	if (root->m_childs.empty() == false)
	{
		std::vector<GameObject*>::iterator it = root->m_childs.begin();
		for (; it != root->m_childs.end(); it++)
		{
			if ((*it)->HiddenFromOutliner() == false)
			{
				toDelete.push(*it);
			}
		}
			LOG("Scene cleared");
	}
}

void ModuleGoManager::SaveSceneNow()
{
	// xml object were we will store all data
	pugi::xml_document data;
	pugi::xml_node root_node;
	pugi::xml_node Components_node;

	root_node = data.append_child("Scene");

	root_node.append_attribute("SceneName") = sceneName.data();

	//Saving GameObjects
	root->Save(root_node.append_child("GameObjects"));

	Components_node = root_node.append_child("Components");

	//Saving components
	std::multimap<ComponentType, Component*>::iterator comp = components.begin();
	for (; comp != components.end(); comp++)
	{
		if (comp->second->object->HiddenFromOutliner() == false)
		{
			comp->second->Save(Components_node.append_child("Component"));
		}
	}

	char path[524];
	sprintf(path, "Assets/Scenes/%s%s", sceneName.data(), SCENE_FORMAT);

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
	sprintf(scenePath, "Assets/Scenes/%s%s", sceneName.data(), SCENE_FORMAT);


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

					std::map<uint64_t, GameObject*>::iterator parent = UIDlib.find(parentUID);
					if (parent != UIDlib.end())
					{
						toAdd->m_parent = parent->second;
						parent->second->m_childs.push_back(toAdd);
					}
					UIDlib.insert(std::pair<uint64_t, GameObject*>(UID, toAdd));
					if (UID != 0)
					{
						dynamicGO.push_back(toAdd);
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
							Component* c = go->second->AddComponent(type, "", true);
							if (c != nullptr)
							{
								c->LoadSpecifics(comp.child("Specific"));
							}
						}
					}
				}

				GameObject* sceneRoot = UIDlib.find(0)->second;
				for (std::vector<GameObject*>::iterator it = sceneRoot->m_childs.begin(); it != sceneRoot->m_childs.end(); it++)
				{
					AddGOtoRoot((*it));
				}
				//Deleting a Gameobject will also delete and clear all his childs. In this special case we don't want that
				sceneRoot->m_childs.clear();
				RELEASE(sceneRoot);

				LOG("Scene loaded: %s", sceneName.data());
			}
		}
	}

}



void ModuleGoManager::SetStatic(bool Static, GameObject * GO)
{
	if (Static != GO->IsStatic())
	{
		GO->SetStatic(Static);
		if (Static)
		{
			if (GO->m_parent != nullptr)
			{
				SetStatic(true, GO->m_parent);
			}
			App->m_goManager->quadTree.Add(GO);
			for (std::vector<GameObject*>::iterator it = App->m_goManager->dynamicGO.begin(); it != App->m_goManager->dynamicGO.end(); it++)
			{
				if ((*it) == GO)
				{
					dynamicGO.erase(it);
					break;
				}
			}
		}
		else
		{
			if (GO->m_childs.empty() == false)
			{
				for (std::vector<GameObject*>::iterator it = GO->m_childs.begin(); it != GO->m_childs.end(); it++)
				{
					SetStatic(false, (*it));
				}
			}
			quadTree.Remove(GO);
			dynamicGO.push_back(GO);
		}
	}
}

void ModuleGoManager::SetChildsStatic(bool Static, GameObject * GO)
{
	SetStatic(Static, GO);
	if (Static == true)
	{
		if (GO->m_childs.empty() == false)
		{
			for (std::vector<GameObject*>::iterator it = GO->m_childs.begin(); it != GO->m_childs.end(); it++)
			{
				SetChildsStatic(Static,(*it));
			}
		}
	}
}



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
	for (std::vector<GameObject*>::iterator GO = colls.begin(); GO != colls.end() && colls.empty() == false; GO++)
	{
		float distanceNear;
		float distanceFar;
		//The distance is normalized between [0,1] and is the relative position in the Segment the AABB collides
		if ((*GO)->m_obb.Intersects(ray, distanceNear, distanceFar) == true)
		{
			candidates.insert(std::pair<float, GameObject*>(MIN(distanceNear, distanceFar), (*GO)));
		}
	}

	//Checking all the possible collisions in order
	for (std::map<float, GameObject*>::iterator check = candidates.begin(); check != candidates.end() && collided == false && candidates.empty() == false; check++)
	{
		float collisionDistance = floatMax;
		//One object may have more than a single mesh, so we'll check them one by one
		if (check->second->HasComponent<Mesh>())
		{
			std::vector<Mesh*> meshes = check->second->GetComponents<Mesh>();
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


Mesh_RenderInfo ModuleGoManager::GetMeshData(Mesh * getFrom)
{
	Mesh_RenderInfo ret = getFrom->GetMeshInfo();

	ret.m_transform = getFrom->object->GetTransform()->GetGlobalTransform();

	if (getFrom->object->HasComponent<Material>())
	{
		Material* mat = getFrom->object->GetComponent<Material>();
		if (mat->toDelete == false)
		{
			ret.m_meshColor = mat->GetColor();
			ret.m_textureBuffer = mat->GetTexture(getFrom->texMaterialIndex);
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

void ModuleGoManager::RenderGOs(const ViewPort & port, const std::vector<GameObject*>& exclusiveGOs)
{
	std::vector<GameObject*> toRender;

	//This vector will generally be empty. It is only used when we send certain GOs we want to render exclusively
	if (exclusiveGOs.empty() == true)
	{
		//Call the Draw function of all the components, so they do what they need to
		std::multimap<ComponentType, Component*>::iterator comp = components.begin();
		for (; comp != components.end(); comp++)
		{
			if (comp->second->object->IsActive())
			{
				comp->second->Draw(port);
				if (comp->second->object->HasComponent<Billboard>())
				{
					Transform* camTransform = port.m_camera->object->GetTransform();
					comp->second->object->GetComponent<Billboard>()->UpdateNow(camTransform->GetGlobalPos(), camTransform->Up());
				}
			}
		}
		TIMER_START("Cam culling longest");
		bool aCamHadCulling = false;
		//Finding all the cameras that have culling on, and collecting all the GOs we need to render
		std::multimap<ComponentType, Component*>::iterator it = components.find(ComponentType::camera);
		for (; it != components.end() && it->first == ComponentType::camera; it++)
		{
			if (((Camera*)(it->second))->HasCulling())
			{
				aCamHadCulling = true;
				//If a m_camera has ortographiv view, we'll need to test culling against an AABB instead of against it frustum
				if (((Camera*)(it->second))->GetFrustum()->type == FrustumType::PerspectiveFrustum)
				{
					toRender = FilterCollisions(*((Camera*)(it->second))->GetFrustum());
				}
				else
				{
					toRender = FilterCollisions(((Camera*)(it->second))->GetFrustum()->MinimalEnclosingAABB());
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
	}
	else
	{
		App->m_renderer3D->SetViewPort(*App->m_renderer3D->FindViewPort(port.m_ID));
		for (std::vector<GameObject*>::const_iterator toInsert = exclusiveGOs.begin(); toInsert != exclusiveGOs.end(); toInsert++)
		{
			toRender.push_back(*toInsert);
		}
	}

	TIMER_START("GO render longest");
	//And now, we render them
	TIMER_RESET_STORED("Mesh slowest");
	for(GameObject* go : toRender)
	{
		if (go->HasComponent<Mesh>())
		{
			std::vector<Mesh*> meshes = go->GetComponents<Mesh>();
			if (meshes.empty() == false)
			{
				for (std::vector<Mesh*>::iterator mesh = meshes.begin(); mesh != meshes.end(); mesh++)
				{
					if ((*mesh)->IsEnabled() && (*mesh)->toDelete == false)
					{
						TIMER_START("Mesh slowest");
						Mesh_RenderInfo info = GetMeshData(*mesh);
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
	GO->m_parent = root;
	root->m_childs.push_back(GO);
}

void ModuleGoManager::CreateRootGameObject()
{
	if (root == nullptr)
	{
		GameObject* ret = new GameObject();

		LOG("Creating root node for scene");
		//Setting Name
		ret->SetName("Root");

		//Setting parent
		ret->m_parent = nullptr;

		//Setting transform
		math::Quat rot = math::Quat::identity;

		root = ret;
	}
	else
	{
		LOG("Be careful! You almost created a second root node!");
	}
}

void ModuleGoManager::DeleteGOs()
{
	while (toDelete.empty() == false)
	{
		LOG("Erasing GO %s", toDelete.top()->GetName());			
		delete toDelete.top();
		toDelete.pop();
	}
}
