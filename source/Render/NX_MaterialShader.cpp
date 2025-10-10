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

    mPrograms[Variant::SCENE_LIT]       = gpu::Program(vertSceneShader, fragLitShader);
    mPrograms[Variant::SCENE_UNLIT]     = gpu::Program(vertSceneShader, fragUnlitShader);
    mPrograms[Variant::SCENE_WIREFRAME] = gpu::Program(vertSceneShader, geomWireframe, fragUnlitShader);
    mPrograms[Variant::SCENE_PREPASS]   = gpu::Program(vertSceneShader, fragPrepass);
    mPrograms[Variant::SCENE_SHADOW]    = gpu::Program(vertShadowShader, fragShadow);
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

    mPrograms[Variant::SCENE_LIT]       = gpu::Program(vertSceneShader, fragLitShader);
    mPrograms[Variant::SCENE_UNLIT]     = gpu::Program(vertSceneShader, fragUnlitShader);
    mPrograms[Variant::SCENE_WIREFRAME] = gpu::Program(vertSceneShader, geomWireframe, fragUnlitShader);
    mPrograms[Variant::SCENE_PREPASS]   = gpu::Program(vertSceneShader, fragPrepass);
    mPrograms[Variant::SCENE_SHADOW]    = gpu::Program(vertShadowShader, fragShadow);

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
            NX_INTERNAL_LOG(E, "RENDER: Failed to reserve space for dynamic uniform buffer range infos");
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
