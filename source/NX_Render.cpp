/* NX_Render.cpp -- API definition for Nexium's render module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/NX_Render.h>
#include <NX/NX_Image.h>
#include <NX/NX_Core.h>
#include <NX/NX_Math.h>

#include "./Core/NX_InternalLog.hpp"

#include "./Render/NX_RenderState.hpp"
#include "./Render/NX_InstanceBuffer.hpp"
#include "./Render/NX_Texture.hpp"
#include "./Render/NX_Font.hpp"
#include "./Detail/Helper.hpp"

/* === Texture - Public API === */

NX_Texture* NX_CreateTexture(const NX_Image* image)
{
    if (image == nullptr) {
        NX_INTERNAL_LOG(E, "RENDER: Failed to load texture; Image is null");
        return nullptr;
    }
    return gRender->textures.createTexture(*image);
}

NX_Texture* NX_LoadTexture(const char* filePath)
{
    NX_Image image = NX_LoadImage(filePath);
    if (image.pixels == nullptr) return nullptr;

    NX_Texture* texture = NX_CreateTexture(&image);
    NX_DestroyImage(&image);

    return texture;
}

NX_Texture* NX_LoadTextureAsData(const char* filePath)
{
    NX_Image image = NX_LoadImageAsData(filePath);
    NX_Texture* texture = NX_CreateTexture(&image);
    NX_DestroyImage(&image);
    return texture;
}

void NX_DestroyTexture(NX_Texture* texture)
{
    gRender->textures.destroyTexture(texture);
}

void NX_SetDefaultTextureFilter(NX_TextureFilter filter)
{
    gRender->textures.setDefaultFilter(filter);
}

void NX_SetDefaultTextureAnisotropy(float anisotropy)
{
    gRender->textures.setDefaultAnisotropy(anisotropy);
}

void NX_SetTextureParameters(NX_Texture* texture, NX_TextureFilter filter, NX_TextureWrap wrap, float anisotropy)
{
    texture->setParameters(filter, wrap, anisotropy);
}

void NX_SetTextureAnisotropy(NX_Texture* texture, float anisotropy)
{
    texture->setAnisotropy(anisotropy);
}

void NX_SetTextureFilter(NX_Texture* texture, NX_TextureFilter filter)
{
    texture->setFilter(filter);
}

void NX_SetTextureWrap(NX_Texture* texture, NX_TextureWrap wrap)
{
    texture->setWrap(wrap);
}

void NX_GenerateMipmap(NX_Texture* texture)
{
    texture->generateMipmap();
}

void NX_QueryTexture(NX_Texture* texture, int* w, int* h)
{
    if (w) *w = texture->width();
    if (h) *h = texture->height();
}

/* === Font - Public API === */

NX_Font* NX_LoadFont(const char* filePath, NX_FontType type, int baseSize, int* codepoints, int codepointCount)
{
    size_t dataSize = 0;
    void* fileData = NX_LoadFile(filePath, &dataSize);
    NX_Font* font = NX_LoadFontFromMem(fileData, dataSize, type, baseSize, codepoints, codepointCount);
    SDL_free(fileData);
    return font;
}

NX_Font* NX_LoadFontFromMem(const void* fileData, size_t dataSize, NX_FontType type, int baseSize, int* codepoints, int codepointCount)
{
    return gRender->fonts.create(fileData, dataSize, type, baseSize, codepoints, codepointCount);
}

void NX_DestroyFont(NX_Font* font)
{
    gRender->fonts.destroy(font);
}

NX_Vec2 NX_MeasureCodepoints(const NX_Font* font, const int* codepoints, int length, float fontSize, NX_Vec2 spacing)
{
    const NX_Font& fnt = font ? *font : gRender->assets.font();
    return fnt.measureCodepoints(codepoints, length, fontSize, spacing);
}

NX_Vec2 NX_MeasureText(const NX_Font* font, const char* text, float fontSize, NX_Vec2 spacing)
{
    const NX_Font& fnt = font ? *font : gRender->assets.font();
    return fnt.measureText(text, fontSize, spacing);
}

/* === RenderTexture - Public API === */

NX_RenderTexture* NX_CreateRenderTexture(int w, int h)
{
    return gRender->textures.createRenderTexture(w, h);
}

void NX_DestroyRenderTexture(NX_RenderTexture* target)
{
    gRender->textures.destroyRenderTexture(target);
}

NX_Texture* NX_GetRenderTexture(NX_RenderTexture* target)
{
    return &target->texture();
}

void NX_BlitRenderTexture(const NX_RenderTexture* target, int xDst, int yDst, int wDst, int hDst, bool linear)
{
    target->blit(xDst, yDst, wDst, hDst, linear);
}

/* === Shader - Public API === */

NX_Shader* NX_CreateShader(const char* vertCode, const char* fragCode)
{
    return gRender->programs.createShader(vertCode, fragCode);
}

NX_Shader* NX_LoadShader(const char* vertFile, const char* fragFile)
{
    char* vertCode = vertFile ? NX_LoadFileText(vertFile) : nullptr;
    char* fragCode = fragFile ? NX_LoadFileText(fragFile) : nullptr;

    NX_Shader* shader = gRender->programs.createShader(vertCode, fragCode);

    SDL_free(vertCode);
    SDL_free(fragCode);

    return shader;
}

void NX_DestroyShader(NX_Shader* shader)
{
    gRender->programs.destroyShader(shader);
}

void NX_SetShaderTexture(NX_Shader* shader, int slot, const NX_Texture* texture)
{
    shader->setTexture(slot, texture ? &texture->gpuTexture() : nullptr);
}

void NX_UpdateStaticShaderBuffer(NX_Shader* shader, size_t offset, size_t size, const void* data)
{
    shader->updateStaticBuffer(offset, size, data);
}

void NX_UpdateDynamicShaderBuffer(NX_Shader* shader, size_t size, const void* data)
{
    shader->updateDynamicBuffer(size, data);
}

/* === Draw2D - Public API === */

void NX_Begin2D(NX_RenderTexture* target)
{
    gRender->overlay.setRenderTexture(target);
    NX_IVec2 size = target ? target->framebuffer().dimensions() : NX_GetWindowSize();
    gRender->overlay.setProjection(NX_Mat4Ortho(0, size.x, size.y, 0, 0, 1));
    gRender->overlay.clear();
}

void NX_End2D(void)
{
    gRender->overlay.flush();
    gRender->overlay.blit();
}

void NX_SetColor2D(NX_Color color)
{
    gRender->overlay.setColor(color);
}

void NX_SetTexture2D(const NX_Texture* texture)
{
    gRender->overlay.setTexture(texture);
}

void NX_SetFont2D(const NX_Font* font)
{
    gRender->overlay.setFont(font);
}

void NX_SetShader2D(NX_Shader* shader)
{
    gRender->overlay.setShader(shader);
}

void NX_Push2D(void)
{
    gRender->overlay.push();
}

void NX_Pop2D(void)
{
    gRender->overlay.pop();
}

void NX_Translate2D(NX_Vec2 translation)
{
    gRender->overlay.translate(translation);
}

void NX_Rotate2D(float radians)
{
    gRender->overlay.rotate(radians);
}

void NX_Scale2D(NX_Vec2 scale)
{
    gRender->overlay.scale(scale);
}

