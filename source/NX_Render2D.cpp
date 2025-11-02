#include <NX/NX_Render2D.h>

#include <NX/NX_Codepoint.h>
#include <NX/NX_Runtime.h>
#include <NX/NX_Display.h>
#include <NX/NX_Window.h>
#include <NX/NX_Init.h>

#include "./INX_GlobalAssets.hpp"
#include "./INX_AssetDecoder.hpp"
#include "./INX_PoolAssets.hpp"
#include "./NX_Shader2D.hpp"
#include "./NX_Texture.hpp"

#include "./Detail/Util/StaticArray.hpp"
#include "./Detail/Util/ObjectRing.hpp"
#include "./Detail/Util/Memory.hpp"

#include "./Detail/GPU/VertexArray.hpp"
#include "./Detail/GPU/Pipeline.hpp"
#include "./Detail/GPU/Buffer.hpp"
#include "NX/NX_Math.h"

#include <shaders/screen.vert.h>
#include <shaders/overlay.frag.h>

// ============================================================================
// INTERNAL TYPES
// ============================================================================

enum class INX_DrawMode2D {
    SHAPE, TEXT
};

struct INX_DrawCall2D {
    /** Constructors */
    INX_DrawCall2D() = default;
    INX_DrawCall2D(NX_Shader2D* s, const NX_Texture* t, size_t o);
    INX_DrawCall2D(NX_Shader2D* s, const NX_Font* f, size_t o);

    /** Shader related data */
    NX_Shader2D::TextureArray shaderTextures;
    int shaderDynamicRangeIndex;
    NX_Shader2D* shader;

    /** Drawable */
    union {
        const NX_Texture* texture;
        const NX_Font* font;
    };

    /** Draw call info */
    size_t offset, count;       //< Offset and count in the index buffer (in number of indices)
    INX_DrawMode2D mode;
};

inline INX_DrawCall2D::INX_DrawCall2D(NX_Shader2D* s, const NX_Texture* t, size_t o)
    : shader(s), texture(t), offset(o), count(0), mode(INX_DrawMode2D::SHAPE)
{
    if (s != nullptr) {
        shaderTextures = s->GetTextures();
        shaderDynamicRangeIndex = s->GetDynamicRangeIndex();
    }
}

inline INX_DrawCall2D::INX_DrawCall2D(NX_Shader2D* s, const NX_Font* f, size_t o)
    : shader(s), font(f), offset(o), count(0), mode(INX_DrawMode2D::TEXT)
{ }

struct INX_VertexBuffer2D {
    gpu::VertexArray vao{};
    gpu::Buffer vbo{};
    gpu::Buffer ebo{};
};

struct INX_FrameUniform2D {
    alignas(16) NX_Mat4 projection;
    alignas(4) float time;
};

// ============================================================================
// LOCAL STATE
// ============================================================================

struct INX_Render2DState {
    /** Constants */
    static constexpr int MaxDrawCalls = 128;
    static constexpr int MaxVertices = 4096;
    static constexpr int MaxIndices = 6144;

    /** CPU Buffers */
    util::StaticArray<INX_DrawCall2D, MaxDrawCalls> drawCalls{};
    util::StaticArray<NX_Vertex2D, MaxVertices> vertices{};
    util::StaticArray<uint16_t, MaxIndices> indices{};
    util::StaticArray<NX_Mat3, 16> matrixStack{};

    /** GPU Buffers */
    INX_VertexBuffer2D vertexBuffer{};
    gpu::Buffer uniformBuffer{};

    /** Framebuffer */
    gpu::Framebuffer framebuffer{};
    gpu::Texture targetColor{};

    /** Programs */
    gpu::Program programOverlay{};

    /** Current State */
    NX_Color currentColor{NX_WHITE};
    NX_Shader2D* currentShader{nullptr};
    const NX_Font* currentFont{nullptr};
    const NX_Texture* currentTexture{nullptr};
    const NX_RenderTexture* currentTarget{nullptr};
};

static util::UniquePtr<INX_Render2DState> INX_Render2D{};

// ============================================================================
// INTERNAL FUNCTIONS
// ============================================================================

bool INX_Render2DState_Init(NX_AppDesc* desc)
{
    INX_Render2D = util::MakeUnique<INX_Render2DState>();
    if (INX_Render2D == nullptr) {
        return false;
    }

    /* --- Set default app descrition values --- */

    if (desc->render2D.resolution < NX_IVEC2_ONE) {
        desc->render2D.resolution = NX_GetDisplaySize();
    }

    if (desc->render2D.sampleCount < 1) {
        desc->render2D.sampleCount = 1;
    }

    /* --- Push first transform matrix --- */

    INX_Render2D->matrixStack.push_back(NX_MAT3_IDENTITY);

    /* --- Create the vertex buffer --- */

    INX_VertexBuffer2D& vertexBuffer = INX_Render2D->vertexBuffer;

    size_t vboSize = INX_Render2DState::MaxVertices * sizeof(NX_Vertex2D);
    size_t eboSize = INX_Render2DState::MaxIndices * sizeof(uint16_t);

    vertexBuffer.vbo = gpu::Buffer(GL_ARRAY_BUFFER, vboSize, nullptr, GL_DYNAMIC_DRAW);
    vertexBuffer.ebo = gpu::Buffer(GL_ELEMENT_ARRAY_BUFFER, eboSize, nullptr, GL_DYNAMIC_DRAW);

    vertexBuffer.vao = gpu::VertexArray(&vertexBuffer.ebo, {
        gpu::VertexBufferDesc {
            .buffer = &vertexBuffer.vbo,
            .attributes = {
                gpu::VertexAttribute {
                    .location = 0,
                    .size = 2,
                    .type = GL_FLOAT,
                    .normalized = false,
                    .stride = sizeof(NX_Vertex2D),
                    .offset = offsetof(NX_Vertex2D, position),
                    .divisor = 0
                },
                gpu::VertexAttribute {
                    .location = 1,
                    .size = 2,
                    .type = GL_FLOAT,
                    .normalized = false,
                    .stride = sizeof(NX_Vertex2D),
                    .offset = offsetof(NX_Vertex2D, texcoord),
                    .divisor = 0
                },
                gpu::VertexAttribute {
                    .location = 2,
                    .size = 4,
                    .type = GL_FLOAT,
                    .normalized = false,
                    .stride = sizeof(NX_Vertex2D),
                    .offset = offsetof(NX_Vertex2D, color),
                    .divisor = 0
                }
            }
        }
    });

    /* --- Create the uniform buffer --- */

    INX_Render2D->uniformBuffer = gpu::Buffer(
        GL_UNIFORM_BUFFER, sizeof(INX_FrameUniform2D),
        nullptr, GL_DYNAMIC_DRAW
    );

    /* --- Create the color target and the framebuffer --- */

    INX_Render2D->targetColor = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D,
            .internalFormat = GL_RGBA8,
            .data = nullptr,
            .width = desc->render2D.resolution.x,
            .height = desc->render2D.resolution.y,
            .depth = 0,
            .mipmap = false
        },
        gpu::TextureParam
        {
            .minFilter = GL_LINEAR,
            .magFilter = GL_LINEAR,
            .sWrap = GL_CLAMP_TO_EDGE,
            .tWrap = GL_CLAMP_TO_EDGE,
            .rWrap = GL_CLAMP_TO_EDGE
        }
    );

    INX_Render2D->framebuffer = gpu::Framebuffer({
        &INX_Render2D->targetColor
    });

    if (desc->render2D.sampleCount > 1) {
        INX_Render2D->framebuffer.setSampleCount(desc->render2D.sampleCount);
    }

    /* --- Create programs --- */

    INX_Render2D->programOverlay = gpu::Program(
        gpu::Shader(
            GL_VERTEX_SHADER,
            INX_ShaderDecoder(
                SCREEN_VERT,
                SCREEN_VERT_SIZE
            )
        ),
        gpu::Shader(
            GL_FRAGMENT_SHADER,
            INX_ShaderDecoder(
                OVERLAY_FRAG,
                OVERLAY_FRAG_SIZE
            )
        )
    );

    return true;
}

void INX_Render2DState_Quit()
{
    INX_Render2D.reset();
}

