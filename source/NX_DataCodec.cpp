/* NX_DataCodec.c -- API definition for Nexium's data codec module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/NX_DataCodec.h>
#include <NX/NX_Memory.h>
#include <NX/NX_Log.h>

#include <SDL3/SDL_stdinc.h>
#include <zlib.h>

void* NX_CompressData(const void* data, size_t dataSize, size_t* outputSize)
{
    /* --- Validate input parameters --- */

    if (!data || dataSize == 0 || !outputSize) {
        if (outputSize) *outputSize = 0;
        return NULL;
    }

    /* --- Allocate buffer for header and compressed data --- */

    uLongf maxCompressedSize = compressBound(static_cast<uLongf>(dataSize));
    size_t totalSize = sizeof(uint64_t) + maxCompressedSize;
    void* buffer = NX_Malloc(totalSize);
    if (!buffer) {
        *outputSize = 0;
        return NULL;
    }

    /* --- Compress data after header --- */

    void* compressedData = static_cast<uint8_t*>(buffer) + sizeof(uint64_t);
    uLongf compressedSize = maxCompressedSize;
    int result = compress(
        static_cast<Bytef*>(compressedData),
        &compressedSize,
        static_cast<const Bytef*>(data),
        static_cast<uLongf>(dataSize)
    );

    if (result != Z_OK) {
        NX_Free(buffer);
        *outputSize = 0;
        return NULL;
    }

    /* --- Write uncompressed size to header --- */

    uint64_t uncompressedSizeHeader = static_cast<uint64_t>(dataSize);
    SDL_memcpy(buffer, &uncompressedSizeHeader, sizeof(uint64_t));

    /* --- Set output size and return buffer --- */

    *outputSize = sizeof(uint64_t) + static_cast<size_t>(compressedSize);

    return buffer;
}

void* NX_DecompressData(const void* data, size_t dataSize, size_t* outputSize)
{
    /* --- Validate input parameters --- */

    if (!data || dataSize < sizeof(uint64_t) || !outputSize) {
        return nullptr;
    }

    /* --- Read uncompressed size from header --- */

    uint64_t uncompressedSize;
    SDL_memcpy(&uncompressedSize, data, sizeof(uint64_t));

    /* --- Allocate decompression buffer --- */

    void* decompressedData = NX_Malloc(static_cast<size_t>(uncompressedSize));
    if (!decompressedData) {
        return nullptr;
    }

    /* --- Decompress data after header --- */

    const void* compressedData = static_cast<const uint8_t*>(data) + sizeof(uint64_t);
    size_t compressedSize = dataSize - sizeof(uint64_t);
    uLongf destLen = static_cast<uLongf>(uncompressedSize);
    int result = uncompress(
        static_cast<Bytef*>(decompressedData),
        &destLen,
        static_cast<const Bytef*>(compressedData),
        static_cast<uLongf>(compressedSize)
    );

    if (result != Z_OK) {
        NX_Free(decompressedData);
        return nullptr;
    }

    /* --- Set output size and return buffer --- */

    *outputSize = static_cast<size_t>(destLen);

    return decompressedData;
}

void* NX_CompressText(const char* text, size_t* outputSize)
{
    if (!text || !outputSize) {
        if (outputSize) *outputSize = 0;
        return NULL;
    }

    return NX_CompressData(text, strlen(text), outputSize);
}

char* NX_DecompressText(const void* data, size_t dataSize)
{
    /* --- Validate input parameters --- */

    if (!data || dataSize < sizeof(uint64_t)) {
        return nullptr;
    }

    /* --- Read uncompressed size from header --- */

    uint64_t uncompressedSize;
    SDL_memcpy(&uncompressedSize, data, sizeof(uint64_t));

    /* --- Allocate decompression buffer with extra byte for null terminator --- */

    char* decompressedText = NX_Malloc<char>(static_cast<size_t>(uncompressedSize) + 1);
    if (!decompressedText) {
        return nullptr;
    }

    /* --- Decompress data after header --- */

    const void* compressedData = static_cast<const uint8_t*>(data) + sizeof(uint64_t);
    size_t compressedSize = dataSize - sizeof(uint64_t);
    uLongf destLen = static_cast<uLongf>(uncompressedSize);
    int result = uncompress(
        reinterpret_cast<Bytef*>(decompressedText),
        &destLen,
        static_cast<const Bytef*>(compressedData),
        static_cast<uLongf>(compressedSize)
    );

    if (result != Z_OK) {
        NX_Free(decompressedText);
        return nullptr;
    }

    /* --- Add null terminator --- */

    decompressedText[destLen] = '\0';

    return decompressedText;
}