void NX_DrawShape2D(NX_PrimitiveType type, const NX_Vec2* points, int pointCount, float thickness)
{
    if (thickness > 0.0f) {
        // Convert thickness to pixels and negate it to avoid doing a redundant conversion
        thickness = -gRender->overlay.toPixelSize(thickness);
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
        thickness = -gRender->overlay.toPixelSize(thickness);
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
    const NX_Color& color = gRender->overlay.currentColor();

    NX_Vertex2D v0 = { .position = p0, .texcoord = NX_VEC2_ZERO, .color = color };
    NX_Vertex2D v1 = { .position = p1, .texcoord = NX_VEC2_ONE, .color = color };

    NX_DrawLineEx2D(&v0, &v1, thickness);
}

void NX_DrawLineEx2D(const NX_Vertex2D* v0, const NX_Vertex2D* v1, float thickness)
{
    // NOTE: We draw the lines with quads for compatibility reasons
    gRender->overlay.ensureDrawCall(overlay::DrawCall::Mode::SHAPE, 4, 6);

    /* --- Calculation of pixel thickness if necessary --- */

    if (thickness > 0.0f) {
        thickness = gRender->overlay.toPixelSize(thickness);
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

    uint16_t baseIndex = gRender->overlay.nextVertexIndex();

    /* --- Adding vertices and indices --- */

    gRender->overlay.addVertex(NX_Vertex2D{
        .position = p0 + NX_VEC2(nx, ny),
        .texcoord = v0->texcoord,
        .color = v0->color
    });

    gRender->overlay.addVertex(NX_Vertex2D{
        .position = p0 - NX_VEC2(nx, ny),
        .texcoord = v0->texcoord,
        .color = v0->color
    });

    gRender->overlay.addVertex(NX_Vertex2D{
        .position = p1 - NX_VEC2(nx, ny),
        .texcoord = v1->texcoord,
        .color = v1->color
    });

    gRender->overlay.addVertex(NX_Vertex2D{
        .position = p1 + NX_VEC2(nx, ny),
        .texcoord = v1->texcoord,
        .color = v1->color
    });

    // Triangle 1: 0, 1, 2
    gRender->overlay.addIndex(baseIndex + 0);
    gRender->overlay.addIndex(baseIndex + 1);
    gRender->overlay.addIndex(baseIndex + 2);

    // Triangle 2: 0, 2, 3
    gRender->overlay.addIndex(baseIndex + 0);
    gRender->overlay.addIndex(baseIndex + 2);
    gRender->overlay.addIndex(baseIndex + 3);
}

void NX_DrawTriangle2D(NX_Vec2 p0, NX_Vec2 p1, NX_Vec2 p2)
{
    const NX_Color& color = gRender->overlay.currentColor();

    NX_Vertex2D v0 = { .position = p0, .texcoord = NX_VEC2(0.0f, 0.0f), .color = color };
    NX_Vertex2D v1 = { .position = p1, .texcoord = NX_VEC2(0.5f, 0.5f), .color = color };
    NX_Vertex2D v2 = { .position = p2, .texcoord = NX_VEC2(1.0f, 1.0f), .color = color };

    NX_DrawTriangleEx2D(&v0, &v1, &v2);
}

void NX_DrawTriangleEx2D(const NX_Vertex2D* v0, const NX_Vertex2D* v1, const NX_Vertex2D* v2)
{
    gRender->overlay.ensureDrawCall(overlay::DrawCall::Mode::SHAPE, 3, 3);

    uint16_t baseIndex = gRender->overlay.nextVertexIndex();

    gRender->overlay.addVertex(*v0);
    gRender->overlay.addVertex(*v1);
    gRender->overlay.addVertex(*v2);

    gRender->overlay.addIndex(baseIndex + 0);
    gRender->overlay.addIndex(baseIndex + 1);
    gRender->overlay.addIndex(baseIndex + 2);
}

void NX_DrawQuad2D(NX_Vec2 p0, NX_Vec2 p1, NX_Vec2 p2, NX_Vec2 p3)
{
    const NX_Color& color = gRender->overlay.currentColor();

    NX_Vertex2D v0 = { .position = p0, .texcoord = NX_VEC2(0.0f, 0.0f), .color = color };
    NX_Vertex2D v1 = { .position = p1, .texcoord = NX_VEC2(1.0f, 0.0f), .color = color };
    NX_Vertex2D v2 = { .position = p2, .texcoord = NX_VEC2(1.0f, 1.0f), .color = color };
    NX_Vertex2D v3 = { .position = p3, .texcoord = NX_VEC2(0.0f, 1.0f), .color = color };

    NX_DrawQuadEx2D(&v0, &v1, &v2, &v3);
}

void NX_DrawQuadEx2D(const NX_Vertex2D* v0, const NX_Vertex2D* v1, const NX_Vertex2D* v2, const NX_Vertex2D* v3)
{
    gRender->overlay.ensureDrawCall(overlay::DrawCall::Mode::SHAPE, 4, 6);

    uint16_t baseIndex = gRender->overlay.nextVertexIndex();

    gRender->overlay.addVertex(*v0);
    gRender->overlay.addVertex(*v1);
    gRender->overlay.addVertex(*v2);
    gRender->overlay.addVertex(*v3);

    gRender->overlay.addIndex(baseIndex + 0);
    gRender->overlay.addIndex(baseIndex + 1);
    gRender->overlay.addIndex(baseIndex + 2);

    gRender->overlay.addIndex(baseIndex + 0);
    gRender->overlay.addIndex(baseIndex + 2);
    gRender->overlay.addIndex(baseIndex + 3);
}

void NX_DrawRect2D(float x, float y, float w, float h)
{
    gRender->overlay.ensureDrawCall(overlay::DrawCall::Mode::SHAPE, 4, 6);

    uint16_t baseIndex = gRender->overlay.nextVertexIndex();

    gRender->overlay.addVertex(x, y, 0.0f, 0.0f);
    gRender->overlay.addVertex(x + w, y, 1.0f, 0.0f);
    gRender->overlay.addVertex(x + w, y + h, 1.0f, 1.0f);
    gRender->overlay.addVertex(x, y + h, 0.0f, 1.0f);

    gRender->overlay.addIndex(baseIndex + 0);
    gRender->overlay.addIndex(baseIndex + 1);
    gRender->overlay.addIndex(baseIndex + 2);

    gRender->overlay.addIndex(baseIndex + 0);
    gRender->overlay.addIndex(baseIndex + 2);
    gRender->overlay.addIndex(baseIndex + 3);
}

void NX_DrawRectBorder2D(float x, float y, float w, float h, float thickness)
{
    const NX_Color& color = gRender->overlay.currentColor();

    NX_Vertex2D v0 = { .position = {x,     y},     .texcoord = {0.0f, 0.0f}, .color = color };
    NX_Vertex2D v1 = { .position = {x + w, y},     .texcoord = {1.0f, 0.0f}, .color = color };
    NX_Vertex2D v2 = { .position = {x + w, y + h}, .texcoord = {1.0f, 1.0f}, .color = color };
    NX_Vertex2D v3 = { .position = {x,     y + h}, .texcoord = {0.0f, 1.0f}, .color = color };

    if (thickness > 0.0f) {
        // Convert thickness to pixels and negate it to avoid doing a redundant conversion
        thickness = -gRender->overlay.toPixelSize(thickness);
    }

    NX_DrawLineEx2D(&v0, &v1, thickness);
    NX_DrawLineEx2D(&v1, &v2, thickness);
    NX_DrawLineEx2D(&v2, &v3, thickness);
    NX_DrawLineEx2D(&v3, &v0, thickness);
}

void NX_DrawRectRounded2D(float x, float y, float w, float h, float radius, int segments)
{
    radius = fminf(radius, fminf(w * 0.5f, h * 0.5f));

    if (radius <= 0.0f) {
        NX_DrawQuad2D(NX_VEC2(x, y), NX_VEC2(x + w, y), NX_VEC2(x + w, y + h), NX_VEC2(x, y + h));
        return;
    }

    /* --- Calculation of vertices and indices --- */

    int cornerVertices = segments + 1;
    int totalVertices = 4 * cornerVertices + 8;
    int totalIndices = 4 * segments * 3 + 12;

    gRender->overlay.ensureDrawCall(overlay::DrawCall::Mode::SHAPE, totalVertices, totalIndices);

    uint16_t baseIndex = gRender->overlay.nextVertexIndex();
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
        gRender->overlay.addVertex(cx, cy, 0.5f, 0.5f);
        for (int i = 0; i <= segments; i++) {
            float angle = startAngle + i * angleStep;
            gRender->overlay.addVertex(
                cx + cosf(angle) * radius,
                cy + sinf(angle) * radius,
                0.5f, 0.5f
            );
            if (i > 0) {
                gRender->overlay.addIndex(baseIndex + centerIdx);
                gRender->overlay.addIndex(baseIndex + currentIndex - 1);
                gRender->overlay.addIndex(baseIndex + currentIndex);
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
            gRender->overlay.addVertex(
                rectData[rect][i * 2],
                rectData[rect][i * 2 + 1],
                uvs[i][0], uvs[i][1]
            );
        }
        uint16_t indices[6] = {0, 1, 2, 0, 2, 3};
        for (int i = 0; i < 6; i++) {
            gRender->overlay.addIndex(baseIndex + rectStart + indices[i]);
        }
        currentIndex += 4;
    }
}

void NX_DrawRectRoundedBorder2D(float x, float y, float w, float h, float radius, int segments, float thickness)
{
    /* --- Calculation of the minimum radius required per corner --- */

    radius = fminf(radius, fminf(w * 0.5f, h * 0.5f));

    if (radius <= 0.0f) {
        NX_DrawRectBorder2D(x, y, w, h, thickness);
        return;
    }

    /* --- Calculation of pixel thickness if necessary --- */

    if (thickness > 0.0f) {
        thickness = gRender->overlay.toPixelSize(thickness);
    }
    else {
        thickness = -thickness;
    }

    /* --- Pre-calculation ​​and declaration of the draw call that will be made */

    float halfThickness = thickness * 0.5f;
    float innerRadius = fmaxf(0.0f, radius - halfThickness);
    float outerRadius = radius + halfThickness;

    int arcVertices = (segments + 1) * 2;
    int totalVertices = 4 * arcVertices + 16;
    int totalIndices = 4 * segments * 6 + 48;

    gRender->overlay.ensureDrawCall(overlay::DrawCall::Mode::SHAPE, totalVertices, totalIndices);

    uint16_t baseIndex = gRender->overlay.nextVertexIndex();
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
            float cosA = cosf(angle);
            float sinA = sinf(angle);

            // Vertices inner/outer
            gRender->overlay.addVertex(cx + cosA * innerRadius, cy + sinA * innerRadius, 0.5f, 0.5f);
            gRender->overlay.addVertex(cx + cosA * outerRadius, cy + sinA * outerRadius, 0.5f, 0.5f);

            if (i > 0) {
                uint16_t base = baseIndex + cornerStart + (i - 1) * 2;
                // Quad with 2 triangles
                gRender->overlay.addIndex(base);
                gRender->overlay.addIndex(base + 1);
                gRender->overlay.addIndex(base + 2);
                gRender->overlay.addIndex(base + 2);
                gRender->overlay.addIndex(base + 1);
                gRender->overlay.addIndex(base + 3);
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
            gRender->overlay.addVertex(
                straightData[seg][i * 2],
                straightData[seg][i * 2 + 1],
                uvs[i][0], uvs[i][1]
            );
        }
        uint16_t indices[6] = {0, 1, 2, 0, 2, 3};
        for (int i = 0; i < 6; i++) {
            gRender->overlay.addIndex(baseIndex + segStart + indices[i]);
        }
        currentIndex += 4;
    }
}

void NX_DrawCircle2D(NX_Vec2 center, float radius, int segments)
{
    if (segments < 3) segments = 32;

    gRender->overlay.ensureDrawCall(overlay::DrawCall::Mode::SHAPE, segments + 1, segments * 3);

    uint16_t baseIndex = gRender->overlay.nextVertexIndex();

    gRender->overlay.addVertex(center.x, center.y, 0.5f, 0.5f);

    float delta = NX_TAU / segments;
    float cosDelta = cosf(delta);
    float sinDelta = sinf(delta);

    float inv2r = 1.0f / (2.0f * radius);

    float cx = radius;
    float cy = 0.0f;

    for (int i = 0; i < segments; i++) {
        float x = center.x + cx;
        float y = center.y + cy;

        float u = 0.5f + cx * inv2r;
        float v = 0.5f + cy * inv2r;

        gRender->overlay.addVertex(x, y, u, v);

        float new_cx = cx * cosDelta - cy * sinDelta;
        cy = cx * sinDelta + cy * cosDelta;
        cx = new_cx;
    }

    for (int i = 0; i < segments; i++) {
        int next = (i + 1) % segments;
        gRender->overlay.addIndex(baseIndex + 0);
        gRender->overlay.addIndex(baseIndex + 1 + i);
        gRender->overlay.addIndex(baseIndex + 1 + next);
    }
}

void NX_DrawCircleBorder2D(NX_Vec2 p, float radius, int segments, float thickness)
{
    if (segments < 3) segments = 32;

    if (thickness > 0.0f) {
        // Convert thickness to pixels and negate it to avoid doing a redundant conversion
        thickness = -gRender->overlay.toPixelSize(thickness);
    }

    float delta = NX_TAU / segments;
    float cosDelta = cosf(delta);
    float sinDelta = sinf(delta);

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

    gRender->overlay.ensureDrawCall(overlay::DrawCall::Mode::SHAPE, segments + 1, segments * 3);

    uint16_t baseIndex = gRender->overlay.nextVertexIndex();

    gRender->overlay.addVertex(center.x, center.y, 0.5f, 0.5f);

    float delta = NX_TAU / segments;
    float cosDelta = cosf(delta);
    float sinDelta = sinf(delta);

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

        gRender->overlay.addVertex(x, y, u, v);

        float newUX = ux * cosDelta - uy * sinDelta;
        uy = ux * sinDelta + uy * cosDelta;
        ux = newUX;
    }

    for (int i = 0; i < segments; i++) {
        int next = (i + 1) % segments;
        gRender->overlay.addIndex(baseIndex + 0);
        gRender->overlay.addIndex(baseIndex + 1 + i);
        gRender->overlay.addIndex(baseIndex + 1 + next);
    }
}

void NX_DrawEllipseBorder2D(NX_Vec2 p, NX_Vec2 r, int segments, float thickness)
{
    if (segments < 3) segments = 32;

    if (thickness > 0.0f) {
        // Convert thickness to pixels and negate it to avoid doing a redundant conversion
        thickness = -gRender->overlay.toPixelSize(thickness);
    }

    float delta = NX_TAU / segments;
    float cosDelta = cosf(delta);
    float sinDelta = sinf(delta);

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

    float cosDelta = cosf(deltaAngle);
    float sinDelta = sinf(deltaAngle);

    float cosA = cosf(startAngle);
    float sinA = sinf(startAngle);

    gRender->overlay.ensureDrawCall(overlay::DrawCall::Mode::SHAPE, segments + 2, segments * 3);

    uint16_t baseIndex = gRender->overlay.nextVertexIndex();

    gRender->overlay.addVertex(center.x, center.y, 0.5f, 0.5f);

    for (int i = 0; i <= segments; i++) {
        float x = center.x + radius * cosA;
        float y = center.y + radius * sinA;

        float u = 0.5f + 0.5f * cosA;
        float v = 0.5f + 0.5f * sinA;

        gRender->overlay.addVertex(x, y, u, v);

        float newCos = cosA * cosDelta - sinA * sinDelta;
        float newSin = sinA * cosDelta + cosA * sinDelta;
        cosA = newCos;
        sinA = newSin;
    }

    for (int i = 0; i < segments; i++) {
        gRender->overlay.addIndex(baseIndex + 0);
        gRender->overlay.addIndex(baseIndex + 1 + i);
        gRender->overlay.addIndex(baseIndex + 1 + i + 1);
    }
}

