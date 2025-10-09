/* NX_Render.h -- API declaration for Nexium's render module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_RENDER_H
#define NX_RENDER_H

#include "./NX_Platform.h"
#include "./NX_Image.h"
#include "./NX_Math.h"
#include <stdint.h>

/* === Enums === */

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
 * @brief Bitfield type representing types of instance data stored in an instance buffer.
 * 
 * These flags can be combined using bitwise OR to specify multiple types at once.
 */
typedef uint8_t NX_InstanceData;

#define NX_INSTANCE_POSITION   (1 << 0)    ///< Instance data contains positions (NX_Vec3).
#define NX_INSTANCE_ROTATION   (1 << 1)    ///< Instance data contains rotations (NX_Quat).
#define NX_INSTANCE_SCALE      (1 << 2)    ///< Instance data contains scales (NX_Vec3).
#define NX_INSTANCE_COLOR      (1 << 3)    ///< Instance data contains colors (NX_Color).
#define NX_INSTANCE_CUSTOM     (1 << 4)    ///< Instance data contains custom data (NX_Vec4).

/**
 * @brief Extra flags for NX_Environment specifying rendering behaviors.
 *
 * These flags control optional rendering features that Nexium can enable per-environment.
 */
typedef uint32_t NX_EnvironmentFlag;

#define NX_ENV_SORT_OPAQUE              (1 << 0)    ///< Sort opaque objects front-to-back
#define NX_ENV_SORT_TRANSPARENT         (1 << 1)    ///< Sort transparent objects back-to-front
#define NX_ENV_VIEW_FRUSTUM_CULLING     (1 << 2)    ///< Enable view frustum culling (camera)
#define NX_ENV_SHADOW_FRUSTUM_CULLING   (1 << 3)    ///< Enable shadow frustum culling

/**
 * @brief Defines the type of projection used by a camera.
 */
typedef enum NX_Projection {
    NX_PROJECTION_PERSPECTIVE,      ///< Standard perspective projection. Objects appear smaller when farther.
    NX_PROJECTION_ORTHOGRAPHIC      ///< Orthographic projection. Objects keep the same size regardless of distance.
} NX_Projection;

/**
 * @brief Defines the texture filtering method.
 *
 * Determines how textures are sampled when scaled.
 */
typedef enum NX_TextureFilter {
    NX_TEXTURE_FILTER_POINT,        ///< Nearest-neighbor filtering. Fastest, pixelated look.
    NX_TEXTURE_FILTER_BILINEAR,     ///< Linear interpolation between 4 texels. Smooth but slightly blurry.
    NX_TEXTURE_FILTER_TRILINEAR     ///< Linear interpolation with mipmaps. Smooth and reduces aliasing at distance.
} NX_TextureFilter;

/**
 * @brief Defines the texture wrapping mode.
 *
 * Determines behavior when texture coordinates exceed [0, 1].
 */
typedef enum NX_TextureWrap {
    NX_TEXTURE_WRAP_CLAMP,          ///< Coordinates outside [0,1] are clamped to the edge pixel.
    NX_TEXTURE_WRAP_REPEAT,         ///< Texture repeats/tiled across the surface.
    NX_TEXTURE_WRAP_MIRROR          ///< Texture repeats but mirrors on each tile.
} NX_TextureWrap;

/**
 * @brief Defines the type of font used for rendering.
 */
typedef enum NX_FontType {
    NX_FONT_NORMAL,             ///< Standard vector font, anti-aliased, general-purpose text.
    NX_FONT_LIGHT,              ///< Light/thin vector font, finer strokes, good for small UI text.
    NX_FONT_MONO,               ///< Monochrome bitmap font, pixel-perfect, very fast to load.
    NX_FONT_SDF                 ///< Signed Distance Field font, scalable, smooth rendering at arbitrary sizes.
} NX_FontType;

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
    NX_SHADING_WIREFRAME        ///< Renders only mesh edges in wireframe mode.
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
 * @brief Animation Update modes.
 *
 * Controls wether to allow external animation matrices
 */
typedef enum NX_AnimMode {
    NX_ANIM_INTERNAL,           ///< Default animation solution
    NX_ANIM_CUSTOM,             ///< User supplied matrices
} NX_AnimMode;

/**
 * @brief Types of lights supported by the rendering engine.
 *
 * Each light type has different behaviors and use cases.
 */
typedef enum NX_LightType {
    NX_LIGHT_DIR,               ///< Directional light, affects the entire scene with parallel rays.
    NX_LIGHT_SPOT,              ///< Spot light, emits light in a cone shape.
    NX_LIGHT_OMNI               ///< Omni light, emits light in all directions from a single point.
} NX_LightType;

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

/* === Handlers === */

/**
 * @brief Opaque handle to a render texture.
 *
 * Represents a render texture containing a depth color target.
 */
typedef struct NX_RenderTexture NX_RenderTexture;

/**
 * @brief Opaque handle to a GPU vertex buffer.
 *
 * Represents a collection of vertices stored on the GPU.
 */
typedef struct NX_VertexBuffer NX_VertexBuffer;

/**
 * @brief Opaque handle to a GPU instance buffer.
 *
 * Represents per-instance data stored on the GPU, used for instanced rendering.
 */
typedef struct NX_InstanceBuffer NX_InstanceBuffer;

/**
 * @brief Opaque handle to a GPU texture.
 *
 * Represents a 2D image stored on the GPU.
 * Can be used for material maps or UI elements.
 */
typedef struct NX_Texture NX_Texture;

/**
 * @brief Opaque handle to a cubemap texture.
 *
 * Cubemaps are used for skyboxes or for generating reflection probes.
 * Stores 6 textures corresponding to the faces of a cube.
 */
typedef struct NX_Cubemap NX_Cubemap;

/**
 * @brief Opaque handle to a reflection probe.
 *
 * Represents precomputed environment reflections.
 * Can be used to add realistic reflections on materials.
 */
typedef struct NX_ReflectionProbe NX_ReflectionProbe;

/**
 * @brief Opaque handle to a light source.
 *
 * Represents a light in the scene.
 * Can be used for directional, spot or omni-directional lights.
 */
typedef struct NX_Light NX_Light;

/**
 * @brief Opaque handle to a font stored on the GPU.
 *
 * Represents a loaded font for text rendering.
 * Supports bitmap or SDF rendering modes depending on NX_FontType.
 */
typedef struct NX_Font NX_Font;

/**
 * @brief Opaque handle to a shader.
 *
 * Represents a customizable shader for 2D rendering.
 * Provides overrideable vertex/fragment entry points.
 */
typedef struct NX_Shader NX_Shader;

/**
 * @brief Opaque handle to a material shader.
 *
 * Represents a customizable shader used by a material.
 * Provides overrideable vertex/fragment entry points.
 */
typedef struct NX_MaterialShader NX_MaterialShader;

/* === Structures === */

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

/**
 * @brief Describes parameters for procedural skybox generation.
 *
 * This structure defines the appearance of a procedural skybox,
 * including sun orientation, sky gradients, ground color, and
 * atmospheric effects such as haze.
 */
typedef struct NX_Skybox {
    NX_Vec3 sunDirection;     ///< Direction of the sun (world space).
    NX_Color skyColorTop;     ///< Sky color at the zenith (top).
    NX_Color skyColorHorizon; ///< Sky color at the horizon.
    NX_Color sunColor;        ///< Color of the sun disk and light.
    NX_Color groundColor;     ///< Ground or floor color.
    float sunSize;            ///< Apparent angular size of the sun (in radians).
    float haze;               ///< Strength of atmospheric haze/scattering (0 = none).
    float energy;             ///< Intensity/brightness multiplier for the sky lighting.
} NX_Skybox;

/**
 * @brief Represents a 3D scene environment.
 *
 * Stores scene bounds, background/ambient colors, sky settings,
 * global adjustments, and post-processing parameters.
 */
