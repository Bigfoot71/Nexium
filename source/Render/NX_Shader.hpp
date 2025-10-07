#ifndef NX_RENDER_SHADER_HPP
#define NX_RENDER_SHADER_HPP

#include <NX/NX_Render.h>

#include "../Detail/Util/DynamicArray.hpp"
#include "../Detail/Util/String.hpp"

#include "../Detail/GPU/Pipeline.hpp"
#include "../Detail/GPU/Program.hpp"
#include "../Detail/GPU/Buffer.hpp"

#include <array>

/* === Declaration === */

class NX_Shader {
public:
    enum Shader { SHAPE_COLOR, SHAPE_TEXTURE, TEXT_BITMAP, TEXT_SDF, SHADER_COUNT };
    enum Sampler { SAMPLER_0, SAMPLER_1, SAMPLER_2, SAMPLER_3, SAMPLER_COUNT, };
    enum Uniform { STATIC_UNIFORM, DYNAMIC_UNIFORM, UNIFORM_COUNT };

public:
    using TextureArray = std::array<const gpu::Texture*, SAMPLER_COUNT>;

public:
    NX_Shader();
    NX_Shader(const char* vertex, const char* fragment);

    /** Texture getter/setter */
    void getTextures(TextureArray& textures);
    void setTexture(int slot, const gpu::Texture* texture);

    /** Uniform buffer uploading functions */
    void updateStaticBuffer(size_t offset, size_t size, const void* data);
    void updateDynamicBuffer(size_t size, const void* data);

    /** Binding functions */
    void bindUniformBuffers(const gpu::Pipeline& pipeline, int dynamicRangeIndex);
    void bindTextures(const gpu::Pipeline& pipeline, const TextureArray& textures, const gpu::Texture& defaultTexture);

    /** Dynamic buffer management */
    void clearDynamicBuffer();

    /** Getters */
    gpu::Program& program(Shader shader);
    int dynamicRangeIndex() const;

private:
    /** Used at construction to generate final shader code */
    void insertUserCode(util::String& source, const char* dst, const char* src);

private:
    /** Built-in sampler names */
    static constexpr const char* SamplerName[SAMPLER_COUNT] = {
        "Texture0", "Texture1", "Texture2", "Texture3",
    };

    /** Built-in uniform block names */
    static constexpr const char* UniformName[UNIFORM_COUNT] = {
        "StaticBuffer", "DynamicBuffer"
    };

    /** Built-in uniform block binding points */
    static constexpr int SamplerBinding[SAMPLER_COUNT] {
        31, 30, 29, 28,
    };

    /** Built-in uniform block binding points */
    static constexpr int UniformBinding[UNIFORM_COUNT] {
        15, 14
    };

private:
    struct DynamicBuffer {
        struct Range { size_t offset, size; };
        util::DynamicArray<Range> ranges{};
        int currentRangeIndex{};
        size_t currentOffset{};
        gpu::Buffer buffer{};
    };

    struct SamplerSlot {
        const gpu::Texture* texture{};
        bool exists{};
    };

private:
    std::array<gpu::Program, SHADER_COUNT> mPrograms{};
    std::array<SamplerSlot, SAMPLER_COUNT> mTextures{};
    DynamicBuffer mDynamicBuffer{};
    gpu::Buffer mStaticBuffer{};
};

/* === Public Implementation === */

inline void NX_Shader::getTextures(TextureArray& textures)
{
    for (int i = 0; i < SAMPLER_COUNT; i++) {
        textures[i] = mTextures[i].texture;
    }
}

inline void NX_Shader::setTexture(int slot, const gpu::Texture* texture)
{
    if (slot < 0 || slot >= SAMPLER_COUNT) {
        NX_INTERNAL_LOG(E, "RENDER: Unable to set shader texture at slot %i/%i; Exceeds the number of slot", slot, SAMPLER_COUNT);
        return;
    }

    if (!mTextures[slot].exists) {
        NX_INTERNAL_LOG(E, "RENDER: Unable to set shader texture at slot %i/%i; This slot is not defined in the shader", slot, SAMPLER_COUNT);
        return;
    }

    mTextures[slot].texture = texture;
}

inline void NX_Shader::clearDynamicBuffer()
{
    mDynamicBuffer.currentOffset = 0;
    mDynamicBuffer.ranges.clear();
}

inline gpu::Program& NX_Shader::program(Shader shader)
{
    return mPrograms[shader];
}

inline int NX_Shader::dynamicRangeIndex() const
{
    return mDynamicBuffer.currentRangeIndex;
}

/* === Private Implementation === */

inline void NX_Shader::insertUserCode(util::String& source, const char* dst, const char* src)
{
    if (src == nullptr) return;
    if (size_t pos = source.find(dst); pos != std::string::npos) {
        source.replace(pos, SDL_strlen(dst), src);
    }
}

#endif // NX_RENDER_SHADER_HPP