static void INX_Render2D_Flush()
{
    if (INX_Render2D->drawCalls.empty() || INX_Render2D->vertices.empty()) {
        return;
    }

    /* --- Upload to vertex buffer --- */

    INX_Render2D->vertexBuffer.vbo.upload(
        0, INX_Render2D->vertices.size() * sizeof(NX_Vertex2D),
        INX_Render2D->vertices.data()
    );

    INX_Render2D->vertexBuffer.ebo.upload(
        0, INX_Render2D->indices.size() * sizeof(uint16_t),
        INX_Render2D->indices.data()
    );

    /* --- Setup pipeline --- */

    gpu::Pipeline pipeline;

    pipeline.setBlendMode(gpu::BlendMode::Premultiplied);
    pipeline.bindVertexArray(INX_Render2D->vertexBuffer.vao);
    pipeline.bindUniform(0, INX_Render2D->uniformBuffer);
    pipeline.bindFramebuffer(INX_Render2D->framebuffer);
    pipeline.setViewport(INX_Render2D->framebuffer);

    /* --- Render all draw calls --- */

    for (const INX_DrawCall2D& call : INX_Render2D->drawCalls)
    {
        const NX_Shader2D* shader = INX_Assets.Select(call.shader, INX_Shader2DAsset::DEFAULT);

        shader->BindUniforms(pipeline, call.shaderDynamicRangeIndex);
        shader->BindTextures(pipeline, call.shaderTextures);

        switch (call.mode) {
        case INX_DrawMode2D::SHAPE:
            if (call.texture != nullptr) {
                pipeline.useProgram(shader->GetProgram(NX_Shader2D::Variant::SHAPE_TEXTURE));
                pipeline.bindTexture(0, *reinterpret_cast<const gpu::Texture*>(call.texture));
            }
            else {
                pipeline.useProgram(shader->GetProgram(NX_Shader2D::Variant::SHAPE_COLOR));
            }
            break;
        case INX_DrawMode2D::TEXT:
            const NX_Font* font = INX_Assets.Select(call.font, INX_FontAsset::DEFAULT);
            switch (NX_GetFontType(font)) {
            case NX_FONT_NORMAL:
            case NX_FONT_LIGHT:
            case NX_FONT_MONO:
                pipeline.useProgram(shader->GetProgram(NX_Shader2D::Variant::TEXT_BITMAP));
                break;
            case NX_FONT_SDF:
                pipeline.useProgram(shader->GetProgram(NX_Shader2D::Variant::TEXT_SDF));
                break;
            }
            pipeline.bindTexture(0, font->texture->gpu);
            break;
        }

        pipeline.drawElements(
            GL_TRIANGLES, GL_UNSIGNED_SHORT,
            call.offset, call.count
        );
    }

    /* --- Reset --- */

    INX_Render2D->drawCalls.clear();
    INX_Render2D->vertices.clear();
    INX_Render2D->indices.clear();
}

static void INX_Render2D_EnsureDrawCall(INX_DrawMode2D mode, int vertices, int indices)
{
    if (INX_Render2D->vertices.size() + vertices > INX_Render2DState::MaxVertices ||
        INX_Render2D->indices.size() + indices > INX_Render2DState::MaxIndices) {
        INX_Render2D_Flush();
    }

    if (INX_Render2D->drawCalls.empty()) {
        switch (mode) {
        case INX_DrawMode2D::SHAPE:
            INX_Render2D->drawCalls.emplace_back(
                INX_Render2D->currentShader,
                INX_Render2D->currentTexture,
                0
            );
            break;
        case INX_DrawMode2D::TEXT:
            INX_Render2D->drawCalls.emplace_back(
                INX_Render2D->currentShader,
                INX_Render2D->currentFont,
                0
            );
            break;
        }
        return;
    }

    INX_DrawCall2D& call = *INX_Render2D->drawCalls.back();

    if (call.count == 0) {
        call.shader = INX_Render2D->currentShader;
        call.mode = mode;
        switch (mode) {
        case INX_DrawMode2D::SHAPE:
            call.texture = INX_Render2D->currentTexture;
            break;
        case INX_DrawMode2D::TEXT:
            call.font = INX_Render2D->currentFont;
            break;
        }
        return;
    }

    if (call.mode == mode && call.shader == INX_Render2D->currentShader) {
        switch (call.mode) {
        case INX_DrawMode2D::SHAPE:
            if (call.texture == INX_Render2D->currentTexture) {
                return;
            }
            break;
        case INX_DrawMode2D::TEXT:
            if (call.font == INX_Render2D->currentFont) {
                return;
            }
            break;
        }
    }

    if (INX_Render2D->drawCalls.size() == INX_Render2DState::MaxDrawCalls) {
        INX_Render2D_Flush();
    }

    switch (mode) {
    case INX_DrawMode2D::SHAPE:
        INX_Render2D->drawCalls.emplace_back(
            INX_Render2D->currentShader,
            INX_Render2D->currentTexture,
            INX_Render2D->indices.size()
        );
        break;
    case INX_DrawMode2D::TEXT:
        INX_Render2D->drawCalls.emplace_back(
            INX_Render2D->currentShader,
            INX_Render2D->currentFont,
            INX_Render2D->indices.size()
        );
        break;
    }
}

static void INX_Render2D_Blit()
{
    INX_Render2D->framebuffer.resolve();

    gpu::Pipeline pipeline;

    if (INX_Render2D->currentTarget != nullptr) {
        pipeline.bindFramebuffer(INX_Render2D->currentTarget->gpu);
        pipeline.setViewport(INX_Render2D->currentTarget->gpu);
    }
    else {
        pipeline.setViewport(NX_GetWindowSize());
    }

    pipeline.bindTexture(0, INX_Render2D->targetColor);
    pipeline.useProgram(INX_Render2D->programOverlay);
    pipeline.setBlendMode(gpu::BlendMode::Premultiplied);
    pipeline.draw(GL_TRIANGLES, 3);
}

static uint16_t INX_Render2D_NextVertexIndex()
{
    return static_cast<uint16_t>(INX_Render2D->vertices.size());
}

static void INX_Render2D_AddVertex(float x, float y, float u, float v)
{
    SDL_assert(INX_Render2D->vertices.size() < INX_Render2DState::MaxVertices);
    INX_Render2D->vertices.emplace_back(
        NX_VEC2(x, y) * (*INX_Render2D->matrixStack.back()),
        NX_VEC2(u, v), INX_Render2D->currentColor
    );
}

static void INX_Render2D_AddVertex(const NX_Vertex2D& vertex)
{
    SDL_assert(INX_Render2D->vertices.size() < INX_Render2DState::MaxVertices);
    INX_Render2D->vertices.emplace_back(
        vertex.position * (*INX_Render2D->matrixStack.back()),
        vertex.texcoord, vertex.color
    );
}

static void INX_Render2D_AddIndex(uint16_t index)
{
    SDL_assert(INX_Render2D->indices.size() < INX_Render2DState::MaxIndices);
    INX_Render2D->indices.emplace_back(index);
    INX_Render2D->drawCalls.back()->count++;
}

static float INX_Render2D_ToPixelSize(float unit)
{
    if (!NX_IsMat3Identity(INX_Render2D->matrixStack.back())) {
        const NX_Mat3& mat = *INX_Render2D->matrixStack.back();
        float scaleX = sqrtf(mat.m00 * mat.m00 + mat.m01 * mat.m01);
        float scaleY = sqrtf(mat.m10 * mat.m10 + mat.m11 * mat.m11);
        float avgScale = (scaleX + scaleY) * 0.5f;
        unit /= avgScale;
    }
    return unit;
}

// ============================================================================
// PUBLIC API
// ============================================================================

void NX_Begin2D(NX_RenderTexture* target)
{
    NX_IVec2 size = target ? target->gpu.dimensions() : NX_GetWindowSize();

    INX_Render2D->uniformBuffer.uploadObject(INX_FrameUniform2D {
        .projection = NX_Mat4Ortho(0, size.x, size.y, 0, 0, 1),
        .time = static_cast<float>(NX_GetElapsedTime())
    });
    INX_Render2D->currentTarget = target;

    // TODO: Move the clear during the first draw
    gpu::Pipeline([&](gpu::Pipeline pipeline) {
        pipeline.bindFramebuffer(INX_Render2D->framebuffer);
        pipeline.clear(INX_Render2D->framebuffer, NX_BLANK);
    });
}

void NX_End2D()
{
    INX_Render2D_Flush();
    INX_Render2D_Blit();

    // REVIEW: We can collect the used programs rather than iterating over all programs
    INX_Pool.ForEach<NX_Shader2D>([&](NX_Shader2D& shader) {
        shader.ClearDynamicBuffer();
    });
}

