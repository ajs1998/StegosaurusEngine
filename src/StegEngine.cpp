#include "StegEngine.h"

#include "StegCrypt.h"
#include "RGBImage.h"
#include "StegTimer.h"

using namespace Steg;

void StegEngine::Encode(Image& image, const std::vector<byte>& data, const EncoderSettings& settings) {

    // Start the Encode Timer
    StegTimer::StartTimer(StegTimer::TimerLabel::ENCODE);

    // Encrypt the payload if necessary
    std::vector<byte> payload;
    if (settings.Encryption.EncryptPayload) {
        payload = StegCrypt::Encrypt(settings.Encryption.EncryptionPassword, data, settings.Encryption.Algo);
    } else {
        payload = data;
    }

    // Number of bytes in the data payload
    uint32_t payloadByteCount = payload.size();

    // Check size constraints in a separate method
    if (!CanEncode(image, payloadByteCount, settings)) {
        throw std::runtime_error("Not enough space in image to encode data");
    }

    // Width of the image
    uint32_t width = image.GetWidth();

    // Height of the image
    uint32_t height = image.GetHeight();

    // Pixel width of the image
    uint32_t bytesPerPixel = image.GetPixelWidth();

    // Number of pixels in the image
    uint32_t pixelCount = width * height;

    // Skip over the alpha channel while encoding
    bool skipAlpha = !(image.HasAlpha() && settings.EncodeInAlpha);

    /* Prepend data vector with header information */

    std::vector<byte> header;

    // Add headerByteCount to header
    // This value is 6 right now, but this will be different for variable header sizes
    // Note: This number includes this byte
    header.push_back(byte(HeaderSize));

    // Add dataByteCount to header
    header.push_back((byte) (payloadByteCount >> 24 & 0xFF));
    header.push_back((byte) (payloadByteCount >> 16 & 0xFF));
    header.push_back((byte) (payloadByteCount >> 8 & 0xFF));
    header.push_back((byte) (payloadByteCount & 0xFF));

    // Add encoder settings to header
    byte settingsByte = settings.ToByte();
    header.push_back(settingsByte);

    // Create an index vector that holds all possible indices for data to be hidden in
    // An index corresponds to a byte within a pixel and the seed is the first byte of the first channel of the first pixel
    // Index 0 is invalid because the seed for the RNG is stored there
    uint32_t indexCount = pixelCount * bytesPerPixel;

    // Get the seed for the RNG
    // It will always be the first byte of the image.
    uint32_t seed = image.GetByte(0);

    // Create the RNG
    // Random Engine generates integers on [0, indexCount - 2]
    RNG rng(seed, indexCount - 2);

    // Fill the index vector
    std::vector<uint32_t> indices = GenerateIndices(indexCount, rng);

    /* Hide information in the image */

    // Write header information first
    // Since encoding information will be unavailable when decoding, default to the most conservative settings
    // DataDepth for the header is effectively 1 bit
    // skipAlpha is effectively true
    uint32_t byteIndex, k = 0;
    for (uint32_t i = 0; i < header.size(); i++) {
        byte datum = header[i];

        // Get each part and insert it into the image
        for (uint32_t partIndex = 0; partIndex < 8; partIndex++) {
            // Skip bytes until byteIndex is a color channel
            do {
                byteIndex = indices[k++];
            } while (image.IsAlphaIndex(byteIndex));

            byte shiftAmount = 7 - partIndex;
            byte part = (datum >> shiftAmount) & 0x1;

            // Combine the data with the image
            part |= image.GetByte(byteIndex) & (0xFF << 1);
            image.SetByte(byteIndex, part);
        }
    }

    const uint16_t pixelMask = GetPixelMask(image.GetBitDepth(), settings.DataDepth);
    const byte partMask = GetPartMask(image.GetBitDepth(), settings.DataDepth);

    // Write data payload next
    // Get a byte of data and insert it into the image
    for (uint32_t i = 0; i < payloadByteCount; i++) {
        byte datum = payload[i];

        // Split the byte into parts
        uint32_t partCount = 8 / settings.DataDepth;

        // Get each part and insert it into the image
        for (uint32_t partIndex = 0; partIndex < partCount; partIndex++) {
            byteIndex = indices[k++];

            if (skipAlpha) {
                // Skip bytes until byteIndex is a color channel
                while (image.IsAlphaIndex(byteIndex)) {
                    byteIndex = indices[k++];
                }
            }

            byte shiftAmount = (8 - settings.DataDepth) - (partIndex * settings.DataDepth);
            byte part = (datum >> shiftAmount) & partMask;

            // Combine the data with the image
            part |= image.GetByte(byteIndex) & pixelMask;
            image.SetByte(byteIndex, part);
        }
    }

    // End the Encode Timer
    StegTimer::EndTimer(StegTimer::TimerLabel::ENCODE);

}