void NX_DrawPieSliceBorder2D(NX_Vec2 center, float radius, float startAngle, float endAngle, int segments, float thickness)
{
    if (segments < 1) segments = 16;

    if (thickness > 0.0f) {
        // Convert thickness to pixels and negate it to avoid doing a redundant conversion
        thickness = -gRender->overlay.toPixelSize(thickness);
    }

    float angleDiff = endAngle - startAngle;
    angleDiff = NX_WrapRadians(angleDiff);
    if (angleDiff < 0.0f) angleDiff += NX_TAU;

    float deltaAngle = angleDiff / segments;
    float cosDelta = cosf(deltaAngle);
    float sinDelta = sinf(deltaAngle);

    float cosA = cosf(startAngle);
    float sinA = sinf(startAngle);

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

    gRender->overlay.ensureDrawCall(overlay::DrawCall::Mode::SHAPE, segments * 2, segments * 6);

    uint16_t baseIndex = gRender->overlay.nextVertexIndex();

    float deltaAngle = NX_TAU / segments;
    float cosDelta = cosf(deltaAngle);
    float sinDelta = sinf(deltaAngle);

    float cosA = 1.0f;
    float sinA = 0.0f;
    float innerScale = innerRadius / outerRadius;

    for (int i = 0; i < segments; i++) {
        float outerX = center.x + outerRadius * cosA;
        float outerY = center.y + outerRadius * sinA;
        float outerU = 0.5f + 0.5f * cosA;
        float outerV = 0.5f + 0.5f * sinA;
        gRender->overlay.addVertex(outerX, outerY, outerU, outerV);

        float innerX = center.x + innerRadius * cosA;
        float innerY = center.y + innerRadius * sinA;
        float innerU = 0.5f + 0.5f * innerScale * cosA;
        float innerV = 0.5f + 0.5f * innerScale * sinA;
        gRender->overlay.addVertex(innerX, innerY, innerU, innerV);

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

        gRender->overlay.addIndex(outerCurr);
        gRender->overlay.addIndex(innerCurr);
        gRender->overlay.addIndex(outerNext);

        gRender->overlay.addIndex(innerCurr);
        gRender->overlay.addIndex(innerNext);
        gRender->overlay.addIndex(outerNext);
    }
}

void NX_DrawRingBorder2D(NX_Vec2 center, float innerRadius, float outerRadius, int segments, float thickness)
{
    if (segments < 3) segments = 32;
    if (innerRadius >= outerRadius) return;

    if (thickness > 0.0f) {
        // Convert thickness to pixels and negate it to avoid doing a redundant conversion
        thickness = -gRender->overlay.toPixelSize(thickness);
    }

    float deltaAngle = NX_TAU / segments;
    float cosDelta = cosf(deltaAngle);
    float sinDelta = sinf(deltaAngle);

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

    float cosDelta = cosf(deltaAngle);
    float sinDelta = sinf(deltaAngle);

    float cosA = cosf(startAngle);
    float sinA = sinf(startAngle);

    gRender->overlay.ensureDrawCall(overlay::DrawCall::Mode::SHAPE, (segments + 1) * 2, segments * 6);

    uint16_t baseIndex = gRender->overlay.nextVertexIndex();

    for (int i = 0; i <= segments; i++) {
        float outerX = center.x + outerRadius * cosA;
        float outerY = center.y + outerRadius * sinA;
        float outerU = 0.5f + 0.5f * cosA;
        float outerV = 0.5f + 0.5f * sinA;
        gRender->overlay.addVertex(outerX, outerY, outerU, outerV);

        float innerScale = innerRadius / outerRadius;
        float innerX = center.x + innerRadius * cosA;
        float innerY = center.y + innerRadius * sinA;
        float innerU = 0.5f + 0.5f * innerScale * cosA;
        float innerV = 0.5f + 0.5f * innerScale * sinA;
        gRender->overlay.addVertex(innerX, innerY, innerU, innerV);

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

        gRender->overlay.addIndex(outerCurr);
        gRender->overlay.addIndex(innerCurr);
        gRender->overlay.addIndex(outerNext);

        gRender->overlay.addIndex(innerCurr);
        gRender->overlay.addIndex(innerNext);
        gRender->overlay.addIndex(outerNext);
    }
}

void NX_DrawRingArcBorder2D(NX_Vec2 center, float innerRadius, float outerRadius,
                            float startAngle, float endAngle, int segments, float thickness)
{
    if (segments < 1) segments = 16;
    if (innerRadius >= outerRadius) return;

    if (thickness > 0.0f) {
        // Convert thickness to pixels and negate it to avoid doing a redundant conversion
        thickness = -gRender->overlay.toPixelSize(thickness);
    }

    float angleDiff = endAngle - startAngle;
    angleDiff = NX_WrapRadians(angleDiff);
    if (angleDiff < 0.0f) angleDiff += NX_TAU;

    float deltaAngle = angleDiff / segments;
    float cosDelta = cosf(deltaAngle);
    float sinDelta = sinf(deltaAngle);

    float cosA = cosf(startAngle);
    float sinA = sinf(startAngle);

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
        thickness = -gRender->overlay.toPixelSize(thickness);
    }

    float angleDiff = endAngle - startAngle;
    angleDiff = NX_WrapRadians(angleDiff);
    if (angleDiff < 0.0f) angleDiff += NX_TAU;

    float deltaAngle = angleDiff / segments;

    float cosDelta = cosf(deltaAngle);
    float sinDelta = sinf(deltaAngle);

    float x = radius * cosf(startAngle);
    float y = radius * sinf(startAngle);

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
        thickness = -gRender->overlay.toPixelSize(thickness);
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
        thickness = -gRender->overlay.toPixelSize(thickness);
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
        thickness = -gRender->overlay.toPixelSize(thickness);
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

    const NX_Font& font = gRender->overlay.currentFont();
    const NX_Font::Glyph& glyph = font.getGlyph(codepoint);

    /* --- Calculate the scale factor based on font size --- */

    float scale = fontSize / font.baseSize();

    /* --- Calculate the destination of the character with scaling --- */

    float xDst = position.x + glyph.xOffset * scale;
    float yDst = position.y + glyph.yOffset * scale;
    float wDst = glyph.wGlyph * scale;
    float hDst = glyph.hGlyph * scale;

    /* --- Convert the source rect to texture coordinates --- */

    float iwAtlas = 1.0f / font.gpuTexture().width();
    float ihAtlas = 1.0f / font.gpuTexture().height();

    float u0 = glyph.xAtlas * iwAtlas;
    float v0 = glyph.yAtlas * ihAtlas;

    float u1 = u0 + glyph.wGlyph * iwAtlas;
    float v1 = v0 + glyph.hGlyph * ihAtlas;

    /* --- Push the character to the batch with scaled dimensions --- */

    gRender->overlay.ensureDrawCall(overlay::DrawCall::Mode::TEXT, 4, 6);

    uint16_t baseIndex = gRender->overlay.nextVertexIndex();

    gRender->overlay.addVertex(xDst, yDst, u0, v0);
    gRender->overlay.addVertex(xDst, yDst + hDst, u0, v1);
    gRender->overlay.addVertex(xDst + wDst, yDst + hDst, u1, v1);
    gRender->overlay.addVertex(xDst + wDst, yDst, u1, v0);

    gRender->overlay.addIndex(baseIndex + 0);
    gRender->overlay.addIndex(baseIndex + 1);
    gRender->overlay.addIndex(baseIndex + 2);

    gRender->overlay.addIndex(baseIndex + 0);
    gRender->overlay.addIndex(baseIndex + 2);
    gRender->overlay.addIndex(baseIndex + 3);
}

void NX_DrawCodepoints2D(const int* codepoints, int length, NX_Vec2 position, float fontSize, NX_Vec2 spacing)
{
    const NX_Font& font = gRender->overlay.currentFont();
    float scale = fontSize / font.baseSize();
    NX_Vec2 offset = NX_VEC2_ZERO;

    for (int i = 0; i < length; i++)
    {
        const NX_Font::Glyph& glyph = font.getGlyph(codepoints[i]);

        if (codepoints[i] == '\n') {
            offset.y += (fontSize + spacing.y);
            offset.x = 0.0f;
        }
        else {
            if (codepoints[i] != ' ' && codepoints[i] != '\t') {
                NX_DrawCodepoint2D(codepoints[i], position + offset, fontSize);
            }

            if (glyph.xAdvance == 0) {
                offset.x += ((float)(glyph.wGlyph) * scale + spacing.x);
            }
            else {
                offset.x += ((float)(glyph.xAdvance) * scale + spacing.x);
            }
        }
    }
}

void NX_DrawText2D(const char* text, NX_Vec2 position, float fontSize, NX_Vec2 spacing)
{
    const NX_Font& font = gRender->overlay.currentFont();
    float scale = fontSize / font.baseSize();
    int size = (int)strlen(text);
    NX_Vec2 offset = NX_VEC2_ZERO;

    for (int i = 0; i < size;)
    {
        int codepointByteCount = 0;
        int codepoint = NX_GetCodepointNext(&text[i], &codepointByteCount);

        const NX_Font::Glyph& glyph = font.getGlyph(codepoint);

        if (codepoint == '\n') {
            offset.y += (fontSize + spacing.y);
            offset.x = 0.0f;
        }
        else {
            if (codepoint != ' ' && codepoint != '\t') {
                NX_DrawCodepoint2D(codepoint, position + offset, fontSize);
            }

            if (glyph.xAdvance == 0) {
                offset.x += ((float)(glyph.wGlyph) * scale + spacing.x);
            }
            else {
                offset.x += ((float)(glyph.xAdvance) * scale + spacing.x);
            }
        }

        i += codepointByteCount;
    }
}

/* === Draw3D - Public API === */

void NX_Begin3D(const NX_Camera* camera, const NX_Environment* env, const NX_RenderTexture* target)
{
    gRender->scene.begin(
        camera ? *camera : NX_GetDefaultCamera(),
        env ? *env : NX_GetDefaultEnvironment(),
        target
    );
}

void NX_End3D(void)
{
    gRender->scene.end();
}

void NX_DrawMesh3D(const NX_Mesh* mesh, const NX_Material* material, const NX_Transform* transform)
{
    gRender->scene.drawCalls().push(
        mesh, nullptr, 0,
        material ? *material : NX_GetDefaultMaterial(),
        transform ? *transform : NX_TRANSFORM_IDENTITY
    );
}

void NX_DrawMeshInstanced3D(const NX_Mesh* mesh, const NX_InstanceBuffer* instances, int instanceCount,
                            const NX_Material* material, const NX_Transform* transform)
{
    gRender->scene.drawCalls().push(
        mesh, instances, instanceCount,
        material ? *material : NX_GetDefaultMaterial(),
        transform ? *transform : NX_TRANSFORM_IDENTITY
    );
}

