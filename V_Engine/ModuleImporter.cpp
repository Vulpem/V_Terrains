#include "Globals.h"
#include "Application.h"

#include "ModuleImporter.h"

#include "ModuleFileSystem.h"

#include "ModuleResourceManager.h"

#include "GameObject.h"

#include "AllResources.h"
#include "AllComponents.h"

#include "Devil\include\il.h"
#include "Devil\include\ilu.h"
#include "Devil\include\ilut.h"

#include "Assimp\include\cimport.h"
#include "Assimp\include\scene.h"
#include "Assimp\include\postprocess.h"
#include "Assimp\include\cfileio.h"

#pragma comment(lib, "Assimp/libx86/assimp.lib")

#pragma comment(lib, "Devil/libx86/DevIL.lib")
#pragma comment(lib, "Devil/libx86/ILU.lib")
#pragma comment(lib, "Devil/libx86/ILUT.lib")

//------------------------- MODULE --------------------------------------------------------------------------------

ModuleImporter::ModuleImporter() : Module()
{
}

// DestructorF
ModuleImporter::~ModuleImporter()
{

}

// Called before render is available
bool ModuleImporter::Init()
{
	bool ret = true;

	//Init for assimp
	struct aiLogStream stream;
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_DEBUGGER, nullptr);
	aiAttachLogStream(&stream);

	//Init for devIL
	ilInit();

	ILuint devilError = ilGetError();

	if (devilError != IL_NO_ERROR)
	{
		printf("Devil Error (ilInit: %s\n", iluErrorString(devilError));
		exit(2);
	}
		
	return ret;
}

void ModuleImporter::OnEnable()
{
	ilutRenderer(ILUT_OPENGL);
}

// Called before quitting
void ModuleImporter::OnDisable()
{
	aiDetachAllLogStreams();
}




// ------------------------------- IMPORTING ------------------------------- 

//Import a file, without specificating which type of file it is.
//The "overWritting" bool will check the resource manager metadata overwrite existing objects instead of creating new ones
std::vector<MetaInf> ModuleImporter::Import(const char * path, bool overWritting)
{
	std::vector<MetaInf> ret;
	ret = Import3dScene(path, overWritting);

	if (ret.empty())
	{
		ret = ImportImage(path, overWritting);
	}
	if (ret.empty())
	{
		ret = ImportShader(path, overWritting);
	}
	return ret;
}

//Import a specific 3D model and decompose it
std::vector<MetaInf> ModuleImporter::Import3dScene(const char * filePath, bool overWritting)
{
	std::vector<MetaInf> ret;
	//Making sure the file recieved is supported by the assimp library
	std::string fmt = IsolateFileFormat(filePath);
	std::string supportedFormats;
	supportedFormats += " FBX";
	for (uint n = 0; n < aiGetImportFormatCount(); n++)
	{
		supportedFormats += " ";
		supportedFormats += aiGetImportFormatDescription(n)->mFileExtensions;		
	}
	if (supportedFormats.find(fmt) == std::string::npos)
	{
		return ret;
	}

	LOG("\n ------ [Started importing 3D Scene] ------ ");
	LOG("Importing 3D scene: %s", filePath);

	//Loading the aiScene from Assimp
	const aiScene* scene = aiImportFileEx(filePath, aiProcessPreset_TargetRealtime_MaxQuality, App->m_fileSystem->GetAssimpIO());

	if (scene != nullptr)
	{
		if (scene->HasMeshes())
		{
			uint64_t uid = 0;
			if (overWritting == true)
			{
				const MetaInf* inf = App->m_resourceManager->GetMetaData(filePath, ComponentType::GO, "RootNode");
				if (inf) { uid = inf->uid; }
			}
			ret = ImportGameObject(filePath, scene->mRootNode, scene, uid, overWritting);
		}
		if (scene)
		{
			aiReleaseImport(scene);
			scene = nullptr;
		}
		LOG(" ------ [End of importing 3D Scene] ------\n");
	}
	else
	{
		LOG("Error loading scene %s", filePath);
		LOG("%s", aiGetErrorString());
	}
	return ret;
}


//Import any image to dds
std::vector<MetaInf> ModuleImporter::ImportImage(const char * filePath, bool overWritting)
{
	std::vector<MetaInf> ret;

	// Extracted from
	//http://openil.sourceforge.net/features.php
	std::string supportedFormats("bmp dcx dds hdr icns ico cur iff gif jpg jpe jpeg jp2 lbm png raw tif tga");

	if (supportedFormats.find(IsolateFileFormat(filePath)) == std::string::npos)
	{
		return ret;
	}

	LOG("\nStarted importing texture %s", filePath);
	char* buffer = nullptr;
	uint size;

	size = App->m_fileSystem->Load(filePath, &buffer);
	if (size > 0)
	{
		ILuint image;
		ILuint newSize;
		ILubyte *data;

		ilGenImages(1, &image);
		ilBindImage(image);

		if (ilLoadL(IL_TYPE_UNKNOWN, (const void*)buffer, size))
		{
			ilSetInteger(IL_DXTC_FORMAT, IL_DXT5);// To pick a specific DXT compression use
			newSize = ilSaveL(IL_DDS, NULL, 0); // Get the size of the data buffer

			if (newSize > 0)
			{
				data = new ILubyte[newSize]; // allocate data buffer

				ilEnable(IL_FILE_OVERWRITE);
				if (ilSaveL(IL_DDS, data, newSize) > 0)
				{
					uint64_t uid = 0;
					if (overWritting)
					{
						const MetaInf* inf = App->m_resourceManager->GetMetaData(filePath, ComponentType::texture, IsolateFileName(filePath).data());
						if (inf)
						{
							uid = inf->uid;
						}
					}

					// Save to buffer with the ilSaveIL function
					if (uid == 0)
					{
						uid = GenerateUUID();
					}
					char toCreate[524];
					sprintf(toCreate, "Library/Textures/%llu%s", uid, TEXTURE_FORMAT);

					App->m_fileSystem->Save(toCreate, (const char*)data, newSize);

					MetaInf tmp;
					tmp.name = IsolateFileName(filePath);
					tmp.type = ComponentType::texture;
					tmp.uid = uid;
					ret.push_back(tmp);

					LOG("Succesfully imported!");
				}
				else
				{
					LOG("devIl couldn't create the .dds!");
				}
				RELEASE_ARRAY(data);
			}
		}
		else
		{
			LOG("devIl couldn't load the image");
		}
		ilDeleteImages(1, &image);
	}
	else
	{
		LOG("Couldn't open the file!");
	}
	RELEASE_ARRAY(buffer);
	return ret;
}