void NX_SetColor2D(NX_Color color)
{
    INX_Render2D->currentColor = color;
}

void NX_SetTexture2D(const NX_Texture* texture)
{
    INX_Render2D->currentTexture = texture;
}

void NX_SetFont2D(const NX_Font* font)
{
    INX_Render2D->currentFont = font;
}

void NX_SetShader2D(NX_Shader2D* shader)
{
    INX_Render2D->currentShader = shader;
}

void NX_Push2D()
{
    if (!INX_Render2D->matrixStack.push_back(*INX_Render2D->matrixStack.back())) {
        NX_LOG(W, "RENDER: Transformation 2D stack overflow");
    }
}

void NX_Pop2D()
{
    if (INX_Render2D->matrixStack.size() > 1) {
        INX_Render2D->matrixStack.pop_back();
    }
}

void NX_Translate2D(NX_Vec2 translation)
{
    NX_Mat3& mat = *INX_Render2D->matrixStack.back();
    mat = mat * NX_Mat3Translate2D(translation);
}

void NX_Rotate2D(float radians)
{
    NX_Mat3& mat = *INX_Render2D->matrixStack.back();
    mat = mat * NX_Mat3Rotate2D(radians);
}

void NX_Scale2D(NX_Vec2 scale)
{
    NX_Mat3& mat = *INX_Render2D->matrixStack.back();
    mat = mat * NX_Mat3Scale2D(scale);
}

void NX_DrawShape2D(NX_PrimitiveType type, const NX_Vec2* points, int pointCount, float thickness)
{
    if (thickness > 0.0f) {
        // Convert thickness to pixels and negate it to avoid doing a redundant conversion
        thickness = -INX_Render2D_ToPixelSize(thickness);
    }

    switch (type) {
    case NX_PRIMITIVE_POINTS:
        thickness *= 0.5f;
        for (int i = 0; i < pointCount; i++) {
            NX_DrawQuad2D(
                points[i] + NX_VEC2(-thickness, -thickness),
                points[i] + NX_VEC2(+thickness, -thickness),
                points[i] + NX_VEC2(+thickness, +thickness),
                points[i] + NX_VEC2(-thickness, +thickness)
            );
        }
        break;
    case NX_PRIMITIVE_LINES:
        for (int i = 0; i < pointCount; i += 2) {
            NX_DrawLine2D(points[i], points[i + 1], thickness);
        }
        break;
    case NX_PRIMITIVE_LINE_STRIP:
        for (int i = 0; i < pointCount - 1; i++) {
            NX_DrawLine2D(points[i], points[i + 1], thickness);
        }
        break;
    case NX_PRIMITIVE_LINE_LOOP:
        for (int i = 0; i < pointCount; i++) {
            NX_DrawLine2D(points[i], points[(i + 1) % pointCount], thickness);
        }
        break;
    case NX_PRIMITIVE_TRIANGLES:
        if (thickness == 0.0f) {
            for (int i = 0; i < pointCount; i += 3) {
                NX_DrawTriangle2D(points[i], points[i + 1], points[i + 2]);
            }
        }
        else {
            for (int i = 0; i < pointCount; i += 3) {
                NX_DrawLine2D(points[i + 0], points[i + 1], thickness);
                NX_DrawLine2D(points[i + 1], points[i + 2], thickness);
                NX_DrawLine2D(points[i + 2], points[i + 0], thickness);
            }
        }
        break;
    case NX_PRIMITIVE_TRIANGLE_STRIP:
        if (thickness == 0.0f) {
            for (int i = 0; i < pointCount - 2; i++) {
                if (i % 2 == 0) {
                    NX_DrawTriangle2D(points[i], points[i + 1], points[i + 2]);
                } else {
                    NX_DrawTriangle2D(points[i + 1], points[i], points[i + 2]);
                }
            }
        }
        else {
            NX_DrawLine2D(points[0], points[1], thickness);
            for (int i = 0; i < pointCount - 2; i++) {
                NX_DrawLine2D(points[i], points[i + 2], thickness);
            }
            NX_DrawLine2D(points[pointCount - 2], points[pointCount - 1], thickness);
        }
        break;
    case NX_PRIMITIVE_TRIANGLE_FAN:
        if (thickness == 0.0f) {
            for (int i = 1; i < pointCount - 1; i++) {
                NX_DrawTriangle2D(points[0], points[i], points[i + 1]);
            }
        }
        else {
            for (int i = 1; i < pointCount - 1; i++) {
                NX_DrawLine2D(points[i], points[i + 1], thickness);
            }
            NX_DrawLine2D(points[1], points[pointCount - 1], thickness);
        }
        break;
    }
}

void NX_DrawShapeEx2D(NX_PrimitiveType type, const NX_Vertex2D* vertices, int vertexCount, float thickness)
{
    if (thickness > 0.0f) {
        // Convert thickness to pixels and negate it to avoid doing a redundant conversion
        thickness = -INX_Render2D_ToPixelSize(thickness);
    }

    switch (type) {
    case NX_PRIMITIVE_POINTS:
        thickness *= 0.5f;
        for (int i = 0; i < vertexCount; i++) {
            NX_DrawQuad2D(
                vertices[i].position + NX_VEC2(-thickness, -thickness),
                vertices[i].position + NX_VEC2(+thickness, -thickness),
                vertices[i].position + NX_VEC2(+thickness, +thickness),
                vertices[i].position + NX_VEC2(-thickness, +thickness)
            );
        }
        break;
    case NX_PRIMITIVE_LINES:
        for (int i = 0; i < vertexCount; i += 2) {
            NX_DrawLineEx2D(&vertices[i], &vertices[i + 1], thickness);
        }
        break;
    case NX_PRIMITIVE_LINE_STRIP:
        for (int i = 0; i < vertexCount - 1; i++) {
            NX_DrawLineEx2D(&vertices[i], &vertices[i + 1], thickness);
        }
        break;
    case NX_PRIMITIVE_LINE_LOOP:
        for (int i = 0; i < vertexCount; i++) {
            NX_DrawLineEx2D(&vertices[i], &vertices[(i + 1) % vertexCount], thickness);
        }
        break;
    case NX_PRIMITIVE_TRIANGLES:
        if (thickness == 0.0f) {
            for (int i = 0; i < vertexCount; i += 3) {
                NX_DrawTriangleEx2D(&vertices[i], &vertices[i + 1], &vertices[i + 2]);
            }
        }
        else {
            for (int i = 0; i < vertexCount; i += 3) {
                NX_DrawLineEx2D(&vertices[i + 0], &vertices[i + 1], thickness);
                NX_DrawLineEx2D(&vertices[i + 1], &vertices[i + 2], thickness);
                NX_DrawLineEx2D(&vertices[i + 2], &vertices[i + 0], thickness);
            }
        }
        break;
    case NX_PRIMITIVE_TRIANGLE_STRIP:
        if (thickness == 0.0f) {
            for (int i = 0; i < vertexCount - 2; i++) {
                if (i % 2 == 0) {
                    NX_DrawTriangleEx2D(&vertices[i], &vertices[i + 1], &vertices[i + 2]);
                } else {
                    NX_DrawTriangleEx2D(&vertices[i + 1], &vertices[i], &vertices[i + 2]);
                }
            }
        }
        else {
            NX_DrawLineEx2D(&vertices[0], &vertices[1], thickness);
            for (int i = 0; i < vertexCount - 2; i++) {
                NX_DrawLineEx2D(&vertices[i], &vertices[i + 2], thickness);
            }
            NX_DrawLineEx2D(&vertices[vertexCount - 2], &vertices[vertexCount - 1], thickness);
        }
        break;
    case NX_PRIMITIVE_TRIANGLE_FAN:
        if (thickness == 0.0f) {
            for (int i = 1; i < vertexCount - 1; i++) {
                NX_DrawTriangleEx2D(&vertices[0], &vertices[i], &vertices[i + 1]);
            }
        }
        else {
            for (int i = 1; i < vertexCount - 1; i++) {
                NX_DrawLineEx2D(&vertices[i], &vertices[i + 1], thickness);
            }
            NX_DrawLineEx2D(&vertices[1], &vertices[vertexCount - 1], thickness);
        }
        break;
    }
}