char* NX_EncodeBase64(const void* data, size_t dataSize, size_t* outputSize)
{
    // Base64 conversion table according to RFC 4648
    constexpr char b64Table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    /* --- Validation of input parameters --- */

    if (data == nullptr || dataSize < 0 || outputSize == nullptr) {
        return nullptr;
    }

    /* --- Special case: empty data --- */

    if (dataSize == 0) {
        char* empty = NX_Malloc<char>(1);
        if (empty != nullptr) {
            empty[0] = '\0';
            *outputSize = 1;
        }
        return empty;
    }

    /* --- Calculation of output size (4 characters per group of 3 bytes + null terminator) --- */

    const int completeGroups = dataSize / 3;
    const int remainingBytes = dataSize % 3;
    const int paddingCount = (remainingBytes > 0) ? (3 - remainingBytes) : 0;
    const int totalOutputSize = (completeGroups + (remainingBytes > 0 ? 1 : 0)) * 4 + 1;

    /* --- Memory allocation --- */

    char* encodedData = NX_Malloc<char>(totalOutputSize);
    if (encodedData == nullptr) {
        return nullptr;
    }

    auto bData = static_cast<const uint8_t*>(data);
    int outputIndex = 0;
    int inputIndex = 0;

    /* --- Processing complete 3-byte groups --- */

    for (int group = 0; group < completeGroups; ++group, inputIndex += 3) {
        const uint32_t triplet = (static_cast<uint32_t>(bData[inputIndex]) << 16) |
                                 (static_cast<uint32_t>(bData[inputIndex + 1]) << 8) |
                                  static_cast<uint32_t>(bData[inputIndex + 2]);

        encodedData[outputIndex++] = b64Table[(triplet >> 18) & 0x3F];
        encodedData[outputIndex++] = b64Table[(triplet >> 12) & 0x3F];
        encodedData[outputIndex++] = b64Table[(triplet >> 6) & 0x3F];
        encodedData[outputIndex++] = b64Table[triplet & 0x3F];
    }

    /* --- Processing remaining bytes (with padding) --- */

    if (remainingBytes > 0) {
        uint32_t triplet = static_cast<uint32_t>(bData[inputIndex]) << 16;
        if (remainingBytes == 2) {
            triplet |= static_cast<uint32_t>(bData[inputIndex + 1]) << 8;
        }
        encodedData[outputIndex++] = b64Table[(triplet >> 18) & 0x3F];
        encodedData[outputIndex++] = b64Table[(triplet >> 12) & 0x3F];
        encodedData[outputIndex++] = (remainingBytes == 2) ? b64Table[(triplet >> 6) & 0x3F] : '=';
        encodedData[outputIndex++] = '=';
    }

    /* --- Null Terminator --- */

    encodedData[outputIndex] = '\0';
    *outputSize = totalOutputSize;

    return encodedData;
}

