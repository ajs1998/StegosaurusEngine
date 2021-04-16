#include "include/Image.h"
#include "lodepng.h"

using namespace Steg;

Image::Image(uint32_t width, uint32_t height, const PixelMode& mode)
        : Width(width), Height(height), PixelCount(width * height), Mode(mode) {
    Data.resize(PixelCount * GetPixelWidth(mode), 0);
}

Image::Image(const std::string& imagePath) {
    std::vector<byte> file;
    lodepng::State state;

    uint32_t error = lodepng::load_file(file, imagePath);
    if (error) {
        throw std::runtime_error("Could not load image file: " + imagePath);
    }

    error = lodepng::decode(Data, Width, Height, state, file);
    if (error) {
        throw std::runtime_error("Could not load image file: " + imagePath);
    }

    PixelCount = Width * Height;

    auto colorType = state.info_raw.colortype;
    auto bitDepth = state.info_raw.bitdepth;
    switch (colorType) {
        case LCT_GREY:
            if (bitDepth == 8) {
                Mode = PixelMode::GRAY_8;
            } else if (bitDepth == 16) {
                Mode = PixelMode::GRAY_16;
            } else {
                Mode = PixelMode::INVALID;
            }
            break;
        case LCT_GREY_ALPHA:
            if (bitDepth == 8) {
                Mode = PixelMode::GRAYA_8;
            } else if (bitDepth == 16) {
                Mode = PixelMode::GRAYA_16;
            } else {
                Mode = PixelMode::INVALID;
            }
            break;
        case LCT_RGB:
            if (bitDepth == 8) {
                Mode = PixelMode::RGB_8;
            } else if (bitDepth == 16) {
                Mode = PixelMode::RGB_16;
            } else {
                Mode = PixelMode::INVALID;
            }
            break;
        case LCT_RGBA:
            if (bitDepth == 8) {
                Mode = PixelMode::RGBA_8;
            } else if (bitDepth == 16) {
                Mode = PixelMode::RGBA_16;
            } else {
                Mode = PixelMode::INVALID;
            }
            break;
        default:
            Mode = PixelMode::INVALID;
            throw std::invalid_argument("Invalid LodePNG Color Type: " + colorType);
    }

}

void Image::SaveImage(const std::string& imagePath) const {
    LodePNGColorType type;
    uint32_t depth;
    switch (Mode) {
        default:
        case PixelMode::GRAY_8:
            type = LodePNGColorType::LCT_GREY;
            depth = 8;
            break;
        case PixelMode::GRAY_16:
            type = LodePNGColorType::LCT_GREY;
            depth = 16;
            break;
        case PixelMode::GRAYA_8:
            type = LodePNGColorType::LCT_GREY_ALPHA;
            depth = 8;
            break;
        case PixelMode::GRAYA_16:
            type = LodePNGColorType::LCT_GREY_ALPHA;
            depth = 16;
            break;
        case PixelMode::RGB_8:
            type = LodePNGColorType::LCT_RGB;
            depth = 8;
            break;
        case PixelMode::RGB_16:
            type = LodePNGColorType::LCT_RGB;
            depth = 16;
            break;
        case PixelMode::RGBA_8:
            type = LodePNGColorType::LCT_RGBA;
            depth = 8;
            break;
        case PixelMode::RGBA_16:
            type = LodePNGColorType::LCT_RGBA;
            depth = 16;
            break;
    }

    if (depth == 16) {
        throw std::invalid_argument("16-bit image saving is not available yet");
    }

    unsigned error = lodepng::encode(imagePath, Data, Width, Height, type, depth);
    if (error) {
        throw std::runtime_error("Could not encode and save file to " + imagePath);
    }
}

// TODO This type is insufficient for 16 bit modes
uint64_t Image::GetColor(uint32_t x, uint32_t y) const {
    uint32_t bitDepth = GetBitDepth(Mode);
    uint32_t channels = GetChannelCount(Mode);

    uint32_t pixelIndex = (y * Width + x) * GetPixelWidth(Mode);

    uint64_t color = 0;
    if (bitDepth == 8) {
        for (uint32_t i = 0; i < channels; i++) {
            color <<= 8;
            byte value = Data[pixelIndex + i];
            color |= value;
        }
    } else if (bitDepth == 16) {
        for (uint32_t i = 0; i < channels; i++) {
            color <<= 16;
            byte value0 = Data[pixelIndex + (2 * i)];
            byte value1 = Data[pixelIndex + (2 * i) + 1];
            color |= value0;
            color <<= 8;
            color |= value1;
        }
    }
    return color;

}

byte Image::GetByte(uint32_t index) const {
    return Data[index];
}

