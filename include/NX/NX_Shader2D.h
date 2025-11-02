#ifndef NX_SHADER_2D_H
#define NX_SHADER_2D_H

#include "./NX_Texture.h"
#include "./NX_API.h"

// ============================================================================
// TYPES DEFINITIONS
// ============================================================================

/**
 * @brief Opaque handle to a 2D shader.
 *
 * Represents a customizable shader for 2D rendering.
 * Provides overrideable vertex/fragment entry points.
 */
typedef struct NX_Shader2D NX_Shader2D;

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Creates a custom 2D shader from GLSL source code.
 *
 * 2D shaders allow you to override the default 2D rendering pipeline by providing
 * custom vertex and/or fragment stages. At least one stage must be provided.
 *
 * Vertex stage (`void vertex()`) is called after vertex attributes (position, UV, color)
 * are prepared, allowing you to transform vertices, adjust colors, or apply per-vertex
 * effects such as waves or distortions.
 *
 * Fragment stage (`void fragment()`) is called after all per-vertex data has been
 * interpolated, allowing you to modify final pixel color, sample textures differently,
 * or apply custom shading effects.
 *
 * You have access to built-in global variables such as transformation matrices,
 * UV coordinates, vertex color, and TIME.
 *
 * See the shader documentation for more details. (coming soon)
 *
 * @param vertCode Source code of the vertex shader stage (can be NULL if not used).
 * @param fragCode Source code of the fragment shader stage (can be NULL if not used).
 * @return Pointer to the created NX_Shader2D, or NULL on failure.
 */
NXAPI NX_Shader2D* NX_CreateShader2D(const char* vertCode, const char* fragCode);

/**
 * @brief Loads a custom 2D shader from GLSL source files.
 *
 * Same behavior as NX_CreateShader2D, but loads the shader code from files.
 *
 * @param vertFile Path to the vertex shader file (can be NULL if not used).
 * @param fragFile Path to the fragment shader file (can be NULL if not used).
 * @return Pointer to the created NX_Shader2D, or NULL on failure.
 */
NXAPI NX_Shader2D* NX_LoadShader2D(const char* vertFile, const char* fragFile);

/**
 * @brief Destroys a 2D shader and releases associated GPU resources.
 *
 * @param shader Pointer to the NX_Shader2D to destroy.
 */
NXAPI void NX_DestroyShader2D(NX_Shader2D* shader);

/**
 * @brief Assigns a texture to a 2D shader sampler.
 *
 * This function sets a texture for a specific sampler slot in a 2D shader.
 * The shader must declare the sampler with one of the predefined names:
 * "Texture0", "Texture1", "Texture2", or "Texture3", all of type `sampler2D`.
 *
 * If `texture` is `NULL`, a default white texture will be used instead.
 *
 * @param shader Pointer to the NX_Shader2D to modify.
 * @param slot Index of the sampler to assign (0 to 3). The slot must correspond
 *             to a sampler declared in the shader with the matching name.
 * @param texture Pointer to the NX_Texture to bind, or `NULL` to use a white texture.
 *
 * @note Up to 4 texture samplers are supported per shader. It is the user's
 *       responsibility to ensure the shader defines the corresponding sampler names.
 */
NXAPI void NX_SetShader2DTexture(NX_Shader2D* shader, int slot, const NX_Texture* texture);

/**
 * @brief Updates the static uniform buffer of a 2D shader.
 *
 * Static buffers are defined in the shader as an uniform block named `StaticBuffer`.
 * They are constant across all draw calls using this shader. If multiple updates are
 * made during a frame, only the last update takes effect.
 *
 * @note Static buffers can be updated partially or completely.
 * @note The uniform block must use `std140` layout and respect 16-byte alignment and padding rules.
 *
 * @param shader Pointer to the NX_Shader2D.
 * @param offset Offset in bytes into the StaticBuffer to update.
 * @param size Size in bytes of the data to upload.
 * @param data Pointer to the data to upload.
 */
NXAPI void NX_UpdateStaticShader2DBuffer(NX_Shader2D* shader, size_t offset, size_t size, const void* data);

/**
 * @brief Updates the dynamic uniform buffer of a 2D shader for the next draw call.
 *
 * Dynamic buffers are defined in the shader as an uniform block named `DynamicBuffer`.
 * They are cleared at the end of each frame and can be set independently for each draw call.
 * This allows you to have different dynamic data per draw call.
 *
 * @note Dynamic buffers must be fully uploaded in a single call.
 * @note The uniform block must use `std140` layout and respect 16-byte alignment and padding rules.
 *
 * @param shader Pointer to the NX_Shader2D.
 * @param size Size in bytes of the data to upload.
 * @param data Pointer to the data to upload.
 */
NXAPI void NX_UpdateDynamicShader2DBuffer(NX_Shader2D* shader, size_t size, const void* data);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_SHADER_2D_H
