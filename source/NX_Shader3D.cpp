/* NX_Shader3D.cpp -- API definition for Nexium's 3D shader module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "./NX_Shader3D.hpp"

#include <NX/NX_Filesystem.h>

#include "./INX_AssetDecoder.hpp"
#include "./INX_GlobalPool.hpp"

#include <shaders/scene.vert.h>
#include <shaders/scene_lit.frag.h>
#include <shaders/scene_unlit.frag.h>
#include <shaders/scene_prepass.frag.h>
#include <shaders/scene_shadow.frag.h>

// ============================================================================
// OPAQUE DEFINITION
// ============================================================================

NX_Shader3D::NX_Shader3D()
{
    /* --- Compile shaders --- */

    INX_ShaderDecoder vertSceneCode(SCENE_VERT, SCENE_VERT_SIZE);
    INX_ShaderDecoder fragLitCode(SCENE_LIT_FRAG, SCENE_LIT_FRAG_SIZE);
    INX_ShaderDecoder fragUnlitCode(SCENE_UNLIT_FRAG, SCENE_UNLIT_FRAG_SIZE);
    INX_ShaderDecoder fragPrepassCode(SCENE_PREPASS_FRAG, SCENE_PREPASS_FRAG_SIZE);
    INX_ShaderDecoder fragShadowCode(SCENE_SHADOW_FRAG, SCENE_SHADOW_FRAG_SIZE);

    gpu::Shader vertScene(GL_VERTEX_SHADER, vertSceneCode);
    gpu::Shader vertShadow(GL_VERTEX_SHADER, vertSceneCode, {"SHADOW"});
    gpu::Shader fragLit(GL_FRAGMENT_SHADER, fragLitCode);
    gpu::Shader fragUnlit(GL_FRAGMENT_SHADER, fragUnlitCode);
    gpu::Shader fragPrepass(GL_FRAGMENT_SHADER, fragPrepassCode);
    gpu::Shader fragShadow(GL_FRAGMENT_SHADER, fragShadowCode);

    /* --- Link all programs --- */

    mPrograms[Variant::SCENE_LIT]       = gpu::Program(vertScene, fragLit);
    mPrograms[Variant::SCENE_UNLIT]     = gpu::Program(vertScene, fragUnlit);
    mPrograms[Variant::SCENE_PREPASS]   = gpu::Program(vertScene, fragPrepass);
    mPrograms[Variant::SCENE_SHADOW]    = gpu::Program(vertShadow, fragShadow);
}