typedef struct NX_Environment {

    NX_BoundingBox bounds;          ///< Scene bounds, used for directional light shadows and spatial calculations.
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
        float density;              ///< fog density
        float start;                ///< fog start distance (linear only)
        float end;                  ///< fog end distance (linear only)
        float skyAffect;            ///< influence of sky color on the fog
        NX_Color color;             ///< fog color
        NX_Fog mode;                ///< fog mode
    } fog;

    struct {
        float intensity;            ///< Overall strength of the SSAO effect (scales the occlusion).
        float radius;               ///< Sampling radius in view-space units; larger values capture broader occlusion.
        float power;                ///< Exponent applied to the SSAO term; higher values darken occlusion and sharpen falloff.
        float bias;                 ///< Small depth offset to reduce self-occlusion artifacts on flat surfaces.
        bool enabled;               ///< Enables or disables the SSAO pass.
    } ssao;

    struct {
        float threshold;            ///< HDR threshold used for bloom extraction.
        float softThreshold;        ///< Softening factor applied during prefiltering.
        float filterRadius;         ///< Radius of the blur filter used for bloom spreading.
        float strength;             ///< Intensity of the bloom effect when blended with the scene.
        NX_Bloom mode;              ///< Mode used to combine the bloom effect with the scene.
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

/**
 * @brief Represents a 3D mesh.
 *
 * Stores vertex and index data, shadow casting settings, bounding box, and layer information.
 * Can represent a static or skinned mesh.
 */
typedef struct NX_Mesh {

    NX_VertexBuffer* buffer;            ///< GPU vertex buffer for rendering.
    NX_Vertex3D* vertices;              ///< Pointer to vertex data in CPU memory.
    uint32_t* indices;                  ///< Pointer to index data in CPU memory.

    int vertexCount;                    ///< Number of vertices.
    int indexCount;                     ///< Number of indices.

    NX_ShadowCastMode shadowCastMode;   ///< Shadow casting mode for the mesh.
    NX_ShadowFaceMode shadowFaceMode;   ///< Which faces are rendered into the shadow map.
    NX_BoundingBox aabb;                ///< Axis-Aligned Bounding Box in local space.
    NX_Layer layerMask;                 ///< Bitfield indicating the rendering layer(s) of this mesh.

} NX_Mesh;

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
        bool prePass;               ///< Enable depth pre-pass to reduce overdraw or support alpha cutoff; may be costly with heavy vertex shaders. Default: false
    } depth;

    float alphaCutOff;              ///< Fragments with alpha below this value are discarded (only with depth pre-pass). Default: 1e-6f.
    NX_Vec2 texOffset;              ///< Texture coordinate offset. Default: vec2(0,0)
    NX_Vec2 texScale;               ///< Texture coordinate scaling. Default: vec2(1,1)

    NX_BillboardMode billboard;     ///< Billboard mode applied to the object
    NX_ShadingMode shading;         ///< Describes the shading mode, lit or not
    NX_BlendMode blend;             ///< Blending mode for rendering. Default: Opaque
    NX_CullMode cull;               ///< Face culling mode. Default: Back face

    NX_MaterialShader* shader;      ///< Pointer to an optional material shader. Default: NULL

} NX_Material;

/**
 * @brief Stores bone information for skeletal animation.
 *
 * Contains the bone name and the index of its parent bone.
 */
typedef struct NX_BoneInfo {
    char name[32];   ///< Bone name (max 31 characters + null terminator).
    int parent;      ///< Index of the parent bone (-1 if root).
} NX_BoneInfo;

/**
 * @brief Represents a skeletal animation for a model.
 *
 * This structure holds the animation data for a skinned model,
 * including per-frame bone transformation poses.
 */
typedef struct NX_ModelAnimation {

    int boneCount;                  ///< Number of bones in the skeleton affected by this animation.
    int frameCount;                 ///< Total number of frames in the animation sequence.

    NX_BoneInfo* bones;             ///< Array of bone metadata (name, parent index, etc.) defining the skeleton hierarchy.

    NX_Mat4** frameGlobalPoses;     ///< 2D array [frame][bone]. Global bone matrices (relative to model space).
    NX_Transform** frameLocalPoses; ///< 2D array [frame][bone]. Local bone transforms (TRS relative to parent).

    char name[32];                  ///< Name identifier for the animation (e.g., "Walk", "Jump").

} NX_ModelAnimation;

/**
 * @brief Represents a complete 3D model with meshes and materials.
 *
 * Contains multiple meshes and their associated materials, along with animation or bounding information.
 */
typedef struct NX_Model {

    NX_Mesh** meshes;                   ///< Array of meshes composing the model.
    NX_Material* materials;             ///< Array of materials used by the model.
    int* meshMaterials;                 ///< Array of material indices, one per mesh.

    int meshCount;                      ///< Number of meshes.
    int materialCount;                  ///< Number of materials.

    NX_BoundingBox aabb;                ///< Axis-Aligned Bounding Box encompassing the whole model.

    NX_Mat4* boneOverride;              ///< Array of matrices we'll use if we have it instead of internal calculations, Used in skinning.
    NX_Mat4* boneBindPose;              ///< Array of matrices representing the bind pose of the model, this is the pose used by default for non-animated skinned models.
    NX_Mat4* boneOffsets;               ///< Array of offset (inverse bind) matrices, one per bone. Transforms mesh-space vertices to bone space. Used in skinning.

    NX_BoneInfo* bones;                 ///< Bones information (skeleton). Defines the hierarchy and names of bones.
    int boneCount;                      ///< Number of bones.

    const NX_ModelAnimation* anim;      ///< Pointer to the currently assigned animation for this model (optional).
    NX_AnimMode animMode;               ///< Animation mode for the model; specifies whether to use the model’s animation and frame or the boneOverride.
    float animFrame;                    ///< Current animation frame index. Used for sampling bone poses from the animation.

} NX_Model;