void* NX_DecodeBase64(const char* text, size_t* outputSize)
{
    /* --- Validation of input parameters --- */

    if (text == nullptr || outputSize == nullptr) {
        return nullptr;
    }

    /* --- Base64 decoding table (255 = invalid character) --- */

    constexpr uint8_t base64DecodeTable[256] = {
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 0-15
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 16-31
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  62, 255, 255, 255,  63, // 32-47  ('+' = 62, '/' = 63)
         52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255, 255, 254, 255, 255, // 48-63  ('0'-'9' = 52-61, '=' = 254)
        255,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14, // 64-79  ('A'-'O' = 0-14)
         15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255, // 80-95  ('P'-'Z' = 15-25)
        255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40, // 96-111 ('a'-'o' = 26-40)
         41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51, 255, 255, 255, 255, 255, // 112-127 ('p'-'z' = 41-51)
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 128-143
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 144-159
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 160-175
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 176-191
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 192-207
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 208-223
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 224-239
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255  // 240-255
    };

    const int textLength = static_cast<int>(strlen(text));

    /* --- Length check (must be a multiple of 4) --- */

    if (textLength == 0) {
        *outputSize = 0;
        return nullptr;
    }

    if (textLength % 4 != 0) {
        NX_LOG(W, "CORE: Invalid Base64 string length (not multiple of 4)");
        return nullptr;
    }

    /* --- Padding count --- */

    int paddingCount = 0;
    if (text[textLength - 1] == '=') {
        paddingCount++;
        if (textLength > 1 && text[textLength - 2] == '=') {
            paddingCount++;
        }
    }

    /* --- Validation of padding (maximum 2 '=' characters) --- */

    if (paddingCount > 2) {
        NX_LOG(W, "CORE: Invalid Base64 padding");
        return nullptr;
    }

    /* --- Calculating the output size --- */

    const int decodedSize = (textLength / 4) * 3 - paddingCount;
    if (decodedSize < 0) {
        NX_LOG(W, "CORE: Invalid Base64 string");
        return nullptr;
    }

    /* --- Memory allocation --- */

    uint8_t* decodedData = NX_Malloc<uint8_t>(decodedSize);
    if (decodedData == nullptr) {
        return nullptr;
    }

    int outputIndex = 0;
    int inputIndex = 0;

    /* --- Processing in blocks of 4 characters --- */

    const int numCompleteGroups = textLength / 4;

    for (int group = 0; group < numCompleteGroups; ++group, inputIndex += 4)
    {
        /* --- Reading the 4 Base64 characters --- */

        const uint8_t c0 = static_cast<uint8_t>(text[inputIndex]);
        const uint8_t c1 = static_cast<uint8_t>(text[inputIndex + 1]);
        const uint8_t c2 = static_cast<uint8_t>(text[inputIndex + 2]);
        const uint8_t c3 = static_cast<uint8_t>(text[inputIndex + 3]);

        /* --- Decoding via the lookup table --- */

        const uint8_t v0 = base64DecodeTable[c0];
        const uint8_t v1 = base64DecodeTable[c1];
        const uint8_t v2 = base64DecodeTable[c2];
        const uint8_t v3 = base64DecodeTable[c3];

        /* --- Character validation (254 = padding '=', 255 = invalid) --- */

        if (v0 == 255 || v1 == 255 || 
            (v2 == 255 && c2 != '=') || 
            (v3 == 255 && c3 != '=')) {
            NX_LOG(W, "CORE: Invalid Base64 character detected");
            NX_Free(decodedData);
            return nullptr;
        }

        /* --- Rebuild the sixtets (replace padding with 0) --- */

        const uint32_t sextet0 = (v0 != 254) ? v0 : 0;
        const uint32_t sextet1 = (v1 != 254) ? v1 : 0;
        const uint32_t sextet2 = (v2 != 254) ? v2 : 0;
        const uint32_t sextet3 = (v3 != 254) ? v3 : 0;

        /* --- 24-bit triplet combination --- */

        const uint32_t triplet = (sextet0 << 18) | (sextet1 << 12) | (sextet2 << 6) | sextet3;

        /* --- Extracting bytes --- */

        decodedData[outputIndex++] = static_cast<uint8_t>((triplet >> 16) & 0xFF);

        if (outputIndex < decodedSize) {
            decodedData[outputIndex++] = static_cast<uint8_t>((triplet >> 8) & 0xFF);
        }

        if (outputIndex < decodedSize) {
            decodedData[outputIndex++] = static_cast<uint8_t>(triplet & 0xFF);
        }
    }

    *outputSize = decodedSize;

    return decodedData;
}

uint32_t NX_ComputeCRC32(void* data, size_t dataSize)
{
    return crc32(0L, (const uint8_t*)data, dataSize);
}

