#include "./NX_Shader3D.hpp"

#include "../Assets/ShaderDecoder.hpp"

#include <shaders/scene.vert.h>
#include <shaders/scene_lit.frag.h>
#include <shaders/scene_unlit.frag.h>
#include <shaders/scene_prepass.frag.h>
#include <shaders/scene_shadow.frag.h>

/* === Public Implementation === */

NX_Shader3D::NX_Shader3D()
{
    /* --- Compile shaders --- */

    assets::ShaderDecoder vertSceneCode(SCENE_VERT, SCENE_VERT_SIZE);
    assets::ShaderDecoder fragLitCode(SCENE_LIT_FRAG, SCENE_LIT_FRAG_SIZE);
    assets::ShaderDecoder fragUnlitCode(SCENE_UNLIT_FRAG, SCENE_UNLIT_FRAG_SIZE);
    assets::ShaderDecoder fragPrepassCode(SCENE_PREPASS_FRAG, SCENE_PREPASS_FRAG_SIZE);
    assets::ShaderDecoder fragShadowCode(SCENE_SHADOW_FRAG, SCENE_SHADOW_FRAG_SIZE);

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

    util::String vertSceneCode = assets::ShaderDecoder(SCENE_VERT, SCENE_VERT_SIZE).code();
    util::String fragLitCode   = assets::ShaderDecoder(SCENE_LIT_FRAG, SCENE_LIT_FRAG_SIZE).code();
    util::String fragUnlitCode = assets::ShaderDecoder(SCENE_UNLIT_FRAG, SCENE_UNLIT_FRAG_SIZE).code();

    /* --- Insert user code --- */

    insertUserCode(vertSceneCode, vertMarker, vert);
    insertUserCode(fragLitCode,   fragMarker, frag);
    insertUserCode(fragUnlitCode, fragMarker, frag);

    /* --- Compile shaders --- */

    gpu::Shader vertScene(GL_VERTEX_SHADER, vertSceneCode.data());
    gpu::Shader vertShadow(GL_VERTEX_SHADER, vertSceneCode.data(), {"SHADOW"});
    gpu::Shader fragLit(GL_FRAGMENT_SHADER, fragLitCode.data());
    gpu::Shader fragUnlit(GL_FRAGMENT_SHADER, fragUnlitCode.data());
    gpu::Shader fragPrepass(GL_FRAGMENT_SHADER, assets::ShaderDecoder(SCENE_PREPASS_FRAG, SCENE_PREPASS_FRAG_SIZE));
    gpu::Shader fragShadow(GL_FRAGMENT_SHADER, assets::ShaderDecoder(SCENE_SHADOW_FRAG, SCENE_SHADOW_FRAG_SIZE));

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

    /* --- Setup texture samplers --- */

    gpu::Pipeline([this](const gpu::Pipeline& pipeline) {
        for (int i = 0; i < VariantCount; ++i) {
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