void NX_DrawLine2D(NX_Vec2 p0, NX_Vec2 p1, float thickness)
{
    const NX_Color& color = INX_Render2D->currentColor;

    NX_Vertex2D v0 = { .position = p0, .texcoord = NX_VEC2_ZERO, .color = color };
    NX_Vertex2D v1 = { .position = p1, .texcoord = NX_VEC2_ONE, .color = color };

    NX_DrawLineEx2D(&v0, &v1, thickness);
}

void NX_DrawLineEx2D(const NX_Vertex2D* v0, const NX_Vertex2D* v1, float thickness)
{
    // NOTE: We draw the lines with quads for compatibility reasons
    INX_Render2D_EnsureDrawCall(INX_DrawMode2D::SHAPE, 4, 6);

    /* --- Calculation of pixel thickness if necessary --- */

    if (thickness > 0.0f) {
        thickness = INX_Render2D_ToPixelSize(thickness);
    }
    else {
        thickness = -thickness;
    }

    /* --- Calculation of the offset required in each direction --- */

    const NX_Vec2& p0 = v0->position;
    const NX_Vec2& p1 = v1->position;

    NX_Vec2 d = NX_Vec2Direction(p0, p1);
    float nx = -d.y * thickness * 0.5f;
    float ny = +d.x * thickness * 0.5f;

    uint16_t baseIndex = INX_Render2D_NextVertexIndex();

    /* --- Adding vertices and indices --- */

    INX_Render2D_AddVertex(NX_Vertex2D{
        .position = p0 + NX_VEC2(nx, ny),
        .texcoord = v0->texcoord,
        .color = v0->color
    });

    INX_Render2D_AddVertex(NX_Vertex2D{
        .position = p0 - NX_VEC2(nx, ny),
        .texcoord = v0->texcoord,
        .color = v0->color
    });

    INX_Render2D_AddVertex(NX_Vertex2D{
        .position = p1 - NX_VEC2(nx, ny),
        .texcoord = v1->texcoord,
        .color = v1->color
    });

    INX_Render2D_AddVertex(NX_Vertex2D{
        .position = p1 + NX_VEC2(nx, ny),
        .texcoord = v1->texcoord,
        .color = v1->color
    });

    // Triangle 1: 0, 1, 2
    INX_Render2D_AddIndex(baseIndex + 0);
    INX_Render2D_AddIndex(baseIndex + 1);
    INX_Render2D_AddIndex(baseIndex + 2);

    // Triangle 2: 0, 2, 3
    INX_Render2D_AddIndex(baseIndex + 0);
    INX_Render2D_AddIndex(baseIndex + 2);
    INX_Render2D_AddIndex(baseIndex + 3);
}

void NX_DrawTriangle2D(NX_Vec2 p0, NX_Vec2 p1, NX_Vec2 p2)
{
    const NX_Color& color = INX_Render2D->currentColor;

    NX_Vertex2D v0 = { .position = p0, .texcoord = NX_VEC2(0.0f, 0.0f), .color = color };
    NX_Vertex2D v1 = { .position = p1, .texcoord = NX_VEC2(0.5f, 0.5f), .color = color };
    NX_Vertex2D v2 = { .position = p2, .texcoord = NX_VEC2(1.0f, 1.0f), .color = color };

    NX_DrawTriangleEx2D(&v0, &v1, &v2);
}

void NX_DrawTriangleEx2D(const NX_Vertex2D* v0, const NX_Vertex2D* v1, const NX_Vertex2D* v2)
{
    INX_Render2D_EnsureDrawCall(INX_DrawMode2D::SHAPE, 3, 3);

    uint16_t baseIndex = INX_Render2D_NextVertexIndex();

    INX_Render2D_AddVertex(*v0);
    INX_Render2D_AddVertex(*v1);
    INX_Render2D_AddVertex(*v2);

    INX_Render2D_AddIndex(baseIndex + 0);
    INX_Render2D_AddIndex(baseIndex + 1);
    INX_Render2D_AddIndex(baseIndex + 2);
}

void NX_DrawQuad2D(NX_Vec2 p0, NX_Vec2 p1, NX_Vec2 p2, NX_Vec2 p3)
{
    const NX_Color& color = INX_Render2D->currentColor;

    NX_Vertex2D v0 = { .position = p0, .texcoord = NX_VEC2(0.0f, 0.0f), .color = color };
    NX_Vertex2D v1 = { .position = p1, .texcoord = NX_VEC2(1.0f, 0.0f), .color = color };
    NX_Vertex2D v2 = { .position = p2, .texcoord = NX_VEC2(1.0f, 1.0f), .color = color };
    NX_Vertex2D v3 = { .position = p3, .texcoord = NX_VEC2(0.0f, 1.0f), .color = color };

    NX_DrawQuadEx2D(&v0, &v1, &v2, &v3);
}

void NX_DrawQuadEx2D(const NX_Vertex2D* v0, const NX_Vertex2D* v1, const NX_Vertex2D* v2, const NX_Vertex2D* v3)
{
    INX_Render2D_EnsureDrawCall(INX_DrawMode2D::SHAPE, 4, 6);

    uint16_t baseIndex = INX_Render2D_NextVertexIndex();

    INX_Render2D_AddVertex(*v0);
    INX_Render2D_AddVertex(*v1);
    INX_Render2D_AddVertex(*v2);
    INX_Render2D_AddVertex(*v3);

    INX_Render2D_AddIndex(baseIndex + 0);
    INX_Render2D_AddIndex(baseIndex + 1);
    INX_Render2D_AddIndex(baseIndex + 2);

    INX_Render2D_AddIndex(baseIndex + 0);
    INX_Render2D_AddIndex(baseIndex + 2);
    INX_Render2D_AddIndex(baseIndex + 3);
}

void NX_DrawRect2D(float x, float y, float w, float h)
{
    INX_Render2D_EnsureDrawCall(INX_DrawMode2D::SHAPE, 4, 6);

    uint16_t baseIndex = INX_Render2D_NextVertexIndex();

    INX_Render2D_AddVertex(x, y, 0.0f, 0.0f);
    INX_Render2D_AddVertex(x + w, y, 1.0f, 0.0f);
    INX_Render2D_AddVertex(x + w, y + h, 1.0f, 1.0f);
    INX_Render2D_AddVertex(x, y + h, 0.0f, 1.0f);

    INX_Render2D_AddIndex(baseIndex + 0);
    INX_Render2D_AddIndex(baseIndex + 1);
    INX_Render2D_AddIndex(baseIndex + 2);

    INX_Render2D_AddIndex(baseIndex + 0);
    INX_Render2D_AddIndex(baseIndex + 2);
    INX_Render2D_AddIndex(baseIndex + 3);
}

void NX_DrawRectBorder2D(float x, float y, float w, float h, float thickness)
{
    const NX_Color& color = INX_Render2D->currentColor;

    NX_Vertex2D v0 = { .position = {x,     y},     .texcoord = {0.0f, 0.0f}, .color = color };
    NX_Vertex2D v1 = { .position = {x + w, y},     .texcoord = {1.0f, 0.0f}, .color = color };
    NX_Vertex2D v2 = { .position = {x + w, y + h}, .texcoord = {1.0f, 1.0f}, .color = color };
    NX_Vertex2D v3 = { .position = {x,     y + h}, .texcoord = {0.0f, 1.0f}, .color = color };

    if (thickness > 0.0f) {
        // Convert thickness to pixels and negate it to avoid doing a redundant conversion
        thickness = -INX_Render2D_ToPixelSize(thickness);
    }

    NX_DrawLineEx2D(&v0, &v1, thickness);
    NX_DrawLineEx2D(&v1, &v2, thickness);
    NX_DrawLineEx2D(&v2, &v3, thickness);
    NX_DrawLineEx2D(&v3, &v0, thickness);
}