const uint32_t* NX_ComputeMD5(void* data, size_t dataSize)
{
    /* --- MD5 constants --- */

    // Per-round shift amounts (4 rounds of 16 operations each)
    constexpr uint32_t shiftAmounts[64] = {
        7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  // Round 1
        5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  // Round 2
        4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  // Round 3
        6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21   // Round 4
    };

    // Binary integer parts of the sines of integers (radians) as constants
    constexpr uint32_t sineConstants[64] = {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
        0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
        0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
        0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
        0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
        0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
        0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
        0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
        0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
        0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
        0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
        0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
        0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
    };

    /* --- Initialize hash state (MD5 magic numbers) --- */

    static uint32_t hash[4];
    hash[0] = 0x67452301;
    hash[1] = 0xefcdab89;
    hash[2] = 0x98badcfe;
    hash[3] = 0x10325476;

    /* --- Calculate padded message size --- */

    // Message must be padded to 448 bits (mod 512), then append 64-bit length
    // This ensures total size is a multiple of 512 bits (64 bytes)

    const size_t paddedSize = ((((dataSize + 8) / 64) + 1) * 64) - 8;
    const uint32_t bitLength = static_cast<uint32_t>(dataSize * 8);

    /* --- Allocate and prepare padded message --- */

    uint8_t* message = NX_Calloc<uint8_t>(paddedSize + 64);
    if (!message) {
        return hash;
    }

    // Copy original data
    SDL_memcpy(message, data, dataSize);

    // Append '1' bit (0x80 = 10000000 in binary)
    message[dataSize] = 0x80;

    // Append original message length in bits (little-endian, lower 32 bits only)
    SDL_memcpy(message + paddedSize, &bitLength, sizeof(uint32_t));

    /* --- Process message in 512-bit (64-byte) chunks --- */

    for (size_t offset = 0; offset < paddedSize; offset += 64)
    {
        // Break chunk into sixteen 32-bit words (little-endian)
        const uint32_t* words = reinterpret_cast<const uint32_t*>(message + offset);

        // Initialize working variables with current hash state
        uint32_t a = hash[0];
        uint32_t b = hash[1];
        uint32_t c = hash[2];
        uint32_t d = hash[3];

        /* --- Perform 64 operations (4 rounds of 16 operations) --- */

        for (int i = 0; i < 64; ++i)
        {
            uint32_t f, g;

            // Select auxiliary function and word index based on round
            if (i < 16) {
                f = (b & c) | ((~b) & d);
                g = i;
            }
            else if (i < 32) {
                f = (d & b) | ((~d) & c);
                g = (5 * i + 1) % 16;
            }
            else if (i < 48) {
                f = b ^ c ^ d;
                g = (3 * i + 5) % 16;
            }
            else {
                f = c ^ (b | (~d));
                g = (7 * i) % 16;
            }

            // Rotate left
            const uint32_t temp = d;
            d = c;
            c = b;
            const uint32_t rotated = (a + f + sineConstants[i] + words[g]);
            b = b + ((rotated << shiftAmounts[i]) | (rotated >> (32 - shiftAmounts[i])));
            a = temp;
        }

        /* --- Add this chunk's hash to result --- */

        hash[0] += a;
        hash[1] += b;
        hash[2] += c;
        hash[3] += d;
    }

    /* --- Cleanup and return --- */

    NX_Free(message);

    return hash;
}