NX_Shader3D::NX_Shader3D(const char* vert, const char* frag)
{
    /* --- Constants --- */

    constexpr const char* vertMarker = "#define vertex()";
    constexpr const char* fragMarker = "#define fragment()";

    /* --- Prepare base sources --- */

    util::String vertSceneCode = INX_ShaderDecoder(SCENE_VERT, SCENE_VERT_SIZE).GetCode();
    util::String fragLitCode   = INX_ShaderDecoder(SCENE_LIT_FRAG, SCENE_LIT_FRAG_SIZE).GetCode();
    util::String fragUnlitCode = INX_ShaderDecoder(SCENE_UNLIT_FRAG, SCENE_UNLIT_FRAG_SIZE).GetCode();

    /* --- Process and insert the user code --- */

    if (vert != nullptr) {
        util::String vertUser = ProcessUserCode(vert);
        InsertUserCode(vertSceneCode, vertMarker, vertUser.GetCString());
    }

    if (frag != nullptr) {
        util::String fragUser = ProcessUserCode(frag);
        InsertUserCode(fragLitCode,   fragMarker, fragUser.GetCString());
        InsertUserCode(fragUnlitCode, fragMarker, fragUser.GetCString());
    }

    /* --- Compile shaders --- */

    gpu::Shader vertScene(GL_VERTEX_SHADER, vertSceneCode.GetCString());
    gpu::Shader vertShadow(GL_VERTEX_SHADER, vertSceneCode.GetCString(), {"SHADOW"});
    gpu::Shader fragLit(GL_FRAGMENT_SHADER, fragLitCode.GetCString());
    gpu::Shader fragUnlit(GL_FRAGMENT_SHADER, fragUnlitCode.GetCString());
    gpu::Shader fragPrepass(GL_FRAGMENT_SHADER, INX_ShaderDecoder(SCENE_PREPASS_FRAG, SCENE_PREPASS_FRAG_SIZE));
    gpu::Shader fragShadow(GL_FRAGMENT_SHADER, INX_ShaderDecoder(SCENE_SHADOW_FRAG, SCENE_SHADOW_FRAG_SIZE));

    /* --- Link all programs --- */

    mPrograms[Variant::SCENE_LIT]       = gpu::Program(vertScene, fragLit);
    mPrograms[Variant::SCENE_UNLIT]     = gpu::Program(vertScene, fragUnlit);
    mPrograms[Variant::SCENE_PREPASS]   = gpu::Program(vertScene, fragPrepass);
    mPrograms[Variant::SCENE_SHADOW]    = gpu::Program(vertShadow, fragShadow);

    /* --- Collect uniform block sizes and setup bindings --- */

    size_t bufferSize[UNIFORM_COUNT] = {};
    for (int i = 0; i < VariantCount; ++i) {
        auto& program = mPrograms[i];
        for (int j = 0; j < UNIFORM_COUNT; ++j) {
            int blockIndex = program.GetUniformBlockIndex(UniformName[j]);
            if (blockIndex < 0) continue;
            program.SetUniformBlockBinding(blockIndex, UniformBinding[j]);
            if (bufferSize[j] == 0) {
                bufferSize[j] = program.GetUniformBlockSize(blockIndex);
            }
        }
    }

    /* --- Allocate uniform buffers --- */

    if (bufferSize[STATIC_UNIFORM] > 0) {
        mStaticBuffer = gpu::Buffer(GL_UNIFORM_BUFFER, bufferSize[STATIC_UNIFORM], nullptr, GL_DYNAMIC_DRAW);
    }

    if (bufferSize[DYNAMIC_UNIFORM] > 0) {
        int alignment = gpu::Pipeline::GetUniformBufferOffsetAlignment();
        int alignedSize = NX_ALIGN_UP(8 * bufferSize[DYNAMIC_UNIFORM], alignment);
        mDynamicBuffer.buffer = gpu::Buffer(GL_UNIFORM_BUFFER, alignedSize, nullptr, GL_DYNAMIC_DRAW);
        if (!mDynamicBuffer.ranges.Reserve(8)) {
            NX_LOG(E, "RENDER: Dynamic uniform buffer range info reservation failed (requested: 8 entries)");
        }
    }
}

// ============================================================================
// PUBLIC API
// ============================================================================

NX_Shader3D* NX_CreateShader3D(const char* vertCode, const char* fragCode)
{
    return INX_Pool.Create<NX_Shader3D>(vertCode, fragCode);
}

NX_Shader3D* NX_LoadShader3D(const char* vertFile, const char* fragFile)
{
    char* vertCode = vertFile ? NX_LoadFileText(vertFile) : nullptr;
    char* fragCode = fragFile ? NX_LoadFileText(fragFile) : nullptr;

    NX_Shader3D* shader = INX_Pool.Create<NX_Shader3D>(vertCode, fragCode);

    NX_Free(vertCode);
    NX_Free(fragCode);

    return shader;
}

void NX_DestroyShader3D(NX_Shader3D* shader)
{
    INX_Pool.Destroy(shader);
}

void NX_SetShader3DTexture(NX_Shader3D* shader, int slot, const NX_Texture* texture)
{
    shader->SetTexture(slot, texture);
}

void NX_UpdateStaticShader3DBuffer(NX_Shader3D* shader, size_t offset, size_t size, const void* data)
{
    shader->UpdateStaticBuffer(offset, size, data);
}

void NX_UpdateDynamicShader3DBuffer(NX_Shader3D* shader, size_t size, const void* data)
{
    shader->UpdateDynamicBuffer(size, data);
}
