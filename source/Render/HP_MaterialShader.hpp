#ifndef HP_RENDER_MATERIAL_SHADER_HPP
#define HP_RENDER_MATERIAL_SHADER_HPP

#include <Hyperion/HP_Render.h>

#include "../Detail/Util/DynamicArray.hpp"
#include "../Detail/GPU/Pipeline.hpp"
#include "../Detail/GPU/Program.hpp"
#include "../Detail/GPU/Buffer.hpp"

#include <string_view>
#include <array>

/* === Declaration === */

class HP_MaterialShader {
public:
    enum Shader { SCENE_LIT, SCENE_UNLIT, SCENE_WIREFRAME, PREPASS, SHADOW, SHADER_COUNT };
    enum Sampler { TEXTURE_0, TEXTURE_1, TEXTURE_2, TEXTURE_3, TEXTURE_COUNT, };
    enum Uniform { STATIC_UNIFORM, DYNAMIC_UNIFORM, UNIFORM_COUNT };

public:
    using TextureArray = std::array<const gpu::Texture*, TEXTURE_COUNT>;

public:
    HP_MaterialShader();
    HP_MaterialShader(const char* vertex, const char* fragment);

    /** Texture getter/setter */
    void getTextures(TextureArray& textures);
    void setTexture(int slot, const gpu::Texture* texture);

    /** Uniform buffer uploading functions */
    void updateStaticBuffer(size_t offset, size_t size, const void* data);
    void updateDynamicBuffer(size_t size, const void* data);

    /** Binding functions */
    void bindUniformBuffers(const gpu::Pipeline& pipeline, HP_ShadingMode shading, int dynamicRangeIndex);
    void bindUniformBuffers(const gpu::Pipeline& pipeline, Shader shader, int dynamicRangeIndex);
    void bindTextures(const gpu::Pipeline& pipeline, const TextureArray& textures, const gpu::Texture& defaultTexture);

    /** Dynamic buffer management */
    void clearDynamicBuffer();

    /** Getters */
    gpu::Program& program(HP_ShadingMode shading);
    gpu::Program& program(Shader shader);
    int dynamicRangeIndex() const;

private:
    /** Used at construction to generate final shader code */
    void processCode(std::string& source, const std::string_view& define, const char* code);

    /** Conversion helpers */
    static Shader shaderFromShadingMode(HP_ShadingMode shading);

private:
    /** Built-in sampler names */
    static constexpr const char* SamplerName[TEXTURE_COUNT] = {
        "Texture0", "Texture1", "Texture2", "Texture3",
    };

    /** Built-in uniform block names */
    static constexpr const char* UniformName[UNIFORM_COUNT] = {
        "StaticBuffer", "DynamicBuffer"
    };

    /** Built-in uniform block binding points */
    static constexpr int SamplerBinding[TEXTURE_COUNT] {
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
    std::array<SamplerSlot, TEXTURE_COUNT> mTextures{};
    DynamicBuffer mDynamicBuffer{};
    gpu::Buffer mStaticBuffer{};
};

/* === Public Implementation === */

inline void HP_MaterialShader::getTextures(TextureArray& textures)
{
    for (int i = 0; i < TEXTURE_COUNT; i++) {
        textures[i] = mTextures[i].texture;
    }
}

inline void HP_MaterialShader::setTexture(int slot, const gpu::Texture* texture)
{
    if (slot < 0 || slot >= TEXTURE_COUNT) {
        HP_INTERNAL_LOG(E, "RENDER: Unable to set material shader texture at slot %i/%i; Exceeds the number of slot", slot, TEXTURE_COUNT);
        return;
    }

    if (!mTextures[slot].exists) {
        HP_INTERNAL_LOG(E, "RENDER: Unable to set material shader texture at slot %i/%i; This slot is not defined in the material shader", slot, TEXTURE_COUNT);
        return;
    }

    mTextures[slot].texture = texture;
}

inline void HP_MaterialShader::bindUniformBuffers(const gpu::Pipeline& pipeline, HP_ShadingMode shading, int dynamicRangeIndex)
{
    bindUniformBuffers(pipeline, shaderFromShadingMode(shading), dynamicRangeIndex);
}

inline void HP_MaterialShader::clearDynamicBuffer()
{
    mDynamicBuffer.currentOffset = 0;
    mDynamicBuffer.ranges.clear();
}

inline gpu::Program& HP_MaterialShader::program(HP_ShadingMode shading)
{
    return mPrograms[shaderFromShadingMode(shading)];
}

inline gpu::Program& HP_MaterialShader::program(Shader shader)
{
    return mPrograms[shader];
}

inline int HP_MaterialShader::dynamicRangeIndex() const
{
    return mDynamicBuffer.currentRangeIndex;
}

/* === Private Implementation === */

inline HP_MaterialShader::Shader HP_MaterialShader::shaderFromShadingMode(HP_ShadingMode shading)
{
    switch (shading) {
    case HP_SHADING_LIT:
        return SCENE_LIT;
    case HP_SHADING_UNLIT:
        return SCENE_UNLIT;
    case HP_SHADING_WIREFRAME:
        return SCENE_WIREFRAME;
    default:
        break;
    }
    return SCENE_LIT;
}

#endif // HP_RENDER_MATERIAL_SHADER_HPP
