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
// TYPES DEFINITIONS
// ============================================================================

/**
 * @brief Bitfield flags controlling optional per-pass rendering behaviors.
 *
 * These flags allow enabling or disabling automatic operations such as 
 * frustum culling and draw call sorting for specific rendering passes.
 */
typedef uint32_t NX_RenderFlags;

#define NX_RENDER_FRUSTUM_CULLING          (1 << 0)     ///< Enables naive frustum culling over all draw calls

#define NX_RENDER_SORT_OPAQUE              (1 << 1)     ///< Sort opaque objects front-to-back
#define NX_RENDER_SORT_PREPASS             (1 << 2)     ///< Sort pre-pass objects front-to-back
#define NX_RENDER_SORT_TRANSPARENT         (1 << 3)     ///< Sort transparent objects back-to-front

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Begins a 3D scene rendering pass.
 *
 * Starts rendering for the main 3D scene using the given camera and environment.
 * This function uses the default render target (backbuffer) and default render options.
 *
 * @param camera Pointer to the camera to use (can be NULL to use the default camera).
 * @param env Pointer to the environment to use (can be NULL to use the default environment).
 * @param flags Render flags controlling optional per-pass behaviors (e.g. frustum culling, sorting).
 *
 * @note This function automatically renders to the backbuffer.
 *       For custom render targets, use NX_BeginEx3D().
 * @note The rendering pass is explicit; you must call NX_End3D() to finalize it.
 * @note Ensure no other render pass is active when calling this function.
 */
NXAPI void NX_Begin3D(const NX_Camera* camera, const NX_Environment* env, NX_RenderFlags flags);

/**
 * @brief Begins an extended 3D scene rendering pass.
 *
 * Starts rendering for the main 3D scene using the given camera, environment,
 * and a custom render target. This version provides full control over render flags
 * and output destination.
 *
 * @param camera Pointer to the camera to use (can be NULL to use the default camera).
 * @param env Pointer to the environment to use (can be NULL to use the default environment).
 * @param target Render texture to draw into (can be NULL to render to the backbuffer).
 * @param flags Render flags controlling optional per-pass behaviors (e.g. frustum culling, sorting).
 *
 * @note The rendering pass is explicit; you must call NX_End3D() to finalize it.
 * @note Ensure no other render pass is active when calling this function.
 */
NXAPI void NX_BeginEx3D(const NX_Camera* camera, const NX_Environment* env, const NX_RenderTexture* target, NX_RenderFlags flags);

/**
 * @brief Ends the current 3D scene rendering pass.
 *
 * Renders all accumulated draw calls, applies post-processing effects, 
 * and outputs the final image to the render target specified in NX_Begin3D (or the backbuffer if NULL).
 *
 * @note Must be called after NX_Begin3D().
 * @note Logs a warning if no scene render pass is active.
 */
NXAPI void NX_End3D(void);

/**
 * @brief Begins shadow map rendering for a specific light.
 *
 * Starts rendering into the shadow map associated with the given light.
 *
 * @param light Pointer to the light whose shadow map will be rendered. Must have shadows enabled.
 * @param camera Optional pointer to a camera used for determining the shadow frustum.
 *               It is required for directional lights (to center the shadow frustum around the camera)
 *               and for correct rendering of billboard shadows. Can be NULL in other cases,
 *               in which case the default camera will be used.
 * @param flags Render flags controlling optional per-pass behaviors 
 *              (currently only affects frustum culling; sorting flags are ignored).
 *
 * @note You must call NX_EndShadow3D() to finalize the shadow rendering pass.
 * @note Ensure no other render pass is active when calling this function.
 * @note A warning will be logged if the light has no valid shadow map assigned.
 */
NXAPI void NX_BeginShadow3D(NX_Light* light, const NX_Camera* camera, NX_RenderFlags flags);

/**
 * @brief Ends the current shadow map rendering pass.
 *
 * Finalizes rendering into the shadow map of the active light.
 * Resets internal state to allow other render passes to begin.
 *
 * @note Must be called after NX_BeginShadow3D().
 * @note Logs a warning if no shadow pass is active.
 */
NXAPI void NX_EndShadow3D();

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
