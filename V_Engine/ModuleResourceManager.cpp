#include "Globals.h"
#include "Application.h"

#include "ModuleResourceManager.h"

#include "ModuleImporter.h"
#include "ModuleFileSystem.h"

#include "R_Resource.h"

#include <queue>

#include "OpenGL.h"


ModuleResourceManager::ModuleResourceManager(Application* app, bool start_enabled) : Module(app, start_enabled)//, resBaseFolder("Assets", "Assets")
{
}

// Destructor
ModuleResourceManager::~ModuleResourceManager()
{
}

// Called before render is available
void ModuleResourceManager::Start()
{
	m_defaultVertexBuf = std::string(
		"#version 330 core\n"
		"\n"
		"layout (location = 0) in vec3 position;\n"
		"layout (location = 1) in vec3 normal;\n"
		"layout (location = 2) in vec2 texCoord;\n"
		"\n"
		"out vec4 ourColor;\n"
		"out vec2 TexCoord;\n"
		"out vec3 pos;\n"
		"\n"
		"uniform mat4 model_matrix;\n"
		"uniform mat4 view_matrix;\n"
		"uniform mat4 projection_matrix;\n"
		"uniform vec4 material_color;\n"
		"uniform vec3 global_light_direction;\n"
		"uniform vec4 ambient_color;\n"
		"uniform int use_light;\n"
		"\n"
		"void main()\n"
		"{\n"
		"   vec4 p = model_matrix * vec4(position, 1.0f);"
		"   pos = p.xyz;"
		"	mat4 transform = projection_matrix * view_matrix;\n"
		"	gl_Position = transform * p;\n"
		"   if (use_light != 0)\n"
		"   {\n"
		"		vec3 norm = mat3(model_matrix) * normal;\n"
		"		float light_intensity = dot(global_light_direction, normalize(norm));\n"
		"		light_intensity = max(light_intensity,ambient_color.x);\n"
		"		ourColor = material_color * light_intensity;\n"
		"		ourColor.w = material_color.w;\n"
		"   }\n"
		"   else\n"
		"   {\n"
		"		ourColor = material_color;\n"
		"   }\n"
		"	TexCoord = texCoord;\n"
		"}\n"
	);

	m_defaultFragmentBuf = std::string(
		"#version 330 core\n"
		"\n"
		"in vec4 ourColor;\n"
		"in vec2 TexCoord;\n"
		"in vec3 pos;\n"
		"\n"
		"out vec4 color;\n"
		"\n"
		"uniform int fog_distance;\n"
		"uniform vec3 fog_color;\n"
		"uniform float max_height;\n"
		"uniform sampler2D ourTexture;\n"
		"uniform int has_texture;\n"
		"\n"
		"void main()\n"
		"{\n"
		""
		"	vec4 col = vec4(1, 1, 1, 1);\n"
		"	if(has_texture != 0) {\n"
		"		col = ourColor * texture(ourTexture, TexCoord);\n"
		"	} else {\n"
		"		col = ourColor; \n"
		"	}\n"
		"	float distanceFog = (gl_FragCoord.z / gl_FragCoord.w) / fog_distance;\n"
		"	distanceFog = min(distanceFog, 1);\n"
		"	float heightFog = (1 - (pos.y / max_height));\n"
		"	heightFog = pow(heightFog, 3);\n"
		"   heightFog = max(heightFog, 0);\n"
		"	float fog = distanceFog + (heightFog * 0.5f) * pow(distanceFog, 0.25f);\n"
		"	fog = min(fog, 1);\n"
		"	color = mix(col, vec4(fog_color, 1.f), fog);\n"
		"}"
	);

	CreateLibraryDirs();
	LoadMetaData();
	Refresh();
	GenerateDefaultShader();
}

UpdateStatus ModuleResourceManager::PreUpdate()
{
	DeleteNow();
	return UpdateStatus::Continue;
}