//https://www.opengl.org/wiki/Shader_Compilation#Shader_object_compilation

//Compile any amount of shaders and store the resulting program in a binary precompiled file
std::vector<MetaInf> ModuleImporter::ImportShader(const char * filePath, bool overWritting)
{
	std::vector<MetaInf> ret;
	std::string supportedFormats;
	supportedFormats += std::string(SHADER_FRAGMENT_FORMAT).substr(1);
	supportedFormats += " ";
	supportedFormats += std::string(SHADER_VERTEX_FORMAT).substr(1);
	if (supportedFormats.find(IsolateFileFormat(filePath)) == std::string::npos)
	{
		return ret;
	}

	LOG("\nStarted importing shader %s", filePath);

	std::string shaderName = IsolateFileName(filePath);

	if (overWritting == false)
	{
		//If already exists a shader with this name, this won't be imported
		const MetaInf* existingShader = App->m_resourceManager->GetMetaData(ComponentType::shader, shaderName.data());
		if (existingShader != nullptr)
		{
			ret.push_back(*existingShader);
			return ret;
		}
	}

	std::string vertexFile = RemoveFormat(filePath) + SHADER_VERTEX_FORMAT;
	std::string fragmentFile = RemoveFormat(filePath) + SHADER_FRAGMENT_FORMAT;
	

	char* vertexBuffer = nullptr;
	uint vertexSize;

	char* fragmentBuffer = nullptr;
	uint fragmentSize;

	vertexSize = App->m_fileSystem->Load(vertexFile.data(), &vertexBuffer) + 1;
	fragmentSize = App->m_fileSystem->Load(fragmentFile.data(), &fragmentBuffer) + 1;
	if (vertexBuffer != nullptr) { vertexBuffer[vertexSize - 1] = '\0'; }
	if (fragmentBuffer != nullptr) { fragmentBuffer[fragmentSize - 1] = '\0'; }

	if (vertexSize > 0 || fragmentSize > 0)
	{
		GLuint shaderProgram = 0;
		std::string compilationResult = CompileShader(vertexBuffer, fragmentBuffer, shaderProgram);

		if (shaderProgram != 0)
		{
			//Getting shader program in binary form and storing it
			GLint binaryLength = 0;
			glGetProgramiv(shaderProgram, GL_PROGRAM_BINARY_LENGTH, &binaryLength);

			//Buffer where the program will be stored. Here at the beggining, we'll store the format
			char* binaryProgram = new char[binaryLength + sizeof(GLenum) + sizeof(GLint)];
			//Where we'll store the length of the program
			char* sizeIt = binaryProgram + sizeof(GLenum);
			//from here on, the program itself
			char* programIt = sizeIt + sizeof(GLint);
			GLenum binaryFormat;
			glGetProgramBinary(shaderProgram, binaryLength, NULL, &binaryFormat, programIt);
			memcpy(binaryProgram, &binaryFormat, sizeof(GLenum));
			memcpy(sizeIt, &binaryLength, sizeof(GLint));

			uint64_t uid = 0;
			if (overWritting)
			{
				const MetaInf* inf = App->m_resourceManager->GetMetaData(filePath, ComponentType::shader, IsolateFileName(filePath).data());
				if (inf)
				{
					uid = inf->uid;
				}
			}

			// Save to buffer with the ilSaveIL function
			if (uid == 0)
			{
				uid = GenerateUUID();
			}
			char toCreate[524];
			sprintf(toCreate, "Library/Shaders/%llu%s", uid, SHADER_PROGRAM_FORMAT);

			App->m_fileSystem->Save(toCreate, binaryProgram, binaryLength + sizeof(GLenum) + sizeof(GLint));

			MetaInf tmp;
			tmp.name = shaderName;
			tmp.type = ComponentType::shader;
			tmp.uid = uid;
			ret.push_back(tmp);

			LOG("Succesfully imported!");
		}

		char* resultLog = new char[compilationResult.length() + 1];
		strcpy(resultLog, compilationResult.data());
		char toCreate[524];
		sprintf(toCreate, "%s%s", App->m_importer->RemoveFormat(filePath).data(), "_result.txt");
		App->m_fileSystem->Save(toCreate, resultLog, compilationResult.length() + 1);

		sprintf(toCreate, "%s%s%s", "Library/Shaders/", App->m_importer->IsolateFileName(filePath).data(), "_result.txt");
		App->m_fileSystem->Save(toCreate, resultLog, compilationResult.length() + 1);
		RELEASE_ARRAY(resultLog);
	}

	return ret;
}

