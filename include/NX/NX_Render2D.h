#ifndef NX_RENDER_2D_H
#define NX_RENDER_2D_H

#include "./NX_RenderTexture.h"
#include "./NX_Texture.h"
#include "./NX_Render.h"
#include "./NX_Font.h"
#include "./NX_Math.h"
#include "./NX_API.h"

// ============================================================================
// TYPES DEFINITIONS
// ============================================================================



// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Begins 2D rendering.
 *
 * Sets up the rendering state for drawing 2D primitives.
 *
 * @param target Render texture to draw into (can be NULL to render to the screen).
 */
NXAPI void NX_Begin2D(NX_RenderTexture* target);

/**
 * @brief Ends 2D rendering.
 *
 * Flushes any pending 2D draw calls and restores previous rendering state.
 */
NXAPI void NX_End2D(void);

/**
 * @brief Sets the default color for 2D drawing.
 * @param color Color to use for subsequent 2D drawing operations.
 * @note The default color is NX_WHITE.
 */
NXAPI void NX_SetColor2D(NX_Color color);

/**
 * @brief Sets the default texture for 2D drawing.
 * @param texture Pointer to the NX_Texture to use.
 * @note The default texture (NULL) is a white texture.
 */
NXAPI void NX_SetTexture2D(const NX_Texture* texture);

/**
 * @brief Sets the default font for 2D drawing.
 * @param font Pointer to the NX_Font to use.
 * @note The default font (NULL) is Vera Sans rendered in SDF with a base size of 32.
 */
NXAPI void NX_SetFont2D(const NX_Font* font);

/**
 * @brief Sets the default shader for 2D drawing.
 * @param font Pointer to the NX_Shader2D to use.
 * @note The default shader will be used if NULL.
 */
NXAPI void NX_SetShader2D(NX_Shader2D* shader);

/**
 * @brief Pushes the current 2D transformation matrix onto the stack.
 * @note Maximum stack depth is 16 matrices.
 */
NXAPI void NX_Push2D(void);

/**
 * @brief Pops the current 2D transformation matrix from the stack.
 */
NXAPI void NX_Pop2D(void);

/**
 * @brief Applies a translation to the current 2D transformation.
 * @param translation Translation vector in x and y.
 */
NXAPI void NX_Translate2D(NX_Vec2 translation);

/**
 * @brief Applies a rotation to the current 2D transformation.
 * @param radians Rotation angle in radians.
 */
NXAPI void NX_Rotate2D(float radians);

/**
 * @brief Applies a scale to the current 2D transformation.
 * @param scale Scale factors in x and y.
 */
NXAPI void NX_Scale2D(NX_Vec2 scale);

/**
 * @brief Draws a 2D shape using an array of 2D points and a specified primitive type.
 * @param type The type of primitive (points, lines, triangles, etc).
 * @param points Array of 2D points defining the shape.
 * @param pointCount Number of points in the array.
 * @param thickness For points: controls point size.
 *                  For lines: controls line thickness.
 *                  For triangles: if != 0.0f, only the edges are drawn.
 *                  For triangle strips or fans: only the outer edges are drawn if != 0.0f.
 * @note This is the high-level, simpler version for drawing shapes without custom vertex attributes.
 *       For per-vertex color or texture coordinates, use NX_DrawShapeEx2D instead.
 */
NXAPI void NX_DrawShape2D(NX_PrimitiveType type, const NX_Vec2* points, int pointCount, float thickness);

/**
 * @brief Draws a 2D shape using an array of vertices and a specified primitive type.
 * @param type The type of primitive (points, lines, triangles, etc).
 * @param vertices Array of NX_Vertex2D defining the vertices.
 * @param vertexCount Number of vertices in the array.
 * @param thickness For points: controls point size.
 *                  For lines: controls line thickness.
 *                  For triangles: if != 0.0f, only the edges are drawn.
 *                  For triangle strips or fans: only the outer edges are drawn if != 0.0f.
 * @note This is the low-level drawing function; for most UI use cases, prefer the higher-level NX_DrawShape2D.
 */
