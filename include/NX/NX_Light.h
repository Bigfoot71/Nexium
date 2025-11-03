/* NX_Light.h -- API declaration for Nexium's light module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_LIGHT_H
#define NX_LIGHT_H

#include "./NX_Camera.h"
#include "./NX_Math.h"
#include "./NX_API.h"

// ============================================================================
// TYPES DEFINITIONS
// ============================================================================

/**
 * @brief Defines shadow casting behavior for meshes.
 */
typedef enum NX_ShadowCastMode {
    NX_SHADOW_CAST_ENABLED,     ///< Cast shadows and render normally (default).
    NX_SHADOW_CAST_ONLY,        ///< Only cast shadows, not rendered in main pass.
    NX_SHADOW_CAST_DISABLED     ///< Do not cast shadows.
} NX_ShadowCastMode;

/**
 * @brief Shadow rendering faces.
 *
 * Determines which faces of a mesh are rendered into the shadow map.
 */
typedef enum NX_ShadowFaceMode {
    NX_SHADOW_FACE_AUTO,        ///< Use material culling to decide which faces to render.
    NX_SHADOW_FACE_FRONT,       ///< Render only front faces into the shadow map.
    NX_SHADOW_FACE_BACK,        ///< Render only back faces into the shadow map.
    NX_SHADOW_FACE_BOTH         ///< Render both front and back faces (disable culling).
} NX_ShadowFaceMode;

/**
 * @brief Modes for updating shadow maps.
 *
 * Determines how often the shadow maps are refreshed.
 */
typedef enum NX_ShadowUpdateMode {
    NX_SHADOW_UPDATE_CONTINUOUS,       ///< Shadow maps update every frame.
    NX_SHADOW_UPDATE_INTERVAL,         ///< Shadow maps update at defined time intervals.
    NX_SHADOW_UPDATE_MANUAL,           ///< Shadow maps update only when explicitly requested.
} NX_ShadowUpdateMode;

/**
 * @brief Types of lights supported by the rendering engine.
 *
 * Each light type has different behaviors and use cases.
 */
typedef enum NX_LightType {
    NX_LIGHT_DIR,               ///< Directional light, affects the entire scene with parallel rays.
    NX_LIGHT_SPOT,              ///< Spot light, emits light in a cone shape.
    NX_LIGHT_OMNI,              ///< Omni light, emits light in all directions from a single point.
    NX_LIGHT_TYPE_COUNT
} NX_LightType;

/**
 * @brief Opaque handle to a light source.
 *
 * Represents a light in the scene.
 * Can be used for directional, spot or omni-directional lights.
 */
typedef struct NX_Light NX_Light;

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Creates a new light of the given type.
 * @param type Type of light (directional, point, spot, etc.).
 * @return Pointer to a newly created NX_Light.
 * @note Lights are inactive by default after creation.
 */
NXAPI NX_Light* NX_CreateLight(NX_LightType type);

/**
 * @brief Destroys a light and frees its resources.
 * @param light Pointer to the NX_Light to destroy.
 */
NXAPI void NX_DestroyLight(NX_Light* light);

/**
 * @brief Checks if a light is active.
 * @param light Pointer to the NX_Light.
 * @return true if the light is active, false otherwise.
 */
NXAPI bool NX_IsLightActive(const NX_Light* light);

/**
 * @brief Activates or deactivates a light.
 * @param light Pointer to the NX_Light.
 * @param active true to enable, false to disable.
 */
NXAPI void NX_SetLightActive(NX_Light* light, bool active);

/**
 * @brief Gets the layer mask required for a light to be considered in the scene.
 * @param light Pointer to the NX_Light.
 * @return Current layer mask.
 * @note Default is NX_LAYER_01. Changes take effect immediately.
 */
NXAPI NX_Layer NX_GetLightLayerMask(const NX_Light* light);

/**
 * @brief Sets the layer mask required for a light to be considered in the scene.
 * @param light Pointer to the NX_Light.
 * @param layers Layer mask to set.
 * @note Default is NX_LAYER_01. Changes take effect immediately.
 */
NXAPI void NX_SetLightLayerMask(NX_Light* light, NX_Layer layers);

/**
 * @brief Gets the culling mask defining which meshes are lit by the light.
 * @param light Pointer to the NX_Light.
 * @return Current culling mask.
 * @note Default is NX_LAYER_ALL. The GPU still processes the light, but masked meshes receive zero contribution.
 */
NXAPI NX_Layer NX_GetLightCullMask(const NX_Light* light);

/**
 * @brief Sets the culling mask defining which meshes are lit by the light.
 * @param light Pointer to the NX_Light.
 * @param layers Layer mask to set.
 * @note Default is NX_LAYER_ALL. The GPU still processes the light, but masked meshes receive zero contribution.
 */
NXAPI void NX_SetLightCullMask(NX_Light* light, NX_Layer layers);