std::vector<byte> StegEngine::Decode(const Image& image, const std::vector<byte> key) {

    // Start the Decode Timer
    StegTimer::StartTimer(StegTimer::TimerLabel::DECODE);

    // Width of the image
    uint32_t width = image.GetWidth();

    // Height of the image
    uint32_t height = image.GetHeight();

    // Pixel width of the image
    uint32_t bytesPerPixel = image.GetPixelWidth();

    // Number of pixels in the image
    uint32_t pixelCount = width * height;

    // Create an index vector that holds all possible indices for data to be hidden in
    // An index corresponds to a byte within a pixel and the seed is the first byte of the first channel of the first pixel
    // Index 0 is invalid because the seed for the RNG is stored there
    uint32_t indexCount = pixelCount * bytesPerPixel;

    // Get the seed for the RNG
    // It will always be the first byte of the image.
    uint32_t seed = image.GetByte(0);

    // Create the RNG
    // Random Engine generates integers on [0, indexCount - 2]
    RNG rng(seed, indexCount - 2);

    // Fill the index vector
    std::vector<uint32_t> indices = GenerateIndices(indexCount, rng);

    /* Find information in the image */

    // Get the first byte of the header (header size)
    uint32_t byteIndex, k = 0;
    uint32_t headerSize = 0;
    uint32_t partCount = 8;
    for (uint32_t partIndex = 0; partIndex < partCount; partIndex++) {
        // Skip bytes until byteIndex is a color channel
        do {
            byteIndex = indices[k++];
        } while (image.IsAlphaIndex(byteIndex));

        // Extract the data from the image
        headerSize <<= 1;
        headerSize |= image.GetByte(byteIndex) & 0x1;
    }

    if (headerSize == 0) {
        throw "Could not decode image!";
    }

    // Read the rest of the header information
    // Since encoding information is unavailable here, default to the most conservative settings
    // DataDepth for the header is effectively 1 bit
    // skipAlpha is effectively true
    std::vector<byte> header;
    for (uint32_t i = 0; i < headerSize - uint32_t(1); i++) {

        // Get each part and insert it into the image
        byte datum = 0;
        for (uint32_t partIndex = 0; partIndex < partCount; partIndex++) {
            // Skip bytes until byteIndex is a color channel
            do {
                byteIndex = indices[k++];
            } while (image.IsAlphaIndex(byteIndex));

            // Extract the data from the image
            datum <<= 1;
            datum |= image.GetByte(byteIndex) & 0x1;
        }

        header.push_back(datum);
    }

    // Compute the size of the payload
    uint32_t payloadByteCount = header[0];
    payloadByteCount <<= 8;
    payloadByteCount |= header[1];
    payloadByteCount <<= 8;
    payloadByteCount |= header[2];
    payloadByteCount <<= 8;
    payloadByteCount |= header[3];

    // Reconstruct the EncoderSettings
    byte settingsByte = header[4];
    EncoderSettings settings = EncoderSettings::FromByte(settingsByte);
    settings.Encryption.EncryptionPassword = key;

    // Skip over the alpha channel while encoding
    bool skipAlpha = !(image.HasAlpha() && settings.EncodeInAlpha);

    const uint16_t pixelMask = GetPixelMask(image.GetBitDepth(), settings.DataDepth);
    const byte partMask = GetPartMask(image.GetBitDepth(), settings.DataDepth);

    // Read data payload next
    // Get a byte of data and insert it into the image
    std::vector<byte> payload;
    for (uint32_t i = 0; i < payloadByteCount; i++) {
        // Split the byte into parts
        uint32_t partCount = 8 / settings.DataDepth;

        // Get each part and insert it into the image
        byte datum = 0;
        for (uint32_t partIndex = 0; partIndex < partCount; partIndex++) {
            byteIndex = indices[k++];

            if (skipAlpha) {
                // Skip bytes until byteIndex is a color channel
                while (image.IsAlphaIndex(byteIndex)) {
                    byteIndex = indices[k++];
                }
            }

            // Extract the data from the image
            byte shiftAmount = (8 - settings.DataDepth) - (partIndex * settings.DataDepth);
            datum |= (image.GetByte(byteIndex) & partMask) << shiftAmount;
        }

        payload.push_back(datum);
    }

    // Decrypt the payload if necessary
    std::vector<byte> data;
    if (settings.Encryption.EncryptPayload) {
        data = StegCrypt::Decrypt(settings.Encryption.EncryptionPassword, payload, settings.Encryption.Algo);
    } else {
        data = payload;
    }

    // End the Decode Timer
    StegTimer::EndTimer(StegTimer::TimerLabel::DECODE);

    return data;

}

