#include "./NX_MaterialShader.hpp"

#include <shaders/scene.vert.h>
#include <shaders/scene_lit.frag.h>
#include <shaders/scene_unlit.frag.h>
#include <shaders/scene_wireframe.geom.h>
#include <shaders/scene_prepass.frag.h>
#include <shaders/scene_shadow.frag.h>

/* === Public Implementation === */

NX_MaterialShader::NX_MaterialShader()
{
    /* --- Compile shaders --- */

    gpu::Shader vertSceneShader(GL_VERTEX_SHADER, SCENE_VERT);
    gpu::Shader vertShadowShader(GL_VERTEX_SHADER, SCENE_VERT, {"SHADOW"});
    gpu::Shader geomWireframe(GL_GEOMETRY_SHADER, SCENE_WIREFRAME_GEOM);
    gpu::Shader fragLitShader(GL_FRAGMENT_SHADER, SCENE_LIT_FRAG);
    gpu::Shader fragUnlitShader(GL_FRAGMENT_SHADER, SCENE_UNLIT_FRAG);
    gpu::Shader fragPrepass(GL_FRAGMENT_SHADER, SCENE_PREPASS_FRAG);
    gpu::Shader fragShadow(GL_FRAGMENT_SHADER, SCENE_SHADOW_FRAG);

    /* --- Link all programs --- */

    mPrograms[SCENE_LIT]       = gpu::Program(vertSceneShader, fragLitShader);
    mPrograms[SCENE_UNLIT]     = gpu::Program(vertSceneShader, fragUnlitShader);
    mPrograms[SCENE_WIREFRAME] = gpu::Program(vertSceneShader, geomWireframe, fragUnlitShader);
    mPrograms[SCENE_PREPASS]   = gpu::Program(vertSceneShader, fragPrepass);
    mPrograms[SCENE_SHADOW]    = gpu::Program(vertShadowShader, fragShadow);
}

NX_MaterialShader::NX_MaterialShader(const char* vert, const char* frag)
{
    /* --- Constants --- */

    constexpr const char* vertMarker = "#define vertex()";
    constexpr const char* fragMarker = "#define fragment()";

    /* --- Prepare base sources --- */

    util::String vertScene = SCENE_VERT;
    util::String fragLit   = SCENE_LIT_FRAG;
    util::String fragUnlit = SCENE_UNLIT_FRAG;

    /* --- Insert user code --- */

    insertUserCode(vertScene, vertMarker, vert);
    insertUserCode(fragLit,   fragMarker, frag);
    insertUserCode(fragUnlit, fragMarker, frag);

    /* --- Compile shaders --- */

    gpu::Shader vertSceneShader(GL_VERTEX_SHADER, vertScene.data());
    gpu::Shader vertShadowShader(GL_VERTEX_SHADER, vertScene.data(), {"SHADOW"});
    gpu::Shader geomWireframe(GL_GEOMETRY_SHADER, SCENE_WIREFRAME_GEOM);
    gpu::Shader fragLitShader(GL_FRAGMENT_SHADER, fragLit.data());
    gpu::Shader fragUnlitShader(GL_FRAGMENT_SHADER, fragUnlit.data());
    gpu::Shader fragPrepass(GL_FRAGMENT_SHADER, SCENE_PREPASS_FRAG);
    gpu::Shader fragShadow(GL_FRAGMENT_SHADER, SCENE_SHADOW_FRAG);

    /* --- Link all programs --- */

    mPrograms[SCENE_LIT]       = gpu::Program(vertSceneShader, fragLitShader);
    mPrograms[SCENE_UNLIT]     = gpu::Program(vertSceneShader, fragUnlitShader);
    mPrograms[SCENE_WIREFRAME] = gpu::Program(vertSceneShader, geomWireframe, fragUnlitShader);
    mPrograms[SCENE_PREPASS]   = gpu::Program(vertSceneShader, fragPrepass);
    mPrograms[SCENE_SHADOW]    = gpu::Program(vertShadowShader, fragShadow);

    /* --- Collect uniform block sizes and setup bindings --- */

    size_t bufferSize[UNIFORM_COUNT] = {};
    for (int i = 0; i < SHADER_COUNT; ++i) {
        auto& program = mPrograms[i];
        for (int j = 0; j < UNIFORM_COUNT; ++j) {
            int blockIndex = program.getUniformBlockIndex(UniformName[j]);
            if (blockIndex < 0) continue;
            program.setUniformBlockBinding(blockIndex, UniformBinding[j]);
            if (bufferSize[j] == 0) {
                bufferSize[j] = program.getUniformBlockSize(blockIndex);
            }
        }
    }

    /* --- Allocate uniform buffers --- */

    if (bufferSize[STATIC_UNIFORM] > 0) {
        mStaticBuffer = gpu::Buffer(GL_UNIFORM_BUFFER, bufferSize[STATIC_UNIFORM], nullptr, GL_DYNAMIC_DRAW);
    }

    if (bufferSize[DYNAMIC_UNIFORM] > 0) {
        int alignment = gpu::Pipeline::uniformBufferOffsetAlignment();
        int alignedSize = NX_ALIGN_UP(8 * bufferSize[DYNAMIC_UNIFORM], alignment);
        mDynamicBuffer.buffer = gpu::Buffer(GL_UNIFORM_BUFFER, alignedSize, nullptr, GL_DYNAMIC_DRAW);
        if (!mDynamicBuffer.ranges.reserve(8)) {
            NX_INTERNAL_LOG(E, "RENDER: Failed to reserve space for dynamic uniform buffer range infos");
        }
    }

    /* --- Setup texture samplers --- */

    gpu::Pipeline([this](const gpu::Pipeline& pipeline) {
        for (int i = 0; i < SHADER_COUNT; ++i) {
            pipeline.useProgram(mPrograms[i]);
            for (int j = 0; j < SAMPLER_COUNT; ++j) {
                int loc = mPrograms[i].getUniformLocation(SamplerName[j]);
                if (loc < 0) continue;
                pipeline.setUniformInt1(loc, SamplerBinding[j]);
                mTextures[j].exists = true;
            }
        }
    });
}

