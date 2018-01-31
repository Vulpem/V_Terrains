#include "GenImage.h"

#include "../ExternalLibs/Glew/include/glew.h"

namespace VTerrain
{
    uint GenImage::FromRGB(std::vector<float> color, unsigned int w, unsigned int h, bool normalize)
    {
        unsigned int ret = 0;
        glEnable(GL_TEXTURE_2D);
        glActiveTexture(GL_TEXTURE0);
        glGenTextures(1, &ret);

        glBindTexture(GL_TEXTURE_2D, ret);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

        float maxVal = 0.0f;
        if (normalize)
        {
            for (uint n = 0; n < w*h * 3; n++)
            {
                maxVal = utils::Max(maxVal, color[n]);
            }
            for (uint n = 0; n < w * h * 3; n++)
            {
                color[n] = color[n] / maxVal;
            }
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_FLOAT, color.data());

        return ret;
    }

    bool GenImage::FreeImage(uint buffer)
    {
        glDeleteBuffers(1, &buffer);
        return false;
    }
}