// Called every draw update
UpdateStatus ModuleResourceManager::Update()
{
	if (m_autoRefresh)
	{
		m_refreshTimer += Time.dt;
		if (m_refreshTimer > m_refreshInterval)
		{
			m_refreshTimer = 0.0f;
			Refresh();
		}
	}
	return UpdateStatus::Continue;
}

UpdateStatus ModuleResourceManager::PostUpdate()
{
	ReloadNow();
	return UpdateStatus::Continue;
}

// Called before quitting
void ModuleResourceManager::CleanUp()
{
	SaveMetaData();
	for(auto resource : m_resources)
	{
		RELEASE(resource.second);
	};
	m_resources.clear();
	m_uidLib.clear();
}

Resource * ModuleResourceManager::LoadNewResource(std::string resName, ComponentType type)
{
	switch (type)
	{
	case (ComponentType::mesh):
		{
			return (Resource*)App->m_importer->LoadMesh(resName.data());
		}
	case (ComponentType::material):
		{
			return (Resource*)App->m_importer->LoadMaterial(resName.data());
		}
	case (ComponentType::texture):
		{
			return (Resource*)App->m_importer->LoadTexture(resName.data());
		}
	case (ComponentType::shader):
	{
		return (Resource*)App->m_importer->LoadShader(resName.data());
	}
	}
	return nullptr;
}

//Create the Library folders
void ModuleResourceManager::CreateLibraryDirs()
{
	App->m_fileSystem->CreateDir("Library");
	App->m_fileSystem->CreateDir("Library/Meshes");
	App->m_fileSystem->CreateDir("Library/Textures");
	App->m_fileSystem->CreateDir("Library/vGOs");
	App->m_fileSystem->CreateDir("Library/Materials");
	App->m_fileSystem->CreateDir("Library/Meta");
	App->m_fileSystem->CreateDir("Library/Shaders");
	App->m_fileSystem->CreateDir("Assets/Scenes");
}

//Brute-Force reimport all assets. Existing files won't keep their previous UID
void ModuleResourceManager::ReimportAll()
{
	TIMER_START("Res Reimport All");
	ClearLibrary();

	m_metaData.clear();
	m_metaLastModificationDate.clear();

	std::queue<R_Folder> pendant;
	pendant.push(ReadFolder("Assets"));
	while (pendant.empty() == false)
	{
		for (std::vector<std::string>::iterator it = pendant.front().m_subFoldersPath.begin(); it != pendant.front().m_subFoldersPath.end(); it++)
		{
			pendant.push(ReadFolder(it->data()));
		}

		for (std::vector<std::string>::iterator it = pendant.front().m_files.begin(); it != pendant.front().m_files.end(); it++)
		{
			std::string path(pendant.front().m_path);
			path += "/";
			path += it->data();

			std::vector<MetaInf> toAdd = App->m_importer->Import(path.data());
			if (toAdd.empty() == false)
			{
				std::multimap<ComponentType, MetaInf> tmp;
				for (std::vector<MetaInf>::iterator m = toAdd.begin(); m != toAdd.end(); m++)
				{
					tmp.insert(std::pair<ComponentType, MetaInf>(m->type, *m));
				}
				m_metaData.insert(std::pair<std::string, std::multimap<ComponentType, MetaInf>>(path, tmp));

				m_metaLastModificationDate.insert(std::pair<std::string, Date>(path, App->m_fileSystem->ReadFileDate(path.data())));
			}
		}

		pendant.pop();
	}

	SaveMetaData();
	TIMER_READ_MS("Res Reimport All");
}

//Dump all the library files
void ModuleResourceManager::ClearLibrary()
{
	App->m_fileSystem->DeleteDir("Library");
	CreateLibraryDirs();
}