//Import a specific GO. Create a vGO with transform and hierarchy, and call ImportMesh && ImportMaterial
std::vector<MetaInf> ModuleImporter::ImportGameObject(const char* path, const aiNode* NodetoLoad, const aiScene* scene, uint64_t uid, bool overWritting)
{
	std::vector<MetaInf> ret;

	if (uid == 0)
	{
		uid = GenerateUUID();
	}

	//Setting Name

	std::string vGoName = (NodetoLoad->mName.data);
	uint nameLen = vGoName.length();

	LOG("Importing GameObject %s", vGoName.data());

	//					rot + scal + pos				nMeshes			Material
	uint transformSize = sizeof(float) * (4 + 3 + 3) + sizeof(uint) + sizeof(uint);
	char* transform = new char[transformSize];
	char* transformIt = transform;

	aiQuaternion rot;
	aiVector3D scal;
	aiVector3D pos;

	NodetoLoad->mTransformation.Decompose(scal, rot, pos);

	float t[10];
	t[0] = rot.x;
	t[1] = rot.y;
	t[2] = rot.z;
	t[3] = rot.w;
	t[4] = scal.x;
	t[5] = scal.y;
	t[6] = scal.z;
	t[7] = pos.x;
	t[8] = pos.y;
	t[9] = pos.z;

	std::vector<uint> materials;
	std::vector<uint64_t> meshes;
	uint nMeshes = NodetoLoad->mNumMeshes;
	for (uint n = 0; n <nMeshes; n++)
	{
		uint matIndex;

		aiMesh* toLoad = scene->mMeshes[NodetoLoad->mMeshes[n]];

		char meshName[1024];
		sprintf(meshName, "%s%s_%i",vGoName.data(), toLoad->mName.data, n);

		uint64_t meshUid = 0;
		if (overWritting)
		{
			const MetaInf* inf = App->m_resourceManager->GetMetaData(path, ComponentType::mesh, meshName);
			if (inf)
			{
				meshUid = inf->uid;
			}
		}
		uint64_t meshUID = ImportMesh(toLoad, scene, vGoName.data(), matIndex, meshUid);
		MetaInf meshMeta;
		meshMeta.name = meshName;
		meshMeta.uid = meshUID;
		meshMeta.type = ComponentType::mesh;
		ret.push_back(meshMeta);

		meshes.push_back(meshUID);
		materials.push_back(matIndex);
	}
	uint hasMaterial = 0;
	if (materials.empty() == false)
	{
		uint64_t matUid = 0;
		if (overWritting)
		{
			const MetaInf* inf = App->m_resourceManager->GetMetaData(path, ComponentType::mesh, vGoName.data());
			if (inf)
			{
				matUid = inf->uid;
			}
		}

		MetaInf matMeta;
		matMeta.uid = ImportMaterial(scene, materials, vGoName.data(), matUid);
		matMeta.name = vGoName;
		matMeta.type = ComponentType::material;
		ret.push_back(matMeta);
		hasMaterial = 1;
	}

	transformIt = CopyMem<float>(transformIt, t, 10);
	transformIt = CopyMem<uint>(transformIt, &nMeshes);
	transformIt = CopyMem<uint>(transformIt, &hasMaterial);

	uint nChilds = NodetoLoad->mNumChildren;
	unsigned long long* childs = new unsigned long long[nChilds];

	uint childFileSize =
		//nChilds			each child
		sizeof(uint) + sizeof(unsigned long long) * nChilds;

	//Deciding the uid for each child
	for (uint n = 0; n < nChilds; n++)
	{
		childs[n] = 0;
		if (overWritting == true)
		{
			const MetaInf* inf = App->m_resourceManager->GetMetaData(path, ComponentType::GO, NodetoLoad->mChildren[n]->mName.data);
			if (inf != nullptr)
			{
				childs[n] = inf->uid;
			}
		}

		if(childs[n] == 0)
		{
			childs[n] = GenerateUUID();
		}
		
	}

	char* file_childs = new char[childFileSize];
	char* childsIt = file_childs;

	//nCHilds
	childsIt = CopyMem<uint>(childsIt, &nChilds);

	//childs UIDs
	childsIt = CopyMem<unsigned long long>(childsIt, childs, nChilds);


	//Getting the total size of the real file
	uint realFileSize = 0;
	realFileSize += transformSize;
	realFileSize += sizeof(unsigned long long) * nMeshes + 256 * hasMaterial;
	realFileSize += childFileSize;

	//Copying all the buffers we created into a single bigger buffer
	char* realFile = new char[realFileSize];
	char* realIt = realFile;

	//file_0
	realIt = CopyMem<char>(realIt, transform, transformSize);

	for (int n = 0; n < nMeshes; n++)
	{
		realIt = CopyMem<uint64_t>(realIt, &meshes[n]);
	}
	if (hasMaterial)
	{
		realIt = CopyMem<char>(realIt, vGoName.data(), 256);
	}

	//childs
	realIt = CopyMem<char>(realIt, file_childs, childFileSize);

	RELEASE_ARRAY(transform);
	RELEASE_ARRAY(file_childs);

	// ---------------- Creating the save file and writting it -----------------------------------------

	char toCreate[524];
	sprintf(toCreate, "Library/vGOs/%llu%s", uid, GO_FORMAT);

	App->m_fileSystem->Save(toCreate, realFile, realFileSize);

	RELEASE_ARRAY(realFile);

	MetaInf GoMeta;
	GoMeta.name = vGoName;
	GoMeta.uid = uid;
	GoMeta.type = ComponentType::GO;
	ret.push_back(GoMeta);

	//Importing also all the childs
	for (uint n = 0; n < NodetoLoad->mNumChildren; n++)
	{
		std::vector<MetaInf> childsMeta;

		childsMeta = ImportGameObject(path, NodetoLoad->mChildren[n], scene, childs[n], overWritting);

		for (std::vector<MetaInf>::iterator it = childsMeta.begin(); it != childsMeta.end(); it++)
		{
			ret.push_back(*it);
		}
	}

	RELEASE_ARRAY(childs);
	return ret;
}