NXAPI void NX_DrawShapeEx2D(NX_PrimitiveType type, const NX_Vertex2D* vertices, int vertexCount, float thickness);

/**
 * @brief Draws a line segment in 2D.
 * @param p0 Start point of the line.
 * @param p1 End point of the line.
 * @param thickness Line thickness: positive = pixels, negative = scaled by transformation.
 */
NXAPI void NX_DrawLine2D(NX_Vec2 p0, NX_Vec2 p1, float thickness);

/**
 * @brief Draws a line segment in 2D using custom vertex data.
 * @param v0 First vertex of the line, including color/texcoord.
 * @param v1 Second vertex of the line.
 * @param thickness Line thickness: positive = pixels, negative = scaled by transformation.
 * @note Allows specifying per-vertex color or texture coordinates.
 */
NXAPI void NX_DrawLineEx2D(const NX_Vertex2D* v0, const NX_Vertex2D* v1, float thickness);

/**
 * @brief Draws a filled triangle in 2D.
 * @param p0 First vertex of the triangle.
 * @param p1 Second vertex of the triangle.
 * @param p2 Third vertex of the triangle.
 */
NXAPI void NX_DrawTriangle2D(NX_Vec2 p0, NX_Vec2 p1, NX_Vec2 p2);

/**
 * @brief Draws a filled triangle in 2D using custom vertex data.
 * @param v0 First vertex of the triangle, including color/texcoord.
 * @param v1 Second vertex of the triangle.
 * @param v2 Third vertex of the triangle.
 * @note Allows specifying per-vertex color or texture coordinates.
 */
NXAPI void NX_DrawTriangleEx2D(const NX_Vertex2D* v0, const NX_Vertex2D* v1, const NX_Vertex2D* v2);

/**
 * @brief Draws a filled quadrilateral in 2D.
 * @param p0 First vertex of the quad.
 * @param p1 Second vertex of the quad.
 * @param p2 Third vertex of the quad.
 * @param p3 Fourth vertex of the quad.
 */
NXAPI void NX_DrawQuad2D(NX_Vec2 p0, NX_Vec2 p1, NX_Vec2 p2, NX_Vec2 p3);

/**
 * @brief Draws a filled quadrilateral in 2D using custom vertex data.
 * @param v0 First vertex of the quad, including color/texcoord.
 * @param v1 Second vertex of the quad.
 * @param v2 Third vertex of the quad.
 * @param v3 Fourth vertex of the quad.
 * @note Allows specifying per-vertex color or texture coordinates.
 */
NXAPI void NX_DrawQuadEx2D(const NX_Vertex2D* v0, const NX_Vertex2D* v1, const NX_Vertex2D* v2, const NX_Vertex2D* v3);

/**
 * @brief Draws a filled rectangle in 2D.
 * @param x X-coordinate of the rectangle's top-left corner.
 * @param y Y-coordinate of the rectangle's top-left corner.
 * @param w Width of the rectangle.
 * @param h Height of the rectangle.
 */
NXAPI void NX_DrawRect2D(float x, float y, float w, float h);

/**
 * @brief Draws the border of a rectangle in 2D.
 * @param x X-coordinate of the rectangle's top-left corner.
 * @param y Y-coordinate of the rectangle's top-left corner.
 * @param w Width of the rectangle.
 * @param h Height of the rectangle.
 * @param thickness Border thickness: positive = pixels, negative = scaled by transformation.
 */
NXAPI void NX_DrawRectBorder2D(float x, float y, float w, float h, float thickness);

/**
 * @brief Draws a rectangle with rounded corners in 2D.
 * @param x X-coordinate of the top-left corner.
 * @param y Y-coordinate of the top-left corner.
 * @param w Width of the rectangle.
 * @param h Height of the rectangle.
 * @param radius Corner radius.
 * @param segments Number of segments to approximate the corners.
 */
