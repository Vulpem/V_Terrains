#ifndef __ModuleAudio_H__
#define __ModuleAudio_H__

#include "Module.h"
//#include "SDL_mixer\include\SDL_mixer.h"

#define DEFAULT_MUSIC_FADE_TIME 2.0f

class ModuleAudio : public Module
{
public:

	ModuleAudio();
	~ModuleAudio();

	bool Init() override;
	void OnDisable() override;

	bool PlayMusic(const char* path, float fade_time = DEFAULT_MUSIC_FADE_TIME);

	unsigned int LoadFx(const char* path);
	bool PlayFx(unsigned int fx, int repeat = 0);

private:
//	Mix_Music*			m_music;
//	p2List<Mix_Chunk*>	m_fx;
};

#endif // __ModuleAudio_H__