void NX_DrawDynamicMesh3D(const NX_DynamicMesh* dynMesh, const NX_Material* material, const NX_Transform* transform)
{
    gRender->scene.drawCalls().push(
        dynMesh, nullptr, 0,
        material ? *material : NX_GetDefaultMaterial(),
        transform ? *transform : NX_TRANSFORM_IDENTITY
    );
}

void NX_DrawDynamicMeshInstanced3D(const NX_DynamicMesh* dynMesh, const NX_InstanceBuffer* instances, int instanceCount,
                                   const NX_Material* material, const NX_Transform* transform)
{
    gRender->scene.drawCalls().push(
        dynMesh, instances, instanceCount,
        material ? *material : NX_GetDefaultMaterial(),
        transform ? *transform : NX_TRANSFORM_IDENTITY
    );
}

void NX_DrawModel3D(const NX_Model* model, const NX_Transform* transform)
{
    gRender->scene.drawCalls().push(
        *model, nullptr, 0,
        transform ? *transform : NX_TRANSFORM_IDENTITY
    );
}

void NX_DrawModelInstanced3D(const NX_Model* model, const NX_InstanceBuffer* instances, int instanceCount, const NX_Transform* transform)
{
    gRender->scene.drawCalls().push(
        *model, instances, instanceCount,
        transform ? *transform : NX_TRANSFORM_IDENTITY
    );
}

/* === Camera - Public API === */

NX_Camera NX_GetDefaultCamera(void)
{
    return NX_Camera {
        .position = NX_VEC3_ZERO,
        .rotation = NX_QUAT_IDENTITY,
        .nearPlane = 0.05f,
        .farPlane = 4000.0f,
        .fov = 60.0f * NX_DEG2RAD,
        .projection = NX_PROJECTION_PERSPECTIVE,
        .cullMask = NX_LAYER_ALL,
    };
}

void NX_UpdateCameraOrbital(NX_Camera* camera, NX_Vec3 center, float distance, float height, float rotation)
{
    camera->position.x = center.x + distance * cosf(rotation);
    camera->position.z = center.z + distance * sinf(rotation);
    camera->position.y = center.y + height;

    camera->rotation = NX_QuatLookAt(camera->position, center, NX_VEC3_UP);
}

void NX_UpdateCameraFree(NX_Camera* camera, NX_Vec3 movement, NX_Vec3 rotation, float maxPitch)
{
    /* --- Rotation (Euler) --- */

    NX_Vec3 euler = NX_QuatToEuler(camera->rotation);

    euler.x += rotation.x;
    euler.y += rotation.y;
    euler.z += rotation.z;

    if (maxPitch < 0.0f) maxPitch = NX_PI * 0.49f;
    euler.x = NX_CLAMP(euler.x, -maxPitch, maxPitch);
    euler.y = NX_WrapRadians(euler.y);
    euler.z = NX_WrapRadians(euler.z);

    camera->rotation = NX_QuatFromEuler(euler);

    /* --- Translation --- */

    NX_Vec3 forward = NX_Vec3Rotate(NX_VEC3_FORWARD, camera->rotation);
    NX_Vec3 right   = NX_Vec3Rotate(NX_VEC3_RIGHT,   camera->rotation);
    NX_Vec3 up      = NX_Vec3Rotate(NX_VEC3_UP,      camera->rotation);

    NX_Vec3 deltaPos = NX_VEC3_ZERO;
    deltaPos = NX_Vec3MulAdd(forward, -movement.z, deltaPos);
    deltaPos = NX_Vec3MulAdd(right,    movement.x, deltaPos);
    deltaPos = NX_Vec3MulAdd(up,       movement.y, deltaPos);

    camera->position = NX_Vec3Add(camera->position, deltaPos);
}

void NX_UpdateCameraFPS(NX_Camera* camera, NX_Vec3 movement, NX_Vec3 rotation, float maxPitch)
{
    /* --- Rotation (Euler) --- */

    NX_Vec3 euler = NX_QuatToEuler(camera->rotation);

    euler.x += rotation.x;
    euler.y += rotation.y;
    euler.z += rotation.z;

    if (maxPitch < 0.0f) maxPitch = NX_PI * 0.49f;
    euler.x = NX_CLAMP(euler.x, -maxPitch, maxPitch);
    euler.y = NX_WrapRadians(euler.y);
    euler.z = NX_WrapRadians(euler.z);

    camera->rotation = NX_QuatFromEuler(euler);

    /* --- Translation --- */

    NX_Quat yawOnly = NX_QuatFromEuler(NX_VEC3(0.0f, euler.y, 0.0f));

    NX_Vec3 forward = NX_Vec3Rotate(NX_VEC3_FORWARD, yawOnly);
    NX_Vec3 right   = NX_Vec3Rotate(NX_VEC3_RIGHT,   yawOnly);
    NX_Vec3 up      = NX_VEC3_UP;

    NX_Vec3 deltaPos = NX_VEC3_ZERO;
    deltaPos = NX_Vec3MulAdd(forward, -movement.z, deltaPos);
    deltaPos = NX_Vec3MulAdd(right,    movement.x, deltaPos);
    deltaPos = NX_Vec3MulAdd(up,       movement.y, deltaPos);

    camera->position = NX_Vec3Add(camera->position, deltaPos);
}

void NX_ApplyCameraTransform(NX_Camera* camera, NX_Mat4 transform, NX_Vec3 offset)
{
    camera->rotation = NX_QuatFromMat4(&transform);

    NX_Vec3 transformPosition = NX_VEC3(transform.v[2][0], transform.v[2][1], transform.v[2][2]);
    NX_Vec3 rotatedOffset = NX_Vec3TransformByMat4(offset, &transform);

    camera->position = NX_Vec3Add(transformPosition, rotatedOffset);
}

NX_Transform NX_GetCameraTransform(const NX_Camera* camera)
{
    return NX_TRANSFORM_T {
        .translation = camera->position,
        .rotation = camera->rotation,
        .scale = NX_VEC3_ONE
    };
}

/* === Environment - Public API === */

NX_Environment NX_GetDefaultEnvironment(void)
{
    return NX_Environment {
        .background = NX_GRAY,
        .ambient = NX_DARK_GRAY,
        .sky = {
            .cubemap = nullptr,
            .probe = nullptr,
            .rotation = NX_QUAT_IDENTITY,
            .intensity = 1.0f,
            .specular = 1.0f,
            .diffuse = 1.0f
        },
        .fog = {
            .mode = NX_FOG_DISABLED,
            .density = 0.01f,
            .start = 5.0f,
            .end = 50.0f,
            .skyAffect = 0.5f,
            .color = NX_GRAY
        },
        .ssao = {
            .intensity = 1.0f,
            .radius = 0.5f,
            .power = 1.0f,
            .bias = 0.025f,
            .enabled = false
        },
        .bloom {
            .mode = NX_BLOOM_DISABLED,
            .threshold = 0.0f,
            .softThreshold = 0.5f,
            .filterRadius = 0,
            .strength = 0.05f,
            .levels = {
                0.0f,
                0.0f,
                0.0f,
                1.0f,
                0.0f,
                1.0f,
                0.0f,
                0.0f
            }
        },
        .adjustment = {
            .brightness = 1.0f,
            .contrast = 1.0f,
            .saturation = 1.0f
        },
        .tonemap = {
            .mode = NX_TONEMAP_LINEAR,
            .exposure = 1.0f,
            .white = 1.0f
        }
    };
}

/* === Skybox - Public API === */

NX_Cubemap* NX_CreateCubemap(int size, NX_PixelFormat format)
{
    return gRender->cubemaps.createCubemap(size, format);
}

NX_Cubemap* NX_LoadCubemapFromMem(const NX_Image* image)
{
    return gRender->cubemaps.createCubemap(*image);
}

NX_Cubemap* NX_LoadCubemap(const char* filePath)
{
    NX_Image image = NX_LoadImage(filePath);
    if (image.pixels == nullptr) return nullptr;

    NX_Cubemap* cubemap = NX_LoadCubemapFromMem(&image);
    NX_DestroyImage(&image);

    return cubemap;
}

void NX_DestroyCubemap(NX_Cubemap* cubemap)
{
    gRender->cubemaps.destroyCubemap(cubemap);
}

void NX_GenerateSkybox(NX_Cubemap* cubemap, const NX_Skybox* skybox)
{
    gRender->cubemaps.generateSkybox(cubemap, *skybox);
}

/* === ReflectionProbe - Public API === */

NX_ReflectionProbe* NX_CreateReflectionProbe(NX_Cubemap* cubemap)
{
    return gRender->cubemaps.createReflectionProbe(*cubemap);
}

NX_ReflectionProbe* NX_LoadReflectionProbe(const char* filePath)
{
    NX_Cubemap* cubemap = NX_LoadCubemap(filePath);
    if (cubemap == nullptr) return nullptr;

    NX_ReflectionProbe* probe = NX_CreateReflectionProbe(cubemap);
    NX_DestroyCubemap(cubemap);

    return probe;
}

void NX_DestroyReflectionProbe(NX_ReflectionProbe* probe)
{
    gRender->cubemaps.destroyReflectionProbe(probe);
}

void NX_UpdateReflectionProbe(NX_ReflectionProbe* probe, const NX_Cubemap* cubemap)
{
    if (probe != nullptr && cubemap != nullptr) {
        gRender->cubemaps.updateReflectionProbe(probe, *cubemap);
    }
}

/* === Material - Public API === */

NX_Material NX_GetDefaultMaterial(void)
{
    return NX_Material {
        .albedo = {
            .texture = nullptr,
            .color = NX_WHITE,
        },
        .emission = {
            .texture = nullptr,
            .color = NX_WHITE,
            .energy = 0.0f
        },
        .orm {
            .texture = nullptr,
            .aoLightAffect = 0.0f,
            .occlusion = 1.0f,
            .roughness = 1.0f,
            .metalness = 0.0f,
        },
        .normal = {
            .texture = nullptr,
            .scale = 1.0f,
        },
        .depth = {
            .test = NX_DEPTH_TEST_LESS,
            .prePass = false
        },
        .alphaCutOff = 1e-6f,
        .texOffset = NX_VEC2_ZERO,
        .texScale = NX_VEC2_ONE,
        .billboard = NX_BILLBOARD_DISABLED,
        .shading = NX_SHADING_LIT,
        .blend = NX_BLEND_OPAQUE,
        .cull = NX_CULL_BACK,
        .shader = nullptr
    };
}

void NX_DestroyMaterialResources(NX_Material* material)
{
    NX_DestroyTexture(material->albedo.texture);
    NX_DestroyTexture(material->emission.texture);
    NX_DestroyTexture(material->orm.texture);
    NX_DestroyTexture(material->normal.texture);
    NX_DestroyMaterialShader(material->shader);
}

/* === MaterialShader - Public API === */

NX_MaterialShader* NX_CreateMaterialShader(const char* vertCode, const char* fragCode)
{
    return gRender->programs.createMaterialShader(vertCode, fragCode);
}

