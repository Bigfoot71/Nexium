/**
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided "as-is", without any express or implied warranty. In no event
 * will the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including commercial
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not claim that you
 *   wrote the original software. If you use this software in a product, an acknowledgment
 *   in the product documentation would be appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *   as being the original software.
 *
 *   3. This notice may not be removed or altered from any source distribution.
 */

#include <Hyperion/HP_Render.h>
#include <Hyperion/HP_Image.h>
#include <Hyperion/HP_Math.h>

#include "./Core/HP_InternalLog.hpp"

#include "./Render/HP_RenderState.hpp"
#include "./Render/HP_InstanceBuffer.hpp"
#include "./Render/HP_Texture.hpp"
#include "./Render/HP_Font.hpp"
#include "./Detail/Helper.hpp"

/* === Texture - Public API === */

HP_Texture* HP_CreateTexture(const HP_Image* image)
{
    if (image == nullptr) {
        HP_INTERNAL_LOG(E, "RENDER: Failed to load texture; Image is null");
        return nullptr;
    }
    return gRender->textures.createTexture(*image);
}

HP_Texture* HP_LoadTexture(const char* filePath)
{
    HP_Image image = HP_LoadImage(filePath);
    HP_Texture* texture = HP_CreateTexture(&image);
    HP_DestroyImage(&image);
    return texture;
}

void HP_DestroyTexture(HP_Texture* texture)
{
    gRender->textures.destroyTexture(texture);
}

void HP_SetDefaultTextureFilter(HP_TextureFilter filter)
{
    gRender->textures.setDefaultFilter(filter);
}

void HP_SetDefaultTextureAnisotropy(float anisotropy)
{
    gRender->textures.setDefaultAnisotropy(anisotropy);
}

void HP_SetTextureParameters(HP_Texture* texture, HP_TextureFilter filter, HP_TextureWrap wrap, float anisotropy)
{
    texture->setParameters(filter, wrap, anisotropy);
}

void HP_SetTextureAnisotropy(HP_Texture* texture, float anisotropy)
{
    texture->setAnisotropy(anisotropy);
}

void HP_SetTextureFilter(HP_Texture* texture, HP_TextureFilter filter)
{
    texture->setFilter(filter);
}

void HP_SetTextureWrap(HP_Texture* texture, HP_TextureWrap wrap)
{
    texture->setWrap(wrap);
}

void HP_GenerateMipmap(HP_Texture* texture)
{
    texture->generateMipmap();
}

void HP_QueryTexture(HP_Texture* texture, int* w, int* h)
{
    if (w) *w = texture->width();
    if (h) *h = texture->height();
}

/* === Font - Public API === */

HP_Font* HP_LoadFont(const char* filePath, HP_FontType type, int baseSize, int* codepoints, int codepointCount)
{
    size_t dataSize = 0;
    void* fileData = HP_LoadFile(filePath, &dataSize);
    HP_Font* font = HP_LoadFontFromMem(fileData, dataSize, type, baseSize, codepoints, codepointCount);
    SDL_free(fileData);
    return font;
}

HP_Font* HP_LoadFontFromMem(const void* fileData, size_t dataSize, HP_FontType type, int baseSize, int* codepoints, int codepointCount)
{
    return gRender->fonts.create(fileData, dataSize, type, baseSize, codepoints, codepointCount);
}

void HP_DestroyFont(HP_Font* font)
{
    gRender->fonts.destroy(font);
}

HP_Vec2 HP_MeasureCodepoints(const HP_Font* font, const int* codepoints, int length, float fontSize, HP_Vec2 spacing)
{
    const HP_Font& fnt = font ? *font : gRender->assets.font();
    return fnt.measureCodepoints(codepoints, length, fontSize, spacing);
}

HP_Vec2 HP_MeasureText(const HP_Font* font, const char* text, float fontSize, HP_Vec2 spacing)
{
    const HP_Font& fnt = font ? *font : gRender->assets.font();
    return fnt.measureText(text, fontSize, spacing);
}

/* === RenderTexture - Public API === */

HP_RenderTexture* HP_CreateRenderTexture(int w, int h)
{
    return gRender->textures.createRenderTexture(w, h);
}

void HP_DestroyRenderTexture(HP_RenderTexture* target)
{
    gRender->textures.destroyRenderTexture(target);
}

HP_Texture* HP_GetRenderTexture(HP_RenderTexture* target)
{
    return &target->texture();
}

void HP_BlitRenderTexture(const HP_RenderTexture* target, int xDst, int yDst, int wDst, int hDst, bool linear)
{
    target->blit(xDst, yDst, wDst, hDst, linear);
}

/* === Draw2D - Public API === */

void HP_Begin2D(HP_RenderTexture* target)
{
    HP_IVec2 size = HP_GetWindowSize();
    gRender->overlay.setRenderTexture(target);
    gRender->overlay.setProjection(HP_Mat4Ortho(
        0, size.x, size.y, 0, 0, 1
    ));
    gRender->overlay.clear();
}

void HP_End2D(void)
{
    gRender->overlay.flush();
    gRender->overlay.blit();
}

void HP_SetColor2D(HP_Color color)
{
    gRender->overlay.setColor(color);
}

void HP_SetTexture2D(const HP_Texture* texture)
{
    gRender->overlay.setTexture(texture);
}

void HP_SetFont2D(const HP_Font* font)
{
    gRender->overlay.setFont(font);
}

void HP_DrawTriangle2D(HP_Vec2 p0, HP_Vec2 p1, HP_Vec2 p2)
{
    gRender->overlay.ensureDrawCall(overlay::DrawCall::Mode::SHAPE, 3, 3);

    uint16_t baseIndex = gRender->overlay.nextVertexIndex();

    gRender->overlay.addVertex(p0.x, p0.y, 0.0f, 0.0f);
    gRender->overlay.addVertex(p1.x, p1.y, 0.5f, 1.0f);
    gRender->overlay.addVertex(p2.x, p2.y, 1.0f, 0.0f);

    gRender->overlay.addIndex(baseIndex + 0);
    gRender->overlay.addIndex(baseIndex + 1);
    gRender->overlay.addIndex(baseIndex + 2);
}

void HP_DrawTriangleBorder2D(HP_Vec2 p0, HP_Vec2 p1, HP_Vec2 p2, float thickness)
{
    HP_DrawLine2D(p0, p1, thickness);
    HP_DrawLine2D(p1, p2, thickness);
    HP_DrawLine2D(p2, p0, thickness);
}

void HP_DrawTriangleList2D(const HP_Vertex2D* triangles, int triangleCount)
{
    if (triangleCount <= 0) return;
    for (int i = 0; i < triangleCount; i++) {
        gRender->overlay.ensureDrawCall(overlay::DrawCall::Mode::SHAPE, 3, 3);
        uint16_t baseIndex = gRender->overlay.nextVertexIndex();
        const HP_Vertex2D* tri = &triangles[i * 3];
        for (int j = 0; j < 3; j++) {
            gRender->overlay.addVertex(tri[j]);
        }
        gRender->overlay.addIndex(baseIndex + 0);
        gRender->overlay.addIndex(baseIndex + 1);
        gRender->overlay.addIndex(baseIndex + 2);
    }
}

void HP_DrawTriangleStrip2D(const HP_Vertex2D* vertices, int count)
{
    if (count < 3) return;
    for (int i = 0; i < count - 2; i++) {
        gRender->overlay.ensureDrawCall(overlay::DrawCall::Mode::SHAPE, 3, 3);
        uint16_t baseIndex = gRender->overlay.nextVertexIndex();
        if (i % 2 == 0) {
            gRender->overlay.addVertex(vertices[i]);
            gRender->overlay.addVertex(vertices[i + 1]);
            gRender->overlay.addVertex(vertices[i + 2]);
        }
        else {
            gRender->overlay.addVertex(vertices[i]);
            gRender->overlay.addVertex(vertices[i + 2]);
            gRender->overlay.addVertex(vertices[i + 1]);
        }
        gRender->overlay.addIndex(baseIndex + 0);
        gRender->overlay.addIndex(baseIndex + 1);
        gRender->overlay.addIndex(baseIndex + 2);
    }
}

void HP_DrawTriangleFan2D(const HP_Vertex2D* vertices, int count)
{
    if (count < 3) return;
    for (int i = 1; i < count - 1; i++) {
        gRender->overlay.ensureDrawCall(overlay::DrawCall::Mode::SHAPE, 3, 3);
        uint16_t baseIndex = gRender->overlay.nextVertexIndex();
        gRender->overlay.addVertex(vertices[0]);
        gRender->overlay.addVertex(vertices[i]);
        gRender->overlay.addVertex(vertices[i + 1]);
        gRender->overlay.addIndex(baseIndex + 0);
        gRender->overlay.addIndex(baseIndex + 1);
        gRender->overlay.addIndex(baseIndex + 2);
    }
}

void HP_DrawQuad2D(HP_Vec2 p0, HP_Vec2 p1, HP_Vec2 p2, HP_Vec2 p3)
{
    gRender->overlay.ensureDrawCall(overlay::DrawCall::Mode::SHAPE, 4, 6);

    uint16_t baseIndex = gRender->overlay.nextVertexIndex();

    gRender->overlay.addVertex(p0.x, p0.y, 0.0f, 0.0f);
    gRender->overlay.addVertex(p1.x, p1.y, 1.0f, 0.0f);
    gRender->overlay.addVertex(p2.x, p2.y, 1.0f, 1.0f);
    gRender->overlay.addVertex(p3.x, p3.y, 0.0f, 1.0f);

    gRender->overlay.addIndex(baseIndex + 0);
    gRender->overlay.addIndex(baseIndex + 1);
    gRender->overlay.addIndex(baseIndex + 2);

    gRender->overlay.addIndex(baseIndex + 0);
    gRender->overlay.addIndex(baseIndex + 2);
    gRender->overlay.addIndex(baseIndex + 3);
}

