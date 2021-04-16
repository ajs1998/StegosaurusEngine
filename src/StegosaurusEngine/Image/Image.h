#pragma once

#include "../Core.h"

namespace Steg {

    enum class PixelMode {
        GRAY_8, GRAY_16,
        GRAYA_8, GRAYA_16,
        RGB_8, RGB_16,
        RGBA_8, RGBA_16,
        INVALID
    };

    class Image {

    public:

        Image(const std::string& imagePath);

        ~Image() = default;

        void SaveImage(const std::string& imagePath) const;

        uint64_t GetColor(uint32_t x, uint32_t y) const;

        byte GetByte(uint32_t index) const;

        bool IsAlphaIndex(uint32_t index) const;

        void SetColor(uint32_t x, uint32_t y, uint64_t color);

        void SetByte(uint32_t index, byte value);

        uint32_t GetWidth() const;

        uint32_t GetHeight() const;

        PixelMode GetPixelMode() const;

        uint32_t GetBitDepth() const;

        uint32_t GetChannelCount() const;

        uint32_t GetPixelWidth() const;

        bool HasAlpha() const;

    protected:

        Image(uint32_t width, uint32_t height, const PixelMode& mode);

        Image operator=(const Image & other) = delete;

        static PixelMode GetGrayMode(uint32_t bitDepth, bool hasAlpha);

        static PixelMode GetRGBMode(uint32_t bitDepth, bool hasAlpha);

        static uint32_t GetBitDepth(const PixelMode& mode);

        static uint32_t GetChannelCount(const PixelMode& mode);

        static uint32_t GetPixelWidth(const PixelMode& mode);

        static bool HasAlpha(const PixelMode& mode);

    private:

        uint32_t Width;
        uint32_t Height;
        uint32_t PixelCount;
        PixelMode Mode;
        std::vector<byte> Data;

    };
}
