/* NX_Environment.h -- API declaration for Nexium's environment module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_ENVIRONMENT_H
#define NX_ENVIRONMENT_H

#include "./NX_ReflectionProbe.h"
#include "./NX_Cubemap.h"
#include "./NX_Math.h"
#include "./NX_API.h"

// ============================================================================
// MACROS DEFINITIONS
// ============================================================================

#define NX_BASE_ENVIRONMENT NX_LITERAL(NX_Environment)  \
{                                                       \
    .background = NX_GRAY,                              \
    .ambient = NX_DARK_GRAY,                            \
    .sky = {                                            \
        .cubemap = NULL,                                \
        .probe = NULL,                                  \
        .rotation = NX_QUAT_IDENTITY,                   \
        .intensity = 1.0f,                              \
        .specular = 1.0f,                               \
        .diffuse = 1.0f                                 \
    },                                                  \
    .fog = {                                            \
        .mode = NX_FOG_DISABLED,                        \
        .density = 0.01f,                               \
        .start = 5.0f,                                  \
        .end = 50.0f,                                   \
        .skyAffect = 0.5f,                              \
        .color = NX_GRAY                                \
    },                                                  \
    .ssao = {                                           \
        .intensity = 1.0f,                              \
        .radius = 0.5f,                                 \
        .power = 1.0f,                                  \
        .bias = 0.025f,                                 \
        .enabled = false                                \
    },                                                  \
    .bloom {                                            \
        .mode = NX_BLOOM_DISABLED,                      \
        .threshold = 0.0f,                              \
        .softThreshold = 0.5f,                          \
        .filterRadius = 0,                              \
        .strength = 0.05f,                              \
        .levels = {                                     \
            0.0f,                                       \
            0.0f,                                       \
            0.0f,                                       \
            1.0f,                                       \
            0.0f,                                       \
            1.0f,                                       \
            0.0f,                                       \
            0.0f                                        \
        }                                               \
    },                                                  \
    .adjustment = {                                     \
        .brightness = 1.0f,                             \
        .contrast = 1.0f,                               \
        .saturation = 1.0f                              \
    },                                                  \
    .tonemap = {                                        \
        .mode = NX_TONEMAP_LINEAR,                      \
        .exposure = 1.0f,                               \
        .white = 1.0f                                   \
    }                                                   \
}

// ============================================================================
// TYPES DEFINITIONS
// ============================================================================

/**
 * @brief Extra flags for NX_Environment specifying rendering behaviors.
 *
 * These flags control optional rendering features that Nexium can enable per-environment.
 */
typedef uint32_t NX_EnvironmentFlag;

#define NX_ENV_SORT_OPAQUE              (1 << 0)    ///< Sort opaque objects front-to-back
#define NX_ENV_SORT_PREPASS             (1 << 1)    ///< Sort pre-pass objects front-to-back
#define NX_ENV_SORT_TRANSPARENT         (1 << 2)    ///< Sort transparent objects back-to-front

/**
 * @brief Modes for applying bloom effect.
 *
 * Determines how the bloom effect is blended with the scene.
 */
typedef enum NX_Bloom {
    NX_BLOOM_DISABLED,      ///< Bloom effect is disabled.
    NX_BLOOM_MIX,           ///< Interpolates between the scene and the pre-multiplied bloom based on intensity.
    NX_BLOOM_ADDITIVE,      ///< Adds the bloom to the scene, scaled by intensity.
    NX_BLOOM_SCREEN,        ///< Blends the scene with bloom using screen blend mode.
    NX_BLOOM_COUNT          ///< Number of bloom modes (used internally).
} NX_Bloom;

/**
 * @brief Fog effect modes.
 *
 * Determines how fog is applied to the scene, affecting depth perception and atmosphere.
 */
typedef enum NX_Fog {
    NX_FOG_DISABLED,        ///< Fog effect is disabled.
    NX_FOG_LINEAR,          ///< Fog density increases linearly with distance from the camera.
    NX_FOG_EXP2,            ///< Exponential fog (exp2), where density increases exponentially with distance.
    NX_FOG_EXP              ///< Exponential fog, similar to EXP2 but with a different rate of increase.
} NX_Fog;

