/* NX_Render3D.h -- API declaration for Nexium's 3D renderer module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_RENDER_3D_H
#define NX_RENDER_3D_H

#include "./NX_InstanceBuffer.h"
#include "./NX_RenderTexture.h"
#include "./NX_DynamicMesh.h"
#include "./NX_Environment.h"
#include "./NX_Material.h"
#include "./NX_Camera.h"
#include "./NX_Model.h"
#include "./NX_Mesh.h"
#include "./NX_Math.h"
#include "./NX_API.h"

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Begins 3D rendering.
 * Sets up the rendering state for 3D primitives, meshes, and models.
 * @param camera Pointer to the camera to use (can be NULL to use the default camera).
 * @param env Pointer to the environment to use (can be NULL to use the default environment).
 * @param target Render texture to draw into (can be NULL to render to the screen).
 */
NXAPI void NX_Begin3D(const NX_Camera* camera, const NX_Environment* env, const NX_RenderTexture* target);

/**
 * @brief Finalizes 3D rendering.
 * Renders all accumulated draw calls, applies post-processing, and outputs to the final render target.
 */
NXAPI void NX_End3D(void);

/**
 * @brief Draws a 3D mesh.
 * @param mesh Pointer to the mesh to draw (cannot be NULL).
 * @param material Pointer to the material to use (can be NULL to use the default material).
 * @param transform Pointer to the transformation matrix (can be NULL to use identity).
 */
NXAPI void NX_DrawMesh3D(const NX_Mesh* mesh, const NX_Material* material, const NX_Transform* transform);

/**
 * @brief Draws a 3D mesh with instanced rendering.
 *
 * Renders the given mesh multiple times in a single draw call using per-instance data.
 *
 * @param mesh Pointer to the mesh to draw (cannot be NULL).
 * @param instances Pointer to the instance buffer containing per-instance attributes (cannot be NULL).
 * @param instanceCount Number of instances to render (must be > 0).
 * @param material Pointer to the material to use (can be NULL to use the default material).
 * @param transform Pointer to the base transformation matrix applied to all instances
 *                  (can be NULL to use identity).
 *
 * @note No frustum culling is performed for instanced rendering.
 */
NXAPI void NX_DrawMeshInstanced3D(const NX_Mesh* mesh, const NX_InstanceBuffer* instances, int instanceCount,
                                  const NX_Material* material, const NX_Transform* transform);

/**
 * @brief Draws a 3D dynamic mesh.
 *
 * Renders a mesh whose vertex data can change every frame.
 *
 * @param dynMesh Pointer to the dynamic mesh to draw (cannot be NULL).
 * @param material Pointer to the material to use (can be NULL to use the default material).
 * @param transform Pointer to the transformation matrix (can be NULL to use identity).
 */
NXAPI void NX_DrawDynamicMesh3D(const NX_DynamicMesh* dynMesh, const NX_Material* material, const NX_Transform* transform);

/**
 * @brief Draws a 3D dynamic mesh with instanced rendering.
 *
 * Renders the given dynamic mesh multiple times in a single draw call using per-instance data.
 *
 * @param dynMesh Pointer to the dynamic mesh to draw (cannot be NULL).
 * @param instances Pointer to the instance buffer containing per-instance attributes (cannot be NULL).
 * @param instanceCount Number of instances to render (must be > 0).
 * @param material Pointer to the material to use (can be NULL to use the default material).
 * @param transform Pointer to the base transformation matrix applied to all instances
 *                  (can be NULL to use identity).
 *
 * @note No frustum culling is performed for instanced rendering.
 */
NXAPI void NX_DrawDynamicMeshInstanced3D(const NX_DynamicMesh* dynMesh, const NX_InstanceBuffer* instances, int instanceCount,
                                         const NX_Material* material, const NX_Transform* transform);

/**
 * @brief Draws a 3D model.
 * @param model Pointer to the model to draw (cannot be NULL).
 * @param transform Pointer to the transformation matrix (can be NULL to use identity).
 * @note Draws all meshes contained in the model with their associated materials.
 */
NXAPI void NX_DrawModel3D(const NX_Model* model, const NX_Transform* transform);

/**
 * @brief Draws a 3D model with instanced rendering.
 *
 * Renders the given model multiple times in a single draw call using per-instance data.
 * All meshes in the model are drawn with their associated materials.
 *
 * @param model Pointer to the model to draw (cannot be NULL).
 * @param instances Pointer to the instance buffer containing per-instance attributes (cannot be NULL).
 * @param instanceCount Number of instances to render (must be > 0).
 * @param transform Pointer to the base transformation matrix applied to all instances
 *                  (can be NULL to use identity).
 *
 * @note No frustum culling is performed for instanced rendering.
 */
NXAPI void NX_DrawModelInstanced3D(const NX_Model* model, const NX_InstanceBuffer* instances,
                                   int instanceCount, const NX_Transform* transform);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_RENDER_3D_H