uint32_t StegEngine::CalculateAvailableBytes(const Image& image, const EncoderSettings& settings) {

    // Calculate the total available parts
    uint32_t availableParts;
    if (settings.EncodeInAlpha || !image.HasAlpha()) {
        availableParts = image.GetPixelWidth() * image.GetWidth() * image.GetHeight();
    } else {
        uint32_t bytesPerChannel = image.GetBitDepth() / 8;
        availableParts = (image.GetPixelWidth() - bytesPerChannel) * image.GetWidth() * image.GetHeight();
    }

    // Subtract one byte from the available size for the seed (first byte of the image is unavailable)
    availableParts--;

    byte dataDepth = settings.DataDepth;
    uint32_t partsPerByte = 8 / dataDepth;

    if (settings.Encryption.EncryptPayload) {

        uint32_t blockSize = StegCrypt::GetBlockLength(settings.Encryption.Algo);

        // Integer division floors the result (this is good)
        uint32_t maxBytes = availableParts / partsPerByte;

        // Integer division floors the result (this is good)
        uint32_t maxBlocks = maxBytes / blockSize;

        // Subtract 1 for the IV
        uint32_t availableBlocks = maxBlocks - 1;

        // Subtract 1 byte to account for padding
        // 15n bytes of data will get 1 byte of padding
        // 16n bytes of data will get 16 bytes of padding even though size % 16 == 0
        uint32_t availableBytes = availableBlocks * blockSize - 1;

        return availableBytes - HeaderSize;

    } else {

        // Integer division floors the result (this is good)
        return (availableParts / partsPerByte) - HeaderSize;

    }

}

// Ex: bitDepth = 8, dataDepth = 2 => 1111'1111'1111'1100
// Ex: bitDepth = 8, dataDepth = 4 => 1111'1111'1111'0000
uint16_t StegEngine::GetPixelMask(uint32_t imageBitDepth, uint32_t dataBitDepth) {
    uint16_t mask = 0xFFFF;
    mask <<= dataBitDepth;
    if (imageBitDepth == 16) {
        return mask;
    } else if (imageBitDepth == 8) {
        return mask & 0xFF;
    } else {
        throw std::invalid_argument("Invalid Image bit depth: " + imageBitDepth);
    }
}

byte StegEngine::GetPartMask(uint32_t imageBitDepth, uint32_t dataBitDepth) {
    byte mask = 0xFF;
    if (imageBitDepth == 16) {
        return mask >> (8 - dataBitDepth);
    } else if (imageBitDepth == 8) {
        return mask >> (8 - dataBitDepth);
    } else {
        throw std::invalid_argument("Invalid Image bit depth: " + imageBitDepth);
        return 0;
    }
}

std::vector<uint32_t> StegEngine::GenerateIndices(uint32_t indexCount, RNG& rng) {
    std::vector<uint32_t> indices(indexCount - 1);
    for (uint32_t i = 0; i < indices.size(); i++) {
        indices[i] = i + 1;
    }

    // Indices are bytes and they are ordered randomly
    // Note that we pull *** values from the RNG in the process
    for (uint32_t i = 0; i < indices.size() - 2; i++) {
        uint32_t j = i + (rng.Next() % (indexCount - 1 - i));
        std::iter_swap(indices.begin() + i, indices.begin() + j);
    }
    return indices;
}

bool StegEngine::CanEncode(const Image& image, uint32_t payloadSize, const EncoderSettings& settings) {
    uint32_t totalSize = HeaderSize + payloadSize;

    // Calculate the total available parts
    uint32_t availableParts;
    if (settings.EncodeInAlpha || !image.HasAlpha()) {
        availableParts = image.GetPixelWidth() * image.GetWidth() * image.GetHeight();
    } else {
        uint32_t bytesPerChannel = image.GetBitDepth() / 8;
        availableParts = (image.GetPixelWidth() - bytesPerChannel) * image.GetWidth() * image.GetHeight();
    }

    // Subtract one byte from the available size for the seed (first byte of the image is unavailable)
    availableParts--;

    // Check if the payload can be encoded in the image with the given settings
    uint32_t totalParts = totalSize * 8 / settings.DataDepth;
    if (totalParts > availableParts) {
        return false;
    }

    // The payload can be encoded into the image with the specified settings
    return true;

}