void NX_DrawRectRounded2D(float x, float y, float w, float h, float radius, int segments)
{
    radius = std::min(radius, std::min(w * 0.5f, h * 0.5f));

    if (radius <= 0.0f) {
        NX_DrawQuad2D(NX_VEC2(x, y), NX_VEC2(x + w, y), NX_VEC2(x + w, y + h), NX_VEC2(x, y + h));
        return;
    }

    /* --- Calculation of vertices and indices --- */

    int cornerVertices = segments + 1;
    int totalVertices = 4 * cornerVertices + 8;
    int totalIndices = 4 * segments * 3 + 12;

    INX_Render2D_EnsureDrawCall(INX_DrawMode2D::SHAPE, totalVertices, totalIndices);

    uint16_t baseIndex = INX_Render2D_NextVertexIndex();
    uint16_t currentIndex = 0;

    /* --- Corner centers and angle data --- */

    float cornerData[4][4] = {
        {x + radius, y + radius, NX_PI, NX_PI * 1.5f},              // Top-left
        {x + w - radius, y + radius, NX_PI * 1.5f, NX_PI * 2.0f},   // Top-right
        {x + w - radius, y + h - radius, 0.0f, NX_PI * 0.5f},       // Bottom-right
        {x + radius, y + h - radius, NX_PI * 0.5f, NX_PI}           // Bottom-left
    };

    /* --- Corner generation --- */

    for (int corner = 0; corner < 4; corner++) {
        float cx = cornerData[corner][0];
        float cy = cornerData[corner][1];
        float startAngle = cornerData[corner][2];
        float angleRange = cornerData[corner][3] - startAngle;
        float angleStep = angleRange / segments;
        uint16_t centerIdx = currentIndex++;
        INX_Render2D_AddVertex(cx, cy, 0.5f, 0.5f);
        for (int i = 0; i <= segments; i++) {
            float angle = startAngle + i * angleStep;
            INX_Render2D_AddVertex(
                cx + std::cos(angle) * radius,
                cy + std::sin(angle) * radius,
                0.5f, 0.5f
            );
            if (i > 0) {
                INX_Render2D_AddIndex(baseIndex + centerIdx);
                INX_Render2D_AddIndex(baseIndex + currentIndex - 1);
                INX_Render2D_AddIndex(baseIndex + currentIndex);
            }
            currentIndex++;
        }
    }

    /* --- Fill rectangles --- */

    float rectData[3][8] = {
        // Horizontal center
        {x + radius, y, x + w - radius, y, x + w - radius, y + h, x + radius, y + h},
        // Left vertical
        {x, y + radius, x + radius, y + radius, x + radius, y + h - radius, x, y + h - radius},
        // Right vertical
        {x + w - radius, y + radius, x + w, y + radius, x + w, y + h - radius, x + w - radius, y + h - radius}
    };

    for (int rect = 0; rect < 3; rect++) {
        uint16_t rectStart = currentIndex;
        float uvs[4][2] = {
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f},
            {0.0f, 1.0f}
        };
        for (int i = 0; i < 4; i++) {
            INX_Render2D_AddVertex(
                rectData[rect][i * 2],
                rectData[rect][i * 2 + 1],
                uvs[i][0], uvs[i][1]
            );
        }
        uint16_t indices[6] = {0, 1, 2, 0, 2, 3};
        for (int i = 0; i < 6; i++) {
            INX_Render2D_AddIndex(baseIndex + rectStart + indices[i]);
        }
        currentIndex += 4;
    }
}

void NX_DrawRectRoundedBorder2D(float x, float y, float w, float h, float radius, int segments, float thickness)
{
    /* --- Calculation of the minimum radius required per corner --- */

    radius = std::min(radius, std::min(w * 0.5f, h * 0.5f));

    if (radius <= 0.0f) {
        NX_DrawRectBorder2D(x, y, w, h, thickness);
        return;
    }

    /* --- Calculation of pixel thickness if necessary --- */

    if (thickness > 0.0f) {
        thickness = INX_Render2D_ToPixelSize(thickness);
    }
    else {
        thickness = -thickness;
    }

    /* --- Pre-calculation ​​and declaration of the draw call that will be made */

    float halfThickness = thickness * 0.5f;
    float innerRadius = std::max(0.0f, radius - halfThickness);
    float outerRadius = radius + halfThickness;

    int arcVertices = (segments + 1) * 2;
    int totalVertices = 4 * arcVertices + 16;
    int totalIndices = 4 * segments * 6 + 48;

    INX_Render2D_EnsureDrawCall(INX_DrawMode2D::SHAPE, totalVertices, totalIndices);

    uint16_t baseIndex = INX_Render2D_NextVertexIndex();
    uint16_t currentIndex = 0;

    /* --- Corner data --- */

    float cornerData[4][4] = {
        {x + radius, y + radius, NX_PI, NX_PI * 1.5f},
        {x + w - radius, y + radius, NX_PI * 1.5f, NX_PI * 2.0f},
        {x + w - radius, y + h - radius, 0.0f, NX_PI * 0.5f},
        {x + radius, y + h - radius, NX_PI * 0.5f, NX_PI}
    };

    /* --- Generating corner borders --- */

    for (int corner = 0; corner < 4; corner++) {
        float cx = cornerData[corner][0];
        float cy = cornerData[corner][1];
        float startAngle = cornerData[corner][2];
        float angleRange = cornerData[corner][3] - startAngle;
        float angleStep = angleRange / segments;

        uint16_t cornerStart = currentIndex;

        // Generation of pairs of vertices and quads
        for (int i = 0; i <= segments; i++) {
            float angle = startAngle + i * angleStep;
            float cosA = std::cos(angle);
            float sinA = std::sin(angle);

            // Vertices inner/outer
            INX_Render2D_AddVertex(cx + cosA * innerRadius, cy + sinA * innerRadius, 0.5f, 0.5f);
            INX_Render2D_AddVertex(cx + cosA * outerRadius, cy + sinA * outerRadius, 0.5f, 0.5f);

            if (i > 0) {
                uint16_t base = baseIndex + cornerStart + (i - 1) * 2;
                // Quad with 2 triangles
                INX_Render2D_AddIndex(base);
                INX_Render2D_AddIndex(base + 1);
                INX_Render2D_AddIndex(base + 2);
                INX_Render2D_AddIndex(base + 2);
                INX_Render2D_AddIndex(base + 1);
                INX_Render2D_AddIndex(base + 3);
            }
            currentIndex += 2;
        }
    }

    /* --- Straight segments --- */

    float straightData[4][8] = {
        // Top
        {x + radius, y - halfThickness, x + radius, y + halfThickness,
         x + w - radius, y + halfThickness, x + w - radius, y - halfThickness},
        // Right
        {x + w - halfThickness, y + radius, x + w + halfThickness, y + radius,
         x + w + halfThickness, y + h - radius, x + w - halfThickness, y + h - radius},
        // Bottom
        {x + w - radius, y + h - halfThickness, x + w - radius, y + h + halfThickness,
         x + radius, y + h + halfThickness, x + radius, y + h - halfThickness},
        // Left
        {x + halfThickness, y + h - radius, x - halfThickness, y + h - radius,
         x - halfThickness, y + radius, x + halfThickness, y + radius}
    };

    /* --- Generation of straight segments --- */

    for (int seg = 0; seg < 4; seg++) {
        uint16_t segStart = currentIndex;
        float uvs[4][2] = {
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f},
            {0.0f, 1.0f}
        };
        for (int i = 0; i < 4; i++) {
            INX_Render2D_AddVertex(
                straightData[seg][i * 2],
                straightData[seg][i * 2 + 1],
                uvs[i][0], uvs[i][1]
            );
        }
        uint16_t indices[6] = {0, 1, 2, 0, 2, 3};
        for (int i = 0; i < 6; i++) {
            INX_Render2D_AddIndex(baseIndex + segStart + indices[i]);
        }
        currentIndex += 4;
    }
}

void NX_DrawCircle2D(NX_Vec2 center, float radius, int segments)
{
    if (segments < 3) segments = 32;

    INX_Render2D_EnsureDrawCall(INX_DrawMode2D::SHAPE, segments + 1, segments * 3);

    uint16_t baseIndex = INX_Render2D_NextVertexIndex();

    INX_Render2D_AddVertex(center.x, center.y, 0.5f, 0.5f);

    float delta = NX_TAU / segments;
    float cosDelta = std::cos(delta);
    float sinDelta = std::sin(delta);

    float inv2r = 1.0f / (2.0f * radius);

    float cx = radius;
    float cy = 0.0f;

    for (int i = 0; i < segments; i++) {
        float x = center.x + cx;
        float y = center.y + cy;

        float u = 0.5f + cx * inv2r;
        float v = 0.5f + cy * inv2r;

        INX_Render2D_AddVertex(x, y, u, v);

        float new_cx = cx * cosDelta - cy * sinDelta;
        cy = cx * sinDelta + cy * cosDelta;
        cx = new_cx;
    }

    for (int i = 0; i < segments; i++) {
        int next = (i + 1) % segments;
        INX_Render2D_AddIndex(baseIndex + 0);
        INX_Render2D_AddIndex(baseIndex + 1 + i);
        INX_Render2D_AddIndex(baseIndex + 1 + next);
    }
}