const uint32_t* NX_ComputeSHA1(void* data, size_t dataSize)
{
    /* --- SHA-1 constants --- */

    // Round constants (used in different phases of compression)
    constexpr uint32_t roundConstants[4] = {
        0x5A827999,  // Rounds 0-19
        0x6ED9EBA1,  // Rounds 20-39
        0x8F1BBCDC,  // Rounds 40-59
        0xCA62C1D6   // Rounds 60-79
    };

    /* --- Initialize hash state (SHA-1 magic numbers) --- */

    static uint32_t hash[5];
    hash[0] = 0x67452301;
    hash[1] = 0xEFCDAB89;
    hash[2] = 0x98BADCFE;
    hash[3] = 0x10325476;
    hash[4] = 0xC3D2E1F0;

    /* --- Calculate padded message size --- */

    // Message must be padded to 448 bits (mod 512), then append 64-bit length
    // This ensures total size is a multiple of 512 bits (64 bytes)

    const size_t paddedSize = ((((dataSize + 8) / 64) + 1) * 64);

    /* --- Allocate and prepare padded message --- */

    uint8_t* message = NX_Calloc<uint8_t>(paddedSize);
    if (!message) {
        return hash;
    }

    // Copy original data
    SDL_memcpy(message, data, dataSize);

    // Append '1' bit (0x80 = 10000000 in binary)
    message[dataSize] = 0x80;

    // Append original message length in bits (big-endian, as 64-bit value)
    // SHA-1 uses big-endian format unlike MD5
    const uint64_t bitLength = dataSize * 8;
    for (int i = 0; i < 8; ++i) {
        message[paddedSize - 1 - i] = static_cast<uint8_t>((bitLength >> (i * 8)) & 0xFF);
    }

    /* --- Process message in 512-bit (64-byte) chunks --- */

    for (size_t offset = 0; offset < paddedSize; offset += 64)
    {
        /* --- Prepare message schedule (80 words) --- */

        uint32_t w[80] = { 0 };

        // Break chunk into sixteen 32-bit words (big-endian)
        for (int i = 0; i < 16; ++i) {
            w[i] = (static_cast<uint32_t>(message[offset + (i * 4) + 0]) << 24) |
                   (static_cast<uint32_t>(message[offset + (i * 4) + 1]) << 16) |
                   (static_cast<uint32_t>(message[offset + (i * 4) + 2]) << 8)  |
                   (static_cast<uint32_t>(message[offset + (i * 4) + 3]));
        }

        // Extend the sixteen 32-bit words into eighty 32-bit words
        for (int i = 16; i < 80; ++i) {
            const uint32_t temp = w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16];
            w[i] = (temp << 1) | (temp >> 31);  // Rotate left by 1
        }

        /* --- Initialize working variables with current hash state --- */

        uint32_t a = hash[0];
        uint32_t b = hash[1];
        uint32_t c = hash[2];
        uint32_t d = hash[3];
        uint32_t e = hash[4];

        /* --- Perform 80 operations (4 rounds of 20 operations) --- */

        for (int i = 0; i < 80; ++i)
        {
            uint32_t f, k;

            if (i < 20) {
                f = (b & c) | ((~b) & d);
                k = roundConstants[0];
            }
            else if (i < 40) {
                f = b ^ c ^ d;
                k = roundConstants[1];
            }
            else if (i < 60) {
                f = (b & c) | (b & d) | (c & d);
                k = roundConstants[2];
            }
            else {
                f = b ^ c ^ d;
                k = roundConstants[3];
            }

            const uint32_t rotatedA = (a << 5) | (a >> 27);  // Rotate left by 5
            const uint32_t temp = rotatedA + f + e + k + w[i];

            e = d;
            d = c;
            c = (b << 30) | (b >> 2);  // Rotate left by 30
            b = a;
            a = temp;
        }

        /* --- Add this chunk's hash to result --- */

        hash[0] += a;
        hash[1] += b;
        hash[2] += c;
        hash[3] += d;
        hash[4] += e;
    }

    /* --- Cleanup and return --- */

    NX_Free(message);

    return hash;
}