NXAPI void NX_DrawRectRounded2D(float x, float y, float w, float h, float radius, int segments);

/**
 * @brief Draws the border of a rectangle with rounded corners in 2D.
 * @param x X-coordinate of the top-left corner.
 * @param y Y-coordinate of the top-left corner.
 * @param w Width of the rectangle.
 * @param h Height of the rectangle.
 * @param radius Corner radius.
 * @param segments Number of segments to approximate the corners.
 * @param thickness Border thickness: positive = pixels, negative = scaled by transformation.
 */
NXAPI void NX_DrawRectRoundedBorder2D(float x, float y, float w, float h, float radius, int segments, float thickness);

/**
 * @brief Draws a filled circle in 2D.
 * @param center Center position of the circle.
 * @param radius Circle radius.
 * @param segments Number of segments to approximate the circle.
 */
NXAPI void NX_DrawCircle2D(NX_Vec2 center, float radius, int segments);

/**
 * @brief Draws the border of a circle in 2D.
 * @param p Center position of the circle.
 * @param radius Circle radius.
 * @param segments Number of segments to approximate the circle.
 * @param thickness Border thickness: positive = pixels, negative = scaled by transformation.
 */
NXAPI void NX_DrawCircleBorder2D(NX_Vec2 p, float radius, int segments, float thickness);

/**
 * @brief Draws a filled ellipse in 2D.
 * @param center Center position of the ellipse.
 * @param radius X and Y radii of the ellipse.
 * @param segments Number of segments to approximate the ellipse.
 */
NXAPI void NX_DrawEllipse2D(NX_Vec2 center, NX_Vec2 radius, int segments);

/**
 * @brief Draws the border of an ellipse in 2D.
 * @param p Center position of the ellipse.
 * @param r X and Y radii of the ellipse.
 * @param segments Number of segments to approximate the ellipse.
 * @param thickness Border thickness: positive = pixels, negative = scaled by transformation.
 */
NXAPI void NX_DrawEllipseBorder2D(NX_Vec2 p, NX_Vec2 r, int segments, float thickness);

/**
 * @brief Draws a filled pie slice (sector) in 2D.
 * @param center Center of the pie slice.
 * @param radius Radius of the pie slice.
 * @param startAngle Starting angle in radians.
 * @param endAngle Ending angle in radians.
 * @param segments Number of segments to approximate the curve.
 */
NXAPI void NX_DrawPieSlice2D(NX_Vec2 center, float radius, float startAngle, float endAngle, int segments);

/**
 * @brief Draws the border of a pie slice (sector) in 2D.
 * @param center Center of the pie slice.
 * @param radius Radius of the pie slice.
 * @param startAngle Starting angle in radians.
 * @param endAngle Ending angle in radians.
 * @param segments Number of segments to approximate the curve.
 * @param thickness Border thickness: positive = pixels, negative = scaled by transformation.
 */
NXAPI void NX_DrawPieSliceBorder2D(NX_Vec2 center, float radius, float startAngle, float endAngle, int segments, float thickness);

/**
 * @brief Draws a filled ring in 2D.
 * @param center Center of the ring.
 * @param innerRadius Inner radius.
 * @param outerRadius Outer radius.
 * @param segments Number of segments to approximate the ring.
 */
NXAPI void NX_DrawRing2D(NX_Vec2 center, float innerRadius, float outerRadius, int segments);

/**
 * @brief Draws the border of a ring in 2D.
 * @param center Center of the ring.
 * @param innerRadius Inner radius.
 * @param outerRadius Outer radius.
 * @param segments Number of segments to approximate the ring.
 * @param thickness Border thickness: positive = pixels, negative = scaled by transformation.
 */
NXAPI void NX_DrawRingBorder2D(NX_Vec2 center, float innerRadius, float outerRadius, int segments, float thickness);

