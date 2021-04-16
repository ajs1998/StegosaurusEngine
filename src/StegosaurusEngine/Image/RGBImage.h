#pragma once

#include "../Core.h"

#include "Image.h"

namespace Steg {

    struct RGBColor {

        uint16_t Red;
        uint16_t Green;
        uint16_t Blue;
        uint16_t Alpha;

        RGBColor(uint16_t red, uint16_t green, uint16_t blue, uint16_t alpha)
                : Red(red), Green(green), Blue(blue), Alpha(alpha) {}

        uint64_t ToInt(uint32_t bitDepth, bool hasAlpha) {
            uint64_t color = 0;
            color |= Red;
            color <<= bitDepth;
            color |= Green;
            color <<= bitDepth;
            color |= Blue;
            if (hasAlpha) {
                color <<= bitDepth;
                color |= Alpha;
            }
            return color;
        }

    };

    class RGBImage : public Image {

    public:

        RGBImage(uint32_t width, uint32_t height, uint32_t bitDepth, bool hasAlpha);

        RGBImage(const Image& other);

        ~RGBImage() = default;

        RGBImage operator=(const RGBImage& other) = delete;

        RGBColor GetColor(uint32_t x, uint32_t y);

        void SetColor(uint32_t x, uint32_t y, uint16_t red, uint16_t green, uint16_t blue);

        void SetColor(uint32_t x, uint32_t y, uint16_t red, uint16_t green, uint16_t blue, uint16_t alpha);

    private:

        uint32_t BitDepth;
        bool HasAlpha;

    };
}