//Create a vmesh from a certain mesh. COntains all mesh info
uint64_t ModuleImporter::ImportMesh(aiMesh* toLoad, const aiScene* scene, const char* vGoName,uint& materialID, uint64_t uid)
{
	//Importing vertex
	uint m_nVertices = toLoad->mNumVertices;
	float* vertices = new float[m_nVertices * 3];
	memcpy_s(vertices, sizeof(float) * m_nVertices * 3, toLoad->mVertices, sizeof(float) * m_nVertices * 3);

	AABB aabb;
	aabb.SetNegativeInfinity();

	float* it = vertices;
	for (uint n = 0; n < m_nVertices * 3; n += 3)
	{
		float* x = it;
		float* y = x;
		y++;
		float* z = y;
		z++;
		aabb.Enclose(float3(*x, *y, *z));
		it += 3;
	}

	//Importing normals
	float* normals = nullptr;
	uint numNormals = 0;
	if (toLoad->HasNormals())
	{
		numNormals = m_nVertices;
		normals = new float[m_nVertices * 3];
		memcpy(normals, toLoad->mNormals, sizeof(float) * m_nVertices * 3);
	}

	//Importing texture coords
	float* textureCoords = nullptr;
	uint numTextureCoords = 0;
	if (toLoad->HasTextureCoords(0))
	{
		numTextureCoords = m_nVertices;
		textureCoords = new float[m_nVertices * 2];

		aiVector3D* tmpVect = toLoad->mTextureCoords[0];
		for (uint n = 0; n < m_nVertices * 2; n += 2)
		{
			textureCoords[n] = tmpVect->x;
			textureCoords[n + 1] = tmpVect->y;
			tmpVect++;
		}
	}

	//Importing texture path for this mesh
	materialID = toLoad->mMaterialIndex;

	//Importing index (3 per face)
	uint m_nIndices = 0;
	uint* indices = nullptr;

	aiFace* currentFace = toLoad->mFaces;

	m_nIndices = toLoad->mNumFaces * 3;
	indices = new uint[m_nIndices];
	//If this boolean is still false at the end of the for bucle, not a single face had been loaded. This mesh is unexistant
	bool meshExists = false;

	for (uint i = 0; i < m_nIndices; i += 3)
	{
		if (currentFace->mNumIndices != 3)
		{
			LOG("------------------------------------------\nA loaded face had %i vertices, will be ignored!\n------------------------------------------", currentFace->mNumIndices);
		}
		else
		{
			meshExists = true;
			indices[i] = currentFace->mIndices[0];
			indices[i + 1] = currentFace->mIndices[1];
			indices[i + 2] = currentFace->mIndices[2];
		}
		currentFace++;
	}

	uint meshSize =
		//Mesh exists? 
		sizeof(bool) +

		//m_nVertices				   vertices				num_normals   normals
		sizeof(uint) + sizeof(float) * m_nVertices * 3 + sizeof(uint) + sizeof(float) * numNormals * 3

		//num_texture coords  texture Coords		             
		+ sizeof(uint) + sizeof(float) * numTextureCoords * 2 +

		//num indices								indices
		+sizeof(uint) + sizeof(uint) * m_nIndices
		//aabb
		+ sizeof(float) * 6;

	char* mesh = new char[meshSize];
	char* meshIt = mesh;

	//Does this mesh actually exist?
	meshIt = CopyMem<bool>(meshIt, &meshExists);

	//Num vertices
	meshIt = CopyMem<uint>(meshIt, &m_nVertices);

	//Vertices
	meshIt = CopyMem<float>(meshIt, vertices, m_nVertices * 3);

	//Num Normals
	meshIt = CopyMem<uint>(meshIt, &numNormals);

	if (numNormals > 0)
	{
		//Normals
		meshIt = CopyMem<float>(meshIt, normals, numNormals * 3);
	}

	//Num texture coords
	meshIt = CopyMem<uint>(meshIt, &numTextureCoords);

	if (numTextureCoords > 0)
	{
		//texture coords
		meshIt = CopyMem<float>(meshIt, textureCoords, numTextureCoords * 2);
	}

	//m_nIndices
	meshIt = CopyMem<uint>(meshIt, &m_nIndices);

	//indices
	meshIt = CopyMem<uint>(meshIt, indices, m_nIndices);

	//AABB
	meshIt = CopyMem<float3>(meshIt, &aabb.maxPoint);
	meshIt = CopyMem<float3>(meshIt, &aabb.minPoint);


	RELEASE_ARRAY(vertices);
	RELEASE_ARRAY(normals);
	RELEASE_ARRAY(textureCoords);
	RELEASE_ARRAY(indices);

	if (uid == 0)
	{
		uid = GenerateUUID();
	}
	char toCreate[524];
	sprintf(toCreate, "Library/Meshes/%llu%s", uid, MESH_FORMAT);

	App->m_fileSystem->Save(toCreate, mesh, meshSize);

	RELEASE_ARRAY(mesh);

	return uid;
}

