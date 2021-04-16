#include "include/Image.h"
#include "include/GrayImage.h"
#include "lodepng.h"

using namespace Steg;

GrayImage::GrayImage(uint32_t width, uint32_t height, uint32_t bitDepth, bool hasAlpha)
        : Image(width, height, Image::GetGrayMode(bitDepth, hasAlpha)), BitDepth(bitDepth), HasAlpha(hasAlpha) {}

GrayImage::GrayImage(const Image& other)
        : Image(other), BitDepth(other.GetBitDepth()), HasAlpha(other.HasAlpha()) {}

GrayColor GrayImage::GetColor(uint32_t x, uint32_t y) {
    uint64_t color = Image::GetColor(x, y);
    GrayColor result(0, 0);
    if (BitDepth == 8) {
        if (HasAlpha) {
            result.Alpha = color & 0xFF;
            color >>= 8;
        }
        result.Value = color & 0xFF;
    } else if (BitDepth == 16) {
        if (HasAlpha) {
            result.Alpha = color & 0xFFFF;
            color >>= 16;
        }
        result.Value = color & 0xFFFF;
    } else {
        throw std::invalid_argument("Invalid Image bit depth: " + BitDepth);
    }
    return result;
}

void GrayImage::SetColor(uint32_t x, uint32_t y, uint16_t value) {
    uint64_t color = value;
    Image::SetColor(x, y, color);
}

void GrayImage::SetColor(uint32_t x, uint32_t y, uint16_t value, uint16_t alpha) {
    uint64_t color = value;
    if (BitDepth == 8) {
        color <<= 8;
    } else if (BitDepth == 16) {
        color <<= 16;
    } else {
        throw std::invalid_argument("Invalid Image bit depth: " + BitDepth);
    }
    color |= alpha;
    Image::SetColor(x, y, color);
}