void NX_DrawCircleBorder2D(NX_Vec2 p, float radius, int segments, float thickness)
{
    if (segments < 3) segments = 32;

    if (thickness > 0.0f) {
        // Convert thickness to pixels and negate it to avoid doing a redundant conversion
        thickness = -INX_Render2D_ToPixelSize(thickness);
    }

    float delta = NX_TAU / segments;
    float cosDelta = std::cos(delta);
    float sinDelta = std::sin(delta);

    float cx = radius;
    float cy = 0.0f;

    NX_Vec2 prev = { p.x + cx, p.y + cy };

    for (int i = 1; i <= segments; i++) {
        float new_cx = cx * cosDelta - cy * sinDelta;
        float new_cy = cx * sinDelta + cy * cosDelta;
        cx = new_cx;
        cy = new_cy;

        NX_Vec2 curr = { p.x + cx, p.y + cy };

        // Line between prev and curr
        NX_DrawLine2D(prev, curr, thickness);

        prev = curr;
    }
}

void NX_DrawEllipse2D(NX_Vec2 center, NX_Vec2 radius, int segments)
{
    if (segments < 3) segments = 32;

    INX_Render2D_EnsureDrawCall(INX_DrawMode2D::SHAPE, segments + 1, segments * 3);

    uint16_t baseIndex = INX_Render2D_NextVertexIndex();

    INX_Render2D_AddVertex(center.x, center.y, 0.5f, 0.5f);

    float delta = NX_TAU / segments;
    float cosDelta = std::cos(delta);
    float sinDelta = std::sin(delta);

    float inv2rx = 1.0f / (2.0f * radius.x);
    float inv2ry = 1.0f / (2.0f * radius.y);

    float ux = 1.0f;
    float uy = 0.0f;

    for (int i = 0; i < segments; i++) {
        float cx = radius.x * ux;
        float cy = radius.y * uy;

        float x = center.x + cx;
        float y = center.y + cy;

        float u = 0.5f + cx * inv2rx; // = 0.5f + 0.5f * ux
        float v = 0.5f + cy * inv2ry; // = 0.5f + 0.5f * uy

        INX_Render2D_AddVertex(x, y, u, v);

        float newUX = ux * cosDelta - uy * sinDelta;
        uy = ux * sinDelta + uy * cosDelta;
        ux = newUX;
    }

    for (int i = 0; i < segments; i++) {
        int next = (i + 1) % segments;
        INX_Render2D_AddIndex(baseIndex + 0);
        INX_Render2D_AddIndex(baseIndex + 1 + i);
        INX_Render2D_AddIndex(baseIndex + 1 + next);
    }
}

void NX_DrawEllipseBorder2D(NX_Vec2 p, NX_Vec2 r, int segments, float thickness)
{
    if (segments < 3) segments = 32;

    if (thickness > 0.0f) {
        // Convert thickness to pixels and negate it to avoid doing a redundant conversion
        thickness = -INX_Render2D_ToPixelSize(thickness);
    }

    float delta = NX_TAU / segments;
    float cosDelta = std::cos(delta);
    float sinDelta = std::sin(delta);

    NX_Vec2 u = { 1.0f, 0.0f };
    NX_Vec2 prev = p + NX_Vec2{ r.x * u.x, r.y * u.y };

    for (int i = 1; i <= segments; i++)
    {
        u = NX_VEC2(
            u.x * cosDelta - u.y * sinDelta,
            u.x * sinDelta + u.y * cosDelta
        );

        NX_Vec2 curr = p + NX_Vec2{ r.x * u.x, r.y * u.y };
        NX_DrawLine2D(prev, curr, thickness);
        prev = curr;
    }
}

void NX_DrawPieSlice2D(NX_Vec2 center, float radius, float startAngle, float endAngle, int segments)
{
    if (segments < 1) segments = 16;

    float angleDiff = endAngle - startAngle;
    angleDiff = NX_WrapRadians(angleDiff);
    if (angleDiff < 0.0f) angleDiff += NX_TAU;

    float deltaAngle = angleDiff / segments;

    float cosDelta = std::cos(deltaAngle);
    float sinDelta = std::sin(deltaAngle);

    float cosA = std::cos(startAngle);
    float sinA = std::sin(startAngle);

    INX_Render2D_EnsureDrawCall(INX_DrawMode2D::SHAPE, segments + 2, segments * 3);

    uint16_t baseIndex = INX_Render2D_NextVertexIndex();

    INX_Render2D_AddVertex(center.x, center.y, 0.5f, 0.5f);

    for (int i = 0; i <= segments; i++) {
        float x = center.x + radius * cosA;
        float y = center.y + radius * sinA;

        float u = 0.5f + 0.5f * cosA;
        float v = 0.5f + 0.5f * sinA;

        INX_Render2D_AddVertex(x, y, u, v);

        float newCos = cosA * cosDelta - sinA * sinDelta;
        float newSin = sinA * cosDelta + cosA * sinDelta;
        cosA = newCos;
        sinA = newSin;
    }

    for (int i = 0; i < segments; i++) {
        INX_Render2D_AddIndex(baseIndex + 0);
        INX_Render2D_AddIndex(baseIndex + 1 + i);
        INX_Render2D_AddIndex(baseIndex + 1 + i + 1);
    }
}

void NX_DrawPieSliceBorder2D(NX_Vec2 center, float radius, float startAngle, float endAngle, int segments, float thickness)
{
    if (segments < 1) segments = 16;

    if (thickness > 0.0f) {
        // Convert thickness to pixels and negate it to avoid doing a redundant conversion
        thickness = -INX_Render2D_ToPixelSize(thickness);
    }

    float angleDiff = endAngle - startAngle;
    angleDiff = NX_WrapRadians(angleDiff);
    if (angleDiff < 0.0f) angleDiff += NX_TAU;

    float deltaAngle = angleDiff / segments;
    float cosDelta = std::cos(deltaAngle);
    float sinDelta = std::sin(deltaAngle);

    float cosA = std::cos(startAngle);
    float sinA = std::sin(startAngle);

    NX_Vec2 start_pt = { center.x + radius * cosA, center.y + radius * sinA };
    NX_DrawLine2D(center, start_pt, thickness);

    NX_Vec2 prev = start_pt;
    for (int i = 1; i <= segments; i++) {
        float newCos = cosA * cosDelta - sinA * sinDelta;
        float newSin = sinA * cosDelta + cosA * sinDelta;
        cosA = newCos;
        sinA = newSin;

        NX_Vec2 curr = { center.x + radius * cosA, center.y + radius * sinA };
        NX_DrawLine2D(prev, curr, thickness);
        prev = curr;
    }

    NX_DrawLine2D(prev, center, thickness);
}

void NX_DrawRing2D(NX_Vec2 center, float innerRadius, float outerRadius, int segments)
{
    if (segments < 3) segments = 32;
    if (innerRadius >= outerRadius) return;

    INX_Render2D_EnsureDrawCall(INX_DrawMode2D::SHAPE, segments * 2, segments * 6);

    uint16_t baseIndex = INX_Render2D_NextVertexIndex();

    float deltaAngle = NX_TAU / segments;
    float cosDelta = std::cos(deltaAngle);
    float sinDelta = std::sin(deltaAngle);

    float cosA = 1.0f;
    float sinA = 0.0f;
    float innerScale = innerRadius / outerRadius;

    for (int i = 0; i < segments; i++) {
        float outerX = center.x + outerRadius * cosA;
        float outerY = center.y + outerRadius * sinA;
        float outerU = 0.5f + 0.5f * cosA;
        float outerV = 0.5f + 0.5f * sinA;
        INX_Render2D_AddVertex(outerX, outerY, outerU, outerV);

        float innerX = center.x + innerRadius * cosA;
        float innerY = center.y + innerRadius * sinA;
        float innerU = 0.5f + 0.5f * innerScale * cosA;
        float innerV = 0.5f + 0.5f * innerScale * sinA;
        INX_Render2D_AddVertex(innerX, innerY, innerU, innerV);

        float newCos = cosA * cosDelta - sinA * sinDelta;
        float newSin = sinA * cosDelta + cosA * sinDelta;
        cosA = newCos;
        sinA = newSin;
    }

    for (int i = 0; i < segments; i++) {
        int next = (i + 1) % segments;
        int outerCurr = baseIndex + i * 2;
        int innerCurr = baseIndex + i * 2 + 1;
        int outerNext = baseIndex + next * 2;
        int innerNext = baseIndex + next * 2 + 1;

        INX_Render2D_AddIndex(outerCurr);
        INX_Render2D_AddIndex(innerCurr);
        INX_Render2D_AddIndex(outerNext);

        INX_Render2D_AddIndex(innerCurr);
        INX_Render2D_AddIndex(innerNext);
        INX_Render2D_AddIndex(outerNext);
    }
}