/**
 * @brief Gets the light position.
 * @param light Pointer to the NX_Light.
 * @return Position of the light.
 * @note Ignored for directional lights. Default is NX_VEC3_ZERO.
 */
NXAPI NX_Vec3 NX_GetLightPosition(const NX_Light* light);

/**
 * @brief Sets the light position.
 * @param light Pointer to the NX_Light.
 * @param position New position.
 * @note Ignored for directional lights. Default is NX_VEC3_ZERO.
 */
NXAPI void NX_SetLightPosition(NX_Light* light, NX_Vec3 position);

/**
 * @brief Gets the light direction.
 * @param light Pointer to the NX_Light.
 * @return Direction of the light.
 * @note Ignored for point lights. Default is NX_VEC3_FORWARD.
 */
NXAPI NX_Vec3 NX_GetLightDirection(const NX_Light* light);

/**
 * @brief Sets the light direction.
 * @param light Pointer to the NX_Light.
 * @param direction New direction.
 * @note Ignored for point lights. Default is NX_VEC3_FORWARD.
 */
NXAPI void NX_SetLightDirection(NX_Light* light, NX_Vec3 direction);

/**
 * @brief Gets the light color.
 * @param light Pointer to the NX_Light.
 * @return Current light color.
 * @note Alpha is ignored. Default is NX_WHITE.
 */
NXAPI NX_Color NX_GetLightColor(const NX_Light* light);

/**
 * @brief Sets the light color.
 * @param light Pointer to the NX_Light.
 * @param color New light color.
 * @note Alpha is ignored. Default is NX_WHITE.
 */
NXAPI void NX_SetLightColor(NX_Light* light, NX_Color color);

/**
 * @brief Gets the light energy factor.
 * @param light Pointer to the NX_Light.
 * @return Energy multiplier applied to the light color.
 * @note Default is 1.0.
 */
NXAPI float NX_GetLightEnergy(const NX_Light* light);

/**
 * @brief Sets the light energy factor.
 * @param light Pointer to the NX_Light.
 * @param energy Energy multiplier applied to the light color.
 * @note Default is 1.0.
 */
NXAPI void NX_SetLightEnergy(NX_Light* light, float energy);

/**
 * @brief Gets the specular reflection factor.
 * @param light Pointer to the NX_Light.
 * @return Current specular factor.
 * @note Default is 0.5.
 */
NXAPI float NX_GetLightSpecular(const NX_Light* light);

/**
 * @brief Sets the specular reflection factor.
 * @param light Pointer to the NX_Light.
 * @param specular New specular factor.
 * @note Default is 0.5.
 */
NXAPI void NX_SetLightSpecular(NX_Light* light, float specular);

/**
 * @brief Gets the maximum lighting range.
 * @param light Pointer to the NX_Light.
 * @return Current range in world units.
 * @note Ignored for directional lights. Default is 16.0.
 */
NXAPI float NX_GetLightRange(const NX_Light* light);

/**
 * @brief Sets the maximum lighting range.
 * @param light Pointer to the NX_Light.
 * @param range New range in world units.
 * @note For directional lights, the range defines the radius around
         the camera within which objects will be rendered into
         the shadow map. Default is 8.0.
 */
NXAPI void NX_SetLightRange(NX_Light* light, float range);

/**
 * @brief Gets the attenuation factor over the light range.
 * @param light Pointer to the NX_Light.
 * @return Current attenuation factor.
 * @note Ignored for directional lights. Default is 1.0.
 */
NXAPI float NX_GetLightAttenuation(const NX_Light* light);

/**
 * @brief Sets the attenuation factor over the light range.
 * @param light Pointer to the NX_Light.
 * @param attenuation New attenuation factor.
 * @note Ignored for directional lights. Default is 1.0.
 */
NXAPI void NX_SetLightAttenuation(NX_Light* light, float attenuation);

/**
 * @brief Gets the inner cutoff angle of a spotlight.
 * @param light Pointer to the NX_Light.
 * @return Inner cutoff angle in radians.
 * @note Used only for spotlights. Default is ~45°.
 */
NXAPI float NX_GetLightInnerCutOff(const NX_Light* light);

/**
 * @brief Sets the inner cutoff angle of a spotlight.
 * @param light Pointer to the NX_Light.
 * @param radians Inner cutoff angle in radians.
 * @note Used only for spotlights. Default is ~45°.
 */
NXAPI void NX_SetLightInnerCutOff(NX_Light* light, float radians);

/**
 * @brief Gets the outer cutoff angle of a spotlight.
 * @param light Pointer to the NX_Light.
 * @return Outer cutoff angle in radians.
 * @note Used only for spotlights. Default is ~90°.
 */
NXAPI float NX_GetLightOuterCutOff(const NX_Light* light);

/**
 * @brief Sets the outer cutoff angle of a spotlight.
 * @param light Pointer to the NX_Light.
 * @param radians Outer cutoff angle in radians.
 * @note Used only for spotlights. Default is ~90°.
 */
NXAPI void NX_SetLightOuterCutOff(NX_Light* light, float radians);

