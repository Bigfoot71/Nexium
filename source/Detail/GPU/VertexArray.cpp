/**
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided "as-is", without any express or implied warranty. In no event
 * will the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including commercial
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not claim that you
 *   wrote the original software. If you use this software in a product, an acknowledgment
 *   in the product documentation would be appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *   as being the original software.
 *
 *   3. This notice may not be removed or altered from any source distribution.
 */

#include "./VertexArray.hpp"
#include "./Pipeline.hpp"

namespace gpu {

/* === Public Implementation === */

VertexArray::VertexArray(Buffer* indexBuffer, std::initializer_list<VertexBufferDesc> vertexBuffers) noexcept
    : mIndexBuffer(indexBuffer)
{
    // NOTE: gpu::Pipeline already manages dummy VAOs internally
    //       when 'draw' is called without a bound VertexArray.
    //       Therefore, any creation of a VertexArray without
    //       a vertex buffer is considered an error.

    SDL_assert(vertexBuffers.size() != 0);

    /* --- Validate all vertex buffer descriptors and their attributes --- */

    for (const auto& vbDesc : vertexBuffers)
    {
        if (!vbDesc.buffer || !vbDesc.buffer->isValid()) {
            HP_INTERNAL_LOG(E, "GPU: Invalid vertex buffer provided");
            return;
        }

        if (vbDesc.buffer->target() != GL_ARRAY_BUFFER) {
            HP_INTERNAL_LOG(E, "GPU: Vertex buffer must have GL_ARRAY_BUFFER target");
            return;
        }

        if (vbDesc.attributes.size() == 0) {
            HP_INTERNAL_LOG(E, "GPU: Vertex buffer must have at least one attribute");
            return;
        }

        for (const auto& attr : vbDesc.attributes)
        {
            if (!isValidAttributeSize(attr.size)) {
                HP_INTERNAL_LOG(E, "GPU: Invalid attribute size %d for location %u", attr.size, attr.location);
                return;
            }

            if (!isValidAttributeType(attr.type)) {
                HP_INTERNAL_LOG(E, "GPU: Invalid attribute type 0x%x for location %u", attr.type, attr.location);
                return;
            }

            if (attr.stride < 0) {
                HP_INTERNAL_LOG(E, "GPU: Invalid negative stride %d for location %u", attr.stride, attr.location);
                return;
            }

            if (attr.offset < 0) {
                HP_INTERNAL_LOG(E, "GPU: Invalid negative offset %lld for location %u",
                    static_cast<long long>(attr.offset), attr.location);
                return;
            }
        }
    }

    /* --- Validate index buffer if provided --- */

    if (mIndexBuffer) {
        if (!mIndexBuffer->isValid()) {
            HP_INTERNAL_LOG(E, "GPU: Invalid index buffer provided");
            return;
        }

        if (mIndexBuffer->target() != GL_ELEMENT_ARRAY_BUFFER) {
            HP_INTERNAL_LOG(E, "GPU: Index buffer must have GL_ELEMENT_ARRAY_BUFFER target");
            return;
        }
    }

    /* --- Create the OpenGL Vertex Array Object (VAO) --- */

    glGenVertexArrays(1, &mID);
    if (mID == 0) {
        HP_INTERNAL_LOG(E, "GPU: Failed to create vertex array object");
        return;
    }

    /* --- Reserve and store references to vertex buffers --- */

    if (!mVertexBuffers.reserve(vertexBuffers.size())) {
        HP_INTERNAL_LOG(E, "GPU: Failed to allocate buffer to store vertex array buffers");
        glDeleteVertexArrays(1, &mID);
        mID = 0;
        return;
    }

    for (const auto& vbDesc : vertexBuffers) {
        mVertexBuffers.push_back(vbDesc.buffer);
    }

    /* --- Bind VAO, set up vertex attributes and buffer bindings --- */

    Pipeline::withVertexArrayBind(mID,
        [&]()
        {
            if (mIndexBuffer) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer->id());
            }

            for (size_t bufferIndex = 0; bufferIndex < mVertexBuffers.size(); ++bufferIndex)
            {
                Buffer* buffer = mVertexBuffers[bufferIndex];

                glBindBuffer(GL_ARRAY_BUFFER, buffer->id());

                for (const auto& attr : vertexBuffers.begin()[bufferIndex].attributes)
                {
                    glEnableVertexAttribArray(attr.location);

                    if (isIntegerAttributeType(attr.type) && attr.normalized == GL_FALSE) {
                        glVertexAttribIPointer(
                            attr.location, attr.size, attr.type, attr.stride,
                            reinterpret_cast<const void*>(attr.offset)
                        );
                    }
                    else {
                        glVertexAttribPointer(
                            attr.location, attr.size, attr.type, attr.normalized,
                            attr.stride, reinterpret_cast<const void*>(attr.offset)
                        );
                    }

                    if (attr.divisor > 0) {
                        glVertexAttribDivisor(attr.location, attr.divisor);
                    }
                }
            }

            glBindBuffer(GL_ARRAY_BUFFER, 0);

            if (glGetError() != GL_NO_ERROR) {
                HP_INTERNAL_LOG(E, "GPU: Failed to setup vertex array");
                glDeleteVertexArrays(1, &mID);
                mID = 0;
                return;
            }
        }
    );
}

} // namespace gpu