bool Image::IsAlphaIndex(uint32_t index) const {
    switch (Mode) {
        case PixelMode::RGBA_8:
            if (index % 4 == 3) {
                return true;
            }
            return false;
        case PixelMode::RGBA_16:
            if ((index / 2) % 4 == 3) {
                return true;
            }
            return false;
        case PixelMode::GRAYA_8:
            if (index % 2 == 1) {
                return true;
            }
            return false;
        case PixelMode::GRAYA_16:
            if ((index / 2) % 2 == 1) {
                return true;
            }
            return false;
        default:
            return false;
    }
}

// TODO This type is insufficient for 16 bit modes
void Image::SetColor(uint32_t x, uint32_t y, uint64_t color) {
    uint32_t bitDepth = GetBitDepth(Mode);
    uint32_t channels = GetChannelCount(Mode);

    uint32_t pixelIndex = (y * Width + x) * GetPixelWidth(Mode);

    if (bitDepth == 8) {
        for (uint32_t i = 0; i < channels; i++) {
            byte value = (color >> (8 * (channels - 1 - i))) & 0xFF;
            Data[pixelIndex + i] = value;
        }
    } else if (bitDepth == 16) {
        for (uint32_t i = 0; i < channels; i++) {
            uint16_t value = (color >> (16 * (channels - 1 - i))) & 0xFFFF;
            Data[pixelIndex + (2 * i)] = value & 0xFF00;
            Data[pixelIndex + (2 * i) + 1] = value & 0x00FF;
        }
    }
}

void Image::SetByte(uint32_t index, byte value) {
    Data[index] = value;
}

/* Public Getter Methods */

uint32_t Image::GetWidth() const {
    return Width;
}

uint32_t Image::GetHeight() const {
    return Height;
}

PixelMode Image::GetPixelMode() const {
    return Mode;
}

uint32_t Image::GetBitDepth() const {
    return GetBitDepth(Mode);
}

uint32_t Image::GetChannelCount() const {
    return GetChannelCount(Mode);
}

uint32_t Image::GetPixelWidth() const {
    return GetPixelWidth(Mode);
}

bool Image::HasAlpha() const {
    return HasAlpha(Mode);
}

/* Protected Methods */

PixelMode Image::GetGrayMode(uint32_t bitDepth, bool hasAlpha) {
    if (bitDepth == 8) {
        if (hasAlpha) {
            return PixelMode::GRAYA_8;
        } else {
            return PixelMode::GRAY_8;
        }
    } else if (bitDepth == 16) {
        if (hasAlpha) {
            return PixelMode::GRAYA_16;
        } else {
            return PixelMode::GRAY_16;
        }
    } else {
        throw std::invalid_argument("Invalid Image bit depth: " + bitDepth);
        return PixelMode::INVALID;
    }
}

PixelMode Image::GetRGBMode(uint32_t bitDepth, bool hasAlpha) {
    if (bitDepth == 8) {
        if (hasAlpha) {
            return PixelMode::RGBA_8;
        } else {
            return PixelMode::RGB_8;
        }
    } else if (bitDepth == 16) {
        if (hasAlpha) {
            return PixelMode::RGBA_16;
        } else {
            return PixelMode::RGB_16;
        }
    } else {
        throw std::invalid_argument("Invalid Image bit depth: " + bitDepth);
        return PixelMode::INVALID;
    }
}

uint32_t Image::GetBitDepth(const PixelMode& mode) {
    switch (mode) {
        case PixelMode::GRAY_8:     // ------VV
        case PixelMode::GRAYA_8:    // ----VVAA
        case PixelMode::RGB_8:      // --RRGGBB
        case PixelMode::RGBA_8:     // RRGGBBAA
            return 8;
        case PixelMode::GRAY_16:
        case PixelMode::GRAYA_16:
        case PixelMode::RGB_16:
        case PixelMode::RGBA_16:
            return 16;
        default:
            throw std::invalid_argument("Unsupported Pixel Mode");
    }
}

uint32_t Image::GetChannelCount(const PixelMode& mode) {
    switch (mode) {
        case PixelMode::GRAY_8:
        case PixelMode::GRAY_16:
            return 1;
        case PixelMode::GRAYA_8:
        case PixelMode::GRAYA_16:
            return 2;
        case PixelMode::RGB_8:
        case PixelMode::RGB_16:
            return 3;
        case PixelMode::RGBA_8:
        case PixelMode::RGBA_16:
            return 4;
        default:
            throw std::invalid_argument("Unsupported Pixel Mode");
    }
}

uint32_t Image::GetPixelWidth(const PixelMode& mode) {
    return GetBitDepth(mode) * GetChannelCount(mode) / 8;
}

bool Image::HasAlpha(const PixelMode& mode) {
    switch (mode) {
        case PixelMode::GRAY_8:
        case PixelMode::GRAY_16:
        case PixelMode::RGB_8:
        case PixelMode::RGB_16:
            return false;
        case PixelMode::GRAYA_8:
        case PixelMode::GRAYA_16:
        case PixelMode::RGBA_8:
        case PixelMode::RGBA_16:
            return true;
        default:
            throw std::invalid_argument("Unsupported Pixel Mode");
    }
}
