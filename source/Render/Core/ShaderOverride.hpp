#ifndef NX_RENDER_SHADER_OVERRIDE_HPP
#define NX_RENDER_SHADER_OVERRIDE_HPP

#include "../../Detail/Util/DynamicArray.hpp"
#include "../../Detail/Util/String.hpp"

#include "../../Detail/GPU/Pipeline.hpp"
#include "../../Detail/GPU/Program.hpp"
#include "../../Detail/GPU/Texture.hpp"
#include "../../Detail/GPU/Buffer.hpp"

#include <array>

namespace render {

/**
 * Traits structure to define shader variants for a specific shader class.
 * Must be specialized for each derived shader class.
 * 
 * Example:
 *   template<>
 *   struct ShaderTraits<CustomShader> {
 *       enum Variant { VARIANT_A, VARIANT_B, VARIANT_COUNT };
 *   };
 */
template <typename T>
struct ShaderTraits;

/**
 * Base class for custom shader implementations using CRTP pattern.
 * Provides common functionality for:
 * - Multiple shader program variants
 * - Texture binding (up to 4 samplers)
 * - Static and dynamic uniform buffer management
 * 
 * Derived classes must specialize ShaderTraits<Derived> with a Variant enum.
 */
template <typename Derived>
class ShaderOverride {
public:
    /** Shader variant enum from the derived class */
    using Variant = typename ShaderTraits<Derived>::Variant;
    static constexpr size_t VariantCount = static_cast<size_t>(ShaderTraits<Derived>::Variant::VARIANT_COUNT);

    /** Texture sampler slots (0-3) */
    enum Sampler { 
        SAMPLER_0, 
        SAMPLER_1, 
        SAMPLER_2, 
        SAMPLER_3, 
        SAMPLER_COUNT 
    };
    
    /** Uniform buffer types */
    enum UniformBuffer { 
        STATIC_UNIFORM,   // Infrequently updated data
        DYNAMIC_UNIFORM,  // Per-draw data
        UNIFORM_COUNT 
    };

    using TextureArray = std::array<const gpu::Texture*, SAMPLER_COUNT>;

public:
    /** Get all currently bound textures */
    void getTextures(TextureArray& textures);
    
    /** Bind a texture to a specific sampler slot */
    void setTexture(int slot, const gpu::Texture* texture);

    /** Upload data to the static uniform buffer */
    void updateStaticBuffer(size_t offset, size_t size, const void* data);
    
    /** Upload data to the dynamic uniform buffer (creates a new range) */
    void updateDynamicBuffer(size_t size, const void* data);

    /** Bind uniform buffers for the current draw call */
    void bindUniforms(const gpu::Pipeline& pipeline, int dynamicRangeIndex);
    
    /** Bind all textures to their respective sampler units */
    void bindTextures(const gpu::Pipeline& pipeline, const TextureArray& textures, const gpu::Texture& defaultTexture);

    /** Reset dynamic buffer state (must be called at the end of each frame) */
    void clearDynamicBuffer();

    /** Get the shader program for a specific variant */
    gpu::Program& program(Variant variant);
    
    /** Get the current dynamic buffer range index */
    int dynamicRangeIndex() const;

protected:
    /** Helper to inject user code into shader source at a marker position */
    void insertUserCode(util::String& source, const char* marker, const char* userCode);

protected:
    /** Built-in GLSL sampler uniform names */
    static constexpr const char* SamplerName[SAMPLER_COUNT] = {
        "Texture0", "Texture1", "Texture2", "Texture3",
    };

    /** Built-in GLSL uniform block names */
    static constexpr const char* UniformName[UNIFORM_COUNT] = {
        "StaticBuffer", "DynamicBuffer"
    };

    /** Texture unit binding points (31-28) */
    static constexpr int SamplerBinding[SAMPLER_COUNT] = {
        31, 30, 29, 28,
    };

    /** Uniform buffer binding points (15-14) */
    static constexpr int UniformBinding[UNIFORM_COUNT] = {
        15, 14
    };

protected:
    struct DynamicBuffer {
        struct Range { size_t offset, size; };
        util::DynamicArray<Range> ranges{};
        int currentRangeIndex{};
        size_t currentOffset{};
        gpu::Buffer buffer{};
    };

