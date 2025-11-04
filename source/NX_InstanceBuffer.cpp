/* NX_InstanceBuffer.cpp -- API definition for Nexium's instance buffer module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "./NX_InstanceBuffer.hpp"
#include "./INX_GlobalPool.hpp"
#include "./INX_Utils.hpp"

// ============================================================================
// PUBLIC API
// ============================================================================

NX_InstanceBuffer* NX_CreateInstanceBuffer(NX_InstanceData bitfield, size_t count)
{
    NX_InstanceBuffer* buffer = INX_Pool.Create<NX_InstanceBuffer>();

    buffer->bufferFlags = bitfield;
    buffer->allocatedCount = count;

    INX_ForEachBit(bitfield, [buffer, count](int index) {
        size_t size = count * NX_InstanceBuffer::TypeSizes[index];
        buffer->buffers[index] = gpu::Buffer(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    });

    return buffer;
}

void NX_DestroyInstanceBuffer(NX_InstanceBuffer* buffer)
{
    INX_Pool.Destroy(buffer);
}

void NX_RaeallocInstanceBuffer(NX_InstanceBuffer* buffer, size_t count, bool keepData)
{
    for (int i = 0; i < buffer->buffers.size(); i++) {
        if (buffer->buffers[i].IsValid()) {
            size_t size = count * NX_InstanceBuffer::TypeSizes[i];
            buffer->buffers[i].Realloc(size, keepData);
        }
    }

    buffer->allocatedCount = count;
}

void NX_UpdateInstanceBuffer(NX_InstanceBuffer* buffer, NX_InstanceData type, size_t offset, size_t count, const void* data)
{
    type = INX_BitScanForward(type);
    gpu::Buffer& gpu = buffer->buffers[type];

    offset *= NX_InstanceBuffer::TypeSizes[type];
    count *= NX_InstanceBuffer::TypeSizes[type];

    if (!gpu.IsValid()) {
        NX_LOG(E, "RENDER: Cannot upload to instance buffer; type '%s' is not initialized.", NX_InstanceBuffer::TypeNames[type]);
        return;
    }

    if (offset + count > gpu.GetSize()) {
        NX_LOG(E, "RENDER: Upload range out of bounds for type '%s' (offset %zu + count %zu > buffer size %zu).",
            NX_InstanceBuffer::TypeNames[type], offset, count, gpu.GetSize());
        return;
    }

    gpu.Upload(offset, count, data);
}

void* NX_MapInstanceBuffer(NX_InstanceBuffer* buffer, NX_InstanceData type)
{
    type = INX_BitScanForward(type);
    gpu::Buffer& gpu = buffer->buffers[type];

    if (!gpu.IsValid()) {
        NX_LOG(E, "RENDER: Cannot map instance buffer; type '%s' is not initialized.", NX_InstanceBuffer::TypeNames[type]);
        return nullptr;
    }

    return gpu.Map(GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
}

void* NX_MapInstanceBufferRange(NX_InstanceBuffer* buffer, NX_InstanceData type, size_t offset, size_t count)
{
    type = INX_BitScanForward(type);
    gpu::Buffer& gpu = buffer->buffers[type];

    offset *= NX_InstanceBuffer::TypeSizes[type];
    count *= NX_InstanceBuffer::TypeSizes[type];

    if (!gpu.IsValid()) {
        NX_LOG(E, "RENDER: Cannot map instance buffer range; type '%s' is not initialized.", NX_InstanceBuffer::TypeNames[type]);
        return nullptr;
    }

    if (offset + count > gpu.GetSize()) {
        NX_LOG(E, "RENDER: Map range out of bounds for type '%s' (offset %zu + count %zu > buffer size %zu).",
            NX_InstanceBuffer::TypeNames[type], offset, count, gpu.GetSize());
        return nullptr;
    }

    return gpu.MapRange(offset, count, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
}

void NX_UnmapInstanceBuffer(NX_InstanceBuffer* buffer, NX_InstanceData type)
{
    buffer->buffers[INX_BitScanForward(type)].Unmap();
}

void NX_QueryInstanceBuffer(NX_InstanceBuffer* buffer, NX_InstanceData* bitfield, size_t* count)
{
    if (bitfield) *bitfield = buffer->bufferFlags;
    if (count) *count = buffer->allocatedCount;
}
