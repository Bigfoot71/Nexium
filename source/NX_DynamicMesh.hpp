/* NX_DynamicMesh.cpp -- API definition for Nexium's dynamic mesh module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_DYNAMIC_MESH_HPP
#define NX_DYNAMIC_MESH_HPP

#include <NX/NX_DynamicMesh.h>
#include <NX/NX_Shape.h>

#include "./Detail/Util/DynamicArray.hpp"
#include "./NX_Vertex.hpp"

// ============================================================================
// OPAQUE DEFINITION
// ============================================================================

struct NX_DynamicMesh {
    /** Buffers and current state */
    util::DynamicArray<NX_Vertex3D> vertices{};
    NX_VertexBuffer3D* buffer{};
    NX_Vertex3D current{};

    /** Draw parameters */
    NX_ShadowCastMode shadowCastMode;
    NX_ShadowFaceMode shadowFaceMode;
    NX_PrimitiveType primitiveType;
    NX_BoundingBox aabb;
    NX_Layer layerMask;
};

#endif // NX_DYNAMIC_MESH_HPP
