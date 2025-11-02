/* NX_Material.h -- API declaration for Nexium's material module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_MATERIAL_H
#define NX_MATERIAL_H

#include "./NX_Shader3D.h"
#include "./NX_Texture.h"
#include "./NX_Macros.h"
#include "./NX_Math.h"
#include "./NX_API.h"

// ============================================================================
// MACROS DEFINITIONS
// ============================================================================

#define NX_BASE_MATERIAL NX_LITERAL(NX_Material)        \
{                                                       \
    .albedo = {                                         \
        .texture = NULL,                                \
        .color = NX_WHITE,                              \
    },                                                  \
    .emission = {                                       \
        .texture = NULL,                                \
        .color = NX_WHITE,                              \
        .energy = 0.0f                                  \
    },                                                  \
    .orm = {                                            \
        .texture = NULL,                                \
        .aoLightAffect = 0.0f,                          \
        .occlusion = 1.0f,                              \
        .roughness = 1.0f,                              \
        .metalness = 0.0f,                              \
    },                                                  \
    .normal = {                                         \
        .texture = NULL,                                \
        .scale = 1.0f,                                  \
    },                                                  \
    .depth = {                                          \
        .test = NX_DEPTH_TEST_LESS,                     \
        .offset = 0.0f,                                 \
        .scale = 1.0f,                                  \
        .prePass = false                                \
    },                                                  \
    .alphaCutOff = 1e-6f,                               \
    .texOffset = NX_VEC2_ZERO,                          \
    .texScale = NX_VEC2_ONE,                            \
    .billboard = NX_BILLBOARD_DISABLED,                 \
    .shading = NX_SHADING_LIT,                          \
    .blend = NX_BLEND_OPAQUE,                           \
    .cull = NX_CULL_BACK,                               \
    .shader = NULL                                      \
}

// ============================================================================
// TYPES DEFINITIONS
// ============================================================================

/**
 * @brief Defines depth testing modes.
 *
 * Determines whether a fragment is drawn based on its depth value.
 */
typedef enum NX_DepthTest {
    NX_DEPTH_TEST_LESS,         ///< Pass if fragment is closer. Default.
    NX_DEPTH_TEST_GREATER,      ///< Pass if fragment is farther.
    NX_DEPTH_TEST_ALWAYS        ///< Always pass, ignore depth.
} NX_DepthTest;

/**
 * @brief Defines billboard modes for 3D objects.
 *
 * This enumeration defines how a 3D object aligns itself relative to the camera.
 * It provides options to disable billboarding or to enable specific modes of alignment.
 */
typedef enum NX_BillboardMode {
    NX_BILLBOARD_DISABLED,      ///< Billboarding is disabled; the object retains its original orientation.
    NX_BILLBOARD_FRONT,         ///< Full billboarding; the object fully faces the camera, rotating on all axes.
    NX_BILLBOARD_Y_AXIS         /**< Y-axis constrained billboarding; the object rotates only around the Y-axis,
                                     keeping its "up" orientation fixed. This is suitable for upright objects like characters or signs. */
} NX_BillboardMode;

/**
 * @brief Defines the available shading modes for rendering.
 */
typedef enum NX_ShadingMode {
    NX_SHADING_LIT,             ///< Standard lighting and shading applied.
    NX_SHADING_UNLIT,           ///< No lighting, renders with flat color.
} NX_ShadingMode;

/**
 * @brief Defines blending modes for rendering.
 */
typedef enum NX_BlendMode {
    NX_BLEND_OPAQUE,            ///< Standard opaque rendering. Ignores alpha channel.
    NX_BLEND_ALPHA,             ///< Standard alpha blending. Supports transparency.
    NX_BLEND_ADD,               ///< Additive blending. Colors are added to the framebuffer.
    NX_BLEND_MUL                ///< Multiplicative blending. Colors are multiplied with the framebuffer.
} NX_BlendMode;

/**
 * @brief Defines face culling modes.
 *
 * Determines which faces of a mesh are rendered.
 */
typedef enum NX_CullMode {
    NX_CULL_BACK,               ///< Cull back faces only. Default for solid objects.
    NX_CULL_FRONT,              ///< Cull front faces only.
    NX_CULL_NONE                ///< Disable face culling. Render all faces.
} NX_CullMode;

/**
 * @brief Represents a material for a mesh.
 *
 * Contains textures, colors, physical properties, and rendering settings.
 * Supports albedo, emission, ORM (Occlusion-Roughness-Metallic), and normal mapping.
 */
typedef struct NX_Material {

    struct {
        NX_Texture* texture;        ///< Albedo texture (diffuse color). Default: NULL (white texture)
        NX_Color color;             ///< Albedo color multiplier. Default: White
    } albedo;

    struct {
        NX_Texture* texture;        ///< Emission texture (self-illumination). Default: NULL (white texture)
        NX_Color color;             ///< Emission color multiplier. Default: White
        float energy;               ///< Strength of the emission. Default: 0.0f
    } emission;

    struct {
        NX_Texture* texture;        ///< ORM texture (Occlusion-Roughness-Metallic). Default: NULL (white texture)
        float aoLightAffect;        ///< How ambient occlusion affects lighting. Default: 0.0f
        float occlusion;            ///< Occlusion factor. Default: 1.0f
        float roughness;            ///< Surface roughness. Default: 1.0f
        float metalness;            ///< Surface metallic factor. Default: 0.0f
    } orm;

    struct {
        NX_Texture* texture;        ///< Normal map texture. Default: NULL (front facing)
        float scale;                ///< Normal map intensity. Default: 1.0f
    } normal;

    struct {
        NX_DepthTest test;          ///< Controls whether a fragment is visible compared to others. Default: NX_DEPTH_TEST_LESS
        float offset;               ///< Additive depth offset in clip space; + = farther, - = closer. Default: 0.0f
        float scale;                ///< Multiplicative depth scale in clip space; <1 = closer, >1 = farther. Default: 1.0f
        bool prePass;               ///< Enable depth pre-pass to reduce overdraw or support alpha cutoff; may be costly with heavy vertex shaders. Default: false
    } depth;

    float alphaCutOff;              ///< Fragments with alpha below this value are discarded (only with depth pre-pass). Default: 1e-6f.
    NX_Vec2 texOffset;              ///< Texture coordinate offset. Default: vec2(0,0)
    NX_Vec2 texScale;               ///< Texture coordinate scaling. Default: vec2(1,1)

    NX_BillboardMode billboard;     ///< Billboard mode applied to the object
    NX_ShadingMode shading;         ///< Describes the shading mode, lit or not
    NX_BlendMode blend;             ///< Blending mode for rendering. Default: Opaque
    NX_CullMode cull;               ///< Face culling mode. Default: Back face

    NX_Shader3D* shader;            ///< Pointer to an optional material shader. Default: NULL

} NX_Material;

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Returns the current default material.
 *
 * If no material was set, returns NX_BASE_MATERIAL by default.
 */
NXAPI NX_Material NX_GetDefaultMaterial(void);

/**
 * @brief Sets the default material used by the engine.
 *
 * Overrides the material returned by NX_GetDefaultMaterial().
 * Pass NULL to restore the default NX_BASE_MATERIAL.
 */
NXAPI void NX_SetDefaultMaterial(const NX_Material* material);

/**
 * @brief Frees all resources owned by a material (textures, shaders, etc.).
 *
 * Call only if the material is no longer used or shared.
 */
NXAPI void NX_DestroyMaterialResources(NX_Material* material);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_MATERIAL_H