void HP_DrawQuadBorder2D(HP_Vec2 p0, HP_Vec2 p1, HP_Vec2 p2, HP_Vec2 p3, float thickness)
{
    HP_DrawLine2D(p0, p1, thickness);
    HP_DrawLine2D(p1, p2, thickness);
    HP_DrawLine2D(p2, p3, thickness);
    HP_DrawLine2D(p3, p0, thickness);
}

void HP_DrawQuadList2D(const HP_Vertex2D* quads, int quadCount)
{
    if (quadCount <= 0) return;
    for (int i = 0; i < quadCount; i++) {
        gRender->overlay.ensureDrawCall(overlay::DrawCall::Mode::SHAPE, 4, 6);
        uint16_t baseIndex = gRender->overlay.nextVertexIndex();
        const HP_Vertex2D* quad = &quads[i * 4];
        for (int j = 0; j < 4; j++) {
            gRender->overlay.addVertex(quad[j]);
        }
        gRender->overlay.addIndex(baseIndex + 0);
        gRender->overlay.addIndex(baseIndex + 1);
        gRender->overlay.addIndex(baseIndex + 2);
        gRender->overlay.addIndex(baseIndex + 0);
        gRender->overlay.addIndex(baseIndex + 2);
        gRender->overlay.addIndex(baseIndex + 3);
    }
}

void HP_DrawQuadStrip2D(const HP_Vertex2D* vertices, int count)
{
    if (count < 4 || count % 2 != 0) return;
    for (int i = 0; i < count - 2; i += 2) {
        gRender->overlay.ensureDrawCall(overlay::DrawCall::Mode::SHAPE, 4, 6);
        uint16_t baseIndex = gRender->overlay.nextVertexIndex();
        gRender->overlay.addVertex(vertices[i]);
        gRender->overlay.addVertex(vertices[i + 1]);
        gRender->overlay.addVertex(vertices[i + 3]);
        gRender->overlay.addVertex(vertices[i + 2]);
        gRender->overlay.addIndex(baseIndex + 0);
        gRender->overlay.addIndex(baseIndex + 1);
        gRender->overlay.addIndex(baseIndex + 2);
        gRender->overlay.addIndex(baseIndex + 0);
        gRender->overlay.addIndex(baseIndex + 2);
        gRender->overlay.addIndex(baseIndex + 3);
    }
}

void HP_DrawQuadFan2D(const HP_Vertex2D* vertices, int count)
{
    if (count < 4) return;
    for (int i = 1; i < count - 2; i += 2) {
        if (i + 2 >= count) break;
        gRender->overlay.ensureDrawCall(overlay::DrawCall::Mode::SHAPE, 4, 6);
        uint16_t baseIndex = gRender->overlay.nextVertexIndex();
        gRender->overlay.addVertex(vertices[0]);
        gRender->overlay.addVertex(vertices[i]);
        gRender->overlay.addVertex(vertices[i + 1]);
        gRender->overlay.addVertex(vertices[i + 2]);
        gRender->overlay.addIndex(baseIndex + 0);
        gRender->overlay.addIndex(baseIndex + 1);
        gRender->overlay.addIndex(baseIndex + 2);
        gRender->overlay.addIndex(baseIndex + 0);
        gRender->overlay.addIndex(baseIndex + 2);
        gRender->overlay.addIndex(baseIndex + 3);
    }
}

void HP_DrawLine2D(HP_Vec2 p0, HP_Vec2 p1, float thickness)
{
    gRender->overlay.ensureDrawCall(overlay::DrawCall::Mode::SHAPE, 4, 6);

    float dx = p1.x - p0.x;
    float dy = p1.y - p0.y;
    float len_sq = dx * dx + dy * dy;

    if (len_sq < 1e-6f) {
        return;
    }

    float invLen = 1.0f / sqrtf(len_sq);

    dx *= invLen;
    dy *= invLen;

    float nx = -dy * thickness * 0.5f;
    float ny = +dx * thickness * 0.5f;

    uint16_t baseIndex = gRender->overlay.nextVertexIndex();

    gRender->overlay.addVertex(p0.x + nx, p0.y + ny, 0.0f, 0.0f);
    gRender->overlay.addVertex(p0.x - nx, p0.y - ny, 1.0f, 0.0f);
    gRender->overlay.addVertex(p1.x - nx, p1.y - ny, 1.0f, 1.0f);
    gRender->overlay.addVertex(p1.x + nx, p1.y + ny, 0.0f, 1.0f);

    // Triangle 1: 0, 1, 2
    gRender->overlay.addIndex(baseIndex + 0);
    gRender->overlay.addIndex(baseIndex + 1);
    gRender->overlay.addIndex(baseIndex + 2);

    // Triangle 2: 0, 2, 3
    gRender->overlay.addIndex(baseIndex + 0);
    gRender->overlay.addIndex(baseIndex + 2);
    gRender->overlay.addIndex(baseIndex + 3);
}

void HP_DrawLineList2D(const HP_Vec2* lines, int lineCount, float thickness)
{
    if (lineCount <= 0) return;
    for (int i = 0; i < lineCount; i++) {
        const HP_Vec2* line = &lines[i * 2];
        HP_DrawLine2D(line[0], line[1], thickness);
    }
}

void HP_DrawLineStrip2D(const HP_Vec2* points, int count, float thickness)
{
    if (count < 2) return;
    for (int i = 0; i < count - 1; i++) {
        HP_DrawLine2D(points[i], points[i + 1], thickness);
    }
}

void HP_DrawLineLoop2D(const HP_Vec2* points, int count, float thickness)
{
    if (count < 2) return;
    for (int i = 0; i < count - 1; i++) {
        HP_DrawLine2D(points[i], points[i + 1], thickness);
    }
    HP_DrawLine2D(points[count - 1], points[0], thickness);
}

void HP_DrawRect2D(float x, float y, float w, float h)
{
    HP_DrawQuad2D(
        HP_VEC2(x, y),
        HP_VEC2(x + w, y),
        HP_VEC2(x + w, y + h),
        HP_VEC2(x, y + h)
    );
}

void HP_DrawRectBorder2D(float x, float y, float w, float h, float thickness)
{
    HP_DrawQuadBorder2D(
        HP_VEC2(x, y),
        HP_VEC2(x + w, y),
        HP_VEC2(x + w, y + h),
        HP_VEC2(x, y + h),
        thickness
    );
}

void HP_DrawRectEx2D(HP_Vec2 center, HP_Vec2 size, HP_Vec2 pivot, float rotation)
{
    float halfW = size.x * 0.5f;
    float halfH = size.y * 0.5f;

    float pivotOffsetX = (pivot.x - 0.5f) * size.x;
    float pivotOffsetY = (pivot.y - 0.5f) * size.y;

    HP_Vec2 adjustedCenter = {
        center.x - pivotOffsetX,
        center.y - pivotOffsetY
    };

    float cosR = cosf(rotation);
    float sinR = sinf(rotation);

    HP_Vec2 tl = {
        adjustedCenter.x + (-halfW * cosR - -halfH * sinR),
        adjustedCenter.y + (-halfW * sinR + -halfH * cosR)
    };

    HP_Vec2 tr = {
        adjustedCenter.x + ( halfW * cosR - -halfH * sinR),
        adjustedCenter.y + ( halfW * sinR + -halfH * cosR)
    };

    HP_Vec2 br = {
        adjustedCenter.x + ( halfW * cosR -  halfH * sinR),
        adjustedCenter.y + ( halfW * sinR +  halfH * cosR)
    };

    HP_Vec2 bl = {
        adjustedCenter.x + (-halfW * cosR -  halfH * sinR),
        adjustedCenter.y + (-halfW * sinR +  halfH * cosR)
    };

    HP_DrawQuad2D(tl, tr, br, bl);
}

void HP_DrawRectBorderEx2D(HP_Vec2 center, HP_Vec2 size, HP_Vec2 pivot, float rotation, float thickness)
{
    float halfW = size.x * 0.5f;
    float halfH = size.y * 0.5f;

    float pivotOffsetX = (pivot.x - 0.5f) * size.x;
    float pivotOffsetY = (pivot.y - 0.5f) * size.y;

    HP_Vec2 adjustedCenter = {
        center.x - pivotOffsetX,
        center.y - pivotOffsetY
    };

    float cosR = cosf(rotation);
    float sinR = sinf(rotation);

    HP_Vec2 tl = {
        adjustedCenter.x + (-halfW * cosR - -halfH * sinR),
        adjustedCenter.y + (-halfW * sinR + -halfH * cosR)
    };

    HP_Vec2 tr = {
        adjustedCenter.x + ( halfW * cosR - -halfH * sinR),
        adjustedCenter.y + ( halfW * sinR + -halfH * cosR)
    };

    HP_Vec2 br = {
        adjustedCenter.x + ( halfW * cosR -  halfH * sinR),
        adjustedCenter.y + ( halfW * sinR +  halfH * cosR)
    };

    HP_Vec2 bl = {
        adjustedCenter.x + (-halfW * cosR -  halfH * sinR),
        adjustedCenter.y + (-halfW * sinR +  halfH * cosR)
    };

    HP_DrawQuadBorder2D(tl, tr, br, bl, thickness);
}

