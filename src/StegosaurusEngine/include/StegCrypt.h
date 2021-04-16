#pragma once

#include "Core.h"

#include "RNG.h"

namespace Steg {

    class StegCrypt {

    public:

        enum class Algorithm {
            ALGO_AES128,
            ALGO_AES192,
            ALGO_AES256
        };

        static std::vector<byte> Encrypt(const std::vector<byte>& key, const std::vector<byte> inputBytes, Algorithm algo);

        static std::vector<byte> Decrypt(const std::vector<byte>& key, const std::vector<byte> inputBytes, Algorithm algo);

    private:

        static std::vector<byte> GetIV(RNG& rng, uint32_t ivLength);

        static std::vector<byte> DeriveKey(const std::vector<byte>& key, uint32_t keySize, RNG& rng);

        static std::vector<byte> AddPadding(const std::vector<byte> data, uint32_t blockLength);

        static std::vector<byte> RemovePadding(const std::vector<byte> data);

    public:

        static uint32_t GetBlockLength(Algorithm algo);

    };

}