    struct SamplerSlot {
        const gpu::Texture* texture{};
        bool exists{};  // Whether this sampler is declared in the shader
    };

protected:
    std::array<gpu::Program, VariantCount> mPrograms{};
    std::array<SamplerSlot, SAMPLER_COUNT> mTextures{};
    DynamicBuffer mDynamicBuffer{};
    gpu::Buffer mStaticBuffer{};
};

/* === Public Implementation === */

template <typename Derived>
void ShaderOverride<Derived>::getTextures(TextureArray& textures)
{
    for (int i = 0; i < SAMPLER_COUNT; i++) {
        textures[i] = mTextures[i].texture;
    }
}

template <typename Derived>
void ShaderOverride<Derived>::setTexture(int slot, const gpu::Texture* texture)
{
    if (slot < 0 || slot >= SAMPLER_COUNT) {
        NX_INTERNAL_LOG(E, "RENDER: Texture slot %d is out of range [0, %d)", slot, SAMPLER_COUNT);
        return;
    }

    if (!mTextures[slot].exists) {
        NX_INTERNAL_LOG(E, "RENDER: Texture slot %d is not defined in this shader", slot);
        return;
    }

    mTextures[slot].texture = texture;
}

template <typename Derived>
void ShaderOverride<Derived>::updateStaticBuffer(size_t offset, size_t size, const void* data)
{
    if (!mStaticBuffer.isValid()) {
        NX_INTERNAL_LOG(E, "RENDER: No static uniform buffer allocated for this shader");
        return;
    }

    if (offset + size > mStaticBuffer.size()) {
        NX_INTERNAL_LOG(E, 
            "RENDER: Upload out of bounds (offset=%zu + size=%zu > buffer=%zu)",
            offset, size, mStaticBuffer.size()
        );
        return;
    }

    mStaticBuffer.upload(offset, size, data);
}

template <typename Derived>
void ShaderOverride<Derived>::updateDynamicBuffer(size_t size, const void* data)
{
    if (!mDynamicBuffer.buffer.isValid()) {
        NX_INTERNAL_LOG(W, "RENDER: No dynamic uniform buffer allocated for this shader");
        return;
    }

    if (size % 16 != 0) {
        NX_INTERNAL_LOG(W, "RENDER: Dynamic buffer upload size (%zu) must be a multiple of 16 (std140 layout)", size);
        return;
    }

    size_t alignment = gpu::Pipeline::uniformBufferOffsetAlignment();
    size_t alignedOffset = NX_ALIGN_UP(mDynamicBuffer.currentOffset, alignment);

    size_t requiredSize = alignedOffset + size;
    size_t currentSize = mDynamicBuffer.buffer.size();
    size_t maxUBOSize = static_cast<size_t>(gpu::Pipeline::maxUniformBufferSize());

    // Grow buffer if needed
    if (requiredSize > currentSize) {
        size_t newSize = NX_ALIGN_UP(2 * currentSize, alignment);
        while (newSize < requiredSize) {
            newSize *= 2;
            newSize = NX_ALIGN_UP(newSize, alignment);
        }
        
        if (newSize > maxUBOSize) {
            NX_INTERNAL_LOG(E,
                "RENDER: Dynamic buffer resize failed (required=%zu > GPU limit=%zu)",
                newSize, maxUBOSize
            );
            return;
        }
        
        mDynamicBuffer.buffer.realloc(newSize, true);
    }

    // Record this range for binding
    mDynamicBuffer.currentRangeIndex = static_cast<int>(mDynamicBuffer.ranges.size());
    mDynamicBuffer.ranges.emplace_back(alignedOffset, size);

    mDynamicBuffer.buffer.upload(alignedOffset, size, data);
    mDynamicBuffer.currentOffset = alignedOffset + size;
}

template <typename Derived>
void ShaderOverride<Derived>::bindUniforms(const gpu::Pipeline& pipeline, int dynamicRangeIndex)
{
    if (mStaticBuffer.isValid()) {
        pipeline.bindUniform(
            UniformBinding[STATIC_UNIFORM],
            mStaticBuffer
        );
    }

    if (mDynamicBuffer.buffer.isValid() && dynamicRangeIndex >= 0) {
        const auto& range = mDynamicBuffer.ranges[dynamicRangeIndex];
        pipeline.bindUniform(
            UniformBinding[DYNAMIC_UNIFORM],
            mDynamicBuffer.buffer,
            range.offset,
            range.size
        );
    }
}

template <typename Derived>
void ShaderOverride<Derived>::bindTextures(const gpu::Pipeline& pipeline, const TextureArray& textures, const gpu::Texture& defaultTexture)
{
    for (int i = 0; i < SAMPLER_COUNT; i++) {
        if (mTextures[i].exists) {
            const gpu::Texture& tex = textures[i] ? *textures[i] : defaultTexture;
            pipeline.bindTexture(SamplerBinding[i], tex);
        }
    }
}

template <typename Derived>
void ShaderOverride<Derived>::clearDynamicBuffer()
{
    mDynamicBuffer.currentOffset = 0;
    mDynamicBuffer.ranges.clear();
}

template <typename Derived>
gpu::Program& ShaderOverride<Derived>::program(Variant variant)
{
    return mPrograms[static_cast<size_t>(variant)];
}

template <typename Derived>
int ShaderOverride<Derived>::dynamicRangeIndex() const
{
    return mDynamicBuffer.currentRangeIndex;
}

/* === Protected Implementation === */

template <typename Derived>
void ShaderOverride<Derived>::insertUserCode(util::String& source, const char* marker, const char* userCode)
{
    if (userCode == nullptr) return;
    
    if (size_t pos = source.find(marker); pos != std::string::npos) {
        source.replace(pos, SDL_strlen(marker), userCode);
    }
}

} // namespace render

#endif // NX_RENDER_SHADER_OVERRIDE_HPP