void NX_DrawRingBorder2D(NX_Vec2 center, float innerRadius, float outerRadius, int segments, float thickness)
{
    if (segments < 3) segments = 32;
    if (innerRadius >= outerRadius) return;

    if (thickness > 0.0f) {
        // Convert thickness to pixels and negate it to avoid doing a redundant conversion
        thickness = -INX_Render2D_ToPixelSize(thickness);
    }

    float deltaAngle = NX_TAU / segments;
    float cosDelta = std::cos(deltaAngle);
    float sinDelta = std::sin(deltaAngle);

    float cosA = 1.0f;
    float sinA = 0.0f;
    NX_Vec2 outerPrev = { center.x + outerRadius * cosA, center.y + outerRadius * sinA };
    NX_Vec2 innerPrev = { center.x + innerRadius * cosA, center.y + innerRadius * sinA };

    for (int i = 1; i <= segments; i++) {
        float newCos = cosA * cosDelta - sinA * sinDelta;
        float newSin = sinA * cosDelta + cosA * sinDelta;
        cosA = newCos;
        sinA = newSin;

        NX_Vec2 outerCurr = { center.x + outerRadius * cosA, center.y + outerRadius * sinA };
        NX_Vec2 innerCurr = { center.x + innerRadius * cosA, center.y + innerRadius * sinA };

        NX_DrawLine2D(outerPrev, outerCurr, thickness);
        NX_DrawLine2D(innerPrev, innerCurr, thickness);

        outerPrev = outerCurr;
        innerPrev = innerCurr;
    }
}

void NX_DrawRingArc2D(NX_Vec2 center, float innerRadius, float outerRadius,
                      float startAngle, float endAngle, int segments)
{
    if (segments < 1) segments = 16;
    if (innerRadius >= outerRadius) return;

    float angleDiff = endAngle - startAngle;
    angleDiff = NX_WrapRadians(angleDiff);
    if (angleDiff < 0.0f) angleDiff += NX_TAU;

    float deltaAngle = angleDiff / segments;

    float cosDelta = std::cos(deltaAngle);
    float sinDelta = std::sin(deltaAngle);

    float cosA = std::cos(startAngle);
    float sinA = std::sin(startAngle);

    INX_Render2D_EnsureDrawCall(INX_DrawMode2D::SHAPE, (segments + 1) * 2, segments * 6);

    uint16_t baseIndex = INX_Render2D_NextVertexIndex();

    for (int i = 0; i <= segments; i++) {
        float outerX = center.x + outerRadius * cosA;
        float outerY = center.y + outerRadius * sinA;
        float outerU = 0.5f + 0.5f * cosA;
        float outerV = 0.5f + 0.5f * sinA;
        INX_Render2D_AddVertex(outerX, outerY, outerU, outerV);

        float innerScale = innerRadius / outerRadius;
        float innerX = center.x + innerRadius * cosA;
        float innerY = center.y + innerRadius * sinA;
        float innerU = 0.5f + 0.5f * innerScale * cosA;
        float innerV = 0.5f + 0.5f * innerScale * sinA;
        INX_Render2D_AddVertex(innerX, innerY, innerU, innerV);

        float newCosA = cosA * cosDelta - sinA * sinDelta;
        float newSinA = sinA * cosDelta + cosA * sinDelta;
        cosA = newCosA;
        sinA = newSinA;
    }

    for (int i = 0; i < segments; i++) {
        int outerCurr = baseIndex + i * 2;
        int innerCurr = baseIndex + i * 2 + 1;
        int outerNext = baseIndex + (i + 1) * 2;
        int innerNext = baseIndex + (i + 1) * 2 + 1;

        INX_Render2D_AddIndex(outerCurr);
        INX_Render2D_AddIndex(innerCurr);
        INX_Render2D_AddIndex(outerNext);

        INX_Render2D_AddIndex(innerCurr);
        INX_Render2D_AddIndex(innerNext);
        INX_Render2D_AddIndex(outerNext);
    }
}

void NX_DrawRingArcBorder2D(NX_Vec2 center, float innerRadius, float outerRadius,
                            float startAngle, float endAngle, int segments, float thickness)
{
    if (segments < 1) segments = 16;
    if (innerRadius >= outerRadius) return;

    if (thickness > 0.0f) {
        // Convert thickness to pixels and negate it to avoid doing a redundant conversion
        thickness = -INX_Render2D_ToPixelSize(thickness);
    }

    float angleDiff = endAngle - startAngle;
    angleDiff = NX_WrapRadians(angleDiff);
    if (angleDiff < 0.0f) angleDiff += NX_TAU;

    float deltaAngle = angleDiff / segments;
    float cosDelta = std::cos(deltaAngle);
    float sinDelta = std::sin(deltaAngle);

    float cosA = std::cos(startAngle);
    float sinA = std::sin(startAngle);

    NX_Vec2 outerStart = { center.x + outerRadius * cosA, center.y + outerRadius * sinA };
    NX_Vec2 innerStart = { center.x + innerRadius * cosA, center.y + innerRadius * sinA };

    NX_DrawLine2D(innerStart, outerStart, thickness);

    NX_Vec2 outerPrev = outerStart;
    NX_Vec2 innerPrev = innerStart;

    for (int i = 1; i <= segments; i++) {
        float newCos = cosA * cosDelta - sinA * sinDelta;
        float newSin = sinA * cosDelta + cosA * sinDelta;
        cosA = newCos;
        sinA = newSin;

        NX_Vec2 outerCurr = { center.x + outerRadius * cosA, center.y + outerRadius * sinA };
        NX_Vec2 innerCurr = { center.x + innerRadius * cosA, center.y + innerRadius * sinA };

        NX_DrawLine2D(outerPrev, outerCurr, thickness);
        NX_DrawLine2D(innerPrev, innerCurr, thickness);

        outerPrev = outerCurr;
        innerPrev = innerCurr;
    }

    NX_DrawLine2D(innerPrev, outerPrev, thickness);
}

void NX_DrawArc2D(NX_Vec2 center, float radius,
                   float startAngle, float endAngle,
                   int segments, float thickness)
{
    if (segments < 1) segments = 16;

    if (thickness > 0.0f) {
        // Convert thickness to pixels and negate it to avoid doing a redundant conversion
        thickness = -INX_Render2D_ToPixelSize(thickness);
    }

    float angleDiff = endAngle - startAngle;
    angleDiff = NX_WrapRadians(angleDiff);
    if (angleDiff < 0.0f) angleDiff += NX_TAU;

    float deltaAngle = angleDiff / segments;

    float cosDelta = std::cos(deltaAngle);
    float sinDelta = std::sin(deltaAngle);

    float x = radius * std::cos(startAngle);
    float y = radius * std::sin(startAngle);

    float prevX = center.x + x;
    float prevY = center.y + y;

    for (int i = 1; i <= segments; i++) {
        float new_x = x * cosDelta - y * sinDelta;
        float new_y = x * sinDelta + y * cosDelta;
        x = new_x;
        y = new_y;

        float curr_x = center.x + x;
        float curr_y = center.y + y;

        NX_DrawLine2D(NX_VEC2(prevX, prevY), NX_VEC2(curr_x, curr_y), thickness);

        prevX = curr_x;
        prevY = curr_y;
    }
}

void NX_DrawBezierQuad2D(NX_Vec2 p0, NX_Vec2 p1, NX_Vec2 p2, int segments, float thickness)
{
    if (segments < 1) segments = 20;

    if (thickness > 0.0f) {
        // Convert thickness to pixels and negate it to avoid doing a redundant conversion
        thickness = -INX_Render2D_ToPixelSize(thickness);
    }

    float dt = 1.0f / segments;
    float dt2 = dt * dt;

    float x  = p0.x;
    float y  = p0.y;
    float dx = 2.0f * (p1.x - p0.x) * dt;
    float dy = 2.0f * (p1.y - p0.y) * dt;

    float d2x = 2.0f * (p0.x - 2.0f * p1.x + p2.x) * dt2;
    float d2y = 2.0f * (p0.y - 2.0f * p1.y + p2.y) * dt2;

    float hd2x = d2x * 0.5f;
    float hd2y = d2y * 0.5f;

    float prevX = x;
    float prevY = y;

    for (int i = 1; i <= segments; i++) {
        x  += dx + hd2x;
        y  += dy + hd2y;
        dx += d2x;
        dy += d2y;

        NX_DrawLine2D(NX_VEC2(prevX, prevY), NX_VEC2(x, y), thickness);

        prevX = x;
        prevY = y;
    }
}

