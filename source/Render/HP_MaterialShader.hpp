#ifndef HP_RENDER_MATERIAL_SHADER_HPP
#define HP_RENDER_MATERIAL_SHADER_HPP

#include "../Detail/Util/DynamicArray.hpp"
#include "../Detail/GPU/Pipeline.hpp"
#include "../Detail/GPU/Program.hpp"
#include "../Detail/GPU/Buffer.hpp"

#include <string_view>
#include <array>

/* === Declaration === */

class HP_MaterialShader {
public:
    /** Helper enums */
    enum Shader { FORWARD, PREPASS, SHADOW, SHADER_COUNT };
    enum Uniform { STATIC_UNIFORM, DYNAMIC_UNIFORM, UNIFORM_COUNT };

public:
    HP_MaterialShader();
    HP_MaterialShader(const char* vertex, const char* fragment);

    /** Uniform buffer uploading functions */
    void updateStaticBuffer(size_t offset, size_t size, const void* data);
    void updateDynamicBuffer(size_t size, const void* data);

    /** Buffer bindings */
    void bindUniformBuffers(const gpu::Pipeline& pipeline, Shader shader, int dynamicRangeIndex);

    /** Dynamic buffer management */
    void clearDynamicBuffer();

    /** Getters */
    gpu::Program& program(Shader shader);
    int dynamicRangeIndex() const;

private:
    /** Used at construction to generate final shader code */
    void processCode(std::string& source, const std::string_view& define, const char* code);

private:
    /** Built-in uniform block names */
    static constexpr const char* UniformNames[UNIFORM_COUNT] = {
        "StaticBuffer", "DynamicBuffer"
    };

    /** Built-in uniform block binding points */
    static constexpr int UniformBinding[UNIFORM_COUNT] {
        10, 11
    };

private:
    struct DynamicBuffer {
        struct Range { size_t offset, size; };
        util::DynamicArray<Range> ranges{};
        int currentRangeIndex{};
        size_t currentOffset{};
        gpu::Buffer buffer{};
    };

private:
    std::array<gpu::Program, SHADER_COUNT> mPrograms{};
    DynamicBuffer mDynamicBuffer{};
    gpu::Buffer mStaticBuffer{};
};

/* === Public Implementation === */

inline void HP_MaterialShader::clearDynamicBuffer()
{
    mDynamicBuffer.currentOffset = 0;
    mDynamicBuffer.ranges.clear();
}

inline gpu::Program& HP_MaterialShader::program(Shader shader)
{
    return mPrograms[shader];
}

inline int HP_MaterialShader::dynamicRangeIndex() const
{
    return mDynamicBuffer.currentRangeIndex;
}

#endif // HP_RENDER_MATERIAL_SHADER_HPP
