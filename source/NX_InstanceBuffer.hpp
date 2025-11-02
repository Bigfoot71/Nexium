/* NX_InstanceBuffer.hpp-- API definition for Nexium's instance buffer module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_INSTANCE_BUFFER_HPP
#define NX_INSTANCE_BUFFER_HPP

#include <NX/NX_InstanceBuffer.h>
#include <NX/NX_Math.h>

#include "./Detail/GPU/Buffer.hpp"
#include "./Detail/Helper.hpp"
#include <array>

// ============================================================================
// OPAQUE DEFINITION
// ============================================================================

struct NX_InstanceBuffer {

    static constexpr size_t TypeSizes[] = {
        sizeof(NX_Vec3),
        sizeof(NX_Quat),
        sizeof(NX_Vec3),
        sizeof(NX_Color),
        sizeof(NX_Vec4)
    };

    static constexpr const char* TypeNames[] = {
        "NX_INSTANCE_POSITION",
        "NX_INSTANCE_ROTATION",
        "NX_INSTANCE_SCALE",
        "NX_INSTANCE_COLOR",
        "NX_INSTANCE_CUSTOM"
    };

    std::array<gpu::Buffer, 5> buffers{};
    NX_InstanceData bufferFlags{};
    size_t allocatedCount{};

    const gpu::Buffer* GetBuffer(NX_InstanceData type) const;

};

inline const gpu::Buffer* NX_InstanceBuffer::GetBuffer(NX_InstanceData type) const
{
    const gpu::Buffer& buffer = this->buffers[helper::BitScanForward(type)];
    return buffer.isValid() ? &buffer : nullptr;
}

#endif // NX_INSTANCE_BUFFER_HPP