NX_MaterialShader* NX_LoadMaterialShader(const char* vertFile, const char* fragFile)
{
    char* vertCode = vertFile ? NX_LoadFileText(vertFile) : nullptr;
    char* fragCode = fragFile ? NX_LoadFileText(fragFile) : nullptr;

    NX_MaterialShader* shader = gRender->programs.createMaterialShader(vertCode, fragCode);

    SDL_free(vertCode);
    SDL_free(fragCode);

    return shader;
}

void NX_DestroyMaterialShader(NX_MaterialShader* shader)
{
    gRender->programs.destroyMaterialShader(shader);
}

void NX_SetMaterialShaderTexture(NX_MaterialShader* shader, int slot, const NX_Texture* texture)
{
    shader->setTexture(slot, texture ? &texture->gpuTexture() : nullptr);
}

void NX_UpdateStaticMaterialShaderBuffer(NX_MaterialShader* shader, size_t offset, size_t size, const void* data)
{
    shader->updateStaticBuffer(offset, size, data);
}

void NX_UpdateDynamicMaterialShaderBuffer(NX_MaterialShader* shader, size_t size, const void* data)
{
    shader->updateDynamicBuffer(size, data);
}

/* === Mesh - Public API === */

NX_Mesh* NX_CreateMesh(NX_PrimitiveType type, const NX_Vertex3D* vertices, int vertexCount, const uint32_t* indices, int indexCount)
{
    /* --- Validation of parameters --- */

    if (vertices == nullptr || vertexCount == 0) {
        NX_INTERNAL_LOG(E, "RENDER: Failed to create mesh; Vertices and their count cannot be null");
        return nullptr;
    }

    /* --- Copies of input data --- */

    NX_Vertex3D* vCopy = static_cast<NX_Vertex3D*>(SDL_malloc(vertexCount * sizeof(NX_Vertex3D)));
    SDL_memcpy(vCopy, vertices, vertexCount * sizeof(NX_Vertex3D));

    uint32_t* iCopy = nullptr;
    if (indices != nullptr && indexCount > 0) {
        iCopy = static_cast<uint32_t*>(SDL_malloc(indexCount * sizeof(uint32_t)));
        SDL_memcpy(iCopy, indices, indexCount * sizeof(uint32_t));
    }

    /* --- Mesh creation --- */

    NX_Mesh* mesh = gRender->meshes.createMesh(
        type, vCopy, vertexCount,
        iCopy, indexCount,
        true
    );

    if (mesh == nullptr) {
        SDL_free(vCopy);
        SDL_free(iCopy);
        return nullptr;
    }

    return mesh;
}

NX_Mesh* NX_CreateMeshFrom(NX_PrimitiveType type, NX_Vertex3D* vertices, int vertexCount, uint32_t* indices, int indexCount)
{
    if (vertices == nullptr || vertexCount == 0) {
        NX_INTERNAL_LOG(E, "RENDER: Failed to vertex mesh; Vertices and their count cannot be null");
        return nullptr;
    }

    return gRender->meshes.createMesh(
        type, vertices, vertexCount,
        indices, indexCount,
        true
    );
}

void NX_DestroyMesh(NX_Mesh* mesh)
{
    gRender->meshes.destroyMesh(mesh);
}

NX_Mesh* NX_GenMeshQuad(NX_Vec2 size, NX_Vec2 subDiv, NX_Vec3 normal)
{
    /* --- Parameter validation --- */

    size.x = fmaxf(0.1f, size.x);
    size.y = fmaxf(0.1f, size.y);
    int segX = (int)fmaxf(1.0f, subDiv.x);
    int segY = (int)fmaxf(1.0f, subDiv.y);

    float length = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
    if (length < 0.001f) {
        normal = NX_VEC3(0.0f, 0.0f, 1.0f);
        length = 1.0f;
    }
    normal.x /= length;
    normal.y /= length;
    normal.z /= length;

    /* --- Memory allocation --- */

    int vertexCount = (segX + 1) * (segY + 1);
    int indexCount = segX * segY * 6;

    NX_Vertex3D* vertices = static_cast<NX_Vertex3D*>(SDL_malloc(sizeof(NX_Vertex3D) * vertexCount));
    uint32_t* indices = static_cast<uint32_t*>(SDL_malloc(sizeof(uint32_t) * indexCount));

    if (!vertices || !indices) {
        SDL_free(vertices);
        SDL_free(indices);
        return nullptr;
    }

    /* --- Orientation vectors --- */

    NX_Vec3 reference = (fabsf(normal.y) < 0.9f) ? NX_VEC3(0.0f, 1.0f, 0.0f) : NX_VEC3(1.0f, 0.0f, 0.0f);

    NX_Vec3 tangent;
    tangent.x = normal.y * reference.z - normal.z * reference.y;
    tangent.y = normal.z * reference.x - normal.x * reference.z;
    tangent.z = normal.x * reference.y - normal.y * reference.x;

    float tangentLength = sqrtf(tangent.x * tangent.x + tangent.y * tangent.y + tangent.z * tangent.z);
    tangent.x /= tangentLength;
    tangent.y /= tangentLength;
    tangent.z /= tangentLength;

    NX_Vec3 bitangent;
    bitangent.x = normal.y * tangent.z - normal.z * tangent.y;
    bitangent.y = normal.z * tangent.x - normal.x * tangent.z;
    bitangent.z = normal.x * tangent.y - normal.y * tangent.x;

    /* --- Vertex generation --- */

    int vertexIndex = 0;
    for (int y = 0; y <= segY; y++) {
        for (int x = 0; x <= segX; x++) {
            NX_Vertex3D& vertex = vertices[vertexIndex++];

            float u = ((float)x / segX) - 0.5f;
            float v = ((float)y / segY) - 0.5f;
            float localX = u * size.x;
            float localY = v * size.y;

            vertex.position.x = localX * tangent.x + localY * bitangent.x;
            vertex.position.y = localX * tangent.y + localY * bitangent.y;
            vertex.position.z = localX * tangent.z + localY * bitangent.z;

            vertex.texcoord.x = (float)x / segX;
            vertex.texcoord.y = (float)y / segY;
            vertex.normal = normal;
            vertex.tangent = NX_VEC4(tangent.x, tangent.y, tangent.z, 1.0f);
            vertex.color = NX_COLOR(1, 1, 1, 1);
        }
    }

    /* --- Index generation --- */

    int indexIndex = 0;
    for (int y = 0; y < segY; y++) {
        for (int x = 0; x < segX; x++) {
            uint32_t i0 = y * (segX + 1) + x;
            uint32_t i1 = y * (segX + 1) + (x + 1);
            uint32_t i2 = (y + 1) * (segX + 1) + (x + 1);
            uint32_t i3 = (y + 1) * (segX + 1) + x;

            indices[indexIndex++] = i0;
            indices[indexIndex++] = i1;
            indices[indexIndex++] = i2;

            indices[indexIndex++] = i0;
            indices[indexIndex++] = i2;
            indices[indexIndex++] = i3;
        }
    }

    /* --- Mesh creation and finalization --- */

    NX_Mesh* mesh = gRender->meshes.createMesh(
        NX_PRIMITIVE_TRIANGLES,
        vertices, vertexCount,
        indices, indexCount,
        true
    );

    if (mesh == nullptr) {
        SDL_free(vertices);
        SDL_free(indices);
        return nullptr;
    }

    return mesh;
}

NX_Mesh* NX_GenMeshCube(NX_Vec3 size, NX_Vec3 subDiv)
{
    /* --- Parameter validation --- */

    int segX = (int)fmaxf(1.0f, subDiv.x);
    int segY = (int)fmaxf(1.0f, subDiv.y);
    int segZ = (int)fmaxf(1.0f, subDiv.z);

    /* --- Memory allocation --- */

    int verticesFrontBack = (segX + 1) * (segY + 1);
    int verticesLeftRight = (segZ + 1) * (segY + 1);
    int verticesTopBottom = (segX + 1) * (segZ + 1);
    int vertexCount = 2 * (verticesFrontBack + verticesLeftRight + verticesTopBottom);

    int indicesFrontBack = segX * segY * 6;
    int indicesLeftRight = segZ * segY * 6;
    int indicesTopBottom = segX * segZ * 6;
    int indexCount = 2 * (indicesFrontBack + indicesLeftRight + indicesTopBottom);

    NX_Vertex3D* vertices = static_cast<NX_Vertex3D*>(SDL_malloc(sizeof(NX_Vertex3D) * vertexCount));
    uint32_t* indices = static_cast<uint32_t*>(SDL_malloc(sizeof(uint32_t) * indexCount));

    if (!vertices || !indices) {
        SDL_free(vertices);
        SDL_free(indices);
        return nullptr;
    }

    /* --- Face configuration --- */

    struct FaceParams {
        NX_Vec3 normal;
        NX_Vec4 tangent;
        int segsU, segsV;
    };

    FaceParams faces[6] = {
        {NX_VEC3(0, 0, 1), NX_VEC4(1, 0, 0, 1), segX, segY},   // Front (Z+)
        {NX_VEC3(0, 0, -1), NX_VEC4(-1, 0, 0, 1), segX, segY}, // Back (Z-)
        {NX_VEC3(1, 0, 0), NX_VEC4(0, 0, -1, 1), segZ, segY},  // Right (X+)
        {NX_VEC3(-1, 0, 0), NX_VEC4(0, 0, 1, 1), segZ, segY},  // Left (X-)
        {NX_VEC3(0, 1, 0), NX_VEC4(1, 0, 0, 1), segX, segZ},   // Top (Y+)
        {NX_VEC3(0, -1, 0), NX_VEC4(1, 0, 0, 1), segX, segZ}   // Bottom (Y-)
    };

    /* --- Vertex and index generation --- */

    int vertexIndex = 0;
    int indexIndex = 0;
    NX_Vec3 halfSize = size * 0.5f;

    for (int face = 0; face < 6; face++) {
        uint32_t baseVertex = vertexIndex;
        FaceParams& fp = faces[face];

        for (int v = 0; v <= fp.segsV; v++) {
            for (int u = 0; u <= fp.segsU; u++) {
                NX_Vertex3D& vertex = vertices[vertexIndex++];

                float uNorm = (float)u / fp.segsU;
                float vNorm = (float)v / fp.segsV;

                switch (face) {
                    case 0: // Front (Z+)
                        vertex.position.x = -halfSize.x + size.x * uNorm;
                        vertex.position.y = -halfSize.y + size.y * vNorm;
                        vertex.position.z = halfSize.z;
                        break;
                    case 1: // Back (Z-)
                        vertex.position.x = halfSize.x - size.x * uNorm;
                        vertex.position.y = -halfSize.y + size.y * vNorm;
                        vertex.position.z = -halfSize.z;
                        break;
                    case 2: // Right (X+)
                        vertex.position.x = halfSize.x;
                        vertex.position.y = -halfSize.y + size.y * vNorm;
                        vertex.position.z = halfSize.z - size.z * uNorm;
                        break;
                    case 3: // Left (X-)
                        vertex.position.x = -halfSize.x;
                        vertex.position.y = -halfSize.y + size.y * vNorm;
                        vertex.position.z = -halfSize.z + size.z * uNorm;
                        break;
                    case 4: // Top (Y+)
                        vertex.position.x = -halfSize.x + size.x * uNorm;
                        vertex.position.y = halfSize.y;
                        vertex.position.z = halfSize.z - size.z * vNorm;
                        break;
                    case 5: // Bottom (Y-)
                        vertex.position.x = -halfSize.x + size.x * uNorm;
                        vertex.position.y = -halfSize.y;
                        vertex.position.z = -halfSize.z + size.z * vNorm;
                        break;
                }

                vertex.texcoord.x = uNorm;
                vertex.texcoord.y = vNorm;
                vertex.normal = fp.normal;
                vertex.tangent = fp.tangent;
                vertex.color = NX_COLOR(1, 1, 1, 1);
            }
        }

        // Generate indices for this face
        for (int v = 0; v < fp.segsV; v++) {
            for (int u = 0; u < fp.segsU; u++) {
                uint32_t i0 = baseVertex + v * (fp.segsU + 1) + u;
                uint32_t i1 = baseVertex + v * (fp.segsU + 1) + (u + 1);
                uint32_t i2 = baseVertex + (v + 1) * (fp.segsU + 1) + (u + 1);
                uint32_t i3 = baseVertex + (v + 1) * (fp.segsU + 1) + u;

                indices[indexIndex++] = i0;
                indices[indexIndex++] = i1;
                indices[indexIndex++] = i2;

                indices[indexIndex++] = i0;
                indices[indexIndex++] = i2;
                indices[indexIndex++] = i3;
            }
        }
    }

    /* --- Mesh creation and finalization --- */

    NX_Mesh* mesh = gRender->meshes.createMesh(
        NX_PRIMITIVE_TRIANGLES,
        vertices, vertexCount,
        indices, indexCount,
        true
    );

    if (mesh == nullptr) {
        SDL_free(vertices);
        SDL_free(indices);
        return nullptr;
    }

    return mesh;
}

