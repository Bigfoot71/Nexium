#include "./NX_Shader.hpp"

#include <shaders/shape.vert.h>
#include <shaders/shape.frag.h>

/* === Public Implementation === */

NX_Shader::NX_Shader()
{
    /* --- Compile shaders --- */

    gpu::Shader vertShape(GL_VERTEX_SHADER, SHAPE_VERT);
    gpu::Shader fragShapeColor(GL_FRAGMENT_SHADER, SHAPE_FRAG, {"SHAPE_COLOR"});
    gpu::Shader fragShapeTexture(GL_FRAGMENT_SHADER, SHAPE_FRAG, {"SHAPE_TEXTURE"});
    gpu::Shader fragTextBitmap(GL_FRAGMENT_SHADER, SHAPE_FRAG, {"TEXT_BITMAP"});
    gpu::Shader fragTextSDF(GL_FRAGMENT_SHADER, SHAPE_FRAG, {"TEXT_SDF"});

    /* --- Link all programs --- */

    mPrograms[SHAPE_COLOR]   = gpu::Program(vertShape, fragShapeColor);
    mPrograms[SHAPE_TEXTURE] = gpu::Program(vertShape, fragShapeTexture);
    mPrograms[TEXT_BITMAP]   = gpu::Program(vertShape, fragTextBitmap);
    mPrograms[TEXT_SDF]      = gpu::Program(vertShape, fragTextSDF);
}

NX_Shader::NX_Shader(const char* vert, const char* frag)
{
    /* --- Constants --- */

    constexpr const char* vertMarker = "#define vertex()";
    constexpr const char* fragMarker = "#define fragment()";

    /* --- Prepare base sources --- */

    util::String vertStr = SHAPE_VERT;
    util::String fragStr = SHAPE_FRAG;

    /* --- Insert user code --- */

    insertUserCode(vertStr, vertMarker, vert);
    insertUserCode(fragStr, fragMarker, frag);

    /* --- Compile shaders --- */

    gpu::Shader vertShape(GL_VERTEX_SHADER, vertStr.data());
    gpu::Shader fragShapeColor(GL_FRAGMENT_SHADER, fragStr.data(), {"SHAPE_COLOR"});
    gpu::Shader fragShapeTexture(GL_FRAGMENT_SHADER, fragStr.data(), {"SHAPE_TEXTURE"});
    gpu::Shader fragTextBitmap(GL_FRAGMENT_SHADER, fragStr.data(), {"TEXT_BITMAP"});
    gpu::Shader fragTextSDF(GL_FRAGMENT_SHADER, fragStr.data(), {"TEXT_SDF"});

    /* --- Link all programs --- */

    mPrograms[SHAPE_COLOR]   = gpu::Program(vertShape, fragShapeColor);
    mPrograms[SHAPE_TEXTURE] = gpu::Program(vertShape, fragShapeTexture);
    mPrograms[TEXT_BITMAP]   = gpu::Program(vertShape, fragTextBitmap);
    mPrograms[TEXT_SDF]      = gpu::Program(vertShape, fragTextSDF);

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

void NX_Shader::updateStaticBuffer(size_t offset, size_t size, const void* data)
{
    if (!mStaticBuffer.isValid()) {
        NX_INTERNAL_LOG(E, "RENDER: Cannot upload; no static buffer for this shader.");
        return;
    }

    if (offset + size > mStaticBuffer.size()) {
        NX_INTERNAL_LOG(E, 
            "RENDER: Upload out of bounds (%zu > buffer size %zu).",
            offset + size, mStaticBuffer.size()
        );
        return;
    }

    mStaticBuffer.upload(offset, size, data);
}

void NX_Shader::updateDynamicBuffer(size_t size, const void* data)
{
    if (!mDynamicBuffer.buffer.isValid()) {
        NX_INTERNAL_LOG(W, "RENDER: Cannot upload; no dynamic buffer for this shader.");
        return;
    }

    if (size % 16 != 0) { /* std140 requirement */
        NX_INTERNAL_LOG(W, "RENDER: Upload size must be a multiple of 16 (std140 layout).");
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
                "RENDER: Upload failed; required size (%zu bytes) exceeds GPU uniform buffer limit (%zu bytes).",
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

void NX_Shader::bindUniformBuffers(const gpu::Pipeline& pipeline, int dynamicRangeIndex)
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

void NX_Shader::bindTextures(const gpu::Pipeline& pipeline, const TextureArray& textures, const gpu::Texture& defaultTexture)
{
    for (int i = 0; i < SAMPLER_COUNT; i++) {
        if (mTextures[i].exists) {
            pipeline.bindTexture(SamplerBinding[i], textures[i] ? *textures[i] : defaultTexture);
        }
    }
}
