#include "GenImage.h"

#include "../ExternalLibs/Glew/include/glew.h"

namespace VTerrain
{
    uint GenImage::FromRGB(const std::vector<float>& r, const std::vector<float>& g, const std::vector<float>& b, unsigned int w, unsigned int h, bool normalize)
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

        std::vector<float> tmp;
        tmp.resize(w*h * 3, 0.f);

        float maxVal = 0.0f;
        for (uint n = 0; n < w*h; n++)
        {
            tmp[n * 3 + 0] = r[n];
            tmp[n * 3 + 1] = g[n];
            tmp[n * 3 + 2] = b[n];
            if (normalize)
            {
                maxVal = utils::Max(maxVal, tmp[n * 3 + 0]);
                maxVal = utils::Max(maxVal, tmp[n * 3 + 1]);
                maxVal = utils::Max(maxVal, tmp[n * 3 + 2]);
            }
        }

        if (normalize)
        {
            for (uint n = 0; n < w * h * 3; n++)
            {
                tmp[n] = tmp[n] / maxVal;
            }
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_FLOAT, tmp.data());

        return ret;
    }

    uint GenImage::BlackAndWhite(const std::vector<float>& color, unsigned int w, unsigned int h, bool normalize)
    {
        return FromRGB(color, color, color, w, h, normalize);
    }

    bool GenImage::FreeImage(uint buffer)
    {
        glDeleteBuffers(1, &buffer);
        return false;
    }
}