void HP_DrawRectRounded2D(float x, float y, float w, float h, float radius, int segments)
{
    radius = fminf(radius, fminf(w * 0.5f, h * 0.5f));

    if (radius <= 0.0f) {
        HP_DrawQuad2D(HP_VEC2(x, y), HP_VEC2(x + w, y), HP_VEC2(x + w, y + h), HP_VEC2(x, y + h));
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
        {x + radius, y + radius, HP_PI, HP_PI * 1.5f},          // Top-left
        {x + w - radius, y + radius, HP_PI * 1.5f, HP_PI * 2.0f}, // Top-right
        {x + w - radius, y + h - radius, 0.0f, HP_PI * 0.5f},     // Bottom-right
        {x + radius, y + h - radius, HP_PI * 0.5f, HP_PI}         // Bottom-left
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

void HP_DrawRectRoundedBorder2D(float x, float y, float w, float h, float radius, int segments, float thickness)
{
    radius = fminf(radius, fminf(w * 0.5f, h * 0.5f));

    if (radius <= 0.0f) {
        HP_Vec2 corners[4] = {
            {x, y}, {x + w, y}, {x + w, y + h}, {x, y + h}
        };
        for (int i = 0; i < 4; i++) {
            HP_DrawLine2D(corners[i], corners[(i + 1) % 4], thickness);
        }
        return;
    }

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
        {x + radius, y + radius, HP_PI, HP_PI * 1.5f},
        {x + w - radius, y + radius, HP_PI * 1.5f, HP_PI * 2.0f},
        {x + w - radius, y + h - radius, 0.0f, HP_PI * 0.5f},
        {x + radius, y + h - radius, HP_PI * 0.5f, HP_PI}
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

void HP_DrawRectRoundedEx2D(HP_Vec2 center, HP_Vec2 size, HP_Vec2 pivot, float rotation, float radius)
{
    // For rotated rectangles, a tessellation approach is used
    // because it is complex to transform the arcs directly

    float maxRadius = fminf(size.x * 0.5f, size.y * 0.5f);
    radius = fminf(radius, maxRadius);

    float pivotOffsetX = (pivot.x - 0.5f) * size.x;
    float pivotOffsetY = (pivot.y - 0.5f) * size.y;

    HP_Vec2 adjustedCenter = {
        center.x - pivotOffsetX,
        center.y - pivotOffsetY
    };

    #define SEGMENTS 8
    #define TOTAL_POINTS (4 * SEGMENTS)

    HP_Vec2 points[TOTAL_POINTS];

    float halfW = size.x * 0.5f;
    float halfH = size.y * 0.5f;

    float cosR = cosf(rotation);
    float sinR = sinf(rotation);

    float angleStep = (HP_PI * 0.5f) / (SEGMENTS - 1);
    float cosStep = cosf(angleStep);
    float sinStep = sinf(angleStep);

    int pointIndex = 0;

    for (int corner = 0; corner < 4; corner++) {
        float cornerX, cornerY, startAngle;
        switch (corner) {
            case 0: // Top-left
                cornerX = -halfW + radius;
                cornerY = -halfH + radius;
                startAngle = HP_PI;
                break;
            case 1: // Top-right
                cornerX = halfW - radius;
                cornerY = -halfH + radius;
                startAngle = HP_PI * 1.5f;
                break;
            case 2: // Bottom-right
                cornerX = halfW - radius;
                cornerY = halfH - radius;
                startAngle = 0;
                break;
            case 3: // Bottom-left
                cornerX = -halfW + radius;
                cornerY = halfH - radius;
                startAngle = HP_PI * 0.5f;
                break;
        }

        float cosCurrent = cosf(startAngle);
        float sinCurrent = sinf(startAngle);

        for (int i = 0; i < SEGMENTS; i++) {
            float localX = cornerX + radius * cosCurrent;
            float localY = cornerY + radius * sinCurrent;

            float rotX = localX * cosR - localY * sinR;
            float rotY = localX * sinR + localY * cosR;

            points[pointIndex].x = adjustedCenter.x + rotX;
            points[pointIndex].y = adjustedCenter.y + rotY;
            pointIndex++;

            if (i < SEGMENTS - 1) {
                float newCos = cosCurrent * cosStep - sinCurrent * sinStep;
                float newSin = sinCurrent * cosStep + cosCurrent * sinStep;
                cosCurrent = newCos;
                sinCurrent = newSin;
            }
        }
    }

    for (int i = 1; i < TOTAL_POINTS - 1; i++) {
        HP_DrawTriangle2D(points[0], points[i], points[i + 1]);
    }

    #undef TOTAL_POINTS
    #undef SEGMENTS
}

void HP_DrawRectRoundedBorderEx2D(HP_Vec2 center, HP_Vec2 size, HP_Vec2 pivot, float rotation, float radius, float thickness)
{
    float maxRadius = fminf(size.x * 0.5f, size.y * 0.5f);
    radius = fminf(radius, maxRadius);

    float pivotOffsetX = (pivot.x - 0.5f) * size.x;
    float pivotOffsetY = (pivot.y - 0.5f) * size.y;

    HP_Vec2 adjustedCenter = {
        center.x - pivotOffsetX,
        center.y - pivotOffsetY
    };

    #define SEGMENTS 8
    #define TOTAL_POINTS (4 * SEGMENTS)

    HP_Vec2 points[TOTAL_POINTS];

    float halfW = size.x * 0.5f;
    float halfH = size.y * 0.5f;

    float cosR = cosf(rotation);
    float sinR = sinf(rotation);

    float angleStep = (HP_PI * 0.5f) / (SEGMENTS - 1);
    float cosStep = cosf(angleStep);
    float sinStep = sinf(angleStep);

    int pointIndex = 0;

    for (int corner = 0; corner < 4; corner++) {
        float cornerX, cornerY, startAngle;
        switch (corner) {
        case 0:
            cornerX = -halfW + radius;
            cornerY = -halfH + radius;
            startAngle = HP_PI;
            break;
        case 1:
            cornerX = halfW - radius;
            cornerY = -halfH + radius;
            startAngle = HP_PI * 1.5f;
            break;
        case 2:
            cornerX = halfW - radius;
            cornerY = halfH - radius;
            startAngle = 0;
            break;
        case 3:
            cornerX = -halfW + radius;
            cornerY = halfH - radius;
            startAngle = HP_PI * 0.5f;
            break;
        }

        float cosCurrent = cosf(startAngle);
        float sinCurrent = sinf(startAngle);

        for (int i = 0; i < SEGMENTS; i++) {
            float localX = cornerX + radius * cosCurrent;
            float localY = cornerY + radius * sinCurrent;

            float rotX = localX * cosR - localY * sinR;
            float rotY = localX * sinR + localY * cosR;

            points[pointIndex].x = adjustedCenter.x + rotX;
            points[pointIndex].y = adjustedCenter.y + rotY;
            pointIndex++;

            if (i < SEGMENTS - 1) {
                float newCos = cosCurrent * cosStep - sinCurrent * sinStep;
                float newSin = sinCurrent * cosStep + cosCurrent * sinStep;
                cosCurrent = newCos;
                sinCurrent = newSin;
            }
        }
    }

    for (int i = 0; i < TOTAL_POINTS; i++) {
        int next = (i + 1) % TOTAL_POINTS;
        HP_DrawLine2D(points[i], points[next], thickness);
    }

    #undef TOTAL_POINTS
    #undef SEGMENTS
}

void HP_DrawCircle2D(HP_Vec2 center, float radius, int segments)
{
    if (segments < 3) segments = 32;

    gRender->overlay.ensureDrawCall(overlay::DrawCall::Mode::SHAPE, segments + 1, segments * 3);

    uint16_t baseIndex = gRender->overlay.nextVertexIndex();

    gRender->overlay.addVertex(center.x, center.y, 0.5f, 0.5f);

    float delta = (2.0f * HP_PI) / (float)segments;
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

void HP_DrawCircleBorder2D(HP_Vec2 p, float radius, int segments, float thickness)
{
    if (segments < 3) segments = 32;

    float delta = (2.0f * HP_PI) / (float)segments;
    float cosDelta = cosf(delta);
    float sinDelta = sinf(delta);

    float cx = radius;
    float cy = 0.0f;

    HP_Vec2 prev = { p.x + cx, p.y + cy };

    for (int i = 1; i <= segments; i++) {
        float new_cx = cx * cosDelta - cy * sinDelta;
        float new_cy = cx * sinDelta + cy * cosDelta;
        cx = new_cx;
        cy = new_cy;

        HP_Vec2 curr = { p.x + cx, p.y + cy };

        // Line between prev and curr
        HP_DrawLine2D(prev, curr, thickness);

        prev = curr;
    }
}

void HP_DrawEllipse2D(HP_Vec2 center, HP_Vec2 radius, int segments)
{
    if (segments < 3) segments = 32;

    gRender->overlay.ensureDrawCall(overlay::DrawCall::Mode::SHAPE, segments + 1, segments * 3);

    uint16_t baseIndex = gRender->overlay.nextVertexIndex();

    gRender->overlay.addVertex(center.x, center.y, 0.5f, 0.5f);

    float delta = (2.0f * HP_PI) / (float)segments;
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

void HP_DrawEllipseBorder2D(HP_Vec2 p, HP_Vec2 r, int segments, float thickness)
{
    if (segments < 3) segments = 32;

    float delta = (2.0f * HP_PI) / (float)segments;
    float cosDelta = cosf(delta);
    float sinDelta = sinf(delta);

    float ux = 1.0f;
    float uy = 0.0f;

    HP_Vec2 prev = { p.x + r.x * ux, p.y + r.y * uy };

    for (int i = 1; i <= segments; i++) {
        float newUX = ux * cosDelta - uy * sinDelta;
        float newUY = ux * sinDelta + uy * cosDelta;
        ux = newUX;
        uy = newUY;

        HP_Vec2 curr = (HP_Vec2){ p.x + r.x * ux, p.y + r.y * uy };

        HP_DrawLine2D(prev, curr, thickness);
        prev = curr;
    }
}

void HP_DrawPieSlice2D(HP_Vec2 center, float radius, float startAngle, float endAngle, int segments)
{
    if (segments < 1) segments = 16;

    float angleDiff = endAngle - startAngle;
    angleDiff = HP_WrapRadians(angleDiff);
    if (angleDiff < 0.0f) angleDiff += HP_TAU;

    float deltaAngle = angleDiff / (float)segments;

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

void HP_DrawPieSliceBorder2D(HP_Vec2 center, float radius, float startAngle, float endAngle, int segments, float thickness)
{
    if (segments < 1) segments = 16;

    float angleDiff = endAngle - startAngle;
    angleDiff = HP_WrapRadians(angleDiff);
    if (angleDiff < 0.0f) angleDiff += HP_TAU;

    float deltaAngle = angleDiff / (float)segments;
    float cosDelta = cosf(deltaAngle);
    float sinDelta = sinf(deltaAngle);

    float cosA = cosf(startAngle);
    float sinA = sinf(startAngle);

    HP_Vec2 start_pt = { center.x + radius * cosA, center.y + radius * sinA };
    HP_DrawLine2D(center, start_pt, thickness);

    HP_Vec2 prev = start_pt;
    for (int i = 1; i <= segments; i++) {
        float newCos = cosA * cosDelta - sinA * sinDelta;
        float newSin = sinA * cosDelta + cosA * sinDelta;
        cosA = newCos;
        sinA = newSin;

        HP_Vec2 curr = { center.x + radius * cosA, center.y + radius * sinA };
        HP_DrawLine2D(prev, curr, thickness);
        prev = curr;
    }

    HP_DrawLine2D(prev, center, thickness);
}

void HP_DrawRing2D(HP_Vec2 center, float innerRadius, float outerRadius, int segments)
{
    if (segments < 3) segments = 32;
    if (innerRadius >= outerRadius) return;

    gRender->overlay.ensureDrawCall(overlay::DrawCall::Mode::SHAPE, segments * 2, segments * 6);

    uint16_t baseIndex = gRender->overlay.nextVertexIndex();

    float deltaAngle = HP_TAU / (float)segments;
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

void HP_DrawRingBorder2D(HP_Vec2 center, float innerRadius, float outerRadius, int segments, float thickness)
{
    if (segments < 3) segments = 32;
    if (innerRadius >= outerRadius) return;

    float deltaAngle = HP_TAU / (float)segments;
    float cosDelta = cosf(deltaAngle);
    float sinDelta = sinf(deltaAngle);

    float cosA = 1.0f;
    float sinA = 0.0f;
    HP_Vec2 outerPrev = { center.x + outerRadius * cosA, center.y + outerRadius * sinA };
    HP_Vec2 innerPrev = { center.x + innerRadius * cosA, center.y + innerRadius * sinA };

    for (int i = 1; i <= segments; i++) {
        float newCos = cosA * cosDelta - sinA * sinDelta;
        float newSin = sinA * cosDelta + cosA * sinDelta;
        cosA = newCos;
        sinA = newSin;

        HP_Vec2 outerCurr = { center.x + outerRadius * cosA, center.y + outerRadius * sinA };
        HP_Vec2 innerCurr = { center.x + innerRadius * cosA, center.y + innerRadius * sinA };

        HP_DrawLine2D(outerPrev, outerCurr, thickness);
        HP_DrawLine2D(innerPrev, innerCurr, thickness);

        outerPrev = outerCurr;
        innerPrev = innerCurr;
    }
}

void HP_DrawRingArc2D(HP_Vec2 center, float innerRadius, float outerRadius,
                      float startAngle, float endAngle, int segments)
{
    if (segments < 1) segments = 16;
    if (innerRadius >= outerRadius) return;

    float angleDiff = endAngle - startAngle;
    angleDiff = HP_WrapRadians(angleDiff);
    if (angleDiff < 0.0f) angleDiff += HP_TAU;

    float deltaAngle = angleDiff / (float)segments;

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

void HP_DrawRingArcBorder2D(HP_Vec2 center, float innerRadius, float outerRadius,
                            float startAngle, float endAngle, int segments, float thickness)
{
    if (segments < 1) segments = 16;
    if (innerRadius >= outerRadius) return;

    float angleDiff = endAngle - startAngle;
    angleDiff = HP_WrapRadians(angleDiff);
    if (angleDiff < 0.0f) angleDiff += HP_TAU;

    float deltaAngle = angleDiff / (float)segments;
    float cosDelta = cosf(deltaAngle);
    float sinDelta = sinf(deltaAngle);

    float cosA = cosf(startAngle);
    float sinA = sinf(startAngle);

    HP_Vec2 outerStart = { center.x + outerRadius * cosA, center.y + outerRadius * sinA };
    HP_Vec2 innerStart = { center.x + innerRadius * cosA, center.y + innerRadius * sinA };

    HP_DrawLine2D(innerStart, outerStart, thickness);

    HP_Vec2 outerPrev = outerStart;
    HP_Vec2 innerPrev = innerStart;

    for (int i = 1; i <= segments; i++) {
        float newCos = cosA * cosDelta - sinA * sinDelta;
        float newSin = sinA * cosDelta + cosA * sinDelta;
        cosA = newCos;
        sinA = newSin;

        HP_Vec2 outerCurr = { center.x + outerRadius * cosA, center.y + outerRadius * sinA };
        HP_Vec2 innerCurr = { center.x + innerRadius * cosA, center.y + innerRadius * sinA };

        HP_DrawLine2D(outerPrev, outerCurr, thickness);
        HP_DrawLine2D(innerPrev, innerCurr, thickness);

        outerPrev = outerCurr;
        innerPrev = innerCurr;
    }

    HP_DrawLine2D(innerPrev, outerPrev, thickness);
}

void HP_DrawArc2D(HP_Vec2 center, float radius,
                   float startAngle, float endAngle,
                   int segments, float thickness)
{
    if (segments < 1) segments = 16;

    float angleDiff = endAngle - startAngle;
    angleDiff = HP_WrapRadians(angleDiff);
    if (angleDiff < 0.0f) angleDiff += HP_TAU;

    float deltaAngle = angleDiff / (float)segments;

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

        HP_DrawLine2D(HP_VEC2(prevX, prevY), HP_VEC2(curr_x, curr_y), thickness);

        prevX = curr_x;
        prevY = curr_y;
    }
}

void HP_DrawBezierQuad2D(HP_Vec2 p0, HP_Vec2 p1, HP_Vec2 p2, int segments, float thickness)
{
    if (segments < 1) segments = 20;

    float dt = 1.0f / (float)segments;
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

        HP_DrawLine2D(HP_VEC2(prevX, prevY), HP_VEC2(x, y), thickness);

        prevX = x;
        prevY = y;
    }
}

void HP_DrawBezierCubic2D(HP_Vec2 p0, HP_Vec2 p1, HP_Vec2 p2, HP_Vec2 p3, int segments, float thickness)
{
    if (segments < 1) segments = 30;

    float dt  = 1.0f / (float)segments;
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

        HP_DrawLine2D(HP_VEC2(prevX, prevY), HP_VEC2(x, y), thickness);

        prevX = x;
        prevY = y;
    }
}

void HP_DrawSpline2D(const HP_Vec2* points, int count, int segments, float thickness)
{
    if (count < 4) return;
    if (segments < 1) segments = 20;

    for (int i = 1; i < count - 2; i++) {
        HP_Vec2 p0 = points[i - 1];
        HP_Vec2 p1 = points[i];
        HP_Vec2 p2 = points[i + 1];
        HP_Vec2 p3 = points[i + 2];

        float prevX = p1.x;
        float prevY = p1.y;

        for (int j = 1; j <= segments; j++) {
            float t = (float)j / (float)segments;
            float t2 = t * t;
            float t3 = t2 * t;

            // Catmull-Rom coeffs
            float c0 = -0.5f * t3 + t2 - 0.5f * t;
            float c1 =  1.5f * t3 - 2.5f * t2 + 1.0f;
            float c2 = -1.5f * t3 + 2.0f * t2 + 0.5f * t;
            float c3 =  0.5f * t3 - 0.5f * t2;

            float x = c0 * p0.x + c1 * p1.x + c2 * p2.x + c3 * p3.x;
            float y = c0 * p0.y + c1 * p1.y + c2 * p2.y + c3 * p3.y;

            HP_DrawLine2D(HP_VEC2(prevX, prevY), HP_VEC2(x, y), thickness);

            prevX = x;
            prevY = y;
        }
    }
}

void HP_DrawCodepoint2D(int codepoint, HP_Vec2 position, float fontSize)
{
    /* --- Get current font and the glyph data --- */

    const HP_Font& font = gRender->overlay.currentFont();
    const HP_Font::Glyph& glyph = font.getGlyph(codepoint);

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

void HP_DrawCodepoints2D(const int* codepoints, int length, HP_Vec2 position, float fontSize, HP_Vec2 spacing)
{
    const HP_Font& font = gRender->overlay.currentFont();
    float scale = fontSize / font.baseSize();
    HP_Vec2 offset = HP_VEC2_ZERO;

    for (int i = 0; i < length; i++)
    {
        const HP_Font::Glyph& glyph = font.getGlyph(codepoints[i]);

        if (codepoints[i] == '\n') {
            offset.y += (fontSize + spacing.y);
            offset.x = 0.0f;
        }
        else {
            if (codepoints[i] != ' ' && codepoints[i] != '\t') {
                HP_DrawCodepoint2D(codepoints[i], position + offset, fontSize);
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

void HP_DrawText2D(const char* text, HP_Vec2 position, float fontSize, HP_Vec2 spacing)
{
    const HP_Font& font = gRender->overlay.currentFont();
    float scale = fontSize / font.baseSize();
    int size = (int)strlen(text);
    HP_Vec2 offset = HP_VEC2_ZERO;

    for (int i = 0; i < size;)
    {
        int codepointByteCount = 0;
        int codepoint = HP_GetCodepointNext(&text[i], &codepointByteCount);

        const HP_Font::Glyph& glyph = font.getGlyph(codepoint);

        if (codepoint == '\n') {
            offset.y += (fontSize + spacing.y);
            offset.x = 0.0f;
        }
        else {
            if (codepoint != ' ' && codepoint != '\t') {
                HP_DrawCodepoint2D(codepoint, position + offset, fontSize);
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

void HP_Begin3D(const HP_Camera* camera, const HP_Environment* env, const HP_RenderTexture* target)
{
    gRender->scene.begin(
        camera ? *camera : HP_GetDefaultCamera(),
        env ? *env : HP_GetDefaultEnvironment(),
        target
    );
}

void HP_End3D(void)
{
    gRender->scene.end();
}

void HP_DrawMesh3D(const HP_Mesh* mesh, const HP_Material* material, const HP_Transform* transform)
{
    gRender->scene.drawMesh(
        *mesh, nullptr, 0,
        material ? *material : HP_GetDefaultMaterial(),
        transform ? *transform : HP_TRANSFORM_IDENTITY
    );
}

void HP_DrawMeshInstanced3D(const HP_Mesh* mesh, const HP_InstanceBuffer* instances, int instanceCount,
                            const HP_Material* material, const HP_Transform* transform)
{
    gRender->scene.drawMesh(
        *mesh, instances, instanceCount,
        material ? *material : HP_GetDefaultMaterial(),
        transform ? *transform : HP_TRANSFORM_IDENTITY
    );
}

void HP_DrawModel3D(const HP_Model* model, const HP_Transform* transform)
{
    gRender->scene.drawModel(*model, nullptr, 0, transform ? *transform : HP_TRANSFORM_IDENTITY);
}

void HP_DrawModelInstanced3D(const HP_Model* model, const HP_InstanceBuffer* instances, int instanceCount, const HP_Transform* transform)
{
    gRender->scene.drawModel(*model, instances, instanceCount, transform ? *transform : HP_TRANSFORM_IDENTITY);
}

/* === Camera - Public API === */

HP_Camera HP_GetDefaultCamera(void)
{
    return HP_Camera {
        .position = HP_VEC3_ZERO,
        .rotation = HP_QUAT_IDENTITY,
        .nearPlane = 0.05f,
        .farPlane = 4000.0f,
        .fov = 60.0f * HP_DEG2RAD,
        .projection = HP_PROJECTION_PERSPECTIVE,
        .cullMask = HP_LAYER_ALL,
    };
}

void HP_UpdateCameraOrbital(HP_Camera* camera, HP_Vec3 center, float distance, float height, float rotation)
{
    camera->position.x = center.x + distance * cosf(rotation);
    camera->position.z = center.z + distance * sinf(rotation);
    camera->position.y = center.y + height;

    camera->rotation = HP_QuatLookAt(camera->position, center, HP_VEC3_UP);
}

void HP_UpdateCameraFree(HP_Camera* camera, HP_Vec3 movement, HP_Vec3 rotation, float maxPitch)
{
    /* --- Rotation --- */

    HP_Vec3 euler = HP_QuatToEuler(camera->rotation);

    euler.x += rotation.x;
    euler.y += rotation.y;
    euler.z += rotation.z;

    if (maxPitch < 0.0f) maxPitch = HP_PI * 0.49f;
    euler.x = HP_CLAMP(euler.x, -maxPitch, maxPitch);

    camera->rotation = HP_QuatFromEuler(euler);

    /* --- Translation --- */

    HP_Vec3 forward = HP_Vec3Rotate(HP_VEC3_FORWARD, camera->rotation);
    HP_Vec3 right   = HP_Vec3Rotate(HP_VEC3_RIGHT,   camera->rotation);
    HP_Vec3 up      = HP_VEC3_UP;

    HP_Vec3 deltaPos = HP_VEC3_ZERO;
    deltaPos = HP_Vec3MulAdd(forward, movement.z, deltaPos);
    deltaPos = HP_Vec3MulAdd(right, movement.x, deltaPos);
    deltaPos = HP_Vec3MulAdd(up, movement.y, deltaPos);

    camera->position = HP_Vec3Add(camera->position, deltaPos);
}

void HP_UpdateCameraFPS(HP_Camera* camera, HP_Vec3 movement, HP_Vec2 rotation, float maxPitch)
{
    /* --- Rotation --- */

    HP_Vec3 euler = HP_QuatToEuler(camera->rotation);

    euler.x += rotation.x;
    euler.y += rotation.y;

    if (maxPitch < 0.0f) maxPitch = HP_PI * 0.49f;
    euler.x = HP_CLAMP(euler.x, -maxPitch, maxPitch);

    camera->rotation = HP_QuatFromEuler(euler);

    /* --- Translation --- */

    HP_Vec3 forward = HP_Vec3Rotate(HP_VEC3_FORWARD, camera->rotation);
    forward.y = 0.0f;
    forward = HP_Vec3Normalize(forward);

    HP_Vec3 right = HP_Vec3Rotate(HP_VEC3_RIGHT, camera->rotation);
    right.y = 0.0f;
    right = HP_Vec3Normalize(right);

    HP_Vec3 up = HP_VEC3_UP;

    HP_Vec3 deltaPos = HP_VEC3_ZERO;
    deltaPos = HP_Vec3MulAdd(forward, movement.z, deltaPos);
    deltaPos = HP_Vec3MulAdd(right, movement.x, deltaPos);
    deltaPos = HP_Vec3MulAdd(up, movement.y, deltaPos);

    camera->position = HP_Vec3Add(camera->position, deltaPos);
}

void HP_ApplyCameraTransform(HP_Camera* camera, HP_Mat4 transform, HP_Vec3 offset)
{
    camera->rotation = HP_QuatFromMat4(&transform);

    HP_Vec3 transformPosition = HP_VEC3(transform.v[2][0], transform.v[2][1], transform.v[2][2]);
    HP_Vec3 rotatedOffset = HP_Vec3TransformByMat4(offset, &transform);

    camera->position = HP_Vec3Add(transformPosition, rotatedOffset);
}

/* === Environment - Public API === */

HP_Environment HP_GetDefaultEnvironment(void)
{
    return HP_Environment {
        .bounds = {
            HP_VEC3(-10, -10, -10),
            HP_VEC3(+10, +10, +10)
        },
        .background = HP_GRAY,
        .ambient = HP_DARK_GRAY,
        .sky = {
            .cubemap = nullptr,
            .probe = nullptr,
            .rotation = HP_QUAT_IDENTITY,
            .intensity = 1.0f,
            .specular = 1.0f,
            .diffuse = 1.0f
        },
        .ssao = {
            .intensity = 1.0f,
            .radius = 0.5f,
            .power = 1.0f,
            .bias = 0.025f,
            .enabled = false
        },
        .adjustment = {
            .brightness = 1.0f,
            .contrast = 1.0f,
            .saturation = 1.0f
        },
        .tonemap = {
            .mode = HP_TONEMAP_LINEAR,
            .exposure = 1.0f,
            .white = 1.0f
        }
    };
}

/* === Skybox - Public API === */

HP_Cubemap* HP_CreateCubemap(const HP_Image* image)
{
    if (image != nullptr) {
        return gRender->cubemaps.createCubemap(*image);
    }
    return nullptr;
}

HP_Cubemap* HP_LoadCubemap(const char* filePath)
{
    HP_Image image = HP_LoadImage(filePath);
    HP_Cubemap* cubemap = HP_CreateCubemap(&image);
    HP_DestroyImage(&image);
    return cubemap;
}

void HP_DestroyCubemap(HP_Cubemap* cubemap)
{
    gRender->cubemaps.destroyCubemap(cubemap);
}

/* === ReflectionProbe - Public API === */

HP_ReflectionProbe* HP_CreateReflectionProbe(HP_Cubemap* cubemap)
{
    if (cubemap != nullptr) {
        return gRender->cubemaps.createReflectionProbe(*cubemap);
    }
    return nullptr;
}

HP_ReflectionProbe* HP_LoadReflectionProbe(const char* filePath)
{
    HP_Cubemap* cubemap = HP_LoadCubemap(filePath);
    if (cubemap == nullptr) return nullptr;

    HP_ReflectionProbe* probe = HP_CreateReflectionProbe(cubemap);
    HP_DestroyCubemap(cubemap);

    return probe;
}

void HP_DestroyReflectionProbe(HP_ReflectionProbe* probe)
{
    gRender->cubemaps.destroyReflectionProbe(probe);
}

void HP_UpdateReflectionProbe(HP_ReflectionProbe* probe, const HP_Cubemap* cubemap)
{
    if (probe != nullptr && cubemap != nullptr) {
        gRender->cubemaps.updateReflectionProbe(probe, *cubemap);
    }
}

/* === Material - Public API === */

HP_Material HP_GetDefaultMaterial(void)
{
    return HP_Material {
        .albedo = {
            .texture = nullptr,
            .color = HP_WHITE,
        },
        .emission = {
            .texture = nullptr,
            .color = HP_WHITE,
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
        .alphaCutOff = 1e-6f,
        .texOffset = HP_VEC2_ZERO,
        .texScale = HP_VEC2_ONE,
        .billboard = HP_BILLBOARD_DISABLED,
        .blend = HP_BLEND_OPAQUE,
        .cull = HP_CULL_BACK
    };
}

void HP_DestroyMaterialResources(HP_Material* material)
{
    HP_DestroyTexture(material->albedo.texture);
    HP_DestroyTexture(material->emission.texture);
    HP_DestroyTexture(material->orm.texture);
    HP_DestroyTexture(material->normal.texture);
}

/* === Mesh - Public API === */

HP_Mesh* HP_CreateMesh(const HP_Vertex3D* vertices, int vCount, const uint32_t* indices, int iCount)
{
    /* --- Validation of parameters --- */

    if (vertices == nullptr || vCount == 0) {
        HP_INTERNAL_LOG(E, "RENDER: Failed to load mesh; Vertices and their count cannot be null");
        return nullptr;
    }

    /* --- Copies of input data --- */

    HP_Vertex3D* vCopy = static_cast<HP_Vertex3D*>(SDL_malloc(vCount * sizeof(HP_Vertex3D)));
    SDL_memcpy(vCopy, vertices, vCount * sizeof(HP_Vertex3D));

    uint32_t* iCopy = nullptr;
    if (indices != nullptr && iCount > 0) {
        iCopy = static_cast<uint32_t*>(SDL_malloc(iCount * sizeof(uint32_t)));
        SDL_memcpy(iCopy, indices, iCount * sizeof(uint32_t));
    }

    /* --- Mesh creation --- */

    HP_Mesh* mesh = gRender->meshes.createMesh(vCopy, vCount, iCopy, iCount, true);
    if (mesh == nullptr) {
        SDL_free(vCopy);
        SDL_free(iCopy);
        return nullptr;
    }

    return mesh;
}

void HP_DestroyMesh(HP_Mesh* mesh)
{
    gRender->meshes.destroyMesh(mesh);
}

HP_Mesh* HP_GenMeshQuad(HP_Vec2 size, HP_Vec2 subDiv, HP_Vec3 normal)
{
    /* --- Parameter validation --- */

    size.x = fmaxf(0.1f, size.x);
    size.y = fmaxf(0.1f, size.y);
    int segX = (int)fmaxf(1.0f, subDiv.x);
    int segY = (int)fmaxf(1.0f, subDiv.y);

    float length = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
    if (length < 0.001f) {
        normal = HP_VEC3(0.0f, 0.0f, 1.0f);
        length = 1.0f;
    }
    normal.x /= length;
    normal.y /= length;
    normal.z /= length;

    /* --- Memory allocation --- */

    int vertexCount = (segX + 1) * (segY + 1);
    int indexCount = segX * segY * 6;

    HP_Vertex3D* vertices = static_cast<HP_Vertex3D*>(SDL_malloc(sizeof(HP_Vertex3D) * vertexCount));
    uint32_t* indices = static_cast<uint32_t*>(SDL_malloc(sizeof(uint32_t) * indexCount));

    if (!vertices || !indices) {
        SDL_free(vertices);
        SDL_free(indices);
        return nullptr;
    }

    /* --- Orientation vectors --- */

    HP_Vec3 reference = (fabsf(normal.y) < 0.9f) ? HP_VEC3(0.0f, 1.0f, 0.0f) : HP_VEC3(1.0f, 0.0f, 0.0f);

    HP_Vec3 tangent;
    tangent.x = normal.y * reference.z - normal.z * reference.y;
    tangent.y = normal.z * reference.x - normal.x * reference.z;
    tangent.z = normal.x * reference.y - normal.y * reference.x;

    float tangentLength = sqrtf(tangent.x * tangent.x + tangent.y * tangent.y + tangent.z * tangent.z);
    tangent.x /= tangentLength;
    tangent.y /= tangentLength;
    tangent.z /= tangentLength;

    HP_Vec3 bitangent;
    bitangent.x = normal.y * tangent.z - normal.z * tangent.y;
    bitangent.y = normal.z * tangent.x - normal.x * tangent.z;
    bitangent.z = normal.x * tangent.y - normal.y * tangent.x;

    /* --- Vertex generation --- */

    int vertexIndex = 0;
    for (int y = 0; y <= segY; y++) {
        for (int x = 0; x <= segX; x++) {
            HP_Vertex3D& vertex = vertices[vertexIndex++];

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
            vertex.tangent = HP_VEC4(tangent.x, tangent.y, tangent.z, 1.0f);
            vertex.color = HP_COLOR(1, 1, 1, 1);
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

    HP_Mesh* mesh = gRender->meshes.createMesh(vertices, vertexCount, indices, indexCount, true);
    if (mesh == nullptr) {
        SDL_free(vertices);
        SDL_free(indices);
        return nullptr;
    }

    return mesh;
}

HP_Mesh* HP_GenMeshCube(HP_Vec3 size, HP_Vec3 subDiv)
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

    HP_Vertex3D* vertices = static_cast<HP_Vertex3D*>(SDL_malloc(sizeof(HP_Vertex3D) * vertexCount));
    uint32_t* indices = static_cast<uint32_t*>(SDL_malloc(sizeof(uint32_t) * indexCount));

    if (!vertices || !indices) {
        SDL_free(vertices);
        SDL_free(indices);
        return nullptr;
    }

    /* --- Face configuration --- */

    struct FaceParams {
        HP_Vec3 normal;
        HP_Vec4 tangent;
        int segsU, segsV;
    };

    FaceParams faces[6] = {
        {HP_VEC3(0, 0, 1), HP_VEC4(1, 0, 0, 1), segX, segY},   // Front (Z+)
        {HP_VEC3(0, 0, -1), HP_VEC4(-1, 0, 0, 1), segX, segY}, // Back (Z-)
        {HP_VEC3(1, 0, 0), HP_VEC4(0, 0, -1, 1), segZ, segY},  // Right (X+)
        {HP_VEC3(-1, 0, 0), HP_VEC4(0, 0, 1, 1), segZ, segY},  // Left (X-)
        {HP_VEC3(0, 1, 0), HP_VEC4(1, 0, 0, 1), segX, segZ},   // Top (Y+)
        {HP_VEC3(0, -1, 0), HP_VEC4(1, 0, 0, 1), segX, segZ}   // Bottom (Y-)
    };

    /* --- Vertex and index generation --- */

    int vertexIndex = 0;
    int indexIndex = 0;
    HP_Vec3 halfSize = size * 0.5f;

    for (int face = 0; face < 6; face++) {
        uint32_t baseVertex = vertexIndex;
        FaceParams& fp = faces[face];

        for (int v = 0; v <= fp.segsV; v++) {
            for (int u = 0; u <= fp.segsU; u++) {
                HP_Vertex3D& vertex = vertices[vertexIndex++];

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
                vertex.color = HP_COLOR(1, 1, 1, 1);
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

    HP_Mesh* mesh = gRender->meshes.createMesh(vertices, vertexCount, indices, indexCount, true);
    if (mesh == nullptr) {
        SDL_free(vertices);
        SDL_free(indices);
        return nullptr;
    }

    return mesh;
}

HP_Mesh* HP_GenMeshSphere(float radius, int slices, int rings)
{
    /* --- Parameter validation --- */

    radius = fmaxf(0.1f, radius);
    slices = (int)fmaxf(3, slices);
    rings = (int)fmaxf(2, rings);

    /* --- Memory allocation --- */

    int vertexCount = (rings + 1) * (slices + 1);
    int indexCount = rings * slices * 6;

    HP_Vertex3D* vertices = static_cast<HP_Vertex3D*>(SDL_malloc(sizeof(HP_Vertex3D) * vertexCount));
    uint32_t* indices = static_cast<uint32_t*>(SDL_malloc(sizeof(uint32_t) * indexCount));

    if (!vertices || !indices) {
        SDL_free(vertices);
        SDL_free(indices);
        return nullptr;
    }

    /* --- Sphere generation --- */

    int vertexIndex = 0;
    int indexIndex = 0;
    const float piOverRings = HP_PI / rings;
    const float tauOverSlices = HP_TAU / slices;

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

            HP_Vertex3D& vertex = vertices[vertexIndex++];

            vertex.position.x = ringRadius * cosTheta;
            vertex.position.y = y;
            vertex.position.z = ringRadius * sinTheta;

            vertex.normal = HP_VEC3(
                vertex.position.x / radius,
                vertex.position.y / radius,
                vertex.position.z / radius
            );

            vertex.texcoord.x = (float)slice / slices;
            vertex.texcoord.y = (float)ring / rings;
            vertex.tangent = HP_VEC4(-sinTheta, 0.0f, cosTheta, 1.0f);
            vertex.color = HP_COLOR(1, 1, 1, 1);
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

    HP_Mesh* mesh = gRender->meshes.createMesh(vertices, vertexCount, indices, indexCount, true);
    if (mesh == nullptr) {
        SDL_free(vertices);
        SDL_free(indices);
        return nullptr;
    }

    return mesh;
}

HP_Mesh* HP_GenMeshCylinder(float topRadius, float bottomRadius, float height, int slices, int rings, bool topCap, bool bottomCap)
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

    HP_Vertex3D* vertices = static_cast<HP_Vertex3D*>(SDL_malloc(sizeof(HP_Vertex3D) * vertexCount));
    uint32_t* indices = static_cast<uint32_t*>(SDL_malloc(sizeof(uint32_t) * indexCount));

    if (!vertices || !indices) {
        SDL_free(vertices);
        SDL_free(indices);
        return nullptr;
    }

    /* --- Cylinder setup --- */

    int vertexIndex = 0;
    int indexIndex = 0;
    const float angleStep = HP_TAU / slices;
    const float halfHeight = height * 0.5f;

    HP_Vec3 sideNormalBase;
    if (topRadius != bottomRadius) {
        float radiusDiff = bottomRadius - topRadius;
        float normalLength = sqrtf(radiusDiff * radiusDiff + height * height);
        sideNormalBase = HP_VEC3(radiusDiff / normalLength, height / normalLength, 0.0f);
    }
    else {
        sideNormalBase = HP_VEC3(1.0f, 0.0f, 0.0f);
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

            HP_Vertex3D& vertex = vertices[vertexIndex++];

            vertex.position.x = currentRadius * cosAngle;
            vertex.position.y = y;
            vertex.position.z = currentRadius * sinAngle;

            vertex.normal.x = sideNormalBase.x * cosAngle;
            vertex.normal.y = sideNormalBase.y;
            vertex.normal.z = sideNormalBase.x * sinAngle;

            vertex.texcoord.x = (float)slice / slices;
            vertex.texcoord.y = t;
            vertex.tangent = HP_VEC4(-sinAngle, 0.0f, cosAngle, 1.0f);
            vertex.color = HP_COLOR(1, 1, 1, 1);
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

        HP_Vertex3D& centerVertex = vertices[vertexIndex++];
        centerVertex.position = HP_VEC3(0.0f, halfHeight, 0.0f);
        centerVertex.normal = HP_VEC3(0.0f, 1.0f, 0.0f);
        centerVertex.texcoord = HP_VEC2(0.5f, 0.5f);
        centerVertex.tangent = HP_VEC4(1.0f, 0.0f, 0.0f, 1.0f);
        centerVertex.color = HP_COLOR(1, 1, 1, 1);

        for (int slice = 0; slice <= slices; slice++) {
            float angle = slice * angleStep;
            float cosAngle = cosf(angle);
            float sinAngle = sinf(angle);

            HP_Vertex3D& vertex = vertices[vertexIndex++];

            vertex.position.x = topRadius * cosAngle;
            vertex.position.y = halfHeight;
            vertex.position.z = topRadius * sinAngle;
            vertex.normal = HP_VEC3(0.0f, 1.0f, 0.0f);
            vertex.texcoord.x = 0.5f + 0.5f * cosAngle;
            vertex.texcoord.y = 0.5f + 0.5f * sinAngle;
            vertex.tangent = HP_VEC4(1.0f, 0.0f, 0.0f, 1.0f);
            vertex.color = HP_COLOR(1, 1, 1, 1);
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

        HP_Vertex3D& centerVertex = vertices[vertexIndex++];
        centerVertex.position = HP_VEC3(0.0f, -halfHeight, 0.0f);
        centerVertex.normal = HP_VEC3(0.0f, -1.0f, 0.0f);
        centerVertex.texcoord = HP_VEC2(0.5f, 0.5f);
        centerVertex.tangent = HP_VEC4(1.0f, 0.0f, 0.0f, 1.0f);
        centerVertex.color = HP_COLOR(1, 1, 1, 1);

        for (int slice = 0; slice <= slices; slice++) {
            float angle = slice * angleStep;
            float cosAngle = cosf(angle);
            float sinAngle = sinf(angle);

            HP_Vertex3D& vertex = vertices[vertexIndex++];

            vertex.position.x = bottomRadius * cosAngle;
            vertex.position.y = -halfHeight;
            vertex.position.z = bottomRadius * sinAngle;
            vertex.normal = HP_VEC3(0.0f, -1.0f, 0.0f);
            vertex.texcoord.x = 0.5f + 0.5f * cosAngle;
            vertex.texcoord.y = 0.5f + 0.5f * sinAngle;
            vertex.tangent = HP_VEC4(1.0f, 0.0f, 0.0f, 1.0f);
            vertex.color = HP_COLOR(1, 1, 1, 1);
        }

        for (int slice = 0; slice < slices; slice++) {
            indices[indexIndex++] = bottomCapBaseVertex;
            indices[indexIndex++] = bottomCapBaseVertex + 1 + slice;
            indices[indexIndex++] = bottomCapBaseVertex + 1 + (slice + 1);
        }
    }

    /* --- Mesh creation and finalization --- */

    HP_Mesh* mesh = gRender->meshes.createMesh(vertices, vertexCount, indices, indexCount, true);
    if (mesh == nullptr) {
        SDL_free(vertices);
        SDL_free(indices);
        return nullptr;
    }

    return mesh;
}

HP_Mesh* HP_GenMeshCapsule(float radius, float height, int slices, int rings)
{
    /* --- Parameter validation --- */

    radius = fmaxf(0.1f, radius);
    height = fmaxf(0.0f, height);
    slices = HP_MAX(3, slices);
    rings = HP_MAX(2, rings);

    int hemisphereRings = HP_MAX(1, rings / 2);

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

    HP_Vertex3D* vertices = static_cast<HP_Vertex3D*>(SDL_malloc(sizeof(HP_Vertex3D) * vertexCount));
    uint32_t* indices = static_cast<uint32_t*>(SDL_malloc(sizeof(uint32_t) * indexCount));

    if (!vertices || !indices) {
        SDL_free(vertices);
        SDL_free(indices);
        return nullptr;
    }

    /* --- Capsule setup --- */

    int vertexIndex = 0;
    int indexIndex = 0;
    const float PI_OVER_2 = HP_PI * 0.5f;
    const float angleStep = HP_TAU / slices;
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

            HP_Vertex3D& vertex = vertices[vertexIndex++];

            vertex.position.x = ringRadius * cosTheta;
            vertex.position.y = y;
            vertex.position.z = ringRadius * sinTheta;

            vertex.normal.x = sinPhi * cosTheta;
            vertex.normal.y = cosPhi;
            vertex.normal.z = sinPhi * sinTheta;

            vertex.texcoord.x = (float)slice / slices;
            vertex.texcoord.y = 0.5f + 0.5f * ((float)ring / hemisphereRings);
            vertex.tangent = HP_VEC4(-sinTheta, 0.0f, cosTheta, 1.0f);
            vertex.color = HP_COLOR(1, 1, 1, 1);
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

            HP_Vertex3D& topVertex = vertices[vertexIndex++];
            topVertex.position.x = radius * cosTheta;
            topVertex.position.y = halfHeight;
            topVertex.position.z = radius * sinTheta;
            topVertex.normal.x = cosTheta;
            topVertex.normal.y = 0.0f;
            topVertex.normal.z = sinTheta;
            topVertex.texcoord.x = (float)slice / slices;
            topVertex.texcoord.y = 0.5f;
            topVertex.tangent = HP_VEC4(-sinTheta, 0.0f, cosTheta, 1.0f);
            topVertex.color = HP_COLOR(1, 1, 1, 1);
        }

        for (int slice = 0; slice <= slices; slice++) {
            float theta = slice * angleStep;
            float sinTheta = sinf(theta);
            float cosTheta = cosf(theta);

            HP_Vertex3D& bottomVertex = vertices[vertexIndex++];
            bottomVertex.position.x = radius * cosTheta;
            bottomVertex.position.y = -halfHeight;
            bottomVertex.position.z = radius * sinTheta;
            bottomVertex.normal.x = cosTheta;
            bottomVertex.normal.y = 0.0f;
            bottomVertex.normal.z = sinTheta;
            bottomVertex.texcoord.x = (float)slice / slices;
            bottomVertex.texcoord.y = 0.5f;
            bottomVertex.tangent = HP_VEC4(-sinTheta, 0.0f, cosTheta, 1.0f);
            bottomVertex.color = HP_COLOR(1, 1, 1, 1);
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

            HP_Vertex3D& vertex = vertices[vertexIndex++];

            vertex.position.x = ringRadius * cosTheta;
            vertex.position.y = y;
            vertex.position.z = ringRadius * sinTheta;

            vertex.normal.x = sinPhi * cosTheta;
            vertex.normal.y = cosPhi;
            vertex.normal.z = sinPhi * sinTheta;

            vertex.texcoord.x = (float)slice / slices;
            vertex.texcoord.y = 0.5f - 0.5f * ((float)ring / hemisphereRings);
            vertex.tangent = HP_VEC4(-sinTheta, 0.0f, cosTheta, 1.0f);
            vertex.color = HP_COLOR(1, 1, 1, 1);
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

    HP_Mesh* mesh = gRender->meshes.createMesh(vertices, vertexCount, indices, indexCount, true);
    if (mesh == nullptr) {
        SDL_free(vertices);
        SDL_free(indices);
        return nullptr;
    }

    return mesh;
}

void HP_UpdateMeshBuffer(HP_Mesh* mesh)
{
    gRender->meshes.updateMesh(mesh);
}

void HP_UpdateMeshAABB(HP_Mesh* mesh)
{
    if (!mesh || mesh->vertexCount == 0) {
        return;
    }

    const HP_Vertex3D* vertices = mesh->vertices;
    const uint32_t* indices = mesh->indices;

    mesh->aabb.min = HP_VEC3(+FLT_MAX, +FLT_MAX, +FLT_MAX);
    mesh->aabb.max = HP_VEC3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    if (indices) {
        for (int i = 0; i < mesh->indexCount; i++) {
            const HP_Vec3& pos = vertices[indices[i]].position;
            mesh->aabb.min = HP_Vec3Min(mesh->aabb.min, pos);
            mesh->aabb.max = HP_Vec3Max(mesh->aabb.max, pos);
        }
    }
    else {
        for (int i = 0; i < mesh->vertexCount; i++) {
            const HP_Vec3& pos = vertices[i].position;
            mesh->aabb.min = HP_Vec3Min(mesh->aabb.min, pos);
            mesh->aabb.max = HP_Vec3Max(mesh->aabb.max, pos);
        }
    }
}

/* === InstanceBuffer - Public API === */

HP_InstanceBuffer* HP_CreateInstanceBuffer(HP_InstanceData bitfield, size_t count)
{
    return gRender->meshes.createInstanceBuffer(bitfield, count);
}

void HP_DestroyInstanceBuffer(HP_InstanceBuffer* buffer)
{
    gRender->meshes.destroyInstanceBuffer(buffer);
}

void HP_ReserveInstanceBuffer(HP_InstanceBuffer* buffer, HP_InstanceData bitfield, size_t count, bool keepData)
{
    buffer->reserveBufferCapacity(bitfield, count, keepData);
}

void HP_UpdateInstanceBuffer(HP_InstanceBuffer* buffer, HP_InstanceData type, const void* data, size_t offset, size_t count, bool keepData)
{
    buffer->updateBufferData(type, data, offset, count, keepData);
}

void HP_SetInstanceBufferState(HP_InstanceBuffer* buffer, HP_InstanceData bitfield, bool enabled)
{
    buffer->setBufferState(bitfield, enabled);
}

/* === Model - Public API === */

void HP_SetModelImportScale(float value)
{
    gRender->models.setImportScale(value);
}

HP_Model* HP_LoadModel(const char* filePath)
{
    size_t fileSize = 0;
    void* fileData = HP_LoadFile(filePath, &fileSize);
    HP_Model* model = gRender->models.loadModel(fileData, fileSize, helper::getFileExt(filePath));
    SDL_free(fileData);
    return model;
}

HP_Model* HP_LoadModelFromMemory(const void* data, size_t size, const char* hint)
{
    return gRender->models.loadModel(data, size, hint);
}

void HP_DestroyModel(HP_Model* model)
{
    gRender->models.destroyModel(model);
}

void HP_UpdateModelAABB(HP_Model* model, bool updateMeshAABBs)
{
    if (!model || !model->meshes) {
        return;
    }

    HP_Vec3 min = { +FLT_MAX, +FLT_MAX, +FLT_MAX };
    HP_Vec3 max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

    for (uint32_t i = 0; i < model->meshCount; i++) {
        HP_Mesh* mesh = model->meshes[i];
        if (updateMeshAABBs) {
            HP_UpdateMeshAABB(mesh);
        }
        min = HP_Vec3Min(min, mesh->aabb.min);
        max = HP_Vec3Max(max, mesh->aabb.max);
    }

    model->aabb.min = min;
    model->aabb.max = max;
}

void HP_ScaleModelAABB(HP_Model* model, float scale, bool scaleMeshAABBs)
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

HP_ModelAnimation** HP_LoadModelAnimations(const char* filePath, int* animCount, int targetFrameRate)
{
    size_t fileSize = 0;
    void* fileData = HP_LoadFile(filePath, &fileSize);
    HP_ModelAnimation** animations = gRender->models.loadAnimations(fileData, fileSize, helper::getFileExt(filePath), animCount, targetFrameRate);
    SDL_free(fileData);
    return animations;
}

HP_ModelAnimation** HP_LoadModelAnimationsFromMemory(const void* data, unsigned int size, const char* hint, int* animCount, int targetFrameRate)
{
    return gRender->models.loadAnimations(data, size, hint, animCount, targetFrameRate);
}

void HP_DestroyModelAnimations(HP_ModelAnimation** animations, int animCount)
{
    gRender->models.destroyAnimations(animations, animCount);
}

HP_ModelAnimation* HP_GetModelAnimation(HP_ModelAnimation** animations, int animCount, const char* name)
{
    for (int i = 0; i < animCount; i++) {
        if (SDL_strcmp(animations[i]->name, name) == 0) {
            return animations[i];
        }
    }
    return nullptr;
}

/* === Light - Public API === */

HP_Light* HP_CreateLight(HP_LightType type)
{
    return gRender->scene.lights().create(type);
}

void HP_DestroyLight(HP_Light* light)
{
    gRender->scene.lights().destroy(light);
}

bool HP_IsLightActive(const HP_Light* light)
{
    return light->isActive();
}

void HP_SetLightActive(HP_Light* light, bool active)
{
    light->setActive(active);
}

HP_Layer HP_GetLightLayerMask(const HP_Light* light)
{
    return light->layerMask();
}

void HP_SetLightLayerMask(HP_Light* light, HP_Layer layers)
{
    light->setLayerMask(layers);
}

HP_Layer HP_GetLightCullMask(const HP_Light* light)
{
    return light->cullMask();
}

void HP_SetLightCullMask(HP_Light* light, HP_Layer layers)
{
    light->setCullMask(layers);
}

HP_Vec3 HP_GetLightPosition(const HP_Light* light)
{
    return light->position();
}

void HP_SetLightPosition(HP_Light* light, HP_Vec3 position)
{
    light->setPosition(position);
}

HP_Vec3 HP_GetLightDirection(const HP_Light* light)
{
    return light->position();
}

void HP_SetLightDirection(HP_Light* light, HP_Vec3 direction)
{
    light->setDirection(direction);
}

HP_Color HP_GetLightColor(const HP_Light* light)
{
    return light->color();
}

void HP_SetLightColor(HP_Light* light, HP_Color color)
{
    light->setColor(color);
}

float HP_GetLightEnergy(const HP_Light* light)
{
    return light->energy();
}

void HP_SetLightEnergy(HP_Light* light, float energy)
{
    light->setEnergy(energy);
}

float HP_GetLightSpecular(const HP_Light* light)
{
    return light->specular();
}

void HP_SetLightSpecular(HP_Light* light, float specular)
{
    light->setSpecular(specular);
}

float HP_GetLightRange(const HP_Light* light)
{
    return light->range();
}

void HP_SetLightRange(HP_Light* light, float range)
{
    light->setRange(range);
}

float HP_GetLightAttenuation(const HP_Light* light)
{
    return light->energy();
}

void HP_SetLightAttenuation(HP_Light* light, float attenuation)
{
    light->setAttenuation(attenuation);
}

float HP_GetLightInnerCutOff(const HP_Light* light)
{
    return light->innerCutOff();
}

void HP_SetLightInnerCutOff(HP_Light* light, float radians)
{
    light->setInnerCutOff(radians);
}

float HP_GetLightOuterCutOff(const HP_Light* light)
{
    return light->outerCutOff();
}

void HP_SetLightOuterCutOff(HP_Light* light, float radians)
{
    light->setOuterCutOff(radians);
}

void HP_SetLightCutOff(HP_Light* light, float inner, float outer)
{
    light->setInnerCutOff(inner);
    light->setOuterCutOff(outer);
}

bool HP_IsShadowActive(const HP_Light* light)
{
    return light->isShadowActive();
}

void HP_SetShadowActive(HP_Light* light, bool active)
{
    light->setShadowActive(active);
}

HP_Layer HP_GetShadowCullMask(const HP_Light* light)
{
    return light->shadowCullMask();
}

void HP_SetShadowCullMask(HP_Light* light, HP_Layer layers)
{
    light->setShadowCullMask(layers);
}

float HP_GetShadowBleedingBias(const HP_Light* light)
{
    return light->shadowBleedingBias();
}

void HP_SetShadowBleedingBias(HP_Light* light, float bias)
{
    light->setShadowBleedingBias(bias);
}

float HP_GetShadowSoftness(const HP_Light* light)
{
    return light->shadowSoftness();
}

void HP_SetShadowSoftness(HP_Light* light, float softness)
{
    light->setShadowSoftness(softness);
}

float HP_GetShadowLambda(const HP_Light* light)
{
    return light->shadowLambda();
}

void HP_SetShadowLambda(HP_Light* light, float lambda)
{
    light->setShadowLambda(lambda);
}

HP_ShadowUpdateMode HP_GetShadowUpdateMode(const HP_Light* light)
{
    return light->shadowUpdateMode();
}

void HP_SetShadowUpdateMode(HP_Light* light, HP_ShadowUpdateMode mode)
{
    light->setShadowUpdateMode(mode);
}

float HP_GetShadowUpdateInterval(const HP_Light* light)
{
    return light->shadowUpdateInterval();
}

void HP_SetShadowUpdateInterval(HP_Light* light, float sec)
{
    light->setShadowUpdateInterval(sec);
}

void HP_UpdateShadowMap(HP_Light* light)
{
    light->forceShadowMapUpdate();
}
