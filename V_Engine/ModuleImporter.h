#ifndef __MODULE_IMPORTER__
#define __MODULE_IMPORTER__

#include "Module.h"
#include "Globals.h"

#include "Component.h"

#include "Math.h"
#include <vector>
#include <map>

struct aiScene;
struct aiMesh;
struct aiNode;
class GameObject;
class Material;
class mesh;
class R_Mesh;
class R_Material;
class R_Texture;
class R_Shader;

enum ShaderTypes;

struct MetaInf;

class ModuleImporter : public Module
{
public:
	
	ModuleImporter(Application* app, bool start_enabled = true);
	~ModuleImporter();

	bool Init() override;

	void Start() override;
	void CleanUp() override;

// ------------------------------- IMPORTING ------------------------------- 
	std::vector<MetaInf> Import(const char* path, bool overWritting = false);

	std::vector<MetaInf> Import3dScene(const char* filePath, bool overWritting = false);
	std::vector<MetaInf> ImportImage(const char* filePath, bool overWritting = false);
	std::vector<MetaInf> ImportShader(const char * filePath, bool overWritting = false);

private:
	std::vector<MetaInf> ImportGameObject(const char* path, const aiNode* toLoad, const aiScene* scene, uint64_t uid = 0, bool overWriting = false);
	uint64_t ImportMesh(aiMesh* toLoad, const aiScene* scene, const char* vGoName, uint& textureID, uint64_t uid = 0);
	uint64_t ImportMaterial(const aiScene* scene, std::vector<uint>& matsIndex, const char* matName, uint64_t uid = 0);
public:
	// ------------------------------- LOADING ------------------------------- 
	GameObject* LoadVgo(const char* fileName, const char* vGoName, GameObject* parent = nullptr);

	R_Mesh* LoadMesh(const char* resName);
	R_Material* LoadMaterial(const char* resName);
	R_Texture* LoadTexture(const char* resName);
	R_Shader* LoadShader(const char* resName);

// ------------------------------- UTILITY ------------------------------- 
	std::string IsolateFileFormat(const char* file);
	std::string IsolateFileName(const char* file);
	std::string IsolateFile(const char* file);
	std::string NormalizePath(const char* path);
	std::string RemoveFormat(const char* file);

private:
	std::string CompileShader(const char* vertexBuf, const char* fragmentBuf, uint& shaderProgram);

	template <typename type>
	char* CopyMem(char* copyTo, const type* copyFrom, int amount = 1)
	{
		uint bytes = sizeof(type) * amount;
		memcpy(copyTo, copyFrom, bytes);
		return copyTo + bytes;
	}
};

#endif