/* NX_Shader2D.cpp -- API definition for Nexium's 2D shader module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "./NX_Shader2D.hpp"

#include <NX/NX_Filesystem.h>

#include "./INX_AssetDecoder.hpp"
#include "./INX_PoolAssets.hpp"

#include <shaders/shape.vert.h>
#include <shaders/shape.frag.h>

// ============================================================================
// OPAQUE DEFINITION
// ============================================================================

NX_Shader2D::NX_Shader2D()
{
    /* --- Compile shaders --- */

    INX_ShaderDecoder vertCode(SHAPE_VERT, SHAPE_VERT_SIZE);
    INX_ShaderDecoder fragCode(SHAPE_FRAG, SHAPE_FRAG_SIZE);

    gpu::Shader vertShape(GL_VERTEX_SHADER, vertCode);
    gpu::Shader fragShapeColor(GL_FRAGMENT_SHADER, fragCode, {"SHAPE_COLOR"});
    gpu::Shader fragShapeTexture(GL_FRAGMENT_SHADER, fragCode, {"SHAPE_TEXTURE"});
    gpu::Shader fragTextBitmap(GL_FRAGMENT_SHADER, fragCode, {"TEXT_BITMAP"});
    gpu::Shader fragTextSDF(GL_FRAGMENT_SHADER, fragCode, {"TEXT_SDF"});

    /* --- Link all programs --- */

    mPrograms[Variant::SHAPE_COLOR]   = gpu::Program(vertShape, fragShapeColor);
    mPrograms[Variant::SHAPE_TEXTURE] = gpu::Program(vertShape, fragShapeTexture);
    mPrograms[Variant::TEXT_BITMAP]   = gpu::Program(vertShape, fragTextBitmap);
    mPrograms[Variant::TEXT_SDF]      = gpu::Program(vertShape, fragTextSDF);
}

NX_Shader2D::NX_Shader2D(const char* vert, const char* frag)
{
    /* --- Constants --- */

    constexpr const char* vertMarker = "#define vertex()";
    constexpr const char* fragMarker = "#define fragment()";

    /* --- Prepare base sources --- */

    util::String vertCode = INX_ShaderDecoder(SHAPE_VERT, SHAPE_VERT_SIZE).code();
    util::String fragCode = INX_ShaderDecoder(SHAPE_FRAG, SHAPE_FRAG_SIZE).code();

    /* --- Process and insert the user code --- */

    if (vert != nullptr) {
        util::String vertUser = ProcessUserCode(vert);
        InsertUserCode(vertCode, vertMarker, vertUser.data());
    }

    if (frag != nullptr) {
        util::String fragUser = ProcessUserCode(frag);
        InsertUserCode(fragCode, fragMarker, fragUser.data());
    }

    /* --- Compile shaders --- */

    gpu::Shader vertShape(GL_VERTEX_SHADER, vertCode.data());
    gpu::Shader fragShapeColor(GL_FRAGMENT_SHADER, fragCode.data(), {"SHAPE_COLOR"});
    gpu::Shader fragShapeTexture(GL_FRAGMENT_SHADER, fragCode.data(), {"SHAPE_TEXTURE"});
    gpu::Shader fragTextBitmap(GL_FRAGMENT_SHADER, fragCode.data(), {"TEXT_BITMAP"});
    gpu::Shader fragTextSDF(GL_FRAGMENT_SHADER, fragCode.data(), {"TEXT_SDF"});

    /* --- Link all programs --- */

    mPrograms[Variant::SHAPE_COLOR]   = gpu::Program(vertShape, fragShapeColor);
    mPrograms[Variant::SHAPE_TEXTURE] = gpu::Program(vertShape, fragShapeTexture);
    mPrograms[Variant::TEXT_BITMAP]   = gpu::Program(vertShape, fragTextBitmap);
    mPrograms[Variant::TEXT_SDF]      = gpu::Program(vertShape, fragTextSDF);

    /* --- Collect uniform block sizes and setup bindings --- */

    size_t bufferSize[UNIFORM_COUNT] = {};
    for (int i = 0; i < VariantCount; ++i) {
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
            NX_LOG(E, "RENDER: Dynamic uniform buffer range info reservation failed (requested: 8 entries)");
        }
    }
}

// ============================================================================
// PUBLIC API
// ============================================================================

NX_Shader2D* NX_CreateShader2D(const char* vertCode, const char* fragCode)
{
    return INX_Pool.Create<NX_Shader2D>(vertCode, fragCode);
}

NX_Shader2D* NX_LoadShader2D(const char* vertFile, const char* fragFile)
{
    char* vertCode = vertFile ? NX_LoadFileText(vertFile) : nullptr;
    char* fragCode = fragFile ? NX_LoadFileText(fragFile) : nullptr;

    NX_Shader2D* shader = INX_Pool.Create<NX_Shader2D>(vertCode, fragCode);

    SDL_free(vertCode);
    SDL_free(fragCode);

    return shader;
}

void NX_DestroyShader2D(NX_Shader2D* shader)
{
    INX_Pool.Destroy(shader);
}

void NX_SetShader2DTexture(NX_Shader2D* shader, int slot, const NX_Texture* texture)
{
    shader->SetTexture(slot, texture);
}

void NX_UpdateStaticShader2DBuffer(NX_Shader2D* shader, size_t offset, size_t size, const void* data)
{
    shader->UpdateStaticBuffer(offset, size, data);
}

void NX_UpdateDynamicShader2DBuffer(NX_Shader2D* shader, size_t size, const void* data)
{
    shader->UpdateDynamicBuffer(size, data);
}