void NX_MaterialShader::updateStaticBuffer(size_t offset, size_t size, const void* data)
{
    if (!mStaticBuffer.isValid()) {
        NX_INTERNAL_LOG(E, 
            "RENDER: Failed to upload data to the static uniform buffer of material shader;"
            "No static buffer was declared for this material shader"
        );
    }

    if (offset + size > mStaticBuffer.size()) {
        NX_INTERNAL_LOG(E, 
            "RENDER: Failed to upload data to the static uniform buffer of material shader;"
            "offset + size (%zu) exceeds size of static buffer (%zu)",
            offset + size, mStaticBuffer.size()
        );
        return;
    }

    mStaticBuffer.upload(offset, size, data);
}

void NX_MaterialShader::updateDynamicBuffer(size_t size, const void* data)
{
    if (!mDynamicBuffer.buffer.isValid()) {
        NX_INTERNAL_LOG(W, 
            "RENDER: Failed to upload data to the dynamic uniform buffer of material shader; "
            "No dynamic buffer was declared for this material shader"
        );
        return;
    }

    if (size % 16 != 0) /* std140 requirement */ {
        NX_INTERNAL_LOG(W, 
            "RENDER: Failed to upload data to the dynamic uniform buffer of material shader; "
            "The size of the data sent must be a multiple of 16"
        );
        return;
    }

    size_t alignment = gpu::Pipeline::uniformBufferOffsetAlignment();
    size_t alignedOffset = NX_ALIGN_UP(mDynamicBuffer.currentOffset, alignment);

    size_t requiredSize = alignedOffset + size;
    size_t currentSize = mDynamicBuffer.buffer.size();
    size_t maxUBOSize = static_cast<size_t>(gpu::Pipeline::maxUniformBufferSize());

    if (requiredSize > currentSize) {
        size_t newSize = NX_ALIGN_UP(2 * currentSize, alignment);
        while (newSize < requiredSize) {
            newSize *= 2;
            newSize = NX_ALIGN_UP(newSize, alignment);
        }
        if (newSize > maxUBOSize) {
            NX_INTERNAL_LOG(E,
                "RENDER: Failed to upload data to the dynamic uniform buffer of material shader; "
                "The required buffer size ({} bytes) exceeds the GPU limit for uniform buffers ({} bytes).",
                newSize, maxUBOSize
            );
            return;
        }
        mDynamicBuffer.buffer.realloc(newSize, true);
    }

    mDynamicBuffer.currentRangeIndex = static_cast<int>(mDynamicBuffer.ranges.size());
    mDynamicBuffer.ranges.emplace_back(alignedOffset, size);

    mDynamicBuffer.buffer.upload(alignedOffset, size, data);
    mDynamicBuffer.currentOffset = alignedOffset + size;
}

void NX_MaterialShader::bindUniformBuffers(const gpu::Pipeline& pipeline, int dynamicRangeIndex)
{
    if (mStaticBuffer.isValid()) {
        pipeline.bindUniform(
            UniformBinding[STATIC_UNIFORM],
            mStaticBuffer
        );
    }

    if (mDynamicBuffer.buffer.isValid() && dynamicRangeIndex >= 0) {
        pipeline.bindUniform(
            UniformBinding[DYNAMIC_UNIFORM],
            mDynamicBuffer.buffer,
            mDynamicBuffer.ranges[dynamicRangeIndex].offset,
            mDynamicBuffer.ranges[dynamicRangeIndex].size
        );
    }
}

void NX_MaterialShader::bindTextures(const gpu::Pipeline& pipeline, const TextureArray& textures, const gpu::Texture& defaultTexture)
{
    for (int i = 0; i < SAMPLER_COUNT; i++) {
        if (mTextures[i].exists) {
            pipeline.bindTexture(SamplerBinding[i], textures[i] ? *textures[i] : defaultTexture);
        }
    }
}