//Save metadata for all files
void ModuleResourceManager::SaveMetaData()
{
	App->m_fileSystem->DeleteDir("Library/Meta");
	App->m_fileSystem->CreateDir("Library/Meta");

	std::map<std::string, std::multimap<ComponentType, MetaInf>>::iterator fileIt = m_metaData.begin();
	for (; fileIt != m_metaData.end(); fileIt++)
	{
		SaveMetaData(fileIt);
	}
}

//Save a specific metadata file
void ModuleResourceManager::SaveMetaData(std::map<std::string, std::multimap<ComponentType, MetaInf>>::iterator fileToSave)
{
	pugi::xml_document data;
	pugi::xml_node root_node;

	uint n = 0;
	char fileName[524];
	sprintf(fileName, "Library/Meta/%s%u%s", App->m_importer->IsolateFileName(fileToSave->first.data()).data(), n, META_FORMAT);
	while (App->m_fileSystem->Exists(fileName) == true)
	{
		sprintf(fileName, "Library/Meta/%s%u%s", App->m_importer->IsolateFileName(fileToSave->first.data()).data(), n, META_FORMAT);
		n++;
	}

	root_node = data.append_child("File");

	pugi::xml_node fileData = root_node.append_child("FileData");
	fileData.append_attribute("name") = fileToSave->first.data();

	std::map<std::string, Date>::iterator date = m_metaLastModificationDate.find(fileToSave->first);

	fileData.append_attribute("year") = date->second.year;
	fileData.append_attribute("month") = date->second.month;
	fileData.append_attribute("day") = date->second.day;
	fileData.append_attribute("hour") = date->second.hour;
	fileData.append_attribute("min") = date->second.min;
	fileData.append_attribute("sec") = date->second.sec;

	std::multimap<ComponentType, MetaInf>::iterator it = fileToSave->second.begin();
	for (; it != fileToSave->second.end(); it++)
	{
		pugi::xml_node link = root_node.append_child("link");
		link.append_attribute("name") = it->second.name.data();
		link.append_attribute("type") = static_cast<int>(it->second.type);
		link.append_attribute("uid") = it->second.uid;
	}

	std::stringstream stream;
	data.save(stream);
	// we are done, so write data to disk
	App->m_fileSystem->Save(fileName, stream.str().c_str(), stream.str().length());
	LOG("Created: %s", fileName);

	data.reset();
	n++;
}

void ModuleResourceManager::LoadMetaData()
{
	LOG("Reloading Metadata from library");

	m_metaData.clear();
	m_metaLastModificationDate.clear();

	std::vector<std::string> folders;
	std::vector<std::string> files;
	App->m_fileSystem->GetFilesIn("Library/Meta", folders, files);

	for (std::vector<std::string>::iterator fileIt = files.begin(); fileIt != files.end(); fileIt++)
	{
		char* buffer;
		std::string path("Library/Meta/");
		path += fileIt->data();
		uint size = App->m_fileSystem->Load(path.data(), &buffer);

		if (size > 0)
		{
			pugi::xml_document data;
			pugi::xml_node root;

			pugi::xml_parse_result result = data.load_buffer(buffer, size);
			RELEASE_ARRAY(buffer);

			if (result != NULL)
			{
				root = data.child("File");
				if (root)
				{
					pugi::xml_node fileMeta = root.child("FileData");					

					std::string name = fileMeta.attribute("name").as_string();

					Date date;
					date.year = fileMeta.attribute("year").as_uint();
					date.month = fileMeta.attribute("month").as_uint();
					date.day = fileMeta.attribute("day").as_uint();
					date.hour = fileMeta.attribute("hour").as_uint();
					date.min = fileMeta.attribute("min").as_uint();
					date.sec = fileMeta.attribute("sec").as_uint();

					m_metaLastModificationDate.insert(std::pair<std::string, Date>(name, date));

					std::multimap<ComponentType, MetaInf> inf;

					pugi::xml_node link = root.child("link");
					while (link != nullptr)
					{
						MetaInf toAdd;
						toAdd.name = link.attribute("name").as_string();
						toAdd.type = (ComponentType)link.attribute("type").as_uint();
						toAdd.uid = link.attribute("uid").as_ullong();

						inf.insert(std::pair<ComponentType, MetaInf>(toAdd.type, toAdd));

						link = link.next_sibling("link");
					}

					m_metaData.insert(std::pair<std::string, std::multimap<ComponentType, MetaInf>>(name, inf));
				}
			}
		}
		else
		{
			LOG("Tried to read an unexisting folder meta.\n%s", path)
		}
	}
}

