/* NX_Camera.h -- API declaration for Nexium's camera module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_CAMERA_H
#define NX_CAMERA_H

#include "./NX_Math.h"
#include "./NX_API.h"

// ============================================================================
// TYPES DEFINITIONS
// ============================================================================

/**
 * @brief Bitfield type used to assign rendering layers for visibility, lighting, and shadows.
 *
 * Layer masks define which entities affect or are affected by others during rendering:
 *
 * - Cameras use `cullMask` to select which mesh and light layers are visible.
 * - Meshes use `layerMask` to indicate which layers they belong to.
 *   A mesh is considered:
 *     - visible if `mesh.layerMask & camera.cullMask != 0`,
 *     - illuminated by a light if `mesh.layerMask & light.lightCullMask != 0`,
 *     - casting shadows for a light if `mesh.layerMask & light.shadowCullMask != 0`.
 * - Lights use:
 *     - `layerMask` to determine if the light is active for a camera (`light.layerMask & camera.cullMask != 0`),
 *     - `lightCullMask` to select which mesh layers it illuminates,
 *     - `shadowCullMask` to select which mesh layers cast shadows.
 *
 * @note By default `cullMasks` are set to NX_LAYER_ALL, and `layerMasks` are set to NX_LAYER_01.
 */
typedef uint16_t NX_Layer;

#define NX_LAYER_NONE   0x0000
#define NX_LAYER_ALL    0xFFFF

#define NX_LAYER_01     (1 << 0)
#define NX_LAYER_02     (1 << 1)
#define NX_LAYER_03     (1 << 2)
#define NX_LAYER_04     (1 << 3)
#define NX_LAYER_05     (1 << 4)
#define NX_LAYER_06     (1 << 5)
#define NX_LAYER_07     (1 << 6)
#define NX_LAYER_08     (1 << 7)
#define NX_LAYER_09     (1 << 8)
#define NX_LAYER_10     (1 << 9)
#define NX_LAYER_11     (1 << 10)
#define NX_LAYER_12     (1 << 11)
#define NX_LAYER_13     (1 << 12)
#define NX_LAYER_14     (1 << 13)
#define NX_LAYER_15     (1 << 14)
#define NX_LAYER_16     (1 << 15)

/**
 * @brief Defines the type of projection used by a camera.
 */
typedef enum NX_Projection {
    NX_PROJECTION_PERSPECTIVE,      ///< Standard perspective projection. Objects appear smaller when farther.
    NX_PROJECTION_ORTHOGRAPHIC      ///< Orthographic projection. Objects keep the same size regardless of distance.
} NX_Projection;

/**
 * @brief Represents a camera in 3D space.
 *
 * Stores position, orientation, projection parameters,
 * and layer culling information for rendering a scene.
 */
typedef struct NX_Camera {
    NX_Vec3 position;           ///< Camera position in world space.
    NX_Quat rotation;           ///< Camera orientation as a quaternion.
    float nearPlane;            ///< Near clipping plane distance.
    float farPlane;             ///< Far clipping plane distance.
    float fov;                  /**< Vertical field of view:
                                  *   - Perspective: angle in radians.
                                  *   - Orthographic: half-height of the view volume. */
    NX_Projection projection;   ///< Projection type (perspective or orthographic).
    NX_Layer cullMask;          ///< Mask indicating which meshes and lights to render.
} NX_Camera;

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Returns the default camera.
 * @return NX_Camera initialized at (0,0,0) looking forward with identity rotation.
 * @note Near plane = 0.05, Far plane = 4000.0, Vertical FOV = 60 degrees, Perspective projection.
 */
NXAPI NX_Camera NX_GetDefaultCamera(void);

/**
 * @brief Updates an orbital camera around a target point.
 * @param camera Pointer to the camera to update.
 * @param center Center point around which the camera orbits.
 * @param distance Distance from the center.
 * @param height Height above the center point.
 * @param rotation Rotation around the center in radians.
 */
NXAPI void NX_UpdateCameraOrbital(NX_Camera* camera, NX_Vec3 center, float distance, float height, float rotation);

/**
 * @brief Updates a free-moving camera with clamped pitch.
 * @param camera Pointer to the camera to update.
 * @param movement Movement vector to apply.
 * @param rotation Rotation vector (pitch/yaw/roll) in radians.
 * @param maxPitch Maximum pitch (around X) in radians.
 *        If negative, clamp is approximately -89/+89 degrees.
 *        Zero can be useful to make a Doom-like cameras.
 */
NXAPI void NX_UpdateCameraFree(NX_Camera* camera, NX_Vec3 movement, NX_Vec3 rotation, float maxPitch);

/**
 * @brief Updates a first-person camera with clamped pitch.
 * @param camera Pointer to the camera to update.
 * @param movement Movement vector to apply.
 * @param rotation Rotation vector (pitch/yaw/roll) in radians.
 * @param maxPitch Maximum pitch (around X) in radians.
 *        If negative, clamp is approximately -89/+89 degrees.
 *        Zero can be useful to make a Doom-like cameras.
 */
NXAPI void NX_UpdateCameraFPS(NX_Camera* camera, NX_Vec3 movement, NX_Vec3 rotation, float maxPitch);

/**
 * @brief Applies a transformation matrix and optional offset to a camera.
 * @param camera Pointer to the camera to transform.
 * @param transform Transformation matrix to apply.
 * @param offset Optional positional offset applied after the transformation.
 * @note Useful for syncing the camera with a player or object while adding an offset.
 */
NXAPI void NX_ApplyCameraTransform(NX_Camera* camera, NX_Mat4 transform, NX_Vec3 offset);

/**
 * @brief Retrieves the current transformation of a camera.
 * @param camera Pointer to the camera.
 * @return The camera's current transform.
 */
NXAPI NX_Transform NX_GetCameraTransform(const NX_Camera* camera);

/**
 * @brief Retrieves the view matrix of a camera.
 * @param camera Pointer to the camera.
 * @return The camera's current view matrix.
 */
NXAPI NX_Mat4 NX_GetCameraViewMatrix(const NX_Camera* camera);

/**
 * @brief Retrieves the projection matrix of a camera.
 * @param camera Pointer to the camera.
 * @param aspect Aspect ratio (width / height) for the projection.
 * @return The camera's current projection matrix.
 */
NXAPI NX_Mat4 NX_GetCameraProjectionMatrix(const NX_Camera* camera, float aspect);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_CAMERA_H