NX_Mesh* NX_GenMeshSphere(float radius, int slices, int rings)
{
    /* --- Parameter validation --- */

    radius = fmaxf(0.1f, radius);
    slices = (int)fmaxf(3, slices);
    rings = (int)fmaxf(2, rings);

    /* --- Memory allocation --- */

    int vertexCount = (rings + 1) * (slices + 1);
    int indexCount = rings * slices * 6;

    NX_Vertex3D* vertices = static_cast<NX_Vertex3D*>(SDL_malloc(sizeof(NX_Vertex3D) * vertexCount));
    uint32_t* indices = static_cast<uint32_t*>(SDL_malloc(sizeof(uint32_t) * indexCount));

    if (!vertices || !indices) {
        SDL_free(vertices);
        SDL_free(indices);
        return nullptr;
    }

    /* --- Sphere generation --- */

    int vertexIndex = 0;
    int indexIndex = 0;
    const float piOverRings = NX_PI / rings;
    const float tauOverSlices = NX_TAU / slices;

    for (int ring = 0; ring <= rings; ring++) {
        float phi = ring * piOverRings;
        float sinPhi = sinf(phi);
        float cosPhi = cosf(phi);
        float y = radius * cosPhi;
        float ringRadius = radius * sinPhi;

        for (int slice = 0; slice <= slices; slice++) {
            float theta = slice * tauOverSlices;
            float sinTheta = sinf(theta);
            float cosTheta = cosf(theta);

            NX_Vertex3D& vertex = vertices[vertexIndex++];

            vertex.position.x = ringRadius * cosTheta;
            vertex.position.y = y;
            vertex.position.z = ringRadius * sinTheta;

            vertex.normal = NX_VEC3(
                vertex.position.x / radius,
                vertex.position.y / radius,
                vertex.position.z / radius
            );

            vertex.texcoord.x = (float)slice / slices;
            vertex.texcoord.y = (float)ring / rings;
            vertex.tangent = NX_VEC4(-sinTheta, 0.0f, cosTheta, 1.0f);
            vertex.color = NX_COLOR(1, 1, 1, 1);
        }
    }

    /* --- Index generation --- */

    for (int ring = 0; ring < rings; ring++) {
        for (int slice = 0; slice < slices; slice++) {
            uint32_t current = ring * (slices + 1) + slice;
            uint32_t next = current + slices + 1;

            uint32_t i0 = current;
            uint32_t i1 = current + 1;
            uint32_t i2 = next + 1;
            uint32_t i3 = next;

            indices[indexIndex++] = i0;
            indices[indexIndex++] = i1;
            indices[indexIndex++] = i2;

            indices[indexIndex++] = i0;
            indices[indexIndex++] = i2;
            indices[indexIndex++] = i3;
        }
    }

    /* --- Mesh creation and finalization --- */

    NX_Mesh* mesh = gRender->meshes.createMesh(
        NX_PRIMITIVE_TRIANGLES,
        vertices, vertexCount,
        indices, indexCount,
        true
    );

    if (mesh == nullptr) {
        SDL_free(vertices);
        SDL_free(indices);
        return nullptr;
    }

    return mesh;
}

NX_Mesh* NX_GenMeshCylinder(float topRadius, float bottomRadius, float height, int slices, int rings, bool topCap, bool bottomCap)
{
    /* --- Parameter validation --- */

    topRadius = fmaxf(0.0f, topRadius);
    bottomRadius = fmaxf(0.0f, bottomRadius);
    height = fmaxf(0.1f, height);
    slices = (int)fmaxf(3, slices);
    rings = (int)fmaxf(1, rings);

    if (topRadius == 0.0f && bottomRadius == 0.0f) {
        bottomRadius = 1.0f;
    }

    /* --- Memory allocation --- */

    int sideVertices = (rings + 1) * (slices + 1);
    int topCapVertices = (topCap && topRadius > 0.0f) ? slices + 2 : 0;
    int bottomCapVertices = (bottomCap && bottomRadius > 0.0f) ? slices + 2 : 0;
    int vertexCount = sideVertices + topCapVertices + bottomCapVertices;

    int sideIndices = rings * slices * 6;
    int topCapIndices = (topCap && topRadius > 0.0f) ? slices * 3 : 0;
    int bottomCapIndices = (bottomCap && bottomRadius > 0.0f) ? slices * 3 : 0;
    int indexCount = sideIndices + topCapIndices + bottomCapIndices;

    NX_Vertex3D* vertices = static_cast<NX_Vertex3D*>(SDL_malloc(sizeof(NX_Vertex3D) * vertexCount));
    uint32_t* indices = static_cast<uint32_t*>(SDL_malloc(sizeof(uint32_t) * indexCount));

    if (!vertices || !indices) {
        SDL_free(vertices);
        SDL_free(indices);
        return nullptr;
    }

    /* --- Cylinder setup --- */

    int vertexIndex = 0;
    int indexIndex = 0;
    const float angleStep = NX_TAU / slices;
    const float halfHeight = height * 0.5f;

    NX_Vec3 sideNormalBase;
    if (topRadius != bottomRadius) {
        float radiusDiff = bottomRadius - topRadius;
        float normalLength = sqrtf(radiusDiff * radiusDiff + height * height);
        sideNormalBase = NX_VEC3(radiusDiff / normalLength, height / normalLength, 0.0f);
    }
    else {
        sideNormalBase = NX_VEC3(1.0f, 0.0f, 0.0f);
    }

    /* --- Side generation --- */

    uint32_t sideBaseVertex = vertexIndex;

    for (int ring = 0; ring <= rings; ring++) {
        float t = (float)ring / rings;
        float y = -halfHeight + height * t;
        float currentRadius = bottomRadius + (topRadius - bottomRadius) * t;

        for (int slice = 0; slice <= slices; slice++) {
            float angle = slice * angleStep;
            float cosAngle = cosf(angle);
            float sinAngle = sinf(angle);

            NX_Vertex3D& vertex = vertices[vertexIndex++];

            vertex.position.x = currentRadius * cosAngle;
            vertex.position.y = y;
            vertex.position.z = currentRadius * sinAngle;

            vertex.normal.x = sideNormalBase.x * cosAngle;
            vertex.normal.y = sideNormalBase.y;
            vertex.normal.z = sideNormalBase.x * sinAngle;

            vertex.texcoord.x = (float)slice / slices;
            vertex.texcoord.y = t;
            vertex.tangent = NX_VEC4(-sinAngle, 0.0f, cosAngle, 1.0f);
            vertex.color = NX_COLOR(1, 1, 1, 1);
        }
    }

    for (int ring = 0; ring < rings; ring++) {
        for (int slice = 0; slice < slices; slice++) {
            uint32_t i0 = sideBaseVertex + ring * (slices + 1) + slice;
            uint32_t i1 = sideBaseVertex + ring * (slices + 1) + (slice + 1);
            uint32_t i2 = sideBaseVertex + (ring + 1) * (slices + 1) + (slice + 1);
            uint32_t i3 = sideBaseVertex + (ring + 1) * (slices + 1) + slice;

            indices[indexIndex++] = i0;
            indices[indexIndex++] = i2;
            indices[indexIndex++] = i1;

            indices[indexIndex++] = i0;
            indices[indexIndex++] = i3;
            indices[indexIndex++] = i2;
        }
    }

    /* --- Top cap generation --- */

    if (topCap && topRadius > 0.0f) {
        uint32_t topCapBaseVertex = vertexIndex;

        NX_Vertex3D& centerVertex = vertices[vertexIndex++];
        centerVertex.position = NX_VEC3(0.0f, halfHeight, 0.0f);
        centerVertex.normal = NX_VEC3(0.0f, 1.0f, 0.0f);
        centerVertex.texcoord = NX_VEC2(0.5f, 0.5f);
        centerVertex.tangent = NX_VEC4(1.0f, 0.0f, 0.0f, 1.0f);
        centerVertex.color = NX_COLOR(1, 1, 1, 1);

        for (int slice = 0; slice <= slices; slice++) {
            float angle = slice * angleStep;
            float cosAngle = cosf(angle);
            float sinAngle = sinf(angle);

            NX_Vertex3D& vertex = vertices[vertexIndex++];

            vertex.position.x = topRadius * cosAngle;
            vertex.position.y = halfHeight;
            vertex.position.z = topRadius * sinAngle;
            vertex.normal = NX_VEC3(0.0f, 1.0f, 0.0f);
            vertex.texcoord.x = 0.5f + 0.5f * cosAngle;
            vertex.texcoord.y = 0.5f + 0.5f * sinAngle;
            vertex.tangent = NX_VEC4(1.0f, 0.0f, 0.0f, 1.0f);
            vertex.color = NX_COLOR(1, 1, 1, 1);
        }

        for (int slice = 0; slice < slices; slice++) {
            indices[indexIndex++] = topCapBaseVertex;
            indices[indexIndex++] = topCapBaseVertex + 1 + (slice + 1);
            indices[indexIndex++] = topCapBaseVertex + 1 + slice;
        }
    }

    /* --- Bottom cap generation --- */

    if (bottomCap && bottomRadius > 0.0f) {
        uint32_t bottomCapBaseVertex = vertexIndex;

        NX_Vertex3D& centerVertex = vertices[vertexIndex++];
        centerVertex.position = NX_VEC3(0.0f, -halfHeight, 0.0f);
        centerVertex.normal = NX_VEC3(0.0f, -1.0f, 0.0f);
        centerVertex.texcoord = NX_VEC2(0.5f, 0.5f);
        centerVertex.tangent = NX_VEC4(1.0f, 0.0f, 0.0f, 1.0f);
        centerVertex.color = NX_COLOR(1, 1, 1, 1);

        for (int slice = 0; slice <= slices; slice++) {
            float angle = slice * angleStep;
            float cosAngle = cosf(angle);
            float sinAngle = sinf(angle);

            NX_Vertex3D& vertex = vertices[vertexIndex++];

            vertex.position.x = bottomRadius * cosAngle;
            vertex.position.y = -halfHeight;
            vertex.position.z = bottomRadius * sinAngle;
            vertex.normal = NX_VEC3(0.0f, -1.0f, 0.0f);
            vertex.texcoord.x = 0.5f + 0.5f * cosAngle;
            vertex.texcoord.y = 0.5f + 0.5f * sinAngle;
            vertex.tangent = NX_VEC4(1.0f, 0.0f, 0.0f, 1.0f);
            vertex.color = NX_COLOR(1, 1, 1, 1);
        }

        for (int slice = 0; slice < slices; slice++) {
            indices[indexIndex++] = bottomCapBaseVertex;
            indices[indexIndex++] = bottomCapBaseVertex + 1 + slice;
            indices[indexIndex++] = bottomCapBaseVertex + 1 + (slice + 1);
        }
    }

    /* --- Mesh creation and finalization --- */

    NX_Mesh* mesh = gRender->meshes.createMesh(
        NX_PRIMITIVE_TRIANGLES,
        vertices, vertexCount,
        indices, indexCount,
        true
    );

    if (mesh == nullptr) {
        SDL_free(vertices);
        SDL_free(indices);
        return nullptr;
    }

    return mesh;
}