/**
 * @brief Tone mapping modes.
 *
 * Controls how high dynamic range (HDR) colors are mapped to low dynamic range (LDR) for display.
 */
typedef enum NX_Tonemap {
    NX_TONEMAP_LINEAR,      ///< Simple linear mapping of HDR values.
    NX_TONEMAP_REINHARD,    ///< Reinhard tone mapping, a balanced method for compressing HDR values.
    NX_TONEMAP_FILMIC,      ///< Filmic tone mapping, mimicking the response of photographic film.
    NX_TONEMAP_ACES,        ///< ACES tone mapping, a high-quality cinematic rendering technique.
    NX_TONEMAP_AGX,         ///< AGX tone mapping, a modern technique designed to preserve both highlight and shadow details for HDR rendering.
    NX_TONEMAP_COUNT        ///< Number of tone mapping modes (used internally)
} NX_Tonemap;

/**
 * @brief Represents a 3D scene environment.
 *
 * Stores background/ambient colors, sky settings,
 * global adjustments, and post-processing parameters.
 */
typedef struct NX_Environment {

    NX_Color background;            ///< Fallback background color if no skybox is defined.
    NX_Color ambient;               ///< Fallback ambient light color if no reflection probe is defined.

    struct {
        NX_Cubemap* cubemap;        ///< Skybox cubemap texture. If null, 'background' is used.
        NX_ReflectionProbe* probe;  ///< Global reflection probe derived from the skybox. If null, 'ambient' is used.
        NX_Quat rotation;           ///< Orientation applied to the skybox and its reflection probe.
        float intensity;            ///< Overall sky contribution (affects cubemap and IBL).
        float specular;             ///< Specular reflection contribution (prefiltered environment).
        float diffuse;              ///< Diffuse lighting contribution (irradiance).
    } sky;

    struct {
        NX_Fog mode;                ///< fog mode
        float density;              ///< fog density
        float start;                ///< fog start distance (linear only)
        float end;                  ///< fog end distance (linear only)
        float skyAffect;            ///< influence of sky color on the fog
        NX_Color color;             ///< fog color
    } fog;

    struct {
        float intensity;            ///< Overall strength of the SSAO effect (scales the occlusion).
        float radius;               ///< Sampling radius in view-space units; larger values capture broader occlusion.
        float power;                ///< Exponent applied to the SSAO term; higher values darken occlusion and sharpen falloff.
        float bias;                 ///< Small depth offset to reduce self-occlusion artifacts on flat surfaces.
        bool enabled;               ///< Enables or disables the SSAO pass.
    } ssao;

    struct {
        NX_Bloom mode;              ///< Mode used to combine the bloom effect with the scene.
        float threshold;            ///< HDR threshold used for bloom extraction.
        float softThreshold;        ///< Softening factor applied during prefiltering.
        float filterRadius;         ///< Radius of the blur filter used for bloom spreading.
        float strength;             ///< Intensity of the bloom effect when blended with the scene.
        float levels[8];            ///< Bloom contribution factors, lower levels give a local effect, higher levels a global one.
    } bloom;

    struct {
        float brightness;           ///< Global brightness adjustment.
        float contrast;             ///< Global contrast adjustment.
        float saturation;           ///< Global saturation adjustment.
    } adjustment;

    struct {
        NX_Tonemap mode;            ///< Tonemapping operator.
        float exposure;             ///< Exposure compensation.
        float white;                ///< White point reference (unused with AGX).
    } tonemap;

    NX_EnvironmentFlag flags;       ///< Extra flags about rendering behavior.

} NX_Environment;

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Returns the current default environment.
 *
 * If no environment was set, returns NX_BASE_MATERIAL by default.
 */
NXAPI NX_Environment NX_GetDefaultEnvironment(void);

/**
 * @brief Sets the default environment used by Nexium.
 *
 * Overrides the environment returned by NX_GetDefaultMaterial().
 * Pass NULL to restore the default NX_BASE_MATERIAL.
 */
NXAPI void NX_SetDefaultEnvironment(const NX_Environment* env);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_ENVIRONMENT_H
