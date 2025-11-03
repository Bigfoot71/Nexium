#ifndef NX_VERTEX_H
#define NX_VERTEX_H

#include "./NX_Math.h"

// ============================================================================
// TYPES DEFINITIONS
// ============================================================================

/**
 * @brief Defines the geometric primitive type.
 */
typedef enum NX_PrimitiveType {
    NX_PRIMITIVE_POINTS,            ///< Each vertex represents a single point.
    NX_PRIMITIVE_LINES,             ///< Each pair of vertices forms an independent line segment.
    NX_PRIMITIVE_LINE_STRIP,        ///< Connected series of line segments sharing vertices.
    NX_PRIMITIVE_LINE_LOOP,         ///< Closed loop of connected line segments.
    NX_PRIMITIVE_TRIANGLES,         ///< Each set of three vertices forms an independent triangle.
    NX_PRIMITIVE_TRIANGLE_STRIP,    ///< Connected strip of triangles sharing vertices.
    NX_PRIMITIVE_TRIANGLE_FAN       ///< Fan of triangles sharing the first vertex.
} NX_PrimitiveType;

/**
 * @brief Opaque handle to a GPU vertex buffer.
 *
 * Represents a collection of vertices stored on the GPU.
 */
typedef struct NX_VertexBuffer3D NX_VertexBuffer3D;

/**
 * @brief Represents a 2D vertex used for rendering.
 *
 * Contains position, texture coordinates, and color.
 * Suitable for 2D meshes, sprites, and UI elements.
 */
typedef struct NX_Vertex2D {
    NX_Vec2 position;   ///< Vertex position in 2D space.
    NX_Vec2 texcoord;   ///< Texture coordinates for this vertex.
    NX_Color color;     ///< Vertex color (used for tinting).
} NX_Vertex2D;

/**
 * @brief Represents a 3D vertex used for rendering.
 *
 * Contains position, texture coordinates, normals, tangents, color,
 * bone IDs, and weights for skeletal animation.
 * Suitable for meshes, models, and skinned characters.
 */
typedef struct NX_Vertex3D {
    NX_Vec3 position;   ///< Vertex position in 3D space.
    NX_Vec2 texcoord;   ///< Texture coordinates for this vertex.
    NX_Vec3 normal;     ///< Normal vector for lighting calculations.
    NX_Vec4 tangent;    ///< Tangent vector for normal mapping.
    NX_Color color;     ///< Vertex color (used for tinting).
    NX_IVec4 boneIds;   ///< IDs of bones affecting this vertex (for skeletal animation).
    NX_Vec4 weights;    ///< Weights of each bone affecting this vertex.
} NX_Vertex3D;

#endif // NX_VERTEX_H