//Create a vmat from a material, with colors & texture names
uint64_t ModuleImporter::ImportMaterial(const aiScene * scene, std::vector<uint>& matsIndex, const char* matName, uint64_t uid)
{
	if (matsIndex.empty() == false)
	{
		uint realSize = sizeof(uint) + sizeof(long long);
		uint nTextures = matsIndex.size();

		if (uid == 0)
		{
			uid = GenerateUUID();
		}

		uint* materialsSize = new uint[matsIndex.size()];
		char** materials = new char*[matsIndex.size()];
		uint n = 0;
		for (std::vector<uint>::iterator it = matsIndex.begin(); it != matsIndex.end(); it++)
		{
			aiString texturePath;
			scene->mMaterials[(*it)]->GetTexture(aiTextureType::aiTextureType_DIFFUSE, 0, &texturePath);
			char tmp[1024];
			strcpy(tmp, texturePath.data);			

			std::string textureName = IsolateFileName(tmp);
			uint textureNameLen = textureName.length() + 1;

			//Importing color for this mesh
			aiColor3D col;
			float color[3];
				if (scene->mMaterials[(*it)]->Get(AI_MATKEY_COLOR_DIFFUSE, col) == aiReturn_SUCCESS)
				{
					color[0] = col.r;
					color[1] = col.g;
					color[2] = col.b;
				}
			materialsSize[n] = sizeof(uint) + sizeof(char) * textureNameLen + sizeof(float3);
			realSize += materialsSize[n];
			materials[n] = new char[materialsSize[n]];
			char* materialIt = materials[n];

			materialIt = CopyMem<uint>(materialIt, &textureNameLen, 1);
			materialIt = CopyMem<char>(materialIt, textureName.data(), textureNameLen);
			materialIt = CopyMem<float>(materialIt, color, 3);

			n++;
		}

		char* realMat = new char[realSize];
		char* realIt = realMat;

		realIt = CopyMem<uint>(realIt, &nTextures);

		for (int n = 0; n < matsIndex.size(); n++)
		{
			realIt = CopyMem<char>(realIt, materials[n], materialsSize[n]);
		}

		char toCreate[524];
		sprintf(toCreate, "Library/Materials/%llu%s", uid, MATERIAL_FORMAT);

		App->m_fileSystem->Save(toCreate, realMat, realSize);	

		for (int n = 0; n < matsIndex.size(); n++)
		{
			RELEASE_ARRAY(materials[n]);
		}
		RELEASE_ARRAY(materialsSize);
		RELEASE_ARRAY(materials);
		RELEASE_ARRAY(realMat);
		return uid;
	}
	return 0;
}



// ------------------------------- LOADING ------------------------------- 

//The parent variable is for internal use, this is a recursive called function. Please, leave it at NULL, as well as meshesFolder
Gameobject * ModuleImporter::LoadVgo(const char * fileName, const char* vGoName, Gameobject* parent)
{
	const MetaInf* meta = App->m_resourceManager->GetMetaData(fileName, ComponentType::GO, vGoName);
	if (meta != nullptr)
	{
		char path[1024];
		sprintf(path, "Library/vGOs/%llu%s", meta->uid, GO_FORMAT);

		char* file = nullptr;

		if (parent == nullptr)
		{
			LOG("\n ------- [Began loading %s] ---------", fileName);
		}

		LOG("Loading vgo %s", vGoName);

		if (App->m_fileSystem->Exists(path))
		{
			int size = App->m_fileSystem->Load(path, &file);
			if (file != nullptr && size > 0)
			{
				char* It = file;

				//Creating basic components for a GameObject
				Gameobject* ret = new Gameobject;

				//Setting name
				ret->SetName(meta->name.data());
				//Setting parent
				if (parent != nullptr)
				{
					ret->GetTransform()->SetParent(parent->GetTransform());
				}
				else
				{
					ret->GetTransform()->SetParent(nullptr);
				}

				//Setting transform
				float _transform[10];
				uint bytes = sizeof(float) * 10;
				memcpy(_transform, It, bytes);
				It += bytes;

				ret->GetTransform()->SetLocalRot(_transform[0], _transform[1], _transform[2], _transform[3]);
				ret->GetTransform()->SetLocalScale(_transform[4], _transform[5], _transform[6]);
				ret->GetTransform()->SetLocalPos(_transform[7], _transform[8], _transform[9]);

				ret->GetTransform()->UpdateEditorValues();

				//Number of meshes
				uint nMeshes = 0;
				bytes = sizeof(uint);
				memcpy(&nMeshes, It, bytes);
				It += bytes;

				//HasMaterial
				uint hasMaterial = 0;
				bytes = sizeof(uint);
				memcpy(&hasMaterial, It, bytes);
				It += bytes;

				//Loading each mesh
				for (uint n = 0; n < nMeshes; n++)
				{
					uint64_t meshUID;
					bytes = sizeof(uint64_t);
					memcpy(&meshUID, It, bytes);
					It += bytes;
					std::string meshName = App->m_resourceManager->GetMetaData(fileName,ComponentType::mesh, meshUID)->name;
					ret->CreateComponent(ComponentType::mesh, meshName);
				}

				if (hasMaterial != 0)
				{
					char materialName[256];
					bytes = sizeof(char) * 256;
					memcpy(&materialName, It, bytes);
					It += bytes;
					ret->CreateComponent(ComponentType::material, materialName);
				}

				//Num childs
				uint nChilds = 0;
				bytes = sizeof(uint);
				memcpy(&nChilds, It, bytes);
				It += bytes;

				if (nChilds > 0)
				{
					std::vector<uint64_t> childs;
					//Loading each child name into a separate string
					for (uint n = 0; n < nChilds; n++)
					{
						uint64_t childUID;
						bytes = sizeof(unsigned long long);
						memcpy(&childUID, It, bytes);
						It += bytes;

						childs.push_back(childUID);
					}

					std::vector<uint64_t>::iterator childsUID = childs.begin();
					while (childsUID != childs.end())
					{
						const MetaInf* inf = App->m_resourceManager->GetMetaData(fileName, ComponentType::GO, *childsUID);
						if (inf != nullptr)
						{
							Gameobject* child = LoadVgo(fileName, inf->name.data(), ret);
							if (child)
							{
								ret->GetTransform()->AddChild(child->GetTransform());
							}
						}
						else
						{
							LOG("Error loading child for %s", vGoName);
						}
						childsUID++;
					}
				}

				delete[] file;

				return ret;
			}
			else
			{
				LOG("Something went wrong while loading %s", fileName);
			}
		}
		else
		{
			LOG("Woops! This .vgo doesn't really exist.")
		}
	}
	return nullptr;
}

