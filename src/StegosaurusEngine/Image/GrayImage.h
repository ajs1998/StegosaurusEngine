#pragma once

#include "../Core.h"
#include "Image.h"

namespace Steg {

    struct GrayColor {

        uint16_t Value;
        uint16_t Alpha;

        GrayColor(uint16_t value, uint16_t alpha) : Value(value), Alpha(alpha) {}

        uint64_t ToInt(uint32_t bitDepth, bool hasAlpha) {
            uint64_t color = 0;
            color |= Value;
            if (hasAlpha) {
                color <<= bitDepth;
                color |= Alpha;
            }
            return color;
        }

    };

    class GrayImage : public Image {

    public:

        GrayImage(uint32_t width, uint32_t height, uint32_t bitDepth, bool hasAlpha);

        GrayImage(const Image& other);

        ~GrayImage() = default;

        GrayImage operator=(const GrayImage& other) = delete;

        GrayColor GetColor(uint32_t x, uint32_t y);

        void SetColor(uint32_t x, uint32_t y, uint16_t value);

        void SetColor(uint32_t x, uint32_t y, uint16_t value, uint16_t alpha);

    private:

        uint32_t BitDepth;
        bool HasAlpha;

    };
}
