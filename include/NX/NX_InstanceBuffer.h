/* NX_InstanceBuffer.h -- API declaration for Nexium's instance buffer module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_INSTANCE_BUFFER_H
#define NX_INSTANCE_BUFFER_H

#include "./NX_API.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// ============================================================================
// TYPES DEFINITIONS
// ============================================================================

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
 * @brief Opaque handle to a GPU instance buffer.
 *
 * Represents per-instance data stored on the GPU, used for instanced rendering.
 */
typedef struct NX_InstanceBuffer NX_InstanceBuffer;

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

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

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_INSTANCE_BUFFER_H