R_Mesh* ModuleImporter::LoadMesh(const char * resName)
{
	char* file = nullptr;
	R_Mesh* newMesh = nullptr;
	const MetaInf* inf = App->m_resourceManager->GetMetaData(ComponentType::mesh, resName);
	if (inf != nullptr)
	{
		char filePath[526];
		sprintf(filePath, "Library/Meshes/%llu%s", inf->uid, MESH_FORMAT);

		LOG("Loading mesh %s", inf->name.data());

		if (App->m_fileSystem->Exists(filePath))
		{
			int size = App->m_fileSystem->Load(filePath, &file);
			if (file != nullptr && size > 0)
			{
				char* It = file;

				//Does this mesh exist?
				bool _meshExists = true;
				uint bytes = sizeof(bool);
				memcpy(&_meshExists, It, bytes);
				It += bytes;

				if (_meshExists == true)
				{
					newMesh = new R_Mesh(inf->uid);

					newMesh->m_name = resName;

					//Num vertices
					bytes = sizeof(uint);
					memcpy(&newMesh->m_numVertices, It, bytes);
					It += bytes;

					//Actual vertices
					newMesh->m_vertices = new float3[newMesh->m_numVertices];
					bytes = sizeof(float3) * newMesh->m_numVertices;
					memcpy(newMesh->m_vertices, It, bytes);
					It += bytes;	


					//Num normals
					uint numNormals = 0;
					bytes = sizeof(uint);
					memcpy(&numNormals, It, bytes);
					It += bytes;

					if (numNormals > 0)
					{
						//Normals
						newMesh->m_normals = new float3[numNormals];
						bytes = sizeof(float3) * numNormals;
						memcpy(newMesh->m_normals, It, bytes);
						It += bytes;
						newMesh->m_hasNormals = true;
					}

					//Num texture coords
					uint numTextureCoords = 0;
					bytes = sizeof(uint);
					memcpy(&numTextureCoords, It, bytes);
					It += bytes;

					float2* textureCoords = nullptr;
					if (numTextureCoords > 0)
					{
						//Texture coords
						textureCoords = new float2[numTextureCoords];
						bytes = sizeof(float2) * numTextureCoords;
						memcpy(textureCoords, It, bytes);
						It += bytes;
						newMesh->m_hasUVs = true;
					}

					float* data = new float[(3 + 3 + 2) * newMesh->m_numVertices];
					float* data_it = data;
					for (uint n = 0; n < newMesh->m_numVertices; n++)
					{
						memcpy(data_it, &newMesh->m_vertices[n], sizeof(float3));
						data_it += 3;
						
						float3 normal = float3::zero;
						if (newMesh->m_hasNormals)
						{
							normal = newMesh->m_normals[n];
						}
						memcpy(data_it, &normal, sizeof(float3));
						data_it += 3;

						float2 uv = float2::zero;
						if (newMesh->m_hasUVs)
						{
							uv = textureCoords[n];
						}
						memcpy(data_it, &uv, sizeof(float2));
						data_it += 2;
					}

					//Generating data buffer
					glGenBuffers(1, (GLuint*) &(newMesh->m_idData));
					glBindBuffer(GL_ARRAY_BUFFER, newMesh->m_idData);
					glBufferData(GL_ARRAY_BUFFER, sizeof(float) * newMesh->m_numVertices * (3 + 3 + 2), data, GL_STATIC_DRAW);
					//endof Generating vertices buffer

					RELEASE_ARRAY(textureCoords);
					RELEASE_ARRAY(data);

#pragma region Loading indices
					//Num indices
					bytes = sizeof(uint);
					memcpy(&newMesh->m_numIndices, It, bytes);
					It += bytes;

					//Actual indices
					newMesh->m_indices = new uint[newMesh->m_numIndices];
					bytes = sizeof(uint) * newMesh->m_numIndices;
					memcpy(newMesh->m_indices, It, bytes);
					It += bytes;

					//Generating indices buffer
					glGenBuffers(1, (GLuint*) &(newMesh->m_idIndices));
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newMesh->m_idIndices);
					glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * newMesh->m_numIndices, newMesh->m_indices, GL_STATIC_DRAW);
					//endOf generating indices buffer
#pragma endregion

					//AABB maxPoint
					bytes = sizeof(float3);
					memcpy(&newMesh->m_aabb.maxPoint, It, bytes);
					It += bytes;

					//AABB minPoint
					memcpy(&newMesh->m_aabb.minPoint, It, bytes);
					It += bytes;
				}
			}
			RELEASE_ARRAY(file);
		}
	}
	return newMesh;
}

