/* NX_Render.h -- API declaration for Nexium's render module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_RENDER_H
#define NX_RENDER_H

#include "./NX_ReflectionProbe.h"
#include "./NX_InstanceBuffer.h"
#include "./NX_RenderTexture.h"
#include "./NX_Environment.h"
#include "./NX_Material.h"
#include "./NX_Shader3D.h"
#include "./NX_Cubemap.h"
#include "./NX_Texture.h"
#include "./NX_Camera.h"
#include "./NX_Vertex.h"
#include "./NX_Math.h"
#include "./NX_API.h"

#include <stdint.h>

/* === Enums === */

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
    NX_LIGHT_OMNI,              ///< Omni light, emits light in all directions from a single point.
    NX_LIGHT_TYPE_COUNT
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

/* === Handlers === */

/**
 * @brief Opaque handle to a GPU vertex buffer.
 *
 * Represents a collection of vertices stored on the GPU.
 */
typedef struct NX_VertexBuffer NX_VertexBuffer;

/**
 * @brief Opaque handle to a GPU dynamic mesh.
 *
 * Represents a mesh whose vertex data can be modified or rebuilt each frame.
 */
typedef struct NX_DynamicMesh NX_DynamicMesh;

/**
 * @brief Opaque handle to a light source.
 *
 * Represents a light in the scene.
 * Can be used for directional, spot or omni-directional lights.
 */
typedef struct NX_Light NX_Light;

/* === Structures === */

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
    NX_PrimitiveType primitiveType;     ///< Type of primitive that constitutes the vertices.
    NX_BoundingBox aabb;                ///< Axis-Aligned Bounding Box in local space.
    NX_Layer layerMask;                 ///< Bitfield indicating the rendering layer(s) of this mesh.

} NX_Mesh;

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

/** @} */ // end of Draw3D

/**
 * @defgroup Mesh Mesh Functions
 * @{
 */

/**
 * @brief Creates a 3D mesh by copying vertex and index data.
 * @param type Type of primitive that constitutes the vertices.
 * @param vertices Pointer to the vertex array (cannot be NULL).
 * @param vertexCount Number of vertices.
 * @param indices Pointer to the index array (can be NULL).
 * @param indexCount Number of indices.
 * @return Pointer to a newly created NX_Mesh.
 * @note The function copies the data into internal buffers.
 */
NXAPI NX_Mesh* NX_CreateMesh(NX_PrimitiveType type, const NX_Vertex3D* vertices, int vertexCount, const uint32_t* indices, int indexCount);

/**
 * @brief Creates a 3D mesh by taking ownership of pre-allocated vertex and index arrays.
 * @param type Type of primitive that constitutes the vertices.
 * @param vertices Pointer to a pre-allocated vertex array (cannot be NULL).
 * @param vertexCount Number of vertices in the array.
 * @param indices Pointer to a pre-allocated index array (can be NULL).
 * @param indexCount Number of indices in the array.
 * @return Pointer to a newly created NX_Mesh.
 * @note The caller **must not free** the arrays after passing them to this function.
 *       Use NX_CreateMesh() if you prefer the mesh to copy the data instead.
 */
NXAPI NX_Mesh* NX_CreateMeshFrom(NX_PrimitiveType type, NX_Vertex3D* vertices, int vertexCount, uint32_t* indices, int indexCount);

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
NXAPI NX_Mesh* NX_GenMeshQuad(NX_Vec2 size, NX_IVec2 subDiv, NX_Vec3 normal);

/**
 * @brief Generates a cube mesh.
 * @param size Dimensions along X, Y, Z.
 * @param subDiv Subdivision along each axis.
 * @return Pointer to a newly generated NX_Mesh.
 */