void NX_DrawBezierCubic2D(NX_Vec2 p0, NX_Vec2 p1, NX_Vec2 p2, NX_Vec2 p3, int segments, float thickness)
{
    if (segments < 1) segments = 30;

    if (thickness > 0.0f) {
        // Convert thickness to pixels and negate it to avoid doing a redundant conversion
        thickness = -INX_Render2D_ToPixelSize(thickness);
    }

    float dt  = 1.0f / segments;
    float dt2 = dt * dt;
    float dt3 = dt2 * dt;

    float ax = -p0.x + 3.0f * p1.x - 3.0f * p2.x + p3.x;
    float bx =  3.0f * (p0.x - 2.0f * p1.x + p2.x);
    float cx =  3.0f * (p1.x - p0.x);
    float dx =  p0.x;

    float ay = -p0.y + 3.0f * p1.y - 3.0f * p2.y + p3.y;
    float by =  3.0f * (p0.y - 2.0f * p1.y + p2.y);
    float cy =  3.0f * (p1.y - p0.y);
    float dy =  p0.y;

    float x = dx;
    float dx1 = cx * dt + bx * dt2 + ax * dt3;
    float dx2 = 2.0f * bx * dt2 + 6.0f * ax * dt3;
    float dx3 = 6.0f * ax * dt3;

    float y = dy;
    float dy1 = cy * dt + by * dt2 + ay * dt3;
    float dy2 = 2.0f * by * dt2 + 6.0f * ay * dt3;
    float dy3 = 6.0f * ay * dt3;

    float prevX = x;
    float prevY = y;

    for (int i = 1; i <= segments; ++i) {
        x  += dx1;
        dx1 += dx2;
        dx2 += dx3;

        y  += dy1;
        dy1 += dy2;
        dy2 += dy3;

        NX_DrawLine2D(NX_VEC2(prevX, prevY), NX_VEC2(x, y), thickness);

        prevX = x;
        prevY = y;
    }
}

void NX_DrawSpline2D(const NX_Vec2* points, int count, int segments, float thickness)
{
    if (count < 4) return;
    if (segments < 1) segments = 20;

    if (thickness > 0.0f) {
        // Convert thickness to pixels and negate it to avoid doing a redundant conversion
        thickness = -INX_Render2D_ToPixelSize(thickness);
    }

    for (int i = 1; i < count - 2; i++) {
        NX_Vec2 p0 = points[i - 1];
        NX_Vec2 p1 = points[i];
        NX_Vec2 p2 = points[i + 1];
        NX_Vec2 p3 = points[i + 2];

        float prevX = p1.x;
        float prevY = p1.y;

        for (int j = 1; j <= segments; j++) {
            float t = static_cast<float>(j) / segments;
            float t2 = t * t;
            float t3 = t2 * t;

            // Catmull-Rom coeffs
            float c0 = -0.5f * t3 + t2 - 0.5f * t;
            float c1 =  1.5f * t3 - 2.5f * t2 + 1.0f;
            float c2 = -1.5f * t3 + 2.0f * t2 + 0.5f * t;
            float c3 =  0.5f * t3 - 0.5f * t2;

            float x = c0 * p0.x + c1 * p1.x + c2 * p2.x + c3 * p3.x;
            float y = c0 * p0.y + c1 * p1.y + c2 * p2.y + c3 * p3.y;

            NX_DrawLine2D(NX_VEC2(prevX, prevY), NX_VEC2(x, y), thickness);

            prevX = x;
            prevY = y;
        }
    }
}

void NX_DrawCodepoint2D(int codepoint, NX_Vec2 position, float fontSize)
{
    /* --- Get current font and the glyph data --- */

    const NX_Font* font = INX_Assets.Select(INX_Render2D->currentFont, INX_FontAsset::DEFAULT);
    const INX_Glyph& glyph = INX_GetFontGlyph(font, codepoint);

    /* --- Calculate the scale factor based on font size --- */

    float scale = fontSize / font->baseSize;

    /* --- Calculate the destination of the character with scaling --- */

    float xDst = position.x + glyph.xOffset * scale;
    float yDst = position.y + glyph.yOffset * scale;
    float wDst = glyph.wGlyph * scale;
    float hDst = glyph.hGlyph * scale;

    /* --- Convert the source rect to texture coordinates --- */

    float iwAtlas = 1.0f / font->texture->gpu.width();
    float ihAtlas = 1.0f / font->texture->gpu.height();

    float u0 = glyph.xAtlas * iwAtlas;
    float v0 = glyph.yAtlas * ihAtlas;

    float u1 = u0 + glyph.wGlyph * iwAtlas;
    float v1 = v0 + glyph.hGlyph * ihAtlas;

    /* --- Push the character to the batch with scaled dimensions --- */

    INX_Render2D_EnsureDrawCall(INX_DrawMode2D::TEXT, 4, 6);

    uint16_t baseIndex = INX_Render2D_NextVertexIndex();

    INX_Render2D_AddVertex(xDst, yDst, u0, v0);
    INX_Render2D_AddVertex(xDst, yDst + hDst, u0, v1);
    INX_Render2D_AddVertex(xDst + wDst, yDst + hDst, u1, v1);
    INX_Render2D_AddVertex(xDst + wDst, yDst, u1, v0);

    INX_Render2D_AddIndex(baseIndex + 0);
    INX_Render2D_AddIndex(baseIndex + 1);
    INX_Render2D_AddIndex(baseIndex + 2);

    INX_Render2D_AddIndex(baseIndex + 0);
    INX_Render2D_AddIndex(baseIndex + 2);
    INX_Render2D_AddIndex(baseIndex + 3);
}

void NX_DrawCodepoints2D(const int* codepoints, int length, NX_Vec2 position, float fontSize, NX_Vec2 spacing)
{
    const NX_Font* font = INX_Assets.Select(INX_Render2D->currentFont, INX_FontAsset::DEFAULT);
    float scale = fontSize / font->baseSize;
    NX_Vec2 offset = NX_VEC2_ZERO;

    for (int i = 0; i < length; i++)
    {
        const INX_Glyph& glyph = INX_GetFontGlyph(font, codepoints[i]);

        if (codepoints[i] == '\n') {
            offset.y += (fontSize + spacing.y);
            offset.x = 0.0f;
        }
        else {
            if (codepoints[i] != ' ' && codepoints[i] != '\t') {
                NX_DrawCodepoint2D(codepoints[i], position + offset, fontSize);
            }

            if (glyph.xAdvance == 0) {
                offset.x += (static_cast<float>(glyph.wGlyph) * scale + spacing.x);
            }
            else {
                offset.x += (static_cast<float>(glyph.xAdvance) * scale + spacing.x);
            }
        }
    }
}

void NX_DrawText2D(const char* text, NX_Vec2 position, float fontSize, NX_Vec2 spacing)
{
    const NX_Font* font = INX_Assets.Select(INX_Render2D->currentFont, INX_FontAsset::DEFAULT);
    float scale = fontSize / font->baseSize;
    size_t size = SDL_strlen(text);
    NX_Vec2 offset = NX_VEC2_ZERO;

    for (size_t i = 0; i < size;)
    {
        int codepointByteCount = 0;
        int codepoint = NX_GetCodepointNext(&text[i], &codepointByteCount);

        const INX_Glyph& glyph = INX_GetFontGlyph(font, codepoint);

        if (codepoint == '\n') {
            offset.y += (fontSize + spacing.y);
            offset.x = 0.0f;
        }
        else {
            if (codepoint != ' ' && codepoint != '\t') {
                NX_DrawCodepoint2D(codepoint, position + offset, fontSize);
            }

            if (glyph.xAdvance == 0) {
                offset.x += (static_cast<float>(glyph.wGlyph) * scale + spacing.x);
            }
            else {
                offset.x += (static_cast<float>(glyph.xAdvance) * scale + spacing.x);
            }
        }

        i += codepointByteCount;
    }
}