R_Material* ModuleImporter::LoadMaterial(const char * resName)
{
	char* file = nullptr;
	
	const MetaInf* inf = App->m_resourceManager->GetMetaData(ComponentType::material, resName);

	R_Material* mat = nullptr;

	if (inf != nullptr)
	{
		char filePath[526];
		sprintf(filePath, "Library/Materials/%llu%s", inf->uid, MATERIAL_FORMAT);

		LOG("Loading material %s", inf->name.data());

		if (App->m_fileSystem->Exists(filePath))
		{
			int size = App->m_fileSystem->Load(filePath, &file);
			if (file != nullptr && size > 0)
			{
				char* It = file;
				mat = new R_Material(inf->uid);

				uint bytes = 0;
				uint nTextures = 0;

				mat->m_name = resName;

				//NumTextures
				uint numTextures = 0;
				bytes = sizeof(uint);
				memcpy(&numTextures, It, bytes);
				It += bytes;

				for (int n = 0; n < numTextures; n++)
				{
					//Texture name Len
					uint textureNameLen = 0;
					bytes = sizeof(uint);
					memcpy(&textureNameLen, It, bytes);
					It += bytes;

					if (textureNameLen > 1)
					{
						//Texture name
						char* textureName = new char[textureNameLen];
						bytes = sizeof(char) * textureNameLen;
						memcpy(textureName, It, bytes);
						It += bytes;
						std::string path(textureName);
						uint64_t toAdd = App->m_resourceManager->LinkResource(path.data(), ComponentType::texture);
						if (toAdd != 0)
						{
							mat->m_textures.push_back(toAdd);
						}

						delete[] textureName;
					}
					else
					{
						It++;
					}
					//Color
					float color[3];
					bytes = sizeof(float) * 3;
					memcpy(color, It, bytes);
					It += bytes;
					mat->m_color[0] = color[0];
					mat->m_color[1] = color[1];
					mat->m_color[2] = color[2];
					mat->m_color[4] = 1.0f;
				}
				RELEASE_ARRAY(file);
			}
		}
	}
	return mat;
}

R_Texture* ModuleImporter::LoadTexture(const char* resName)
{
	R_Texture* ret = nullptr;
	if (*resName == '\0')
	{
		return ret;
	}

	const MetaInf* inf = App->m_resourceManager->GetMetaData(ComponentType::texture, resName);
	if (inf != nullptr)
	{
		glActiveTexture(0);
		char fullPath[526];
		sprintf(fullPath, "%sLibrary/Textures/%llu%s", NormalizePath(App->m_fileSystem->GetWrittingDirectory().data()).data(), inf->uid, TEXTURE_FORMAT);

		LOG("Loading Texture %s", inf->name.data());

		uint ID = ilutGLLoadImage(fullPath);

		if (ID != 0)
		{
			ret = new R_Texture(inf->uid);
			ret->m_name = inf->name;
			ret->m_bufferID = ID;
			return ret;
		}
		else
		{
			LOG("Error loading texture %s", fullPath);
			for (ILenum error = ilGetError(); error != IL_NO_ERROR; error = ilGetError())
			{
				LOG("devIL got error %d", error);
				//For some reason, this will break and cause a crash
				//LOG("%s", iluErrorString(error));
			}
			return ret;
		}
	}
	else
	{
		LOG("Couldn't find in resources texture %s", resName)
	}
}

R_Shader * ModuleImporter::LoadShader(const char * resName)
{
	char* file = nullptr;
	R_Shader* ret = nullptr;

	const MetaInf* inf = App->m_resourceManager->GetMetaData(ComponentType::shader, resName);
	LOG("Loading shader %s", inf->name.data());
	if (inf != nullptr)
	{
		char filePath[526];
		sprintf(filePath, "Library/Shaders/%llu%s", inf->uid, SHADER_PROGRAM_FORMAT);
		if (App->m_fileSystem->Exists(filePath))
		{
			int size = App->m_fileSystem->Load(filePath, &file);
			if (file != nullptr && size > 0)
			{
				ret = new R_Shader(inf->uid);
				ret->m_name = resName;
				
				char* fileIt = file;

				GLenum binaryFormat;
				memcpy(&binaryFormat, fileIt, sizeof(GLenum));
				fileIt += sizeof(GLenum);

				uint length;
				memcpy(&length, fileIt, sizeof(GLint));
				fileIt += sizeof(GLint);

				ret->m_shaderProgram.m_program = glCreateProgram();
				Shader& shader = ret->m_shaderProgram;
				int program = ret->m_shaderProgram.m_program;

				glProgramBinary(ret->m_shaderProgram.m_program, binaryFormat, fileIt, length);
				RELEASE_ARRAY(file);

				shader.m_modelMatrix = glGetUniformLocation(shader.m_program, "model_matrix");
				shader.m_viewMatrix = glGetUniformLocation(shader.m_program, "view_matrix");
				shader.m_projectionMatrix = glGetUniformLocation(shader.m_program, "projection_matrix");

				shader.m_materialColor = glGetUniformLocation(shader.m_program, "material_color");
				shader.m_hasTexture = glGetUniformLocation(shader.m_program, "has_texture");
				shader.m_useLight = glGetUniformLocation(shader.m_program, "use_light");
				shader.m_time = glGetUniformLocation(shader.m_program, "time");
				shader.m_ambientColor = glGetUniformLocation(shader.m_program, "ambient_color");
				shader.m_globalLightDir = glGetUniformLocation(shader.m_program, "global_light_direction");
				shader.m_fogDistance = glGetUniformLocation(shader.m_program, "fog_distance");
				shader.m_fogColor = glGetUniformLocation(shader.m_program, "fog_color");
				shader.m_maxHeight = glGetUniformLocation(shader.m_program, "max_height");
			}
		}
	}
	return ret;
}