NXAPI NX_Mesh* NX_GenMeshCube(NX_Vec3 size, NX_IVec3 subDiv);

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
 * @defgroup DynamicMesh Dynamic Mesh Functions
 * @{
 */

/**
 * @brief Creates a dynamic mesh.
 *
 * Create a dynamic mesh that can be rebuilt each frame.
 *
 * @param initialCapacity Initial number of vertices to pre-allocate (must be > 0).
 *                        This value is only informative; memory will be reallocated if needed.
 * @return Pointer to a new NX_DynamicMesh, or NULL if creation fails.
 */
NXAPI NX_DynamicMesh* NX_CreateDynamicMesh(size_t initialCapacity);

/**
 * @brief Destroys a dynamic mesh.
 *
 * Releases all GPU and CPU resources associated with the mesh.
 *
 * @param dynMesh Pointer to the dynamic mesh to destroy.
 */
NXAPI void NX_DestroyDynamicMesh(NX_DynamicMesh* dynMesh);

/**
 * @brief Begins recording geometry for a dynamic mesh.
 *
 * All previous geometry is overridden. The primitive type can change between frames.
 *
 * @param dynMesh Pointer to the dynamic mesh (cannot be NULL).
 * @param type Primitive type to use for the new geometry.
 */
NXAPI void NX_BeginDynamicMesh(NX_DynamicMesh* dynMesh, NX_PrimitiveType type);

/**
 * @brief Ends recording and uploads geometry to the GPU.
 *
 * Reallocates memory on the GPU if necessary.
 *
 * @param dynMesh Pointer to the dynamic mesh (cannot be NULL).
 */
NXAPI void NX_EndDynamicMesh(NX_DynamicMesh* dynMesh);

/**
 * @brief Sets the texture coordinate for the next vertex.
 *
 * @param dynMesh Pointer to the dynamic mesh (cannot be NULL).
 * @param texcoord 2D texture coordinate to assign.
 */
NXAPI void NX_SetDynamicMeshTexCoord(NX_DynamicMesh* dynMesh, NX_Vec2 texcoord);

/**
 * @brief Sets the normal vector for the next vertex.
 *
 * @param dynMesh Pointer to the dynamic mesh (cannot be NULL).
 * @param normal 3D normal vector to assign.
 */
NXAPI void NX_SetDynamicMeshNormal(NX_DynamicMesh* dynMesh, NX_Vec3 normal);

/**
 * @brief Sets the tangent vector for the next vertex.
 *
 * @param dynMesh Pointer to the dynamic mesh (cannot be NULL).
 * @param tangent 4D tangent vector to assign.
 */
NXAPI void NX_SetDynamicMeshTangent(NX_DynamicMesh* dynMesh, NX_Vec4 tangent);

/**
 * @brief Sets the color for the next vertex.
 *
 * @param dynMesh Pointer to the dynamic mesh (cannot be NULL).
 * @param color RGBA color to assign.
 */
NXAPI void NX_SetDynamicMeshColor(NX_DynamicMesh* dynMesh, NX_Color color);

/**
 * @brief Adds a vertex to the dynamic mesh.
 *
 * The current attributes (position, normal, tangent, texcoord, color) are used.
 *
 * @param dynMesh Pointer to the dynamic mesh (cannot be NULL).
 * @param position 3D position of the vertex.
 */
NXAPI void NX_AddDynamicMeshVertex(NX_DynamicMesh* dynMesh, NX_Vec3 position);

/**
 * @brief Sets the shadow casting mode for a dynamic mesh.
 *
 * Default is NX_SHADOW_CAST_ENABLED.
 *
 * @param dynMesh Pointer to the dynamic mesh (cannot be NULL).
 * @param mode Shadow casting mode to apply.
 */
NXAPI void NX_SetDynamicMeshShadowCastMode(NX_DynamicMesh* dynMesh, NX_ShadowCastMode mode);

/**
 * @brief Sets the shadow face mode for a dynamic mesh.
 *
 * Default is NX_SHADOW_FACE_AUTO.
 *
 * @param dynMesh Pointer to the dynamic mesh (cannot be NULL).
 * @param mode Shadow face mode to apply.
 */
NXAPI void NX_SetDynamicMeshShadowFaceMode(NX_DynamicMesh* dynMesh, NX_ShadowFaceMode mode);

/**
 * @brief Sets the layer mask for a dynamic mesh.
 *
 * Default is NX_LAYER_01.
 *
 * @param dynMesh Pointer to the dynamic mesh (cannot be NULL).
 * @param mask Layer mask to assign.
 */
NXAPI void NX_SetDynamicMeshLayerMask(NX_DynamicMesh* dynMesh, NX_Layer mask);

/** @} */ // end of DynamicMesh

/**
 * @defgroup Model Model Functions
 * @{
 */

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
NXAPI NX_Model* NX_LoadModelFromDataory(const void* data, size_t size, const char* hint);

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
NXAPI NX_ModelAnimation** NX_LoadModelAnimationsFromDataory(const void* data, unsigned int size, const char* hint, int* animCount, int targetFrameRate);

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

/** @} */ // end of Light

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_RENDER_H