//Check for new or modified files in the Assets folder, and import them. Existing files will keep their UID
void ModuleResourceManager::Refresh()
{
	TIMER_START("Res Refresh");
	std::queue<R_Folder> pendantFolders;
	std::queue<std::string> filesToCheck;

	uint totalFiles = 0;
	uint filesToImport = 0;
	uint filesToReimport = 0;

	pendantFolders.push(ReadFolder("Assets"));
	while (pendantFolders.empty() == false)
	{
		for (std::vector<std::string>::iterator it = pendantFolders.front().m_subFoldersPath.begin(); it != pendantFolders.front().m_subFoldersPath.end(); it++)
		{
			pendantFolders.push(ReadFolder(it->data()));
		}

		for (std::vector<std::string>::iterator it = pendantFolders.front().m_files.begin(); it != pendantFolders.front().m_files.end(); it++)
		{
			std::string path(pendantFolders.front().m_path);
			path += "/";
			path += it->data();
			filesToCheck.push(path.data());			
		}
		pendantFolders.pop();
	}

	std::vector<std::string> metaToSave;

	while (filesToCheck.empty() == false)
	{
		std::string tmp("." + App->m_importer->IsolateFileFormat(filesToCheck.front().data()));
		if (tmp != SCENE_FORMAT && tmp != ".txt")
		{
			totalFiles++;
			bool wantToImport = false;
			bool overwrite = false;
			std::map<std::string, Date>::iterator it = m_metaLastModificationDate.find(filesToCheck.front());
			if (it != m_metaLastModificationDate.end())
			{
				//The file exists in meta
				if (it->second != App->m_fileSystem->ReadFileDate(filesToCheck.front().data()))
				{
					//File existed, but has been modified
					overwrite = true;
					wantToImport = true;
				}
			}
			else
			{
				//File wasn't found in the current metaData
				wantToImport = true;
			}

			if (wantToImport)
			{
				LOG("Reimporting %s", filesToCheck.front().data());
				std::vector<MetaInf> toAdd = App->m_importer->Import(filesToCheck.front().data(), overwrite);
				if (toAdd.empty() == false)
				{
					std::multimap<ComponentType, MetaInf> tmp;
					for (std::vector<MetaInf>::iterator m = toAdd.begin(); m != toAdd.end(); m++)
					{
						tmp.insert(std::pair<ComponentType, MetaInf>(m->type, *m));
						m_toReload.push_back(m->uid);
					}

					metaToSave.push_back(filesToCheck.front());


					//Erasing the old data, so we can insert the new one
					std::map<std::string, std::multimap<ComponentType, MetaInf>>::iterator toPop = m_metaData.find(filesToCheck.front());
					if (toPop != m_metaData.end())
					{
						m_metaData.erase(toPop);
					}
					std::map<std::string, Date>::iterator toPop2 = m_metaLastModificationDate.find(filesToCheck.front());
					if (toPop2 != m_metaLastModificationDate.end())
					{
						m_metaLastModificationDate.erase(toPop2);
					}

					m_metaData.insert(std::pair<std::string, std::multimap<ComponentType, MetaInf>>(filesToCheck.front(), tmp));
					m_metaLastModificationDate.insert(std::pair<std::string, Date>(filesToCheck.front(), App->m_fileSystem->ReadFileDate(filesToCheck.front().data())));

					if (overwrite)
					{
						filesToReimport++;
					}
					else
					{
						filesToImport++;
					}

				}
			}
			else
			{
				//LOG("Up to date: %s", filesToCheck.front().data());
			}
		}
		filesToCheck.pop();
	}

	if (filesToImport != 0 || filesToReimport != 0)
	{
		LOG("Refreshed\n%u files were up to date.\n%u files were actualized.\n%u new files found.", totalFiles - filesToImport - filesToReimport, filesToReimport, filesToImport);
	}

	while(metaToSave.size() > 0)
	{
		SaveMetaData(m_metaData.find(metaToSave.back()));
		metaToSave.pop_back();
	}
	TIMER_READ_MS("Res Refresh");
}

