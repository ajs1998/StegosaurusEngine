#pragma once

#include "Core.h"

#include "Crypt/StegCrypt.h"
#include "RNG.h"
#include "Image/Image.h"

namespace Steg {

    struct EncryptionSettings {

        // TRUE: Encrypt data before encoding
        // FALSE: Leave payload alone
        // Note: Encrypting may increase the payload size slightly
        bool EncryptPayload = false;

        // The password for encryption (this will later derive a key)
        std::vector<byte> EncryptionPassword;

        // Larger block sizes means encryption is more secure, but will occupy more space
        StegCrypt::Algorithm Algo = StegCrypt::Algorithm::ALGO_AES128;

    };

    struct EncoderSettings {

        // Split each data byte into DataDepth bits
        // 1, 2, 4, 8 are valid values. 16 is not valid yet
        byte DataDepth = 2;

        // TRUE: Hide data in alpha channel if available
        // FALSE: Leave alpha channel alone (preferred)
        // Note: Only applies to RGBA_X or GRAYA_X PixelModes
        bool EncodeInAlpha = false;

        // TODO Normalize image option

        EncryptionSettings Encryption;

        byte ToByte() const {

            // DataDepth has 4 possible values so it will occupy 2 bits
            byte result = 0;
            if (DataDepth == 1) {
                result |= 0b00'000000;
            } else if (DataDepth == 2) {
                result |= 0b01'000000;
            } else if (DataDepth == 4) {
                result |= 0b10'000000;
            } else if (DataDepth == 8) {
                result |= 0b11'000000;
            } else {
                throw std::invalid_argument("Invalid data depth: " + DataDepth);
                return 0;
            }

            // Each bool has 2 possible values so they will occupy 1 bit each
            if (EncodeInAlpha) {
                result |= 0b00'1'00000;
            }

            if (Encryption.EncryptPayload) {
                result |= 0b000'1'0000;
                switch (Encryption.Algo) {
                    case StegCrypt::Algorithm::ALGO_AES128:
                        result |= 0b0000'00'00;
                        break;
                    case StegCrypt::Algorithm::ALGO_AES192:
                        result |= 0b0000'01'00;
                        break;
                    case StegCrypt::Algorithm::ALGO_AES256:
                        result |= 0b0000'10'00;
                        break;
                }
            }

            // TODO Add more bool flags here as needed (2 bits left)

            return result;

        }

        static EncoderSettings FromByte(byte settingsByte) {

            EncoderSettings settings;

            byte depth = (settingsByte & 0b11'000000) >> 6;
            if (depth == 0b00) {
                settings.DataDepth = 1;
            } else if (depth == 0b01) {
                settings.DataDepth = 2;
            } else if (depth == 0b10) {
                settings.DataDepth = 4;
            } else if (depth == 0b11) {
                settings.DataDepth = 8;
            }

            settings.EncodeInAlpha = settingsByte & 0b00'1'00000;

            settings.Encryption.EncryptPayload = settingsByte & 0b000'1'0000;

            if (settings.Encryption.EncryptPayload) {
                switch ((settingsByte & 0b0000'11'00) >> 2) {
                    case 0b00:
                        settings.Encryption.Algo = StegCrypt::Algorithm::ALGO_AES128;
                        break;
                    case 0b01:
                        settings.Encryption.Algo = StegCrypt::Algorithm::ALGO_AES192;
                        break;
                    case 0b10:
                        settings.Encryption.Algo = StegCrypt::Algorithm::ALGO_AES256;
                        break;
                }
            }

            // TODO Add more bool flags here as needed (2 bits left)

            return settings;

        }

    };

    class StegEngine {

    public:

        static void Encode(Image& image, const std::vector<byte>& data, const EncoderSettings& settings);

        static std::vector<byte> Decode(const Image& image, const std::vector<byte> key);

        static uint32_t CalculateAvailableBytes(const Image& image, const EncoderSettings& settings);

    private:

        static constexpr uint32_t HeaderSize = 6;

        static uint16_t GetPixelMask(uint32_t imageBitDepth, uint32_t dataBitDepth);

        static byte GetPartMask(uint32_t imageBitDepth, uint32_t dataBitDepth);

        static std::vector<uint32_t> GenerateIndices(uint32_t seed, RNG& rng);

        static bool CanEncode(const Image& image, uint32_t payloadSize, const EncoderSettings& settings);

    };

}