/**
 * @brief Draws a filled ring arc in 2D.
 * @param center Center of the arc.
 * @param innerRadius Inner radius.
 * @param outerRadius Outer radius.
 * @param startAngle Starting angle in radians.
 * @param endAngle Ending angle in radians.
 * @param segments Number of segments to approximate the arc.
 */
NXAPI void NX_DrawRingArc2D(NX_Vec2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments);

/**
 * @brief Draws the border of a ring arc in 2D.
 * @param center Center of the arc.
 * @param innerRadius Inner radius.
 * @param outerRadius Outer radius.
 * @param startAngle Starting angle in radians.
 * @param endAngle Ending angle in radians.
 * @param segments Number of segments to approximate the arc.
 * @param thickness Border thickness: positive = pixels, negative = scaled by transformation.
 */
NXAPI void NX_DrawRingArcBorder2D(NX_Vec2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments, float thickness);

/**
 * @brief Draws an arc in 2D.
 * @param center Center of the arc.
 * @param radius Radius of the arc.
 * @param startAngle Starting angle in radians.
 * @param endAngle Ending angle in radians.
 * @param segments Number of segments to approximate the curve.
 * @param thickness Arc thickness: positive = pixels, negative = scaled by transformation.
 */
NXAPI void NX_DrawArc2D(NX_Vec2 center, float radius, float startAngle, float endAngle, int segments, float thickness);

/**
 * @brief Draws a quadratic Bezier curve in 2D.
 * @param p0 Start point.
 * @param p1 Control point.
 * @param p2 End point.
 * @param segments Number of segments to approximate the curve.
 * @param thickness Line thickness: positive = pixels, negative = scaled by transformation.
 */
NXAPI void NX_DrawBezierQuad2D(NX_Vec2 p0, NX_Vec2 p1, NX_Vec2 p2, int segments, float thickness);

/**
 * @brief Draws a cubic Bezier curve in 2D.
 * @param p0 Start point.
 * @param p1 First control point.
 * @param p2 Second control point.
 * @param p3 End point.
 * @param segments Number of segments to approximate the curve.
 * @param thickness Line thickness: positive = pixels, negative = scaled by transformation.
 */
NXAPI void NX_DrawBezierCubic2D(NX_Vec2 p0, NX_Vec2 p1, NX_Vec2 p2, NX_Vec2 p3, int segments, float thickness);

/**
 * @brief Draws a spline curve through a set of points in 2D.
 * @param points Array of points defining the spline.
 * @param count Number of points.
 * @param segments Number of segments between each pair of points.
 * @param thickness Line thickness: positive = pixels, negative = scaled by transformation.
 */
NXAPI void NX_DrawSpline2D(const NX_Vec2* points, int count, int segments, float thickness);

/**
 * @brief Draws a single Unicode codepoint in 2D.
 * @param codepoint Unicode codepoint to draw.
 * @param position Position in 2D space.
 * @param fontSize Font size in pixels.
 */
NXAPI void NX_DrawCodepoint2D(int codepoint, NX_Vec2 position, float fontSize);

/**
 * @brief Draws an array of Unicode codepoints in 2D.
 * @param codepoints Array of Unicode codepoints.
 * @param length Number of codepoints in the array.
 * @param position Starting position in 2D space.
 * @param fontSize Font size in pixels.
 * @param spacing Additional spacing between characters.
 */
NXAPI void NX_DrawCodepoints2D(const int* codepoints, int length, NX_Vec2 position, float fontSize, NX_Vec2 spacing);

/**
 * @brief Draws a null-terminated text string in 2D.
 * @param text String to draw.
 * @param position Starting position in 2D space.
 * @param fontSize Font size in pixels.
 * @param spacing Additional spacing between characters.
 */
NXAPI void NX_DrawText2D(const char* text, NX_Vec2 position, float fontSize, NX_Vec2 spacing);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_RENDER_2D_H
