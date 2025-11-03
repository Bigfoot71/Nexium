/* NX_Shape.h -- API declaration for Nexium's shape module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_SHAPE_H
#define NX_SHAPE_H

#include "./NX_Math.h"

// ============================================================================
// TYPES DEFINITIONS
// ============================================================================

/**
 * @brief Represents an axis-aligned bounding box (AABB).
 *
 * Defined by minimum and maximum corners.
 * Used for meshes, models, collision, and spatial calculations.
 */
typedef struct NX_BoundingBox {
    NX_Vec3 min;        ///< Minimum corner of the bounding box.
    NX_Vec3 max;        ///< Maximum corner of the bounding box.
} NX_BoundingBox;

#endif // NX_SHAPE_H
