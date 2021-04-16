#include "include/Image.h"
#include "include/RGBImage.h"
#include "lodepng.h"

using namespace Steg;

RGBImage::RGBImage(uint32_t width, uint32_t height, uint32_t bitDepth, bool hasAlpha)
        : Image(width, height, Image::GetRGBMode(bitDepth, hasAlpha)), BitDepth(bitDepth), HasAlpha(hasAlpha) {}

RGBImage::RGBImage(const Image& other)
        : Image(other), BitDepth(other.GetBitDepth()), HasAlpha(other.HasAlpha()) {}

RGBColor RGBImage::GetColor(uint32_t x, uint32_t y) {
    uint64_t color = Image::GetColor(x, y);
    RGBColor result(0, 0, 0, 0);
    if (BitDepth == 8) {
        if (HasAlpha) {
            result.Alpha = color & 0xFF;
            color >>= 8;
        }
        result.Blue = color & 0xFF;
        color >>= 8;
        result.Green = color & 0xFF;
        color >>= 8;
        result.Red = color & 0xFF;
    } else if (BitDepth == 16) {
        if (HasAlpha) {
            result.Alpha = color & 0xFFFF;
            color >>= 16;
        }
        result.Blue = color & 0xFFFF;
        color >>= 16;
        result.Green = color & 0xFFFF;
        color >>= 16;
        result.Red = color & 0xFFFF;
    } else {
        throw std::invalid_argument("Invalid Image bit depth: " + BitDepth);
    }
    return result;
}

void RGBImage::SetColor(uint32_t x, uint32_t y, uint16_t red, uint16_t green, uint16_t blue) {
    if (HasAlpha) {
        throw std::invalid_argument("Must provide an alpha value for RGBA images");
    }
    uint64_t color = red;
    color <<= BitDepth;
    color |= green;
    color <<= BitDepth;
    color |= blue;
    Image::SetColor(x, y, color);
}

void RGBImage::SetColor(uint32_t x, uint32_t y, uint16_t red, uint16_t green, uint16_t blue, uint16_t alpha) {
    if (!HasAlpha) {
        throw std::invalid_argument("Must not provide an alpha value for RGB images");
    }
    uint64_t color = red;
    color <<= BitDepth;
    color |= green;
    color <<= BitDepth;
    color |= blue;
    color <<= BitDepth;
    color |= alpha;
    Image::SetColor(x, y, color);
}