const MetaInf* ModuleResourceManager::GetMetaData(const char * file, ComponentType type, const char * component)
{
	std::map<std::string, std::multimap<ComponentType, MetaInf>>::iterator f = m_metaData.find(file);
	if (f != m_metaData.end())
	{
		std::multimap<ComponentType, MetaInf> ::iterator it = f->second.find(type);
		while (it != f->second.end() && it->first == type)
		{
			if (it->second.name.compare(component) == 0)
			{
				return &it->second;
			}
			it++;
		}
	}
	return nullptr;
}

const MetaInf * ModuleResourceManager::GetMetaData(const char * file, ComponentType type, const uint64_t componentUID)
{
	std::map<std::string, std::multimap<ComponentType, MetaInf>>::iterator f = m_metaData.find(file);
	if (f != m_metaData.end())
	{
		std::multimap<ComponentType, MetaInf> ::iterator it = f->second.find(type);
		while (it != f->second.end() && it->first == type)
		{
			if (it->second.uid == componentUID)
			{
				return &it->second;
			}
			it++;
		}
	}
	return nullptr;
}

//TODO
//Improve this function, since it's slooooow to find stuff and iterates too much
const MetaInf * ModuleResourceManager::GetMetaData(ComponentType type, const char * component)
{
	std::map<std::string, std::multimap<ComponentType, MetaInf>>::iterator f = m_metaData.begin();
	while (f != m_metaData.end())
	{
		std::multimap<ComponentType, MetaInf> ::iterator it = f->second.find(type);
		while (it != f->second.end() && it->first == type)
		{
			if (it->second.name.compare(component) == 0)
			{
				return &it->second;
			}
			it++;
		}
		f++;
	}
	return nullptr;
}

/*
void ModuleResourceManager::RefreshFolder(const char * path)
{
	R_Folder meta = ReadFolderMeta(path);
	R_Folder real = ReadFolder(path);

	for (std::vector<std::string>::iterator metaIt = real.subFoldersPath.begin(); metaIt != real.subFoldersPath.end(); metaIt++)
	{
		for (std::vector<std::string>::iterator realIt = meta.subFoldersPath.begin(); realIt != meta.subFoldersPath.end(); realIt++)
		{

		}
	}

	for (std::vector<std::string>::iterator it = real.subFoldersPath.begin(); it != real.subFoldersPath.end(); it++)
	{

	}

}
*/
R_Folder ModuleResourceManager::ReadFolder(const char * path)
{
	R_Folder ret(App->m_importer->IsolateFileName(path).data(), path);

	std::vector<std::string> folders;
	std::vector<std::string> files;
	App->m_fileSystem->GetFilesIn(path, folders, files);

	for (std::vector<std::string>::iterator it = folders.begin(); it != folders.end(); it++)
	{
		std::string _path(path);
		_path += "/";
		_path += it->data();
		ret.m_subFoldersPath.push_back(_path);
	}
	for (std::vector<std::string>::iterator it = files.begin(); it != files.end(); it++)
	{
		ret.m_files.push_back(it->data());
	}

	return ret;
}

