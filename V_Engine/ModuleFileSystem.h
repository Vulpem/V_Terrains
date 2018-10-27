#ifndef __j1FILESYSTEM_H__
#define __j1FILESYSTEM_H__

#include "Module.h"
#include <vector>

struct SDL_RWops;
int close_sdl_rwops(SDL_RWops *rw);

struct aiFileIO;

struct Date
{
	uint year = 0;
	uint month = 0;
	uint day = 0;
	uint hour = 0;
	uint min = 0;
	uint sec = 0;

	bool operator ==(const Date b)
	{ return (year == b.year && month == b.month && day == b.day && hour == b.hour && min == b.min && sec == b.sec); }

	bool operator !=(const Date b)
	{ return !(*this == b); }
};

class ModuleFileSystem : public Module
{
public:

	ModuleFileSystem(Application* app, bool start_enabled = true);

	virtual ~ModuleFileSystem();

	bool Init() override;
	void CleanUp() override;

	bool AddPath(const char* path_or_zip, const char* mount_point = nullptr);
	bool Exists(const char* file) const;
	bool CreateDir(const char* dir);
	bool DeleteDir(const char* dir);
	bool EraseFile(const char* file);
	bool IsDirectory(const char* file) const;
	std::string GetWrittingDirectory();
	void GetFilesIn(const char * directory, std::vector<std::string>& folders, std::vector<std::string>& files);

	Date ReadFileDate(const char* path);

	unsigned int Load(const char* path, const char* file, char** buffer) const;
	unsigned int Load(const char* file, char** buffer) const;
	SDL_RWops* Load(const char* file) const;

	// IO interfaces for other libs to handle files via PHYSfs
	aiFileIO* GetAssimpIO();

	unsigned int Save(const char* file, const char* buffer, unsigned int size) const;

	std::string GetFileFormat(char* fullPath);
	std::string RemoveFilePath(char* fileWithPath);

private:
	void CreateAssimpIO();

private:
	aiFileIO* AssimpIO = nullptr;
};

#endif // __j1FILESYSTEM_H__