// ------------------------------- UTILITY ------------------------------- 

//This function will return only the file format, without the dot.
std::string ModuleImporter::IsolateFileFormat(const char * file)
{
	char name[1024];
	strcpy(name, file);
		char* tmp = name;
		while (*tmp != '\0')
		{
			tmp++;
		}
		while (*tmp != '.')
		{
			tmp--;
			if (tmp == name)
			{
				return std::string("");
			}
		}
		tmp++;
		return std::string(tmp);
}

//This function will remove everything from the path except the file's actual name. No format, no path
std::string ModuleImporter::IsolateFileName(const char * file)
{
	char name[1024];
	strcpy(name, file);
	char* start = name;
	int size = 0;
	while (*start != '\0')
	{
		size++;
		start++;
	}

	char*  end = start;
	while (size >= 0  && *start != '/' && *start != '\\')
	{
		size--;
		start--;
	}
	start++;

	while (*end != '.' && end != start)
	{
		end--;
	}
	if (end != start)
	{
		*end = '\0';
	}
	return std::string(start);
}

//Returns the file name + format
std::string ModuleImporter::IsolateFile(const char * file)
{
	const char* start = file;

	while (*start != '\0')
	{
		start++;
	}

	while (*start != '/' && *start != '\\' && start != file)
	{
		start--;
	}
	if (start != file)
	{
		start++;
	}
	return std::string(start);
}

//Will change any "\\" into a "/" && will set the start of the path at "Assets", if found
std::string ModuleImporter::NormalizePath(const char * path)
{
	char tmp[MAXLEN];
	strcpy(tmp, path);
	char* it = tmp;

	uint len = 0;
	uint n = 0;

	while (*it != '\0')
	{
		if (*it == '\\')
		{
			*it = '/';
		}
		it++;
		len++;
	}

	it = tmp;

	while (n < len - 7)
	{
		if (it[0] == '/' && it[1] == 'A' && it[2] == 's'&& it[3] == 's'&& it[4] == 'e'&& it[5] == 't'&& it[6] == 's'&& it[7] == '/')
		{
			it++;
			return std::string(it);
		}
		it++;
		n++;
	}
	return std::string(tmp);
}

//Remove the file format from any string, if there's one
std::string ModuleImporter::RemoveFormat(const char * file)
{
	char name[1024];
	strcpy(name, file);
	char* end = name;
	while (*end != '\0')
	{
		end++;
	}

	while (*end != '.')
	{
		end--;
		if (end == name) { return std::string(file); }
	}
	*end = '\0';

	return std::string(name);
}

std::string ModuleImporter::CompileShader(const char* vertexBuf, const char* fragmentBuf, uint & shaderProgram)
{
	std::string ret;
	bool error = false;
	shaderProgram = 0;
	GLint success;
	if (fragmentBuf == nullptr && vertexBuf == nullptr)
	{
		ret = "No vertex or fragment were recieved. Not compiling the shader.";
		return ret;
	}
	ret += "\n------ Vertex shader ------\n";
	GLuint vertexShader;
	if (vertexBuf == nullptr)
	{
		ret += "- No vertex shader found. Using default vertex shader.\n";
		vertexBuf = App->m_resourceManager->m_defaultVertexBuf.c_str();
	}

	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexBuf, NULL);
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (success == 0)
	{
		error = true;
		GLchar infoLog[512];
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);		
		ret += infoLog;
		ret += '\n';
	}
	else
	{
		ret += "Compilation succesfull\n";
	}

	ret += "\n------ Fragment shader ------\n";
	GLuint fragmentShader;
	if (fragmentBuf == nullptr)
	{
		ret += "- No fragment shader found. Using default fragment shader.\n";
		fragmentBuf = App->m_resourceManager->m_defaultFragmentBuf.c_str();
	}

	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentBuf, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (success == 0)
	{
		error = true;
		GLchar infoLog[512];
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);		
		ret += infoLog;
		ret += '\n';
	}
	else
	{
		ret += "Compilation succesfull\n";
	}

	GLuint program;
	program = glCreateProgram();

	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);

	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (success == 0)
	{
		error = true;
		GLchar infoLog[512];
		glGetProgramInfoLog(program, 512, NULL, infoLog);
		ret += "\n------ Shader Program ------\n";
		ret += infoLog;
		ret += '\n';
	}

	if (program != 0 && error == false)
	{
		shaderProgram = program;
		LOG("Compiled shader succesfully.")
	}
	else
	{
		LOG("\nError compiling shader:\n\n%s", ret.data());
	}

	glDetachShader(shaderProgram, vertexShader);
	glDetachShader(shaderProgram, fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return ret;
}