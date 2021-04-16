#include "include/StegCrypt.h"

#include "aes.h"
#include "include/argon2.h"
#include "include/StegTimer.h"

using namespace Steg;

// These methods will handle the IV in the background
std::vector<byte> StegCrypt::Encrypt(const std::vector<byte>& pass, const std::vector<byte> data, Algorithm algo) {

    // Start the Encrypt Timer
    StegTimer::StartTimer(StegTimer::TimerLabel::ENCRYPT);

    // Create a random number generator with seed 0 for the IV
    RNG rng(0);

    // Get the block length of this algorithm
    uint32_t blockLength = GetBlockLength(algo);

    // Derive key from the password
    std::vector<byte> key = DeriveKey(pass, blockLength, rng);

    // IV is BLOCK_SIZE bytes long
    std::vector<byte> iv = GetIV(rng, blockLength);

    // Data is padded to nearest blockLength bytes
    std::vector<byte> dataBuffer = AddPadding(data, blockLength);

    byte *ivBytes = &iv[0];
    byte *keyBytes = &key[0];
    byte *dataBytes = &dataBuffer[0];

    AES_ctx context;
    AES128_init_ctx(&context, keyBytes);
    if (algo == Algorithm::ALGO_AES128) {
        AES128_init_ctx_iv(&context, keyBytes, ivBytes);
        AES128_CBC_encrypt_buffer(&context, dataBytes, dataBuffer.size());
    } else if (algo == Algorithm::ALGO_AES192) {
        AES192_init_ctx_iv(&context, keyBytes, ivBytes);
        AES192_CBC_encrypt_buffer(&context, dataBytes, dataBuffer.size());
    } else if (algo == Algorithm::ALGO_AES256) {
        AES256_init_ctx_iv(&context, keyBytes, ivBytes);
        AES256_CBC_encrypt_buffer(&context, dataBytes, dataBuffer.size());
    } else {
        throw std::invalid_argument("Unsupported Algorithm");
    }

    // Prepend IV to the data buffer
    dataBuffer.insert(dataBuffer.begin(), iv.begin(), iv.end());

    // End the Encrypt Timer
    StegTimer::EndTimer(StegTimer::TimerLabel::ENCRYPT);

    return dataBuffer;

}

std::vector<byte> StegCrypt::Decrypt(const std::vector<byte>& pass, const std::vector<byte> data, Algorithm algo) {

    // Start the Decrypt Timer
    StegTimer::StartTimer(StegTimer::TimerLabel::DECRYPT);

    // Create a random number generator with seed 0 for the IV
    RNG rng(0);

    // Get the block length of this algorithm
    uint32_t blockLength = GetBlockLength(algo);

    // Derive key from the password
    std::vector<byte> key = DeriveKey(pass, blockLength, rng);

    // IV is BLOCK_SIZE bytes long
    std::vector<byte> iv(data.begin(), data.begin() + blockLength);

    // Clip off the IV from the front
    std::vector<byte> dataBuffer(data.begin() + blockLength, data.end());

    byte *ivBytes = &iv[0];
    byte *keyBytes = &key[0];
    byte *dataBytes = &dataBuffer[0];

    AES_ctx context;
    AES128_init_ctx(&context, keyBytes);
    if (algo == Algorithm::ALGO_AES128) {
        AES128_init_ctx_iv(&context, keyBytes, ivBytes);
        AES128_CBC_decrypt_buffer(&context, dataBytes, dataBuffer.size());
    } else if (algo == Algorithm::ALGO_AES192) {
        AES192_init_ctx_iv(&context, keyBytes, ivBytes);
        AES192_CBC_decrypt_buffer(&context, dataBytes, dataBuffer.size());
    } else if (algo == Algorithm::ALGO_AES256) {
        AES256_init_ctx_iv(&context, keyBytes, ivBytes);
        AES256_CBC_decrypt_buffer(&context, dataBytes, dataBuffer.size());
    } else {
        throw std::invalid_argument("Unsupported Algorithm");
    }

    std::vector<byte> decryptedBytes = RemovePadding(dataBuffer);

    // End the Decrypt Timer
    StegTimer::EndTimer(StegTimer::TimerLabel::DECRYPT);

    return decryptedBytes;

}

std::vector<byte> StegCrypt::GetIV(RNG& rng, uint32_t ivLength) {
    std::vector<byte> iv(ivLength);
    uint64_t rand64 = rng.Next();
    rand64 <<= 32;
    rand64 |= rng.Next();
    int k = 0;
    for (int i = 0; i < 2; i++) {
        iv[k++] = ((rand64 >> 56) & 0xFF);
        iv[k++] = ((rand64 >> 40) & 0xFF);
        iv[k++] = ((rand64 >> 32) & 0xFF);
        iv[k++] = ((rand64 >> 48) & 0xFF);
        iv[k++] = ((rand64 >> 24) & 0xFF);
        iv[k++] = ((rand64 >> 16) & 0xFF);
        iv[k++] = ((rand64 >> 8) & 0xFF);
        iv[k++] = (rand64 & 0xFF);
    }
    return iv;
}

std::vector<byte> StegCrypt::DeriveKey(const std::vector<byte>& pass, uint32_t keySize, RNG& rng) {

    // Generate a cryptographic salt
    std::vector<byte> salt(keySize);
    for (uint32_t i = 0; i < keySize; i++) {
        salt[i] = rng.Next() & 0xFF;
    }

    // Array to hold the resulting key bytes
    std::vector<byte> key(keySize);

    // Derive key using Argon2
    argon2i_hash_raw(2, 1 << 8, 1, &pass[0], pass.size(), &salt[0], salt.size(), &key[0], keySize);

    return key;

}

// PKCS7 Padding
std::vector<byte> StegCrypt::AddPadding(const std::vector<byte> data, uint32_t blockLength) {
    std::vector<byte> paddedData(data);
    uint32_t padAmount = blockLength - (data.size() % blockLength);
    for (byte i = 0; i < padAmount; i++) {
        paddedData.push_back(padAmount);
    }
    return paddedData;
}

// PKCS7 Padding
std::vector<byte> StegCrypt::RemovePadding(const std::vector<byte> data) {
    std::vector<byte> unpaddedData(data);
    uint32_t padAmount = data[data.size() - 1];
    for (byte i = 0; i < padAmount; i++) {
        unpaddedData.pop_back();
    }
    return unpaddedData;
}

uint32_t StegCrypt::GetBlockLength(Algorithm algo) {
    switch (algo) {
        case Algorithm::ALGO_AES128:
            return 16;
        case Algorithm::ALGO_AES192:
            return 24;
        case Algorithm::ALGO_AES256:
            return 32;
        default:
            throw std::invalid_argument("Unsupported Algorithm");
            return 0;
    }
}
