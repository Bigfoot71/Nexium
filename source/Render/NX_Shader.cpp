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

    mPrograms[Variant::SHAPE_COLOR]   = gpu::Program(vertShape, fragShapeColor);
    mPrograms[Variant::SHAPE_TEXTURE] = gpu::Program(vertShape, fragShapeTexture);
    mPrograms[Variant::TEXT_BITMAP]   = gpu::Program(vertShape, fragTextBitmap);
    mPrograms[Variant::TEXT_SDF]      = gpu::Program(vertShape, fragTextSDF);
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
            NX_INTERNAL_LOG(E, "RENDER: Dynamic uniform buffer range info reservation failed (requested: 8 entries)");
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