/*void ModuleResourceManager::CreateFolderMeta(R_Folder & folder)
{
	pugi::xml_document data;
	pugi::xml_node root_node;

	std::string fileName(folder.path);
	fileName += META_FORMAT;

	root_node = data.append_child("folder");

	root_node.append_attribute("name") = folder.name.data();

	for (std::vector<std::string>::iterator it = folder.subFoldersPath.begin(); it != folder.subFoldersPath.end(); it++)
	{
		pugi::xml_node sub = root_node.append_child("sub");
		sub.append_attribute("data") = it->data();
	}

	for (std::vector<std::string>::iterator it = folder.files.begin(); it != folder.files.end(); it++)
	{
		pugi::xml_node sub = root_node.append_child("file");
		sub.append_attribute("data") = it->data();
	}

	std::stringstream stream;
	data.save(stream);
	// we are done, so write data to disk
	App->m_fileSystem->Save(fileName.data(), stream.str().c_str(), stream.str().length());
	LOG("Created: %s", fileName.data());

	data.reset();
}
*/
/*R_Folder ModuleResourceManager::ReadFolderMeta(const char * path)
{
	char* buffer;
	std::string _path = path;
	_path += META_FORMAT;
	uint size = App->m_fileSystem->Load(_path.data(), &buffer);

	if (size > 0)
	{
		pugi::xml_document data;
		pugi::xml_node root;

		pugi::xml_parse_result result = data.load_buffer(buffer, size);
		RELEASE(buffer);

		if (result != NULL)
		{
			root = data.child("folder");
			if (root)
			{
				std::string name = root.attribute("name").as_string();

				R_Folder ret(name.data(), path);

				for (pugi::xml_node folder = root.child("sub"); folder != nullptr; folder = folder.next_sibling("folder"))
				{
					ret.subFoldersPath.push_back(folder.attribute("data").as_string());
				}

				for (pugi::xml_node files = root.child("file"); files != nullptr; files = files.next_sibling("file"))
				{
					ret.subFoldersPath.push_back(files.attribute("data").as_string());
				}

				return ret;
			}
		}
	}
	LOG("Tried to read an unexisting folder meta.\n%s", path)
	return R_Folder("", "");
}*/

Resource * ModuleResourceManager::Peek(uint64_t uid) const
{
	std::map<uint64_t, Resource*>::const_iterator it = m_resources.find(uid);
	if (it != m_resources.end())
	{
		return it->second;
	}
	return nullptr;
}

Resource * ModuleResourceManager::LinkResource(uint64_t uid)
{
	Resource* ret = nullptr;
	std::map<uint64_t, Resource*>::iterator it = m_resources.find(uid);
	if (it != m_resources.end())
	{
		ret = it->second;
		ret->m_numReferences++;
	}
	return ret;
}

uint64_t ModuleResourceManager::LinkResource(std::string resName, ComponentType type)
{
	Resource* ret = nullptr;
	std::map<ComponentType, std::map<std::string, uint64_t>>::iterator tmpMap = m_uidLib.find(type);
	//If previosuly there hasn't been loaded any resource with the same type as the requested one, we'll create the new map for this type of components
	if (tmpMap == m_uidLib.end())
	{
		m_uidLib.insert(std::pair<ComponentType, std::map<std::string, uint64_t>>(type, std::map<std::string, uint64_t>()));
		tmpMap = m_uidLib.find(type);
	}
	//We try to find the resource, to see if it's already loaded
	std::map<std::string, uint64_t> ::iterator it = tmpMap->second.find(resName);

	if (it != tmpMap->second.end())
	{
		//If it is, we just link it. If the resource exists but isn't loaded, ret will be nullptr, so we'll load it anyway
		ret = LinkResource(it->second);
	}

	if (ret == nullptr)
	{
		//If it isn't, we load the resource and we insert it to the resource library
		ret = LoadNewResource(resName, type);
		if (ret != nullptr)
		{
			m_resources.insert(std::pair<uint64_t, Resource*>(ret->m_uid, ret));
			tmpMap->second.insert(std::pair<std::string, uint64_t>(ret->m_name, ret->m_uid));
			ret->m_numReferences++;
		}
	}

	if (ret != nullptr)
	{
		return ret->m_uid;
	}
	return 0;
}

