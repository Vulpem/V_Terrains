#ifndef __GLOBALS__
#define __GLOBALS__ 

#include "GlobalFunctions.h"
#include "SDL/include/SDL.h"

#include <map>
#include <vector>
#include <string>
#include <list>

#define LOG(format, ...) log(__FILE__, __LINE__, format, __VA_ARGS__);
void log(const char file[], int line, const char* format, ...);

#define CAP(n) ((n <= 0.0f) ? n=0.0f : (n >= 1.0f) ? n=1.0f : n=n)

#define DEGTORAD 0.0174532925199432957f
#define RADTODEG 57.295779513082320876f
#define HAVE_M_PI

#define EDITOR_FRAME_SAMPLES 50

#define SCENE_FORMAT ".vscene"
#define TEXTURE_FORMAT ".vtexture"
#define GO_FORMAT ".vgo"
#define MESH_FORMAT ".vmesh"
#define MATERIAL_FORMAT ".vmat"
#define SHADER_VERTEX_FORMAT ".vertex"
#define SHADER_FRAGMENT_FORMAT ".fragment"
#define SHADER_PROGRAM_FORMAT ".vsprogram"
#define META_FORMAT ".vmeta"

typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;
typedef unsigned int uint;

enum class UpdateStatus
{
	Continue = 1,
	Stop,
	Error
};

extern GlobalTime Time;

#define USE_EDITOR true

// Configuration -----------
#define SCREEN_WIDTH 1456
#define SCREEN_HEIGHT 1024
#define WIN_MAXIMIZED true
#define WIN_FULLSCREEN false
#define WIN_RESIZABLE true
#define WIN_BORDERLESS false
#define WIN_FULLSCREEN_DESKTOP false
#define VSYNC false
#define DEBUG_KEYS true

#define TITLE "V_Engine"

//USED WHEN GENERATING THE QUAD TREE SIZE
#define WORLD_WIDTH 1400
#define WORLD_DEPTH 1400
#define WORLD_HEIGHT 600

#endif