const uint32_t* NX_ComputeSHA256(void* data, size_t dataSize)
{
    /* --- SHA-256 constants --- */

    // Round constants (first 32 bits of the fractional parts of the cube roots of the first 64 primes)
    constexpr uint32_t roundConstants[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
        0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
        0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
        0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
        0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
        0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
        0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
        0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
        0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    /* --- Initialize hash state (SHA-256 magic numbers) --- */

    // First 32 bits of the fractional parts of the square roots of the first 8 primes
    static uint32_t hash[8];
    hash[0] = 0x6a09e667;
    hash[1] = 0xbb67ae85;
    hash[2] = 0x3c6ef372;
    hash[3] = 0xa54ff53a;
    hash[4] = 0x510e527f;
    hash[5] = 0x9b05688c;
    hash[6] = 0x1f83d9ab;
    hash[7] = 0x5be0cd19;

    /* --- Calculate padded message size --- */

    // Message must be padded to 448 bits (mod 512), then append 64-bit length
    // This ensures total size is a multiple of 512 bits (64 bytes)

    const uint64_t bitLength = static_cast<uint64_t>(dataSize) * 8;
    size_t paddedSize = dataSize + sizeof(uint64_t);
    paddedSize += (64 - (paddedSize % 64)) % 64;
    if (paddedSize < dataSize + sizeof(uint64_t) + 1) {
        paddedSize += 64;
    }

    /* --- Allocate and prepare padded message --- */

    uint8_t* message = NX_Calloc<uint8_t>(paddedSize);
    if (!message) {
        return hash;
    }

    // Copy original data
    SDL_memcpy(message, data, dataSize);

    // Append '1' bit (0x80 = 10000000 in binary)
    message[dataSize] = 0x80;

    // Append original message length in bits (big-endian, as 64-bit value)
    for (int i = 0; i < 8; ++i) {
        message[paddedSize - 8 + i] = static_cast<uint8_t>((bitLength >> (8 * (7 - i))) & 0xFF);
    }

    /* --- Process message in 512-bit (64-byte) chunks --- */

    for (size_t offset = 0; offset < paddedSize; offset += 64)
    {
        /* --- Prepare message schedule (64 words) --- */

        uint32_t w[64];

        // Break chunk into sixteen 32-bit words (big-endian)
        for (int i = 0; i < 16; ++i) {
            w[i] = (static_cast<uint32_t>(message[offset + (i * 4) + 0]) << 24) |
                   (static_cast<uint32_t>(message[offset + (i * 4) + 1]) << 16) |
                   (static_cast<uint32_t>(message[offset + (i * 4) + 2]) << 8)  |
                   (static_cast<uint32_t>(message[offset + (i * 4) + 3]));
        }

        // Extend the sixteen 32-bit words into sixty-four 32-bit words
        for (int i = 16; i < 64; ++i)
        {
            const uint32_t s0_x = w[i - 15];
            const uint32_t s0 = ((s0_x >> 7) | (s0_x << 25)) ^ ((s0_x >> 18) | (s0_x << 14)) ^ (s0_x >> 3);

            const uint32_t s1_x = w[i - 2];
            const uint32_t s1 = ((s1_x >> 17) | (s1_x << 15)) ^ ((s1_x >> 19) | (s1_x << 13)) ^ (s1_x >> 10);

            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }

        /* --- Initialize working variables with current hash state --- */

        uint32_t a = hash[0];
        uint32_t b = hash[1];
        uint32_t c = hash[2];
        uint32_t d = hash[3];
        uint32_t e = hash[4];
        uint32_t f = hash[5];
        uint32_t g = hash[6];
        uint32_t h = hash[7];

        /* --- Perform 64 rounds of compression --- */

        for (int i = 0; i < 64; ++i)
        {
            const uint32_t sum1 = ((e >> 6) | (e << 26)) ^ ((e >> 11) | (e << 21)) ^ ((e >> 25) | (e << 7));

            const uint32_t ch = (e & f) ^ ((~e) & g);
            const uint32_t t1 = h + sum1 + ch + roundConstants[i] + w[i];

            const uint32_t sum0 = ((a >> 2) | (a << 30)) ^ ((a >> 13) | (a << 19)) ^ ((a >> 22) | (a << 10));

            const uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            const uint32_t t2 = sum0 + maj;

            h = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }

        /* --- Add this chunk's hash to result --- */

        hash[0] += a;
        hash[1] += b;
        hash[2] += c;
        hash[3] += d;
        hash[4] += e;
        hash[5] += f;
        hash[6] += g;
        hash[7] += h;
    }

    /* --- Cleanup and return --- */

    NX_Free(message);

    return hash;
}
