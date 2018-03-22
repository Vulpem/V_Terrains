//  V Terrains
//  Procedural terrain generation for modern C++
//  Copyright (C) 2018 David Hernàndez Làzaro
//  
//  "V Terrains" is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or any later version.
//  
//  "V Terrains" is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//  
//  For more details, read "COPYING.txt" and "COPYING.LESSER.txt" included in this project.
//  You should have received a copy of the GNU General Public License along with V Terrains.  If not, see <http://www.gnu.org/licenses/>.
#include "GenImage.h"
#include <assert.h>

#include "../ExternalLibs/Glew/include/glew.h"

#include "../ExternalLibs/Devil/include/il.h"
#include "../ExternalLibs/Devil/include/ilu.h"
#include "../ExternalLibs/Devil/include/ilut.h"

#pragma comment(lib, "Devil/libx86/DevIL.lib")
#pragma comment(lib, "Devil/libx86/ILU.lib")
#pragma comment(lib, "Devil/libx86/ILUT.lib")

namespace VTerrain
{
    void GenImage::Init()
    {
        ilInit();

        ILuint devilError = ilGetError();

        if (devilError != IL_NO_ERROR)
        {
            printf("Devil Error (ilInit: %s\n", iluErrorString(devilError));
            assert(false);
        }

        ilutRenderer(ILUT_OPENGL);
    }
    unsigned int GenImage::FromRGB(const std::vector<float>& color, unsigned int w, unsigned int h)
    {
        unsigned int ret = 0;
        glEnable(GL_TEXTURE_2D);
        glActiveTexture(GL_TEXTURE0);
        glGenTextures(1, &ret);

        glBindTexture(GL_TEXTURE_2D, ret);

        SetParams();

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_FLOAT, color.data());

        return ret;
    }

	unsigned int GenImage::FromRGBA(const std::vector<float>& color, unsigned int w, unsigned int h)
    {
        unsigned int ret = 0;
        glEnable(GL_TEXTURE_2D);
        glActiveTexture(GL_TEXTURE0);
        glGenTextures(1, &ret);

        glBindTexture(GL_TEXTURE_2D, ret);

        SetParams();

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_FLOAT, color.data());

        return ret;
    }

    unsigned int GenImage::FromPath(const std::string & path)
    {
        char filePath[1024];
        strcpy_s(filePath, path.size(), path.data());
        unsigned int ID = ilutGLLoadImage(filePath);
        assert(ID != 0);
        return ID;
    }

    void GenImage::FreeImage(unsigned int& buffer)
    {
        glDeleteTextures(1, &buffer);
        buffer = 0;
    }
    void GenImage::SetParams()
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }
}