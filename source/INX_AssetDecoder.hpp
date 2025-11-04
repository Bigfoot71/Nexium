/* INX_AssetDecoder.hpp -- Internal implementation details containing utils for decoding embedded assets
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_ASSETS_SHADER_DECODER_HPP
#define NX_ASSETS_SHADER_DECODER_HPP

#include <NX/NX_DataCodec.h>
#include "./Detail/Util/Memory.hpp"

// ============================================================================
// SHADER DECODER
// ============================================================================

class INX_ShaderDecoder {
public:
    /** Constructors */
    INX_ShaderDecoder(const void* code, size_t size);

    /** Getters */
    const char* GetCode() const noexcept;
    operator const char*() const noexcept;

private:
    util::UniquePtr<char> mCode{};
};

inline INX_ShaderDecoder::INX_ShaderDecoder(const void* code, size_t size)
{
    char* decomp = NX_DecompressText(code, size);
    mCode = util::UniquePtr<char>(decomp);
}

inline const char* INX_ShaderDecoder::GetCode() const noexcept
{
    return mCode.get();
}

inline INX_ShaderDecoder::operator const char*() const noexcept
{
    return mCode.get();
}

#endif // NX_ASSETS_SHADER_DECODER_HPP