NX_Mesh* NX_GenMeshCapsule(float radius, float height, int slices, int rings)
{
    /* --- Parameter validation --- */

    radius = fmaxf(0.1f, radius);
    height = fmaxf(0.0f, height);
    slices = NX_MAX(3, slices);
    rings = NX_MAX(2, rings);

    int hemisphereRings = NX_MAX(1, rings / 2);

    /* --- Memory allocation --- */

    int cylinderVertices = (height > 0.0f) ? 2 * (slices + 1) : 0;
    int topHemisphereVertices = (hemisphereRings + 1) * (slices + 1);
    int bottomHemisphereVertices = (hemisphereRings + 1) * (slices + 1);

    if (height == 0.0f) {
        bottomHemisphereVertices -= (slices + 1);
    }

    int vertexCount = cylinderVertices + topHemisphereVertices + bottomHemisphereVertices;

    int cylinderIndices = (height > 0.0f) ? slices * 6 : 0;
    int hemisphereIndices = hemisphereRings * slices * 6;
    int indexCount = cylinderIndices + 2 * hemisphereIndices;

    NX_Vertex3D* vertices = static_cast<NX_Vertex3D*>(SDL_malloc(sizeof(NX_Vertex3D) * vertexCount));
    uint32_t* indices = static_cast<uint32_t*>(SDL_malloc(sizeof(uint32_t) * indexCount));

    if (!vertices || !indices) {
        SDL_free(vertices);
        SDL_free(indices);
        return nullptr;
    }

    /* --- Capsule setup --- */

    int vertexIndex = 0;
    int indexIndex = 0;
    const float PI_OVER_2 = NX_PI * 0.5f;
    const float angleStep = NX_TAU / slices;
    const float halfHeight = height * 0.5f;

    /* --- Top hemisphere generation --- */

    uint32_t topHemisphereBaseVertex = vertexIndex;

    for (int ring = 0; ring <= hemisphereRings; ring++) {
        float phi = (float)ring / hemisphereRings * PI_OVER_2;
        float sinPhi = sinf(phi);
        float cosPhi = cosf(phi);
        float y = halfHeight + radius * cosPhi;
        float ringRadius = radius * sinPhi;

        for (int slice = 0; slice <= slices; slice++) {
            float theta = slice * angleStep;
            float sinTheta = sinf(theta);
            float cosTheta = cosf(theta);

            NX_Vertex3D& vertex = vertices[vertexIndex++];

            vertex.position.x = ringRadius * cosTheta;
            vertex.position.y = y;
            vertex.position.z = ringRadius * sinTheta;

            vertex.normal.x = sinPhi * cosTheta;
            vertex.normal.y = cosPhi;
            vertex.normal.z = sinPhi * sinTheta;

            vertex.texcoord.x = (float)slice / slices;
            vertex.texcoord.y = 0.5f + 0.5f * ((float)ring / hemisphereRings);
            vertex.tangent = NX_VEC4(-sinTheta, 0.0f, cosTheta, 1.0f);
            vertex.color = NX_COLOR(1, 1, 1, 1);
        }
    }

    for (int ring = 0; ring < hemisphereRings; ring++) {
        for (int slice = 0; slice < slices; slice++) {
            uint32_t i0 = topHemisphereBaseVertex + ring * (slices + 1) + slice;
            uint32_t i1 = topHemisphereBaseVertex + ring * (slices + 1) + (slice + 1);
            uint32_t i2 = topHemisphereBaseVertex + (ring + 1) * (slices + 1) + (slice + 1);
            uint32_t i3 = topHemisphereBaseVertex + (ring + 1) * (slices + 1) + slice;

            indices[indexIndex++] = i0;
            indices[indexIndex++] = i1;
            indices[indexIndex++] = i2;

            indices[indexIndex++] = i0;
            indices[indexIndex++] = i2;
            indices[indexIndex++] = i3;
        }
    }

    /* --- Cylinder generation --- */

    uint32_t cylinderBaseVertex = vertexIndex;

    if (height > 0.0f) {
        for (int slice = 0; slice <= slices; slice++) {
            float theta = slice * angleStep;
            float sinTheta = sinf(theta);
            float cosTheta = cosf(theta);

            NX_Vertex3D& topVertex = vertices[vertexIndex++];
            topVertex.position.x = radius * cosTheta;
            topVertex.position.y = halfHeight;
            topVertex.position.z = radius * sinTheta;
            topVertex.normal.x = cosTheta;
            topVertex.normal.y = 0.0f;
            topVertex.normal.z = sinTheta;
            topVertex.texcoord.x = (float)slice / slices;
            topVertex.texcoord.y = 0.5f;
            topVertex.tangent = NX_VEC4(-sinTheta, 0.0f, cosTheta, 1.0f);
            topVertex.color = NX_COLOR(1, 1, 1, 1);
        }

        for (int slice = 0; slice <= slices; slice++) {
            float theta = slice * angleStep;
            float sinTheta = sinf(theta);
            float cosTheta = cosf(theta);

            NX_Vertex3D& bottomVertex = vertices[vertexIndex++];
            bottomVertex.position.x = radius * cosTheta;
            bottomVertex.position.y = -halfHeight;
            bottomVertex.position.z = radius * sinTheta;
            bottomVertex.normal.x = cosTheta;
            bottomVertex.normal.y = 0.0f;
            bottomVertex.normal.z = sinTheta;
            bottomVertex.texcoord.x = (float)slice / slices;
            bottomVertex.texcoord.y = 0.5f;
            bottomVertex.tangent = NX_VEC4(-sinTheta, 0.0f, cosTheta, 1.0f);
            bottomVertex.color = NX_COLOR(1, 1, 1, 1);
        }

        for (int slice = 0; slice < slices; slice++) {
            uint32_t i0 = cylinderBaseVertex + slice;
            uint32_t i1 = cylinderBaseVertex + (slice + 1);
            uint32_t i2 = cylinderBaseVertex + (slices + 1) + (slice + 1);
            uint32_t i3 = cylinderBaseVertex + (slices + 1) + slice;

            indices[indexIndex++] = i0;
            indices[indexIndex++] = i1;
            indices[indexIndex++] = i2;

            indices[indexIndex++] = i0;
            indices[indexIndex++] = i2;
            indices[indexIndex++] = i3;
        }
    }

    /* --- Bottom hemisphere generation --- */

    uint32_t bottomHemisphereBaseVertex = vertexIndex;
    int startRing = (height == 0.0f) ? 1 : 0;

    for (int ring = startRing; ring <= hemisphereRings; ring++) {
        float phi = PI_OVER_2 + (float)ring / hemisphereRings * PI_OVER_2;
        float sinPhi = sinf(phi);
        float cosPhi = cosf(phi);
        float y = -halfHeight + radius * cosPhi;
        float ringRadius = radius * sinPhi;

        for (int slice = 0; slice <= slices; slice++) {
            float theta = slice * angleStep;
            float sinTheta = sinf(theta);
            float cosTheta = cosf(theta);

            NX_Vertex3D& vertex = vertices[vertexIndex++];

            vertex.position.x = ringRadius * cosTheta;
            vertex.position.y = y;
            vertex.position.z = ringRadius * sinTheta;

            vertex.normal.x = sinPhi * cosTheta;
            vertex.normal.y = cosPhi;
            vertex.normal.z = sinPhi * sinTheta;

            vertex.texcoord.x = (float)slice / slices;
            vertex.texcoord.y = 0.5f - 0.5f * ((float)ring / hemisphereRings);
            vertex.tangent = NX_VEC4(-sinTheta, 0.0f, cosTheta, 1.0f);
            vertex.color = NX_COLOR(1, 1, 1, 1);
        }
    }

    int effectiveRings = hemisphereRings - startRing;
    for (int ring = 0; ring < effectiveRings; ring++) {
        for (int slice = 0; slice < slices; slice++) {
            uint32_t i0 = bottomHemisphereBaseVertex + ring * (slices + 1) + slice;
            uint32_t i1 = bottomHemisphereBaseVertex + ring * (slices + 1) + (slice + 1);
            uint32_t i2 = bottomHemisphereBaseVertex + (ring + 1) * (slices + 1) + (slice + 1);
            uint32_t i3 = bottomHemisphereBaseVertex + (ring + 1) * (slices + 1) + slice;

            indices[indexIndex++] = i0;
            indices[indexIndex++] = i1;
            indices[indexIndex++] = i2;

            indices[indexIndex++] = i0;
            indices[indexIndex++] = i2;
            indices[indexIndex++] = i3;
        }
    }

    /* --- Mesh creation and finalization --- */

    NX_Mesh* mesh = gRender->meshes.createMesh(
        NX_PRIMITIVE_TRIANGLES,
        vertices, vertexCount,
        indices, indexCount,
        true
    );

    if (mesh == nullptr) {
        SDL_free(vertices);
        SDL_free(indices);
        return nullptr;
    }

    return mesh;
}

void NX_UpdateMeshBuffer(NX_Mesh* mesh)
{
    gRender->meshes.updateMesh(mesh);
}

void NX_UpdateMeshAABB(NX_Mesh* mesh)
{
    if (!mesh || mesh->vertexCount == 0) {
        return;
    }

    const NX_Vertex3D* vertices = mesh->vertices;
    const uint32_t* indices = mesh->indices;

    mesh->aabb.min = NX_VEC3(+FLT_MAX, +FLT_MAX, +FLT_MAX);
    mesh->aabb.max = NX_VEC3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    if (indices) {
        for (int i = 0; i < mesh->indexCount; i++) {
            const NX_Vec3& pos = vertices[indices[i]].position;
            mesh->aabb.min = NX_Vec3Min(mesh->aabb.min, pos);
            mesh->aabb.max = NX_Vec3Max(mesh->aabb.max, pos);
        }
    }
    else {
        for (int i = 0; i < mesh->vertexCount; i++) {
            const NX_Vec3& pos = vertices[i].position;
            mesh->aabb.min = NX_Vec3Min(mesh->aabb.min, pos);
            mesh->aabb.max = NX_Vec3Max(mesh->aabb.max, pos);
        }
    }
}

