#include "./NX_MaterialShader.hpp"

#include <shaders/scene.vert.h>
#include <shaders/scene_lit.frag.h>
#include <shaders/scene_unlit.frag.h>
#include <shaders/scene_wireframe.geom.h>
#include <shaders/scene_prepass.vert.h>
#include <shaders/scene_prepass.frag.h>
#include <shaders/scene_shadow.vert.h>
#include <shaders/scene_shadow.frag.h>

#include <string_view>
#include <string>

/* === Public Implementation === */

NX_MaterialShader::NX_MaterialShader()
{
    mPrograms[SCENE_LIT] = gpu::Program(
        gpu::Shader(GL_VERTEX_SHADER, SCENE_VERT),
        gpu::Shader(GL_FRAGMENT_SHADER, SCENE_LIT_FRAG)
    );

    mPrograms[SCENE_UNLIT] = gpu::Program(
        gpu::Shader(GL_VERTEX_SHADER, SCENE_VERT),
        gpu::Shader(GL_FRAGMENT_SHADER, SCENE_UNLIT_FRAG)
    );

    mPrograms[SCENE_WIREFRAME] = gpu::Program(
        gpu::Shader(GL_VERTEX_SHADER, SCENE_VERT),
        gpu::Shader(GL_GEOMETRY_SHADER, SCENE_WIREFRAME_GEOM),
        gpu::Shader(GL_FRAGMENT_SHADER, SCENE_UNLIT_FRAG, {"WIREFRAME"})
    );

    mPrograms[SCENE_PREPASS] = gpu::Program(
        gpu::Shader(GL_VERTEX_SHADER, SCENE_PREPASS_VERT),
        gpu::Shader(GL_FRAGMENT_SHADER, SCENE_PREPASS_FRAG)
    );

    mPrograms[SCENE_SHADOW] = gpu::Program(
        gpu::Shader(GL_VERTEX_SHADER, SCENE_SHADOW_VERT),
        gpu::Shader(GL_FRAGMENT_SHADER, SCENE_SHADOW_FRAG)
    );
}

NX_MaterialShader::NX_MaterialShader(const char* vert, const char* frag)
{
    /* --- Constants --- */

    constexpr std::string_view vertDefine = "#define vertex()";
    constexpr std::string_view fragDefine = "#define fragment()";

    /* --- Load all base shaders --- */

    std::string vertScene = SCENE_VERT;
    std::string fragSceneLit = SCENE_LIT_FRAG;
    std::string fragSceneUnlit = SCENE_UNLIT_FRAG;
    std::string vertPrepass = SCENE_PREPASS_VERT;
    std::string fragPrepass = SCENE_PREPASS_FRAG;
    std::string vertShadow = SCENE_SHADOW_VERT;
    std::string fragShadow = SCENE_SHADOW_FRAG;

    /* --- Process shaders --- */

    processCode(vertScene, vertDefine, vert);
    processCode(fragSceneLit, fragDefine, frag);
    processCode(fragSceneUnlit, fragDefine, frag);
    processCode(vertPrepass, vertDefine, vert);
    processCode(fragPrepass, fragDefine, frag);
    processCode(vertShadow, vertDefine, vert);
    processCode(fragShadow, fragDefine, frag);

    /* --- Compile shaders --- */

    mPrograms[SCENE_LIT] = gpu::Program(
        gpu::Shader(GL_VERTEX_SHADER, vertScene.c_str()),
        gpu::Shader(GL_FRAGMENT_SHADER, fragSceneLit.c_str())
    );

    mPrograms[SCENE_UNLIT] = gpu::Program(
        gpu::Shader(GL_VERTEX_SHADER, vertScene.c_str()),
        gpu::Shader(GL_FRAGMENT_SHADER, fragSceneUnlit.c_str())
    );

    mPrograms[SCENE_WIREFRAME] = gpu::Program(
        gpu::Shader(GL_VERTEX_SHADER, vertScene.c_str()),
        gpu::Shader(GL_GEOMETRY_SHADER, SCENE_WIREFRAME_GEOM),
        gpu::Shader(GL_FRAGMENT_SHADER, fragSceneUnlit.c_str())
    );

    mPrograms[SCENE_PREPASS] = gpu::Program(
        gpu::Shader(GL_VERTEX_SHADER, vertPrepass.c_str()),
        gpu::Shader(GL_FRAGMENT_SHADER, fragPrepass.c_str())
    );

    mPrograms[SCENE_SHADOW] = gpu::Program(
        gpu::Shader(GL_VERTEX_SHADER, vertShadow.c_str()),
        gpu::Shader(GL_FRAGMENT_SHADER, fragShadow.c_str())
    );

    /* --- Collect uniform blocks --- */

    size_t bufferSize[UNIFORM_COUNT]{};
    for (int i = 0; i < SHADER_COUNT; i++) {
        for (int j = 0; j < UNIFORM_COUNT; j++) {
            int blockIndex = mPrograms[i].getUniformBlockIndex(UniformName[j]);
            if (blockIndex >= 0) {
                mPrograms[i].setUniformBlockBinding(blockIndex, UniformBinding[j]);
                if (bufferSize[j] == 0) {
                    bufferSize[j] = mPrograms[i].getUniformBlockSize(blockIndex);
                }
            }
        }
    }

    /* --- Allocate static uniform buffer if needed --- */

    if (bufferSize[STATIC_UNIFORM] > 0) {
        mStaticBuffer = gpu::Buffer(GL_UNIFORM_BUFFER, bufferSize[STATIC_UNIFORM], nullptr, GL_DYNAMIC_DRAW);
    }

    /* --- Allocate dynamic uniform buffer if needed --- */

    if (bufferSize[DYNAMIC_UNIFORM] > 0) {
        int alignment = gpu::Pipeline::uniformBufferOffsetAlignment();
        int alignedSize = NX_ALIGN_UP(8 * bufferSize[DYNAMIC_UNIFORM], alignment);
        mDynamicBuffer.buffer = gpu::Buffer(GL_UNIFORM_BUFFER, alignedSize, nullptr, GL_DYNAMIC_DRAW);
        if (!mDynamicBuffer.ranges.reserve(8)) {
            NX_INTERNAL_LOG(E, "RENDER: Failed to reserve space for dynamic uniform buffer range infos");
        }
    }

    /* --- Collect and setup all samplers --- */

    gpu::Pipeline([this](const gpu::Pipeline& pipeline) { // NOLINT
        for (int i = 0; i < SHADER_COUNT; i++) {
            pipeline.useProgram(mPrograms[i]);
            for (int j = 0; j < TEXTURE_COUNT; j++) {
                int loc = mPrograms[i].getUniformLocation(SamplerName[j]);
                if (loc >= 0) {
                    pipeline.setUniformInt1(loc, SamplerBinding[j]);
                    mTextures[j].exists = true;
                }
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

void NX_MaterialShader::bindUniformBuffers(const gpu::Pipeline& pipeline, Shader shader, int dynamicRangeIndex)
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
    for (int i = 0; i < TEXTURE_COUNT; i++) {
        if (mTextures[i].exists) {
            pipeline.bindTexture(SamplerBinding[i], textures[i] ? *textures[i] : defaultTexture);
        }
    }
}

/* === Private Implementation === */

void NX_MaterialShader::processCode(std::string& source, const std::string_view& define, const char* code)
{
    /* --- If no code provided keep default shader --- */

    if (code == nullptr) {
        return;
    }

    /* --- Insert use code to the shader --- */

    size_t pos = source.find(define);

    if (pos != std::string::npos) {
        source.replace(pos, define.length(), code);
    }
}