/**
 * @brief Sets both inner and outer cutoff angles of a spotlight.
 * @param light Pointer to the NX_Light.
 * @param inner Inner cutoff angle in radians.
 * @param outer Outer cutoff angle in radians.
 * @note Used only for spotlights. Default is ~45°–90°.
 */
NXAPI void NX_SetLightCutOff(NX_Light* light, float inner, float outer);

/**
 * @brief Checks if shadows are active for the light.
 * @param light Pointer to the NX_Light.
 * @return True if shadows are active, false otherwise.
 * @note Shadows are disabled by default.
 */
NXAPI bool NX_IsShadowActive(const NX_Light* light);

/**
 * @brief Enables or disables shadows for the light.
 * @param light Pointer to the NX_Light.
 * @param active True to enable shadows, false to disable.
 * @note Shadows are disabled by default.
 */
NXAPI void NX_SetShadowActive(NX_Light* light, bool active);

/**
 * @brief Gets the shadow culling mask.
 * @param light Pointer to the NX_Light.
 * @return Current shadow culling mask.
 * @note Unlike the light cull mask, meshes excluded here are completely omitted from shadow maps.
 * @note Changes are applied only on the next shadow map update.
 */
NXAPI NX_Layer NX_GetShadowCullMask(const NX_Light* light);

/**
 * @brief Sets the shadow culling mask.
 * @param light Pointer to the NX_Light.
 * @param layers New shadow culling mask.
 * @note Unlike the light cull mask, meshes excluded here are completely omitted from shadow maps.
 * @note Changes are applied only on the next shadow map update.
 */
NXAPI void NX_SetShadowCullMask(NX_Light* light, NX_Layer layers);

/**
 * @brief Gets the shadow slope bias.
 * @param light Pointer to the NX_Light.
 * @return Current shadow slope bias.
 * @note Default value is 0.005.
 */
NXAPI float NX_GetShadowSlopeBias(NX_Light* light);

/**
 * @brief Sets the shadow slope bias.
 * @param light Pointer to the NX_Light.
 * @param slopeBias New shadow slope bias.
 * @note Default value is 0.005.
 */
NXAPI void NX_SetShadowSlopeBias(NX_Light* light, float slopeBias);

/**
 * @brief Gets the shadow bias.
 * @param light Pointer to the NX_Light.
 * @return Current shadow bias.
 * @note Default value is 0.001.
 */
NXAPI float NX_GetShadowBias(NX_Light* light);

/**
 * @brief Sets the shadow bias.
 * @param light Pointer to the NX_Light.
 * @param bias New shadow bias.
 * @note Default value is 0.001.
 */
NXAPI void NX_SetShadowBias(NX_Light* light, float bias);

/**
 * @brief Gets the shadow softness (poisson disk radius).
 * @param light Pointer to the NX_Light.
 * @return Current shadow softness in texels.
 * @note Default value is 2.0.
 * @note A value below 1.0 produces hard shadows, larger values produce softer, more diffuse shadows.
 */
NXAPI float NX_GetShadowSoftness(const NX_Light* light);

/**
 * @brief Sets the shadow softness (poisson disk radius).
 * @param light Pointer to the NX_Light.
 * @param softness New shadow softness in texels.
 * @note Default value is 2.0.
 * @note A value below 1.0 produces hard shadows, larger values produce softer, more diffuse shadows.
 */
NXAPI void NX_SetShadowSoftness(NX_Light* light, float softness);

/**
 * @brief Gets the shadow map update mode.
 * @param light Pointer to the NX_Light.
 * @return Current shadow update mode.
 */
NXAPI NX_ShadowUpdateMode NX_GetShadowUpdateMode(const NX_Light* light);

/**
 * @brief Sets the shadow map update mode.
 * @param light Pointer to the NX_Light.
 * @param mode Shadow update mode (Continuous, Interval, or Manual).
 * @note Controls when and how often the shadow map is refreshed.
 */
NXAPI void NX_SetShadowUpdateMode(NX_Light* light, NX_ShadowUpdateMode mode);

/**
 * @brief Gets the shadow update interval.
 * @param light Pointer to the NX_Light.
 * @return Update interval in seconds.
 * @note Only relevant when update mode is set to Interval.
 */
NXAPI float NX_GetShadowUpdateInterval(const NX_Light* light);

/**
 * @brief Sets the shadow update interval.
 * @param light Pointer to the NX_Light.
 * @param sec Interval in seconds between shadow updates.
 * @note Only relevant when update mode is set to Interval.
 */
NXAPI void NX_SetShadowUpdateInterval(NX_Light* light, float sec);

/**
 * @brief Forces an immediate shadow map update.
 * @param light Pointer to the NX_Light.
 * @note The shadow map will be refreshed on the next rendering pass.
 *       Useful in Manual update mode, but also works with Interval mode.
 */
NXAPI void NX_UpdateShadowMap(NX_Light* light);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_LIGHT_H