/* === API Functions === */

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @defgroup Texture Texture Functions
 * @{
 */

/**
 * @brief Creates a GPU texture from an image.
 * @param image Pointer to the source image.
 * @return Pointer to a newly created NX_Texture, or NULL on failure.
 */
NXAPI NX_Texture* NX_CreateTexture(const NX_Image* image);

/**
 * @brief Load a texture from a file and decode it for rendering.
 *
 * Automatically converts pixel formats if needed (e.g., L/LA -> RGB/RGBA)
 *
 * @param filePath Path to the texture file
 * @return Pointer to a newly loaded NX_Texture ready for rendering, or NULL on failure
 */
NXAPI NX_Texture* NX_LoadTexture(const char* filePath);

/**
 * @brief Load raw texture data from a file without decoding or conversion.
 *
 * Preserves the original number of channels and pixel layout as stored in the file.
 * Useful if you want to handle texture decoding manually or keep raw data.
 *
 * @param filePath Path to the texture file
 * @return Pointer to a newly loaded NX_Texture containing raw pixel data, or NULL on failure
 */
NXAPI NX_Texture* NX_LoadTextureAsData(const char* filePath);

/**
 * @brief Destroys a GPU texture and frees its resources.
 * @param texture Pointer to the NX_Texture to destroy.
 */
NXAPI void NX_DestroyTexture(NX_Texture* texture);

/**
 * @brief Sets the default texture filter for newly created textures.
 * @param filter Default texture filtering mode.
 * @note The default filter is NX_TEXTURE_FILTER_BILINEAR.
 * @note If NX_TEXTURE_FILTER_TRILINEAR is set, mipmaps will be generated automatically for all new textures.
 */
NXAPI void NX_SetDefaultTextureFilter(NX_TextureFilter filter);

/**
 * @brief Sets the default anisotropy level for newly created textures.
 * @param anisotropy Default anisotropy level (1.0f by default).
 * @note Anisotropy may have no effect on GLES 3.2 depending on platform support.
 * @note The value is automatically clamped to the maximum supported by the platform.
 */
NXAPI void NX_SetDefaultTextureAnisotropy(float anisotropy);

/**
 * @brief Sets filtering, wrapping, and anisotropy parameters for a texture.
 * @param texture Pointer to the NX_Texture.
 * @param filter Texture filtering mode.
 * @param wrap Texture wrapping mode.
 * @param anisotropy Anisotropy level.
 * @note Anisotropy may have no effect on GLES 3.2 depending on platform support.
 * @note The value is automatically clamped to the maximum supported by the platform.
 */
NXAPI void NX_SetTextureParameters(NX_Texture* texture, NX_TextureFilter filter, NX_TextureWrap wrap, float anisotropy);

/**
 * @brief Sets the texture filtering mode.
 * @param texture Pointer to the NX_Texture.
 * @param filter Texture filtering mode.
 */
NXAPI void NX_SetTextureFilter(NX_Texture* texture, NX_TextureFilter filter);

/**
 * @brief Sets the anisotropy level for a texture.
 * @param texture Pointer to the NX_Texture.
 * @param anisotropy Anisotropy level.
 * @note Anisotropy may have no effect on GLES 3.2 depending on platform support.
 * @note The value is automatically clamped to the maximum supported by the platform.
 */
NXAPI void NX_SetTextureAnisotropy(NX_Texture* texture, float anisotropy);

/**
 * @brief Sets the texture wrapping mode.
 * @param texture Pointer to the NX_Texture.
 * @param wrap Texture wrapping mode.
 */
NXAPI void NX_SetTextureWrap(NX_Texture* texture, NX_TextureWrap wrap);

/**
 * @brief Generates mipmaps for a texture.
 * @param texture Pointer to the NX_Texture.
 */
NXAPI void NX_GenerateMipmap(NX_Texture* texture);

/**
 * @brief Queries the dimensions of a texture.
 * @param texture Pointer to the NX_Texture.
 * @param w Pointer to store the width.
 * @param h Pointer to store the height.
 */
NXAPI void NX_QueryTexture(NX_Texture* texture, int* w, int* h);

/** @} */ // end of Texture

/**
 * @defgroup Font Font Functions
 * @{
 */

/**
 * @brief Loads a font from a file.
 * @param filePath Path to the font file.
 * @param type Font type (bitmap or SDF).
 * @param baseSize Base size of the font in pixels.
 * @param codepoints Array of Unicode codepoints to load (can be NULL to load default set).
 * @param codepointCount Number of codepoints in the array.
 * @return Pointer to a newly loaded NX_Font.
 */
NXAPI NX_Font* NX_LoadFont(const char* filePath, NX_FontType type, int baseSize, int* codepoints, int codepointCount);

/**
 * @brief Loads a font from memory.
 * @param fileData Pointer to font file data in memory.
 * @param dataSize Size of the font data in bytes.
 * @param type Font type (bitmap or SDF).
 * @param baseSize Base size of the font in pixels.
 * @param codepoints Array of Unicode codepoints to load (can be NULL to load default set).
 * @param codepointCount Number of codepoints in the array.
 * @return Pointer to a newly loaded NX_Font.
 */
NXAPI NX_Font* NX_LoadFontFromMem(const void* fileData, size_t dataSize, NX_FontType type, int baseSize, int* codepoints, int codepointCount);

/**
 * @brief Destroys a font and frees its resources.
 * @param font Pointer to the NX_Font to destroy.
 */
NXAPI void NX_DestroyFont(NX_Font* font);

/**
 * @brief Measures the size of an array of codepoints in the given font.
 * @param font Pointer to the NX_Font to use (can be NULL to use the default font).
 * @param codepoints Array of Unicode codepoints to measure.
 * @param length Number of codepoints in the array.
 * @param fontSize Font size in pixels.
 * @param spacing Additional spacing between characters.
 * @return Width and height required to render the codepoints.
 */
NXAPI NX_Vec2 NX_MeasureCodepoints(const NX_Font* font, const int* codepoints, int length, float fontSize, NX_Vec2 spacing);

/**
 * @brief Measures the size of a text string in the given font.
 * @param font Pointer to the NX_Font to use (can be NULL to use the default font).
 * @param text Null-terminated string to measure.
 * @param fontSize Font size in pixels.
 * @param spacing Additional spacing between characters.
 * @return Width and height required to render the text.
 */
NXAPI NX_Vec2 NX_MeasureText(const NX_Font* font, const char* text, float fontSize, NX_Vec2 spacing);

/** @} */ // end of Font

/**
 * @defgroup RenderTexture Render Texture Functions
 * @{
 */

/**
 * @brief Creates an off-screen render texture.
 * @param w Width of the render texture in pixels.
 * @param h Height of the render texture in pixels.
 * @return Pointer to the newly created render texture, or NULL on failure.
 */
NXAPI NX_RenderTexture* NX_CreateRenderTexture(int w, int h);

/**
 * @brief Destroys a render texture.
 * @param target Pointer to the render texture to destroy.
 */
NXAPI void NX_DestroyRenderTexture(NX_RenderTexture* target);

/**
 * @brief Retrieves the color texture of a render texture.
 * @param target Pointer to the render texture.
 * @return Pointer to the color texture.
 */
NXAPI NX_Texture* NX_GetRenderTexture(NX_RenderTexture* target);

/**
 * @brief Blits a render texture to the screen.
 * @param target Pointer to the render texture to draw.
 * @param xDst X coordinate of the destination rectangle (in screen space).
 * @param yDst Y coordinate of the destination rectangle (in screen space).
 * @param wDst Width of the destination rectangle.
 * @param hDst Height of the destination rectangle.
 * @param linear If true, applies linear filtering when scaling; otherwise nearest-neighbor.
 */
NXAPI void NX_BlitRenderTexture(const NX_RenderTexture* target, int xDst, int yDst, int wDst, int hDst, bool linear);

/** @} */ // end of RenderTexture

/**
 * @defgroup Shader Shader Functions
 * @{
 */

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
 * @return Pointer to the created NX_Shader, or NULL on failure.
 */
NXAPI NX_Shader* NX_CreateShader(const char* vertCode, const char* fragCode);

/**
 * @brief Loads a custom 2D shader from GLSL source files.
 *
 * Same behavior as NX_CreateShader, but loads the shader code from files.
 *
 * @param vertFile Path to the vertex shader file (can be NULL if not used).
 * @param fragFile Path to the fragment shader file (can be NULL if not used).
 * @return Pointer to the created NX_Shader, or NULL on failure.
 */
NXAPI NX_Shader* NX_LoadShader(const char* vertFile, const char* fragFile);

/**
 * @brief Destroys a 2D shader and releases associated GPU resources.
 *
 * @param shader Pointer to the NX_Shader to destroy.
 */
NXAPI void NX_DestroyShader(NX_Shader* shader);

/**
 * @brief Assigns a texture to a 2D shader sampler.
 *
 * This function sets a texture for a specific sampler slot in a 2D shader.
 * The shader must declare the sampler with one of the predefined names:
 * "Texture0", "Texture1", "Texture2", or "Texture3", all of type `sampler2D`.
 *
 * If `texture` is `NULL`, a default white texture will be used instead.
 *
 * @param shader Pointer to the NX_Shader to modify.
 * @param slot Index of the sampler to assign (0 to 3). The slot must correspond
 *             to a sampler declared in the shader with the matching name.
 * @param texture Pointer to the NX_Texture to bind, or `NULL` to use a white texture.
 *
 * @note Up to 4 texture samplers are supported per shader. It is the user's
 *       responsibility to ensure the shader defines the corresponding sampler names.
 */
NXAPI void NX_SetShaderTexture(NX_Shader* shader, int slot, const NX_Texture* texture);

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
 * @param shader Pointer to the NX_Shader.
 * @param offset Offset in bytes into the StaticBuffer to update.
 * @param size Size in bytes of the data to upload.
 * @param data Pointer to the data to upload.
 */
NXAPI void NX_UpdateStaticShaderBuffer(NX_Shader* shader, size_t offset, size_t size, const void* data);

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
 * @param shader Pointer to the NX_Shader.
 * @param size Size in bytes of the data to upload.
 * @param data Pointer to the data to upload.
 */
NXAPI void NX_UpdateDynamicShaderBuffer(NX_Shader* shader, size_t size, const void* data);

/** @} */ // end of Shader

/**
 * @defgroup Draw2D 2D Drawing Functions
 * @{
 */

/**
 * @brief Begins 2D rendering.
 *
 * Sets up the rendering state for drawing 2D primitives.
 *
 * @param target Render texture to draw into (can be NULL to render to the screen).
 */
NXAPI void NX_Begin2D(NX_RenderTexture* target);

/**
 * @brief Ends 2D rendering.
 *
 * Flushes any pending 2D draw calls and restores previous rendering state.
 */
NXAPI void NX_End2D(void);

/**
 * @brief Sets the default color for 2D drawing.
 * @param color Color to use for subsequent 2D drawing operations.
 * @note The default color is NX_WHITE.
 */
NXAPI void NX_SetColor2D(NX_Color color);

/**
 * @brief Sets the default texture for 2D drawing.
 * @param texture Pointer to the NX_Texture to use.
 * @note The default texture (NULL) is a white texture.
 */
NXAPI void NX_SetTexture2D(const NX_Texture* texture);

/**
 * @brief Sets the default font for 2D drawing.
 * @param font Pointer to the NX_Font to use.
 * @note The default font (NULL) is Vera Sans rendered in SDF with a base size of 32.
 */
NXAPI void NX_SetFont2D(const NX_Font* font);

/**
 * @brief Sets the default shader for 2D drawing.
 * @param font Pointer to the NX_Shader to use.
 * @note The default shader will be used if NULL.
 */
NXAPI void NX_SetShader2D(NX_Shader* shader);

/**
 * @brief Draws a filled triangle in 2D.
 * @param p0 First vertex of the triangle.
 * @param p1 Second vertex of the triangle.
 * @param p2 Third vertex of the triangle.
 */
NXAPI void NX_DrawTriangle2D(NX_Vec2 p0, NX_Vec2 p1, NX_Vec2 p2);

/**
 * @brief Draws the border of a triangle in 2D.
 * @param p0 First vertex of the triangle.
 * @param p1 Second vertex of the triangle.
 * @param p2 Third vertex of the triangle.
 * @param thickness Border thickness in pixels.
 */
NXAPI void NX_DrawTriangleBorder2D(NX_Vec2 p0, NX_Vec2 p1, NX_Vec2 p2, float thickness);

/**
 * @brief Draws a list of 2D triangles.
 * @param triangles Array of NX_Vertex2D defining the triangles.
 * @param triangleCount Number of triangles in the array.
 */
NXAPI void NX_DrawTriangleList2D(const NX_Vertex2D* triangles, int triangleCount);

/**
 * @brief Draws a triangle strip in 2D.
 * @param vertices Array of NX_Vertex2D defining the strip.
 * @param count Number of vertices in the array.
 */
NXAPI void NX_DrawTriangleStrip2D(const NX_Vertex2D* vertices, int count);

/**
 * @brief Draws a triangle fan in 2D.
 * @param vertices Array of NX_Vertex2D defining the fan.
 * @param count Number of vertices in the array.
 */
NXAPI void NX_DrawTriangleFan2D(const NX_Vertex2D* vertices, int count);

/**
 * @brief Draws a filled quadrilateral in 2D.
 * @param p0 First vertex of the quad.
 * @param p1 Second vertex of the quad.
 * @param p2 Third vertex of the quad.
 * @param p3 Fourth vertex of the quad.
 */
NXAPI void NX_DrawQuad2D(NX_Vec2 p0, NX_Vec2 p1, NX_Vec2 p2, NX_Vec2 p3);

/**
 * @brief Draws the border of a quadrilateral in 2D.
 * @param p0 First vertex of the quad.
 * @param p1 Second vertex of the quad.
 * @param p2 Third vertex of the quad.
 * @param p3 Fourth vertex of the quad.
 * @param thickness Border thickness in pixels.
 */
NXAPI void NX_DrawQuadBorder2D(NX_Vec2 p0, NX_Vec2 p1, NX_Vec2 p2, NX_Vec2 p3, float thickness);

/**
 * @brief Draws a list of 2D quads.
 * @param quads Array of NX_Vertex2D defining the quads.
 * @param quadCount Number of quads in the array.
 */
NXAPI void NX_DrawQuadList2D(const NX_Vertex2D* quads, int quadCount);

/**
 * @brief Draws a quad strip in 2D.
 * @param vertices Array of NX_Vertex2D defining the strip.
 * @param count Number of vertices in the array.
 */
NXAPI void NX_DrawQuadStrip2D(const NX_Vertex2D* vertices, int count);

/**
 * @brief Draws a quad fan in 2D.
 * @param vertices Array of NX_Vertex2D defining the fan.
 * @param count Number of vertices in the array.
 */
NXAPI void NX_DrawQuadFan2D(const NX_Vertex2D* vertices, int count);

/**
 * @brief Draws a line segment in 2D.
 * @param p0 Start point of the line.
 * @param p1 End point of the line.
 * @param thickness Line thickness in pixels.
 */
NXAPI void NX_DrawLine2D(NX_Vec2 p0, NX_Vec2 p1, float thickness);

/**
 * @brief Draws a list of 2D line segments.
 * @param lines Array of points defining the line segments (2 points per line).
 * @param lineCount Number of line segments.
 * @param thickness Line thickness in pixels.
 */
NXAPI void NX_DrawLineList2D(const NX_Vec2* lines, int lineCount, float thickness);

/**
 * @brief Draws a connected line strip in 2D.
 * @param points Array of points defining the strip.
 * @param count Number of points in the strip.
 * @param thickness Line thickness in pixels.
 */
NXAPI void NX_DrawLineStrip2D(const NX_Vec2* points, int count, float thickness);

/**
 * @brief Draws a closed line loop in 2D.
 * @param points Array of points defining the loop.
 * @param count Number of points in the loop.
 * @param thickness Line thickness in pixels.
 */
NXAPI void NX_DrawLineLoop2D(const NX_Vec2* points, int count, float thickness);

/**
 * @brief Draws a filled rectangle in 2D.
 * @param x X-coordinate of the rectangle's top-left corner.
 * @param y Y-coordinate of the rectangle's top-left corner.
 * @param w Width of the rectangle.
 * @param h Height of the rectangle.
 */
NXAPI void NX_DrawRect2D(float x, float y, float w, float h);

/**
 * @brief Draws the border of a rectangle in 2D.
 * @param x X-coordinate of the rectangle's top-left corner.
 * @param y Y-coordinate of the rectangle's top-left corner.
 * @param w Width of the rectangle.
 * @param h Height of the rectangle.
 * @param thickness Border thickness in pixels.
 */
NXAPI void NX_DrawRectBorder2D(float x, float y, float w, float h, float thickness);

/**
 * @brief Draws a rectangle in 2D with rotation around a pivot point.
 * @param center Center position of the rectangle.
 * @param size Width and height of the rectangle.
 * @param pivot Normalized pivot point [0..1] from top-left corner.
 * @param rotation Rotation in radians.
 * @note The pivot is the point around which rotation occurs.
 */
NXAPI void NX_DrawRectEx2D(NX_Vec2 center, NX_Vec2 size, NX_Vec2 pivot, float rotation);

/**
 * @brief Draws the border of a rectangle in 2D with rotation around a pivot point.
 * @param center Center position of the rectangle.
 * @param size Width and height of the rectangle.
 * @param pivot Normalized pivot point [0..1] from top-left corner.
 * @param rotation Rotation in radians.
 * @param thickness Border thickness in pixels.
 * @note The pivot is the point around which rotation occurs.
 */
NXAPI void NX_DrawRectBorderEx2D(NX_Vec2 center, NX_Vec2 size, NX_Vec2 pivot, float rotation, float thickness);

/**
 * @brief Draws a rectangle with rounded corners in 2D.
 * @param x X-coordinate of the top-left corner.
 * @param y Y-coordinate of the top-left corner.
 * @param w Width of the rectangle.
 * @param h Height of the rectangle.
 * @param radius Corner radius.
 * @param segments Number of segments to approximate the corners.
 */
NXAPI void NX_DrawRectRounded2D(float x, float y, float w, float h, float radius, int segments);

/**
 * @brief Draws the border of a rectangle with rounded corners in 2D.
 * @param x X-coordinate of the top-left corner.
 * @param y Y-coordinate of the top-left corner.
 * @param w Width of the rectangle.
 * @param h Height of the rectangle.
 * @param radius Corner radius.
 * @param segments Number of segments to approximate the corners.
 * @param thickness Border thickness in pixels.
 */
NXAPI void NX_DrawRectRoundedBorder2D(float x, float y, float w, float h, float radius, int segments, float thickness);

/**
 * @brief Draws a rectangle with rounded corners and rotation around a pivot.
 * @param center Center position of the rectangle.
 * @param size Width and height of the rectangle.
 * @param pivot Normalized pivot point [0..1] from top-left corner.
 * @param rotation Rotation in radians.
 * @param radius Corner radius.
 * @note The pivot is the point around which rotation occurs.
 */
NXAPI void NX_DrawRectRoundedEx2D(NX_Vec2 center, NX_Vec2 size, NX_Vec2 pivot, float rotation, float radius);

/**
 * @brief Draws the border of a rectangle with rounded corners and rotation around a pivot.
 * @param center Center position of the rectangle.
 * @param size Width and height of the rectangle.
 * @param pivot Normalized pivot point [0..1] from top-left corner.
 * @param rotation Rotation in radians.
 * @param radius Corner radius.
 * @param thickness Border thickness in pixels.
 * @note The pivot is the point around which rotation occurs.
 */
NXAPI void NX_DrawRectRoundedBorderEx2D(NX_Vec2 center, NX_Vec2 size, NX_Vec2 pivot, float rotation, float radius, float thickness);

/**
 * @brief Draws a filled circle in 2D.
 * @param center Center position of the circle.
 * @param radius Circle radius.
 * @param segments Number of segments to approximate the circle.
 */
NXAPI void NX_DrawCircle2D(NX_Vec2 center, float radius, int segments);

/**
 * @brief Draws the border of a circle in 2D.
 * @param p Center position of the circle.
 * @param radius Circle radius.
 * @param segments Number of segments to approximate the circle.
 * @param thickness Border thickness in pixels.
 */
NXAPI void NX_DrawCircleBorder2D(NX_Vec2 p, float radius, int segments, float thickness);

/**
 * @brief Draws a filled ellipse in 2D.
 * @param center Center position of the ellipse.
 * @param radius X and Y radii of the ellipse.
 * @param segments Number of segments to approximate the ellipse.
 */
NXAPI void NX_DrawEllipse2D(NX_Vec2 center, NX_Vec2 radius, int segments);

/**
 * @brief Draws the border of an ellipse in 2D.
 * @param p Center position of the ellipse.
 * @param r X and Y radii of the ellipse.
 * @param segments Number of segments to approximate the ellipse.
 * @param thickness Border thickness in pixels.
 */
NXAPI void NX_DrawEllipseBorder2D(NX_Vec2 p, NX_Vec2 r, int segments, float thickness);

/**
 * @brief Draws a filled pie slice (sector) in 2D.
 * @param center Center of the pie slice.
 * @param radius Radius of the pie slice.
 * @param startAngle Starting angle in radians.
 * @param endAngle Ending angle in radians.
 * @param segments Number of segments to approximate the curve.
 */
NXAPI void NX_DrawPieSlice2D(NX_Vec2 center, float radius, float startAngle, float endAngle, int segments);

/**
 * @brief Draws the border of a pie slice (sector) in 2D.
 * @param center Center of the pie slice.
 * @param radius Radius of the pie slice.
 * @param startAngle Starting angle in radians.
 * @param endAngle Ending angle in radians.
 * @param segments Number of segments to approximate the curve.
 * @param thickness Border thickness in pixels.
 */
NXAPI void NX_DrawPieSliceBorder2D(NX_Vec2 center, float radius, float startAngle, float endAngle, int segments, float thickness);

/**
 * @brief Draws a filled ring in 2D.
 * @param center Center of the ring.
 * @param innerRadius Inner radius.
 * @param outerRadius Outer radius.
 * @param segments Number of segments to approximate the ring.
 */
NXAPI void NX_DrawRing2D(NX_Vec2 center, float innerRadius, float outerRadius, int segments);

/**
 * @brief Draws the border of a ring in 2D.
 * @param center Center of the ring.
 * @param innerRadius Inner radius.
 * @param outerRadius Outer radius.
 * @param segments Number of segments to approximate the ring.
 * @param thickness Border thickness in pixels.
 */
NXAPI void NX_DrawRingBorder2D(NX_Vec2 center, float innerRadius, float outerRadius, int segments, float thickness);

/**
 * @brief Draws a filled ring arc in 2D.
 * @param center Center of the arc.
 * @param innerRadius Inner radius.
 * @param outerRadius Outer radius.
 * @param startAngle Starting angle in radians.
 * @param endAngle Ending angle in radians.
 * @param segments Number of segments to approximate the arc.
 */
NXAPI void NX_DrawRingArc2D(NX_Vec2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments);

/**
 * @brief Draws the border of a ring arc in 2D.
 * @param center Center of the arc.
 * @param innerRadius Inner radius.
 * @param outerRadius Outer radius.
 * @param startAngle Starting angle in radians.
 * @param endAngle Ending angle in radians.
 * @param segments Number of segments to approximate the arc.
 * @param thickness Border thickness in pixels.
 */
NXAPI void NX_DrawRingArcBorder2D(NX_Vec2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments, float thickness);

/**
 * @brief Draws an arc in 2D.
 * @param center Center of the arc.
 * @param radius Radius of the arc.
 * @param startAngle Starting angle in radians.
 * @param endAngle Ending angle in radians.
 * @param segments Number of segments to approximate the curve.
 * @param thickness Arc thickness in pixels.
 */
NXAPI void NX_DrawArc2D(NX_Vec2 center, float radius, float startAngle, float endAngle, int segments, float thickness);

/**
 * @brief Draws a quadratic Bezier curve in 2D.
 * @param p0 Start point.
 * @param p1 Control point.
 * @param p2 End point.
 * @param segments Number of segments to approximate the curve.
 * @param thickness Line thickness in pixels.
 */
NXAPI void NX_DrawBezierQuad2D(NX_Vec2 p0, NX_Vec2 p1, NX_Vec2 p2, int segments, float thickness);

/**
 * @brief Draws a cubic Bezier curve in 2D.
 * @param p0 Start point.
 * @param p1 First control point.
 * @param p2 Second control point.
 * @param p3 End point.
 * @param segments Number of segments to approximate the curve.
 * @param thickness Line thickness in pixels.
 */
NXAPI void NX_DrawBezierCubic2D(NX_Vec2 p0, NX_Vec2 p1, NX_Vec2 p2, NX_Vec2 p3, int segments, float thickness);

/**
 * @brief Draws a spline curve through a set of points in 2D.
 * @param points Array of points defining the spline.
 * @param count Number of points.
 * @param segments Number of segments between each pair of points.
 * @param thickness Line thickness in pixels.
 */
NXAPI void NX_DrawSpline2D(const NX_Vec2* points, int count, int segments, float thickness);

/**
 * @brief Draws a single Unicode codepoint in 2D.
 * @param codepoint Unicode codepoint to draw.
 * @param position Position in 2D space.
 * @param fontSize Font size in pixels.
 */
NXAPI void NX_DrawCodepoint2D(int codepoint, NX_Vec2 position, float fontSize);

/**
 * @brief Draws an array of Unicode codepoints in 2D.
 * @param codepoints Array of Unicode codepoints.
 * @param length Number of codepoints in the array.
 * @param position Starting position in 2D space.
 * @param fontSize Font size in pixels.
 * @param spacing Additional spacing between characters.
 */
NXAPI void NX_DrawCodepoints2D(const int* codepoints, int length, NX_Vec2 position, float fontSize, NX_Vec2 spacing);

/**
 * @brief Draws a null-terminated text string in 2D.
 * @param text String to draw.
 * @param position Starting position in 2D space.
 * @param fontSize Font size in pixels.
 * @param spacing Additional spacing between characters.
 */
NXAPI void NX_DrawText2D(const char* text, NX_Vec2 position, float fontSize, NX_Vec2 spacing);

/** @} */ // end of Draw2D

/**
 * @defgroup Draw3D 3D Drawing Functions
 * @{
 */

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

/** @} */ // end of Draw3D

/**
 * @defgroup Camera Camera Functions
 * @{
 */

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

/** @} */ // end of Camera

/**
 * @defgroup Environment Environment Functions
 * @{
 */

/**
 * @brief Returns the default 3D environment.
 * @return NX_Environment initialized with default values.
 *
 * Default environment parameters:
 * - bounds: min=(-10,-10,-10), max=(+10,+10,+10)
 * - background color: NX_GRAY
 * - ambient light: NX_DARK_GRAY
 * - sky:
 *   - cubemap: nullptr
 *   - reflection probe: nullptr
 *   - rotation: identity quaternion
 *   - intensity: 1.0
 *   - specular contribution: 1.0
 *   - diffuse contribution: 1.0
 * - fog:
 *   - density: 0.01
 *   - start: 5.0
 *   - end: 50.0
 *   - skyAffect 0.5
 *   - color: NX_GRAY
 *   - mode: NX_FOG_DISABLED
 * - ssao:
 *   - intensity: 1.0
 *   - radius: 0.5
 *   - power: 1.0
 *   - bias: 0.025
 *   - enabled: false
 * - bloom:
 *   - threshold: 0.0
 *   - softThreshold: 0.5
 *   - filterRadius: 0.0
 *   - strength: 0.05
 *   - mode: disabled
 * - global adjustments:
 *   - brightness: 1.0
 *   - contrast: 1.0
 *   - saturation: 1.0
 * - tonemapping:
 *   - mode: NX_TONEMAP_LINEAR
 *   - exposure: 1.0
 *   - white point: 1.0
 */
NXAPI NX_Environment NX_GetDefaultEnvironment(void);

/** @} */ // end of Environment

/**
 * @defgroup Cubemap Cubemap Functions
 * @{
 */

/**
 * @brief Creates an empty cubemap.
 *
 * Allocates a cubemap texture ready to be filled,
 * either by procedural skybox or rendering a scene.
 *
 * @param size Edge size (in pixels) of each face.
 * @param format Pixel format for the cubemap faces.
 * @return Pointer to a newly created NX_Cubemap.
 * @note On OpenGL ES, requested 32-bit formats may be downgraded to 16-bit depending on hardware support.
 */
NXAPI NX_Cubemap* NX_CreateCubemap(int size, NX_PixelFormat format);

/**
 * @brief Load a cubemap from an image.
 * @param image Pointer to the source NX_Image.
 * @return Pointer to a newly created NX_Cubemap.
 * @note Cubemaps are used for skyboxes or to generate reflection probes.
 * @note Supported image layouts (auto-detected):
 *   - Equirectangular (panorama)
 *   - Horizontal line (faces packed in OpenGL order)
 *   - Vertical line (faces packed in OpenGL order)
 *   - 4x3 cross:
 *           [+Y]
 *       [-X][+Z][+X][-Z]
 *           [-Y]
 *   - 3x4 cross:
 *           [+Y]
 *       [-X][+Z][+X]
 *           [-Y]
 *           [-Z]
 */
NXAPI NX_Cubemap* NX_LoadCubemapFromMem(const NX_Image* image);

/**
 * @brief Loads a cubemap from a file.
 * @param filePath Path to the image file.
 * @return Pointer to a newly loaded NX_Cubemap.
 * @note Cubemaps are used for skyboxes or to generate reflection probes.
 * @note Supported image layouts (auto-detected, same as NX_CreateCubemap).
 */
NXAPI NX_Cubemap* NX_LoadCubemap(const char* filePath);

/**
 * @brief Destroys a cubemap and frees its resources.
 * @param cubemap Pointer to the NX_Cubemap to destroy.
 */
NXAPI void NX_DestroyCubemap(NX_Cubemap* cubemap);

/**
 * @brief Generates a procedural skybox into a cubemap.
 * @param cubemap Destination cubemap to render into.
 * @param skybox Pointer to a skybox description (NX_Skybox).
 */
NXAPI void NX_GenerateSkybox(NX_Cubemap* cubemap, const NX_Skybox* skybox);

/** @} */ // end of Cubemap

/**
 * @defgroup ReflectionProbe Reflection Probe Functions
 * @{
 */

/**
 * @brief Creates a reflection probe from a cubemap.
 * @param cubemap Pointer to the cubemap (cannot be NULL).
 * @return Pointer to a newly created NX_ReflectionProbe.
 * @note Reflection probes capture the environment for specular and diffuse image-based lighting.
 */
NXAPI NX_ReflectionProbe* NX_CreateReflectionProbe(NX_Cubemap* cubemap);

/**
 * @brief Loads a reflection probe from a cubemap file.
 * @param filePath Path to the cubemap image file.
 * @return Pointer to a newly loaded NX_ReflectionProbe.
 * @note The cubemap is used to generate specular and diffuse reflections.
 */
NXAPI NX_ReflectionProbe* NX_LoadReflectionProbe(const char* filePath);

/**
 * @brief Destroys a reflection probe and frees its resources.
 * @param probe Pointer to the NX_ReflectionProbe to destroy.
 */
NXAPI void NX_DestroyReflectionProbe(NX_ReflectionProbe* probe);

/**
 * @brief Updates an existing reflection probe from a new cubemap.
 * @param probe Pointer to the reflection probe to update.
 * @param cubemap Pointer to the new cubemap used for updating (cannot be NULL).
 */
NXAPI void NX_UpdateReflectionProbe(NX_ReflectionProbe* probe, const NX_Cubemap* cubemap);

/** @} */ // end of ReflectionProbe

/**
 * @defgroup Material Material Functions
 * @{
 */

/**
 * @brief Returns the default material.
 * @return NX_Material initialized with default values.
 *
 * Default material parameters:
 * - Albedo:
 *   - texture: nullptr
 *   - color: NX_WHITE
 * - Emission:
 *   - texture: nullptr
 *   - color: NX_WHITE
 *   - energy: 0.0
 * - ORM (Ambient Occlusion / Roughness / Metalness):
 *   - texture: nullptr
 *   - aoLightAffect: 0.0
 *   - occlusion: 1.0
 *   - roughness: 1.0
 *   - metalness: 0.0
 * - Normal map:
 *   - texture: nullptr
 *   - scale: 1.0
 * - Depth:
 *   - test: NX_DEPTH_TEST_LESS
 *   - prePass: false
 * - alphaCutOff: 1e-6 (disables discard by default)
 * - texOffset: (0, 0)
 * - texScale: (1, 1)
 * - billboard mode : NX_BILLBOARD_DISABLED
 * - shading mode : NX_SHADING_LIT
 * - blend mode: NX_BLEND_OPAQUE
 * - cull mode: NX_CULL_BACK
 * - shader: NULL
 */
NXAPI NX_Material NX_GetDefaultMaterial(void);

/**
 * @brief Destroys all resources allocated within a material (e.g., textures, shaders).
 * @param material Pointer to the NX_Material to clean up.
 * @note Only call this if you are certain the resources are no longer needed.
 * @note Do not call this if the resources are shared between multiple materials.
 */
NXAPI void NX_DestroyMaterialResources(NX_Material* material);

/** @} */ // end of Material

/**
 * @defgroup Material Material Functions
 * @{
 */

/**
 * @brief Creates a custom material shader from GLSL source code.
 *
 * Material shaders allow you to override the default rendering pipeline by providing
 * custom vertex and/or fragment stages. At least one stage must be provided.
 *
 * Vertex stage (`void vertex()`) is called after material parameters and model/normal
 * matrices are calculated but before the final vertex transformation. You can adjust
 * positions in local space, colors, normals, etc.
 *
 * Fragment stage (`void fragment()`) is called after default albedo, ORM, and normal
 * maps are computed, allowing you to override or tweak these values before lighting.
 *
 * You also have access to built-in global variables such as matrices, vertex attributes,
 * and TIME.
 *
 * See the shader documentation for more details. (comming soon).
 *
 * @param vertCode Source code of the vertex shader stage (can be NULL if not used).
 * @param fragCode Source code of the fragment shader stage (can be NULL if not used).
 * @return Pointer to the created NX_MaterialShader, or NULL on failure.
 */
NXAPI NX_MaterialShader* NX_CreateMaterialShader(const char* vertCode, const char* fragCode);

/**
 * @brief Loads a custom material shader from GLSL source files.
 *
 * Same behavior as NX_CreateMaterialShader, but loads the shader code from files.
 *
 * @param vertFile Path to the vertex shader file (can be NULL if not used).
 * @param fragFile Path to the fragment shader file (can be NULL if not used).
 * @return Pointer to the created NX_MaterialShader, or NULL on failure.
 */
NXAPI NX_MaterialShader* NX_LoadMaterialShader(const char* vertFile, const char* fragFile);

/**
 * @brief Destroys a material shader and releases associated GPU resources.
 * 
 * @param shader Pointer to the NX_MaterialShader to destroy.
 */
NXAPI void NX_DestroyMaterialShader(NX_MaterialShader* shader);

/**
 * @brief Assign a texture to a material shader sampler.
 *
 * This function sets a texture for a specific sampler slot in a material shader.
 * The shader must declare the sampler with one of the predefined names:
 * "Texture0", "Texture1", "Texture2", or "Texture3", all of type `sampler2D`.
 *
 * If `texture` is `NULL`, a default white texture will be used instead.
 *
 * @param shader Pointer to the NX_MaterialShader to modify.
 * @param slot Index of the sampler to assign (0 to 3). The slot must correspond
 *             to a sampler declared in the shader with the matching name.
 * @param texture Pointer to the NX_Texture to bind, or `NULL` to use a white texture.
 *
 * @note Up to 4 texture samplers are supported per shader. It is the user's
 *       responsibility to ensure the shader defines the corresponding sampler names.
 */
NXAPI void NX_SetMaterialShaderTexture(NX_MaterialShader* shader, int slot, const NX_Texture* texture);

/**
 * @brief Updates the static uniform buffer of a material shader.
 *
 * Static buffers are defined in the shader as an uniform block named `StaticBuffer`.
 * They are constant across all draw calls using this shader. If multiple updates are
 * made during a frame, only the last update takes effect.
 *
 * @note Static buffers can be updated partially or completely.
 * @note The uniform block must use `std140` layout and respect 16-byte alignment and padding rules.
 *
 * @param shader Pointer to the NX_MaterialShader.
 * @param offset Offset in bytes into the StaticBuffer to update.
 * @param size Size in bytes of the data to upload.
 * @param data Pointer to the data to upload.
 */
NXAPI void NX_UpdateStaticMaterialShaderBuffer(NX_MaterialShader* shader, size_t offset, size_t size, const void* data);

/**
 * @brief Updates the dynamic uniform buffer of a material shader for the next draw call.
 *
 * Dynamic buffers are defined in the shader as an uniform block named `DynamicBuffer`.
 * They are cleared at the end of each frame and can be set independently for each draw call.
 * This allows you to have different dynamic data per draw call.
 *
 * @note Dynamic buffers must be fully uploaded in a single call.
 * @note The uniform block must use `std140` layout and respect 16-byte alignment and padding rules.
 *
 * @param shader Pointer to the NX_MaterialShader.
 * @param size Size in bytes of the data to upload.
 * @param data Pointer to the data to upload.
 */
NXAPI void NX_UpdateDynamicMaterialShaderBuffer(NX_MaterialShader* shader, size_t size, const void* data);

/** @} */ // end of Material

/**
 * @defgroup Mesh Mesh Functions
 * @{
 */

/**
 * @brief Creates a 3D mesh by copying vertex and index data.
 * @param vertices Pointer to the vertex array (cannot be NULL).
 * @param vCount Number of vertices.
 * @param indices Pointer to the index array (can be NULL).
 * @param iCount Number of indices.
 * @return Pointer to a newly created NX_Mesh.
 * @note The function copies the data into internal buffers.
 */
NXAPI NX_Mesh* NX_CreateMesh(const NX_Vertex3D* vertices, int vCount, const uint32_t* indices, int iCount);

/**
 * @brief Destroys a 3D mesh and frees its resources.
 * @param mesh Pointer to the NX_Mesh to destroy.
 */
NXAPI void NX_DestroyMesh(NX_Mesh* mesh);

/**
 * @brief Generates a quad mesh.
 * @param size Width and height of the quad.
 * @param subDiv Subdivision along X and Y axes.
 * @param normal Normal vector for the quad surface.
 * @return Pointer to a newly generated NX_Mesh.
 */
NXAPI NX_Mesh* NX_GenMeshQuad(NX_Vec2 size, NX_Vec2 subDiv, NX_Vec3 normal);

/**
 * @brief Generates a cube mesh.
 * @param size Dimensions along X, Y, Z.
 * @param subDiv Subdivision along each axis.
 * @return Pointer to a newly generated NX_Mesh.
 */
NXAPI NX_Mesh* NX_GenMeshCube(NX_Vec3 size, NX_Vec3 subDiv);

/**
 * @brief Generates a sphere mesh.
 * @param radius Sphere radius.
 * @param slices Number of slices (longitudinal divisions).
 * @param rings Number of rings (latitudinal divisions).
 * @return Pointer to a newly generated NX_Mesh.
 */
NXAPI NX_Mesh* NX_GenMeshSphere(float radius, int slices, int rings);

/**
 * @brief Generates a cylinder mesh.
 * @param topRadius Radius of the top cap.
 * @param bottomRadius Radius of the bottom cap.
 * @param height Height of the cylinder.
 * @param slices Number of slices around the cylinder.
 * @param rings Number of subdivisions along the height.
 * @param topCap Whether to generate the top cap.
 * @param bottomCap Whether to generate the bottom cap.
 * @return Pointer to a newly generated NX_Mesh.
 */
NXAPI NX_Mesh* NX_GenMeshCylinder(float topRadius, float bottomRadius, float height, int slices, int rings, bool topCap, bool bottomCap);

/**
 * @brief Generates a capsule mesh.
 * @param radius Capsule radius.
 * @param height Capsule height.
 * @param slices Number of slices around the capsule.
 * @param rings Number of rings along the capsule.
 * @return Pointer to a newly generated NX_Mesh.
 */
NXAPI NX_Mesh* NX_GenMeshCapsule(float radius, float height, int slices, int rings);

/**
 * @brief Uploads the mesh data currently stored in RAM to the GPU.
 * @param mesh Pointer to the NX_Mesh to update.
 * @note Useful after modifying vertices or indices to update the GPU buffers.
 */
NXAPI void NX_UpdateMeshBuffer(NX_Mesh* mesh);

/**
 * @brief Recalculates the Axis-Aligned Bounding Box (AABB) of the mesh.
 * @param mesh Pointer to the NX_Mesh to update.
 * @note Should be called after modifying vertices or transformations.
 */
NXAPI void NX_UpdateMeshAABB(NX_Mesh* mesh);

/** @} */ // end of Mesh

/**
 * @defgroup InstanceBuffer Instance Buffer Functions
 * @{
 */

/**
 * @brief Create a new instance buffer with pre-allocated GPU memory.
 *
 * @param bitfield Specifies the types of instance data the buffer will store.
 *                 (e.g., matrices, colors, custom vec4s). Types are immutable.
 * @param count Initial number of instances to allocate.
 * @return NX_InstanceBuffer* Pointer to the created instance buffer.
 *
 * @note The buffer's supported types cannot change after creation.
 */
NXAPI NX_InstanceBuffer* NX_CreateInstanceBuffer(NX_InstanceData bitfield, size_t count);

/**
 * @brief Destroy an instance buffer and free all associated GPU memory.
 *
 * @param buffer The instance buffer to destroy.
 */
NXAPI void NX_DestroyInstanceBuffer(NX_InstanceBuffer* buffer);

/**
 * @brief Reallocate the instance buffer to hold a new number of instances.
 *
 * @param buffer   The instance buffer to resize.
 * @param count    New total number of instances to allocate.
 * @param keepData If true, existing data is preserved; otherwise it may be discarded.
 *
 * @note Only the size of the buffer changes. Types of instance data are immutable.
 */
NXAPI void NX_ReallocInstanceBuffer(NX_InstanceBuffer* buffer, size_t count, bool keepData);

/**
 * @brief Update a specific type of instance data within the buffer.
 *
 * @param buffer The instance buffer to update.
 * @param type   The type of data to update. Must be a type supported by this buffer.
 * @param offset Index of the first instance to update.
 * @param count  Number of instances to update.
 * @param data   Pointer to the source data to copy.
 *
 * @note The operation does nothing if the buffer does not support the requested type
 *       or if the range exceeds the currently allocated size.
 */
NXAPI void NX_UpdateInstanceBuffer(NX_InstanceBuffer* buffer, NX_InstanceData type, size_t offset, size_t count, const void* data);

/**
 * @brief Map the entire instance data of a specific type for CPU write access.
 *
 * @param buffer The instance buffer to map.
 * @param type The type of instance data to map. Must be supported by this buffer.
 * @return Pointer to the mapped memory, or NULL if the buffer/type is invalid.
 *
 * @note After writing, call NX_UnmapInstanceBuffer to unmap the buffer before NX_End3D.
 */
NXAPI void* NX_MapInstanceBuffer(NX_InstanceBuffer* buffer, NX_InstanceData type);

/**
 * @brief Map a subrange of a specific instance data type for CPU write access.
 *
 * @param buffer The instance buffer to map.
 * @param type The type of instance data to map. Must be supported by this buffer.
 * @param offset Index of the first instance to map.
 * @param count Number of instances to map.
 * @return Pointer to the mapped memory corresponding to the requested range,
 *         or NULL if the buffer/type is invalid or the range is out of bounds.
 *
 * @note After writing, call NX_UnmapInstanceBuffer to unmap the buffer before NX_End3D.
 */
NXAPI void* NX_MapInstanceBufferRange(NX_InstanceBuffer* buffer, NX_InstanceData type, size_t offset, size_t count);

/**
 * @brief Unmap a previously mapped instance data type.
 *
 * @param buffer The instance buffer to unmap.
 * @param type The type of instance data that was previously mapped.
 *
 * @note Must be called after NX_MapInstanceBuffer or NX_MapInstanceBufferRange
 *       before rendering using this buffer.
 */
NXAPI void NX_UnmapInstanceBuffer(NX_InstanceBuffer* buffer, NX_InstanceData type);

/**
 * @brief Retrieve information about the instance buffer.
 *
 * @param buffer   The instance buffer to query.
 * @param bitfield Output pointer to receive the supported instance data types.
 * @param count    Output pointer to receive the current allocated instance count.
 */
NXAPI void NX_QueryInstanceBuffer(NX_InstanceBuffer* buffer, NX_InstanceData* bitfield, size_t* count);

/** @} */ // end of InstanceBuffer

/**
 * @defgroup Model Model Functions
 * @{
 */

/**
 * @brief Sets the scaling factor applied to models when loading.
 * @param value Scaling factor (e.g., 0.01 for meters to centimeters).
 * @note Only affects models loaded after this call and formats that support scaling.
 */
NXAPI void NX_SetModelImportScale(float value);

/**
 * @brief Loads a 3D model from a file.
 * @param filePath Path to the model file.
 * @return Pointer to a newly loaded NX_Model containing meshes and materials.
 */
NXAPI NX_Model* NX_LoadModel(const char* filePath);

/**
 * @brief Loads a 3D model from memory.
 * @param data Pointer to memory buffer containing model data.
 * @param size Size of the memory buffer in bytes.
 * @param hint Hint on the model format (can be NULL).
 * @return Pointer to a newly loaded NX_Model containing meshes and materials.
 */
NXAPI NX_Model* NX_LoadModelFromMemory(const void* data, size_t size, const char* hint);

/**
 * @brief Destroys a 3D model and frees its resources.
 * @param model Pointer to the NX_Model to destroy.
 */
NXAPI void NX_DestroyModel(NX_Model* model);

/**
 * @brief Updates the axis-aligned bounding box (AABB) of a model.
 * @param model Pointer to the NX_Model to update.
 * @param updateMeshAABBs If true, also updates each mesh's bounding box before updating the model AABB.
 */
NXAPI void NX_UpdateModelAABB(NX_Model* model, bool updateMeshAABBs);

/**
 * @brief Scales the axis-aligned bounding box (AABB) of a model by a given factor.
 * @param model Pointer to the NX_Model whose AABB will be scaled.
 * @param scale Scaling factor to apply to the AABB.
 * @param scaleMeshAABBs If true, also scales the AABBs of each mesh before scaling the model AABB.
 */
NXAPI void NX_ScaleModelAABB(NX_Model* model, float scale, bool scaleMeshAABBs);

/**
 * @brief Loads animations from a model file.
 * @param filePath Path to the model file containing animations.
 * @param animCount Pointer to an int that receives the number of animations loaded.
 * @param targetFrameRate Desired frame rate (FPS) for sampling the animations.
 * @return Pointer to an array of NX_ModelAnimation, or NULL on failure.
 * @note Free the returned array using NX_DestroyModelAnimations().
 */
NXAPI NX_ModelAnimation** NX_LoadModelAnimations(const char* filePath, int* animCount, int targetFrameRate);

/**
 * @brief Loads animations from memory data.
 * @param data Pointer to memory buffer containing model animation data.
 * @param size Size of the buffer in bytes.
 * @param hint Hint on the model format (can be NULL).
 * @param animCount Pointer to an int that receives the number of animations loaded.
 * @param targetFrameRate Desired frame rate (FPS) for sampling the animations.
 * @return Pointer to an array of NX_ModelAnimation, or NULL on failure.
 * @note Free the returned array using NX_DestroyModelAnimations().
 */
NXAPI NX_ModelAnimation** NX_LoadModelAnimationsFromMemory(const void* data, unsigned int size, const char* hint, int* animCount, int targetFrameRate);

/**
 * @brief Frees memory allocated for model animations.
 * @param animations Pointer to the animation array to free.
 * @param animCount Number of animations in the array.
 */
NXAPI void NX_DestroyModelAnimations(NX_ModelAnimation** animations, int animCount);

/**
 * @brief Finds a named animation in an array of animations.
 * @param animations Array of NX_ModelAnimation pointers.
 * @param animCount Number of animations in the array.
 * @param name Name of the animation to find (case-sensitive).
 * @return Pointer to the matching animation, or NULL if not found.
 */
NXAPI NX_ModelAnimation* NX_GetModelAnimation(NX_ModelAnimation** animations, int animCount, const char* name);

/** @} */ // end of Model

/**
 * @defgroup Light Light Functions
 * @{
 */

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
 * @note Ignored for directional lights. Default is 16.0.
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
 * @brief Gets the shadow bleeding bias.
 * @param light Pointer to the NX_Light.
 * @return Current bleeding bias value.
 * @note Helps reduce light bleeding near occluders. Default is 0.2.
 */
NXAPI float NX_GetShadowBleedingBias(const NX_Light* light);

/**
 * @brief Sets the shadow bleeding bias.
 * @param light Pointer to the NX_Light.
 * @param bias New bleeding bias value.
 * @note Helps reduce light bleeding near occluders. Default is 0.2.
 */
NXAPI void NX_SetShadowBleedingBias(NX_Light* light, float bias);

/**
 * @brief Gets the shadow softness factor.
 * @param light Pointer to the NX_Light.
 * @return Current softness value, expressed in texels.
 * @note Represents the penumbra radius. Default is 1 / shadowMapResolution.
 */
NXAPI float NX_GetShadowSoftness(const NX_Light* light);

/**
 * @brief Sets the shadow softness factor.
 * @param light Pointer to the NX_Light.
 * @param softness New softness value, expressed in texels.
 * @note Represents the penumbra radius. Default is 1 / shadowMapResolution.
 */
NXAPI void NX_SetShadowSoftness(NX_Light* light, float softness);

/**
 * @brief Gets the shadow lambda factor (EVSM).
 * @param light Pointer to the NX_Light.
 * @return Current lambda value.
 * @note Used only in EVSM mode (not in GLES profile which uses VSM).
 *       Default is 40 for directional lights and 20 for spot/omni lights.
 */
NXAPI float NX_GetShadowLambda(const NX_Light* light);

/**
 * @brief Sets the shadow lambda factor (EVSM).
 * @param light Pointer to the NX_Light.
 * @param lambda New lambda value.
 * @note Used only in EVSM mode (not in GLES profile which uses VSM).
 *       Default is 40 for directional lights and 20 for spot/omni lights.
 */
NXAPI void NX_SetShadowLambda(NX_Light* light, float lambda);

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

/** @} */ // end of Light

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_RENDER_H
