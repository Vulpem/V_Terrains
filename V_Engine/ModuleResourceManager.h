#ifndef __MODULE_RESOURCE_MANAGER__
#define __MODULE_RESOURCE_MANAGER__

#include "Module.h"
#include "Globals.h"

#include "Component.h"
#include "Shader.h"

#include <map>

class Resource;
struct Date;

struct MetaInf
{
	std::string name;
	uint64_t uid;
	ComponentType type;
};

struct R_Folder
{
	R_Folder(const char* name, R_Folder* parent);
	R_Folder(const char* name, const char* path);

	std::string m_name;
	std::string m_path;

	std::vector<std::string> m_subFoldersPath;
	std::vector<std::string> m_files;
};


class ModuleResourceManager : public Module
{
public:
	
	ModuleResourceManager();
	~ModuleResourceManager();

	void OnEnable();
	UpdateStatus PreUpdate() override;
	UpdateStatus Update();
	UpdateStatus PostUpdate();
	void OnDisable();

	void Refresh();
	void ReimportAll();
	void ClearLibrary();
	void CreateLibraryDirs();

	void SaveMetaData();
	void SaveMetaData(std::map<std::string, std::multimap<ComponentType, MetaInf>>::iterator fileToSave);
	void LoadMetaData();

	const MetaInf* GetMetaData(const char* file, ComponentType type, const char* component);
	const MetaInf* GetMetaData(const char* file, ComponentType type, const uint64_t componentUID);
	const MetaInf* GetMetaData(ComponentType type, const char* component);

	Resource* Peek(uint64_t uid) const;

	//Link a resource to a new Resourced Component. Use only once per reference
	Resource* LinkResource(uint64_t uid);
	uint64_t LinkResource(std::string resName, ComponentType type);

	//Unlink a resource from a Resourced Component. Use only once per reference
	void UnlinkResource(Resource* res);
	void UnlinkResource(uint64_t uid);
	void UnlinkResource(std::string fileName, ComponentType type);

	void DeleteNow();
	void ReloadNow();

	const std::vector<Resource*> ReadLoadedResources() const;

	std::vector<std::pair<std::string, std::vector<std::string>>> GetAvaliableResources(ComponentType type = ComponentType::none);

	bool m_autoRefresh = false;
	int m_refreshInterval = 10;

private:
	Resource* LoadNewResource(std::string fileName, ComponentType type);
	R_Folder ReadFolder(const char* path);

	float m_refreshTimer = 0.0f;

	//Allows us to find any resource via its UID
	std::map<uint64_t, Resource*> m_resources;

	//Allows us to find any resource UID via its type and name
	std::map<ComponentType, std::map<std::string, uint64_t>> m_uidLib;

	std::vector<uint64_t> m_toDelete;
	std::vector<uint64_t> m_toReload;

	//metaData of all files
	//The first map contains the name of the file the resource came from and a map of all the resources linked to that file
	//The second map has all resources from the file ordered by type with its correspondant info
	std::map<std::string, std::multimap<ComponentType,MetaInf>> m_metaData;

	//Contains the Date each file was last modified
	std::map<std::string, Date> m_metaLastModificationDate;

public:
	// --------------------------------- Shaders stuff ----------------------------------

	bool GenerateDefaultShader();
	Shader GetDefaultShader() { return m_defaultShader; }

	std::string m_defaultVertexBuf;
	std::string m_defaultFragmentBuf;

	std::string m_shadersResult;

	private:	
	Shader m_defaultShader;
};

#endif