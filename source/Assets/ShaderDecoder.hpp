#ifndef NX_ASSETS_SHADER_DECODER_HPP
#define NX_ASSETS_SHADER_DECODER_HPP

#include <NX/NX_DataCodec.h>

#include "../Detail/Util/Memory.hpp"

namespace assets {

/* === Declaration === */

class ShaderDecoder {
public:
    /** Constructors */
    ShaderDecoder(const void* code, size_t size);

    /** Getters */
    const char* code() const noexcept;
    operator const char*() const noexcept;

private:
    util::UniquePtr<char> mCode{};
};

/* === Public Implementation === */

inline ShaderDecoder::ShaderDecoder(const void* code, size_t size)
{
    char* decomp = NX_DecompressText(code, size);
    mCode = util::UniquePtr<char>(decomp);
}

inline const char* ShaderDecoder::code() const noexcept
{
    return mCode.get();
}

inline ShaderDecoder::operator const char*() const noexcept
{
    return mCode.get();
}

} // namespace assets

#endif // NX_ASSETS_SHADER_DECODER_HPP