void ModuleResourceManager::UnlinkResource(Resource * res)
{
	if (res != nullptr)
	{
		UnlinkResource(res->m_uid);
	}
}

void ModuleResourceManager::UnlinkResource(uint64_t uid)
{
	std::map<uint64_t, Resource*>::iterator it = m_resources.find(uid);
	if (it != m_resources.end() && it->second != nullptr)
	{
		it->second->m_numReferences--;
		if (it->second->m_numReferences <= 0)
		{
			m_toDelete.push_back(it->first);
		}
	}
}

void ModuleResourceManager::UnlinkResource(std::string fileName, ComponentType type)
{
	fileName = App->m_importer->IsolateFileName(fileName.data());
	std::map<ComponentType, std::map<std::string, uint64_t>>::iterator tmpMap = m_uidLib.find(type);
	if (tmpMap != m_uidLib.end())
	{
		std::map<std::string, uint64_t>::iterator it = tmpMap->second.find(fileName);
		if (it != tmpMap->second.end())
		{
			UnlinkResource(it->second);
		}
	}
}

void ModuleResourceManager::DeleteNow()
{
	if (m_toDelete.empty() == false)
	{
		std::vector<uint64_t> tmp = m_toDelete;
		m_toDelete.clear();
		while (tmp.empty() == false)
		{
			uint64_t uid = tmp.back();

			//Erasing the resource itself
			std::map<uint64_t, Resource*>::iterator it = m_resources.find(uid);
			if (it != m_resources.end())
			{
				if (it->second->m_numReferences <= 0)
				{
					RELEASE(it->second);
					m_resources.erase(it);
				}
			}

			tmp.pop_back();
		}
	}
}

void ModuleResourceManager::ReloadNow()
{
	if (m_toReload.empty() == false)
	{
		std::vector<uint64_t> tmp = m_toReload;
		m_toReload.clear();

		while (tmp.empty() == false)
		{
			uint64_t uid = tmp.back();

			//Erasing the resource itself
			std::map<uint64_t, Resource*>::iterator it = m_resources.find(uid);
			if (it != m_resources.end())
			{
				std::string name = it->second->m_name;
				ComponentType type = it->second->GetType();
				uint nRefs = it->second->m_numReferences;

				RELEASE(it->second);
				it->second = LoadNewResource(name, type);
				it->second->m_numReferences = nRefs;
			}

			tmp.pop_back();
		}
	}
}

//Returns all loaded resources. Pretty slow, for debugging use only
const std::vector<Resource*> ModuleResourceManager::ReadLoadedResources() const
{
	std::vector<Resource*> ret;

	std::map<ComponentType, std::map<std::string, uint64_t>>::const_iterator tmpMap = m_uidLib.cbegin();
	//Iterating all maps of components from uidLib. This maps allow us to find the UID of each component through it's type and name
	//There's a different map for each type of component, so they're ordered
	for (; tmpMap != m_uidLib.end(); tmpMap++)
	{
		//Iterating inside each map
		std::map<std::string, uint64_t>::const_iterator it = tmpMap->second.cbegin();
		for (; it != tmpMap->second.end(); it++)
		{
			//Once we have the uid we want to push, we find the corresponding resource
			std::map<uint64_t, Resource*>::const_iterator res = m_resources.find(it->second);
			if (res != m_resources.cend())
			{
				ret.push_back(res->second);
			}
		}
	}

	return ret;
}