/* === DnyamicMesh - Public API === */

NX_DynamicMesh* NX_CreateDynamicMesh(size_t initialCapacity)
{
    return gRender->meshes.createDynamicMesh(initialCapacity);
}

void NX_DestroyDynamicMesh(NX_DynamicMesh* dynMesh)
{
    gRender->meshes.destroyDynamicMesh(dynMesh);
}

void NX_BeginDynamicMesh(NX_DynamicMesh* dynMesh, NX_PrimitiveType type)
{
    dynMesh->begin(type);
}

void NX_EndDynamicMesh(NX_DynamicMesh* dynMesh)
{
    dynMesh->end();
}

void NX_SetDynamicMeshTexCoord(NX_DynamicMesh* dynMesh, NX_Vec2 texcoord)
{
    dynMesh->setTexCoord(texcoord);
}

void NX_SetDynamicMeshNormal(NX_DynamicMesh* dynMesh, NX_Vec3 normal)
{
    dynMesh->setNormal(normal);
}

void NX_SetDynamicMeshTangent(NX_DynamicMesh* dynMesh, NX_Vec4 tangent)
{
    dynMesh->setTangent(tangent);
}

void NX_SetDynamicMeshColor(NX_DynamicMesh* dynMesh, NX_Color color)
{
    dynMesh->setColor(color);
}

void NX_AddDynamicMeshVertex(NX_DynamicMesh* dynMesh, NX_Vec3 position)
{
    dynMesh->addVertex(position);
}

void NX_SetDynamicMeshShadowCastMode(NX_DynamicMesh* dynMesh, NX_ShadowCastMode mode)
{
    dynMesh->shadowCastMode = mode;
}

void NX_SetDynamicMeshShadowFaceMode(NX_DynamicMesh* dynMesh, NX_ShadowFaceMode mode)
{
    dynMesh->shadowFaceMode = mode;
}

void NX_SetDynamicMeshLayerMask(NX_DynamicMesh* dynMesh, NX_Layer mask)
{
    dynMesh->layerMask = mask;
}

/* === InstanceBuffer - Public API === */

NX_InstanceBuffer* NX_CreateInstanceBuffer(NX_InstanceData bitfield, size_t count)
{
    return gRender->meshes.createInstanceBuffer(bitfield, count);
}

void NX_DestroyInstanceBuffer(NX_InstanceBuffer* buffer)
{
    gRender->meshes.destroyInstanceBuffer(buffer);
}

void NX_RaeallocInstanceBuffer(NX_InstanceBuffer* buffer, size_t count, bool keepData)
{
    buffer->realloc(count, keepData);
}

void NX_UpdateInstanceBuffer(NX_InstanceBuffer* buffer, NX_InstanceData type, size_t offset, size_t count, const void* data)
{
    buffer->update(type, offset, count, data);
}

void* NX_MapInstanceBuffer(NX_InstanceBuffer* buffer, NX_InstanceData type)
{
    return buffer->map(type);
}

void* NX_MapInstanceBufferRange(NX_InstanceBuffer* buffer, NX_InstanceData type, size_t offset, size_t count)
{
    return buffer->mapRange(type, offset, count);
}

void NX_UnmapInstanceBuffer(NX_InstanceBuffer* buffer, NX_InstanceData type)
{
    buffer->unmap(type);
}

void NX_QueryInstanceBuffer(NX_InstanceBuffer* buffer, NX_InstanceData* bitfield, size_t* count)
{
    if (bitfield) *bitfield = buffer->instanceFlags();
    if (count) *count = buffer->allocatedCount();
}

/* === Model - Public API === */

NX_Model* NX_LoadModel(const char* filePath)
{
    size_t fileSize = 0;
    void* fileData = NX_LoadFile(filePath, &fileSize);
    if (fileData == nullptr || fileSize == 0) {
        NX_INTERNAL_LOG(E, "RENDER: Failed to load model data: %s", filePath);
        return nullptr;
    }

    NX_Model* model = gRender->models.loadModel(fileData, fileSize, helper::getFileExt(filePath));
    SDL_free(fileData);

    return model;
}

NX_Model* NX_LoadModelFromMemory(const void* data, size_t size, const char* hint)
{
    return gRender->models.loadModel(data, size, hint);
}

void NX_DestroyModel(NX_Model* model)
{
    gRender->models.destroyModel(model);
}

void NX_UpdateModelAABB(NX_Model* model, bool updateMeshAABBs)
{
    if (!model || !model->meshes) {
        return;
    }

    NX_Vec3 min = { +FLT_MAX, +FLT_MAX, +FLT_MAX };
    NX_Vec3 max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

    for (uint32_t i = 0; i < model->meshCount; i++) {
        NX_Mesh* mesh = model->meshes[i];
        if (updateMeshAABBs) {
            NX_UpdateMeshAABB(mesh);
        }
        min = NX_Vec3Min(min, mesh->aabb.min);
        max = NX_Vec3Max(max, mesh->aabb.max);
    }

    model->aabb.min = min;
    model->aabb.max = max;
}

void NX_ScaleModelAABB(NX_Model* model, float scale, bool scaleMeshAABBs)
{
    if (scaleMeshAABBs) {
        for (int i = 0; i < model->meshCount; i++) {
            model->meshes[i]->aabb.min *= scale;
            model->meshes[i]->aabb.max *= scale;
        }
    }

    model->aabb.min *= scale;
    model->aabb.max *= scale;
}

NX_ModelAnimation** NX_LoadModelAnimations(const char* filePath, int* animCount, int targetFrameRate)
{
    size_t fileSize = 0;
    void* fileData = NX_LoadFile(filePath, &fileSize);
    NX_ModelAnimation** animations = gRender->models.loadAnimations(fileData, fileSize, helper::getFileExt(filePath), animCount, targetFrameRate);
    SDL_free(fileData);
    return animations;
}

NX_ModelAnimation** NX_LoadModelAnimationsFromMemory(const void* data, unsigned int size, const char* hint, int* animCount, int targetFrameRate)
{
    return gRender->models.loadAnimations(data, size, hint, animCount, targetFrameRate);
}

void NX_DestroyModelAnimations(NX_ModelAnimation** animations, int animCount)
{
    gRender->models.destroyAnimations(animations, animCount);
}

NX_ModelAnimation* NX_GetModelAnimation(NX_ModelAnimation** animations, int animCount, const char* name)
{
    for (int i = 0; i < animCount; i++) {
        if (SDL_strcmp(animations[i]->name, name) == 0) {
            return animations[i];
        }
    }
    return nullptr;
}

/* === Light - Public API === */

NX_Light* NX_CreateLight(NX_LightType type)
{
    return gRender->scene.lights().create(type);
}

void NX_DestroyLight(NX_Light* light)
{
    gRender->scene.lights().destroy(light);
}

bool NX_IsLightActive(const NX_Light* light)
{
    return light->isActive();
}

void NX_SetLightActive(NX_Light* light, bool active)
{
    light->setActive(active);
}

NX_Layer NX_GetLightLayerMask(const NX_Light* light)
{
    return light->layerMask();
}

void NX_SetLightLayerMask(NX_Light* light, NX_Layer layers)
{
    light->setLayerMask(layers);
}

NX_Layer NX_GetLightCullMask(const NX_Light* light)
{
    return light->cullMask();
}

void NX_SetLightCullMask(NX_Light* light, NX_Layer layers)
{
    light->setCullMask(layers);
}

NX_Vec3 NX_GetLightPosition(const NX_Light* light)
{
    return light->position();
}

void NX_SetLightPosition(NX_Light* light, NX_Vec3 position)
{
    light->setPosition(position);
}

NX_Vec3 NX_GetLightDirection(const NX_Light* light)
{
    return light->position();
}

void NX_SetLightDirection(NX_Light* light, NX_Vec3 direction)
{
    light->setDirection(direction);
}

NX_Color NX_GetLightColor(const NX_Light* light)
{
    return light->color();
}

void NX_SetLightColor(NX_Light* light, NX_Color color)
{
    light->setColor(color);
}

float NX_GetLightEnergy(const NX_Light* light)
{
    return light->energy();
}

void NX_SetLightEnergy(NX_Light* light, float energy)
{
    light->setEnergy(energy);
}

float NX_GetLightSpecular(const NX_Light* light)
{
    return light->specular();
}

void NX_SetLightSpecular(NX_Light* light, float specular)
{
    light->setSpecular(specular);
}

float NX_GetLightRange(const NX_Light* light)
{
    return light->range();
}

void NX_SetLightRange(NX_Light* light, float range)
{
    light->setRange(range);
}

float NX_GetLightAttenuation(const NX_Light* light)
{
    return light->energy();
}

void NX_SetLightAttenuation(NX_Light* light, float attenuation)
{
    light->setAttenuation(attenuation);
}

float NX_GetLightInnerCutOff(const NX_Light* light)
{
    return light->innerCutOff();
}

void NX_SetLightInnerCutOff(NX_Light* light, float radians)
{
    light->setInnerCutOff(radians);
}

float NX_GetLightOuterCutOff(const NX_Light* light)
{
    return light->outerCutOff();
}

void NX_SetLightOuterCutOff(NX_Light* light, float radians)
{
    light->setOuterCutOff(radians);
}

void NX_SetLightCutOff(NX_Light* light, float inner, float outer)
{
    light->setInnerCutOff(inner);
    light->setOuterCutOff(outer);
}

bool NX_IsShadowActive(const NX_Light* light)
{
    return light->isShadowActive();
}

void NX_SetShadowActive(NX_Light* light, bool active)
{
    light->setShadowActive(active);
}

NX_Layer NX_GetShadowCullMask(const NX_Light* light)
{
    return light->shadowCullMask();
}

void NX_SetShadowCullMask(NX_Light* light, NX_Layer layers)
{
    light->setShadowCullMask(layers);
}

NXAPI float NX_GetShadowSlopeBias(NX_Light* light)
{
    return light->shadowSlopeBias();
}

NXAPI void NX_SetShadowSlopeBias(NX_Light* light, float slopeBias)
{
    light->setShadowSlopeBias(slopeBias);
}

NXAPI float NX_GetShadowBias(NX_Light* light)
{
    return light->shadowBias();
}

NXAPI void NX_SetShadowBias(NX_Light* light, float bias)
{
    light->setShadowBias(bias);
}

float NX_GetShadowSoftness(const NX_Light* light)
{
    return light->shadowSoftness();
}

void NX_SetShadowSoftness(NX_Light* light, float softness)
{
    light->setShadowSoftness(softness);
}

NX_ShadowUpdateMode NX_GetShadowUpdateMode(const NX_Light* light)
{
    return light->shadowUpdateMode();
}

void NX_SetShadowUpdateMode(NX_Light* light, NX_ShadowUpdateMode mode)
{
    light->setShadowUpdateMode(mode);
}

float NX_GetShadowUpdateInterval(const NX_Light* light)
{
    return light->shadowUpdateInterval();
}

void NX_SetShadowUpdateInterval(NX_Light* light, float sec)
{
    light->setShadowUpdateInterval(sec);
}

void NX_UpdateShadowMap(NX_Light* light)
{
    light->forceShadowMapUpdate();
}