//Return explanation
//Returns a vector of pairs:
// -first is the file name
// -Second is the vector of resources from that file
std::vector<std::pair<std::string, std::vector<std::string>>> ModuleResourceManager::GetAvaliableResources(ComponentType type)
{
	std::vector<std::pair<std::string, std::vector<std::string>>> ret;

	std::map<std::string, std::multimap<ComponentType, MetaInf>>::iterator f = m_metaData.begin();
	for (;f != m_metaData.end();f++)
	{
		std::vector<std::string> thisFile;
		std::multimap<ComponentType, MetaInf> ::iterator it;
		if (type != ComponentType::none)
		{
			it = f->second.find(type);
		}
		else
		{
			it = f->second.begin();
		}

		for (;it != f->second.end() && (it->first == type || type == ComponentType::none); it++)
		{
			thisFile.push_back(it->second.name);
		}
		if (thisFile.empty() == false)
		{
			ret.push_back(std::pair<std::string, std::vector<std::string>>(f->first, thisFile));
		}
	}
	return ret;
}

bool ModuleResourceManager::GenerateDefaultShader()
{
	std::string ret;
	bool error = false;

	const char* src = m_defaultVertexBuf.c_str();
	GLuint vertexShader;	
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &src, NULL);
	glCompileShader(vertexShader);
	GLint success;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (success == 0)
	{
		error = true;
		GLchar infoLog[512];
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		ret += "\n------ Vertex shader ------\n";
		ret += infoLog;
		ret += '\n';
		LOG("Shader compilation error: %s", infoLog);
	}

	src = m_defaultFragmentBuf.c_str();
	GLuint fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &src, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (success == 0)
	{
		error = true;
		GLchar infoLog[512];
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		ret += "\n------ Fragment shader ------\n";
		ret += infoLog;
		ret += '\n';
		LOG("Shader compilation error: %s", infoLog);
	}

	GLuint shaderProgram;
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (success == 0)
	{
		error = true;
		GLchar infoLog[512];
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		ret += "\n------ Shader Program ------\n";
		ret += infoLog;
		ret += '\n';
		LOG("Shader link error: %s", infoLog);
	}

	if (shaderProgram != 0 && error == false)
	{
		if (m_defaultShader.m_program != 0)
		{
			glDeleteProgram(m_defaultShader.m_program);
		}
		m_defaultShader.m_program = shaderProgram;
	}	

	m_defaultShader.m_modelMatrix = glGetUniformLocation(m_defaultShader.m_program,"model_matrix");
	m_defaultShader.m_viewMatrix = glGetUniformLocation(m_defaultShader.m_program, "view_matrix");
	m_defaultShader.m_projectionMatrix = glGetUniformLocation(m_defaultShader.m_program, "projection_matrix");

	m_defaultShader.m_materialColor = glGetUniformLocation(m_defaultShader.m_program, "material_color");
	m_defaultShader.m_hasTexture = glGetUniformLocation(m_defaultShader.m_program, "has_texture");
	m_defaultShader.m_useLight = glGetUniformLocation(m_defaultShader.m_program, "use_light");
	m_defaultShader.m_time = glGetUniformLocation(m_defaultShader.m_program, "time");
	m_defaultShader.m_ambientColor = glGetUniformLocation(m_defaultShader.m_program, "ambient_color");
	m_defaultShader.m_globalLightDir = glGetUniformLocation(m_defaultShader.m_program, "global_light_direction");
	m_defaultShader.m_fogDistance = glGetUniformLocation(m_defaultShader.m_program, "fog_distance");
	m_defaultShader.m_fogColor = glGetUniformLocation(m_defaultShader.m_program, "fog_color");
	m_defaultShader.m_maxHeight = glGetUniformLocation(m_defaultShader.m_program, "max_height");

	glDetachShader(shaderProgram, vertexShader);
	glDetachShader(shaderProgram, fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	m_shadersResult = ret;

	return !error;
}

R_Folder::R_Folder(const char* name, R_Folder* parent) : m_name(name)
{
	if (parent != nullptr)
	{
		std::string myPath(parent->m_path);
		myPath += "/";
		myPath += name;
		m_path = myPath;
	}
	else
	{
		m_path = name;
	}
}

R_Folder::R_Folder(const char * name, const char * path) : m_name(name), m_path(path)
{
}
