/* Pipeline.hpp -- Scoped GPU pipeline state management class
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_GPU_PIPELINE_HPP
#define NX_GPU_PIPELINE_HPP

#include "../../INX_GlobalState.hpp"  //< Used to get OpenGL profile used (Core/ES)

#include "./VertexArray.hpp"
#include "./Framebuffer.hpp"
#include "./Texture.hpp"
#include "./Program.hpp"
#include "./Buffer.hpp"

#include <SDL3/SDL_assert.h>
#include <glad/gles2.h>

#include <initializer_list>
#include <concepts>
#include <array>

namespace gpu {

/* === Enums === */

enum class ColorWrite {
    Disabled,       // glColorMask(false, false, false, false)
    RGB,            // glColorMask(true, true, true, false)
    RGBA,           // glColorMask(true, true, true, true)
};

enum class DepthMode {
    Disabled,       // glDisable(GL_DEPTH_TEST), glDepthMask(false)
    TestOnly,       // glEnable(GL_DEPTH_TEST), glDepthMask(false)
    WriteOnly,      // glDisable(GL_DEPTH_TEST), glDepthMask(true)
    TestAndWrite,   // glEnable(GL_DEPTH_TEST), glDepthMask(true)
};

enum class DepthFunc {
    Never,          // GL_NEVER
    Less,           // GL_LESS
    Equal,          // GL_EQUAL
    LessEqual,      // GL_LEQUAL
    Greater,        // GL_GREATER
    NotEqual,       // GL_NOTEQUAL
    GreaterEqual,   // GL_GEQUAL
    Always,         // GL_ALWAYS
};

enum class BlendMode {
    Disabled,        // No blending - glDisable(GL_BLEND)
    Alpha,           // Standard alpha blending - GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA + GL_FUNC_ADD
    Premultiplied,   // Premultiplied alpha blending - GL_ONE, GL_ONE_MINUS_SRC_ALPHA + GL_FUNC_ADD
    AddAlpha,        // AddAlpha blending - GL_SRC_ALPHA, GL_ONE + GL_FUNC_ADD
    Additive,        // Additive blending - GL_ONE, GL_ONE + GL_FUNC_ADD
    Multiply,        // Multiply blending - GL_DST_COLOR, GL_ZERO + GL_FUNC_ADD
    Subtract,        // Subtractive blending - GL_SRC_ALPHA, GL_ONE + GL_FUNC_REVERSE_SUBTRACT
    Minimum,         // Minimum blending - GL_ONE, GL_ONE + GL_MIN
};

enum class CullMode {
    Disabled,       // glDisable(GL_CULL_FACE)
    Front,          // glEnable(GL_CULL_FACE), glCullFace(GL_FRONT)
    Back,           // glEnable(GL_CULL_FACE), glCullFace(GL_BACK)
    FrontAndBack,   // glEnable(GL_CULL_FACE), glCullFace(GL_FRONT_AND_BACK)
};

/* === Declaration === */

class Pipeline {
private:
    friend VertexArray;
    friend Framebuffer;
    friend Texture;
    friend Program;
    friend Buffer;

public:
    Pipeline() noexcept;

    template<typename F>
        requires std::invocable<F, const Pipeline&>
    Pipeline(F&& func) noexcept;

    ~Pipeline() noexcept;

    void setColorWrite(ColorWrite mode) const noexcept;
    void setDepthMode(DepthMode mode) const noexcept;
    void setDepthFunc(DepthFunc func) const noexcept;
    void setBlendMode(BlendMode mode) const noexcept;
    void setCullMode(CullMode mode) const noexcept;

    void bindFramebuffer(const Framebuffer& framebuffer) const noexcept;
    void bindVertexArray(const VertexArray& vertexArray) const noexcept;
    void bindTexture(int slot, const Texture& texture) const noexcept;
    void bindStorage(int slot, const Buffer& storage) const noexcept;
    void bindStorage(int slot, const Buffer& storage, size_t offset, size_t size) const noexcept;
    void bindUniform(int slot, const Buffer& uniform) const noexcept;
    void bindUniform(int slot, const Buffer& uniform, size_t offset, size_t size) const noexcept;

    void unbindFramebuffer() const noexcept;
    void unbindVertexArray() const noexcept;
    void unbindTexture(int slot) const noexcept;
    void unbindStorage(int slot) const noexcept;
    void unbindUniform(int slot) const noexcept;

    void useProgram(const Program& program) const noexcept;

    void setUniformUint1(int location, uint32_t value) const noexcept;
    void setUniformUint2(int location, const NX_IVec2& value) const noexcept;
    void setUniformUint3(int location, const NX_IVec3& value) const noexcept;
    void setUniformUint4(int location, const NX_IVec4& value) const noexcept;
    void setUniformInt1(int location, int value) const noexcept;
    void setUniformInt2(int location, const NX_IVec2& value) const noexcept;
    void setUniformInt3(int location, const NX_IVec3& value) const noexcept;
    void setUniformInt4(int location, const NX_IVec4& value) const noexcept;
    void setUniformFloat1(int location, float value) const noexcept;
    void setUniformFloat2(int location, const NX_Vec2& value) const noexcept;
    void setUniformFloat3(int location, const NX_Vec3& value) const noexcept;
    void setUniformFloat3(int location, const NX_Color& value) const noexcept;
    void setUniformFloat4(int location, const NX_Vec4& value) const noexcept;
    void setUniformFloat4(int location, const NX_Quat& value) const noexcept;
    void setUniformFloat4(int location, const NX_Color& value) const noexcept;
    void setUniformMat3(int location, const NX_Mat3& value) const noexcept;
    void setUniformMat3(int location, const NX_Mat4& value) const noexcept;
    void setUniformMat4(int location, const NX_Mat4& value) const noexcept;

    void setViewport(NX_IVec2 size) const noexcept;
    void setViewport(int x, int y, int w, int h) const noexcept;
    void setViewport(const gpu::Framebuffer& dst) const noexcept;

    void clear(const gpu::Framebuffer& framebuffer, NX_Color color = NX_BLACK, float depth = 1.0) noexcept;
    void clearColor(std::initializer_list<std::pair<int, NX_Color>> attachments) const noexcept;
    void clearColor(int attachment, NX_Color color) const noexcept;
    void clearDepth(float depth) const noexcept;

    void draw(GLenum mode, GLsizei count) const noexcept;
    void draw(GLenum mode, GLint first, GLsizei count) const noexcept;

    void drawInstanced(GLenum mode, GLsizei count, GLsizei instanceCount) const noexcept;
    void drawInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instanceCount) const noexcept;

    void drawElements(GLenum mode, GLenum type, GLsizei count) const noexcept;
    void drawElements(GLenum mode, GLenum type, GLint first, GLsizei count) const noexcept;

    void drawElementsInstanced(GLenum mode, GLenum type, GLsizei count, GLsizei instanceCount) const noexcept;
    void drawElementsInstanced(GLenum mode, GLenum type, GLint first, GLsizei count, GLsizei instanceCount) const noexcept;

    void drawArraysIndirect(GLenum mode, const void* indirect) const noexcept;
    void drawElementsIndirect(GLenum mode, GLenum type, const void* indirect) const noexcept;

    void dispatchCompute(GLuint numGroupsX, GLuint numGroupsY, GLuint numGroupsZ) const noexcept;
    void dispatchComputeIndirect(GLintptr indirect) const noexcept;

public:
    /** Non-instantiated operations */
    static void blitToBackBuffer(const gpu::Framebuffer& src, int xDst, int yDst, int wDst, int hDst, bool linear) noexcept;
    static void memoryBarrier(GLbitfield barriers) noexcept;

    /** Hardware info getters */
    static int uniformBufferOffsetAlignment() noexcept;
    static int storageBufferOffsetAlignment() noexcept;
    static int maxUniformBufferSize() noexcept;
    static int maxStorageBufferSize() noexcept;

private:
    // Prevents reentrancy for 'withXBind' functions
#ifndef NDEBUG
    template <GLenum GLTag>
    struct DebugExclusiveBindGuard {
        static inline bool sBound = false;
        DebugExclusiveBindGuard() {
            SDL_assert(sBound == false);
            sBound = true;
        }
        ~DebugExclusiveBindGuard() {
            sBound = false;
        }
    };
#endif

private:
    template <typename F>
        requires std::invocable<F>
    static void withFramebufferBind(GLuint id, F&& func) noexcept;

    template <typename F>
        requires std::invocable<F>
    static void withVertexArrayBind(GLuint id, F&& func) noexcept;

    template <typename F>
        requires std::invocable<F>
    static void withTextureBind(GLenum target, GLuint id, F&& func) noexcept;

    template <typename F>
        requires std::invocable<F>
    static void withBufferBind(GLenum target, GLuint id, F&& func) noexcept;

private:
    void setColorWrite_Internal(ColorWrite mode) const noexcept;
    void setDepthMode_Internal(DepthMode mode) const noexcept;
    void setDepthFunc_Internal(DepthFunc func) const noexcept;
    void setBlendMode_Internal(BlendMode mode) const noexcept;
    void setCullMode_Internal(CullMode mode) const noexcept;

private:
    struct BufferRange {
        size_t offset, size;
        BufferRange() : offset(0), size(0) {}
        BufferRange(size_t o, size_t s) : offset(o), size(s) {}
        bool operator==(const BufferRange& other) const noexcept {
            return (offset == other.offset && size == other.size);
        }
        bool operator!=(const BufferRange& other) const noexcept {
            return (offset != other.offset || size != other.size);
        }
    };

private:
    /** Default pipeline state */
    static constexpr ColorWrite InitialColorWrite = ColorWrite::RGBA;
    static constexpr DepthMode InitialDepthMode = DepthMode::Disabled;
    static constexpr DepthFunc InitialDepthFunc = DepthFunc::Less;
    static constexpr BlendMode InitialBlendMode = BlendMode::Disabled;
    static constexpr CullMode InitialCullMode = CullMode::Disabled;

    /** Current pipeline state */
    static inline ColorWrite sCurrentColorWrite = InitialColorWrite;
    static inline DepthMode sCurrentDepthMode = InitialDepthMode;
    static inline DepthFunc sCurrentDepthFunc = InitialDepthFunc;
    static inline BlendMode sCurrentBlendMode = InitialBlendMode;
    static inline CullMode sCurrentCullMode = InitialCullMode;

    /** Object trackers */
    static inline const Framebuffer* sBindFramebuffer = nullptr;
    static inline const VertexArray* sBindVertexArray = nullptr;
    static inline std::array<const Texture*, 32> sBindTexture{};
    static inline std::array<const Buffer*, 8> sBindStorage{};
    static inline std::array<BufferRange, 8> sStorageRange{};
    static inline std::array<const Buffer*, 16> sBindUniform{};
    static inline std::array<BufferRange, 16> sUniformRange{};
    static inline const Program* sUsedProgram = nullptr;

    /** Default state objects */
    static inline GLuint sDummyVAO{}; //< Used for draw without VAO

    /** Pipeline instance tracker */
    static inline bool sCurrentlyInstanced{false};
};

/* === Public Implementation === */

inline Pipeline::Pipeline() noexcept
{
    SDL_assert(sCurrentlyInstanced == false);
    sCurrentlyInstanced = true;

    /* --- Setup initial state --- */

    if (sDummyVAO == 0)
    {
        glGenVertexArrays(1, &sDummyVAO);
        glBindVertexArray(sDummyVAO);

        setColorWrite_Internal(InitialColorWrite);
        setDepthMode_Internal(InitialDepthMode);
        setDepthFunc_Internal(InitialDepthFunc);
        setBlendMode_Internal(InitialBlendMode);
        setCullMode_Internal(InitialCullMode);

        if (INX_Display.glProfile != SDL_GL_CONTEXT_PROFILE_ES) {
            // NOTE: Enabled by default in GLES 3.2, this avoids cubemap seams issue...
            // SEE: https://www.khronos.org/opengl/wiki/Cubemap_Texture#Seamless_cubemap
            // SEE: https://registry.khronos.org/OpenGL/specs/es/3.2/es_spec_3.2.pdf#section.G.2
            constexpr GLenum GL_TEXTURE_CUBE_MAP_SEAMLESS = 0x884F;
            glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        }
    }
}

template<typename F>
    requires std::invocable<F, const Pipeline&>
Pipeline::Pipeline(F&& func) noexcept
    : Pipeline()
{
    func(*this);
}

inline Pipeline::~Pipeline() noexcept
{
    if (sCurrentColorWrite != InitialColorWrite) {
        setColorWrite_Internal(InitialColorWrite);
        sCurrentColorWrite = InitialColorWrite;
    }
    if (sCurrentDepthMode != InitialDepthMode) {
        setDepthMode_Internal(InitialDepthMode);
        sCurrentDepthMode = InitialDepthMode;
    }
    if (sCurrentDepthFunc != InitialDepthFunc) {
        setDepthFunc_Internal(InitialDepthFunc);
        sCurrentDepthFunc = InitialDepthFunc;
    }
    if (sCurrentBlendMode != InitialBlendMode) {
        setBlendMode_Internal(InitialBlendMode);
        sCurrentBlendMode = InitialBlendMode;
    }
    if (sCurrentCullMode != InitialCullMode) {
        setCullMode_Internal(InitialCullMode);
        sCurrentCullMode = InitialCullMode;
    }
    for (int slot = 0; slot < sBindTexture.size(); ++slot) {
        if (sBindTexture[slot] != nullptr) {
            glActiveTexture(GL_TEXTURE0 + slot);
            glBindTexture(sBindTexture[slot]->target(), 0);
            sBindTexture[slot] = nullptr;
        }
    }
    for (int slot = 0; slot < sBindStorage.size(); ++slot) {
        if (sBindStorage[slot] != nullptr) {
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot, 0);
            sStorageRange[slot] = BufferRange();
            sBindStorage[slot] = nullptr;
        }
    }
    for (int slot = 0; slot < sBindUniform.size(); ++slot) {
        if (sBindUniform[slot] != nullptr) {
            glBindBufferBase(GL_UNIFORM_BUFFER, slot, 0);
            sUniformRange[slot] = BufferRange();
            sBindUniform[slot] = nullptr;
        }
    }
    if (sBindFramebuffer != nullptr) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        sBindFramebuffer = nullptr;
    }
    if (sBindVertexArray != nullptr) {
        glBindVertexArray(sDummyVAO);
        sBindVertexArray = nullptr;
    }
    if (sUsedProgram != nullptr) {
        glUseProgram(0);
        sUsedProgram = nullptr;
    }
    sCurrentlyInstanced = false;
}

inline void Pipeline::setColorWrite(ColorWrite mode) const noexcept
{
    if (mode != sCurrentColorWrite) {
        setColorWrite_Internal(mode);
        sCurrentColorWrite = mode;
    }
}

inline void Pipeline::setDepthMode(DepthMode mode) const noexcept
{
    if (mode != sCurrentDepthMode) {
        setDepthMode_Internal(mode);
        sCurrentDepthMode = mode;
    }
}

inline void Pipeline::setDepthFunc(DepthFunc func) const noexcept
{
    if (func != sCurrentDepthFunc) {
        setDepthFunc_Internal(func);
        sCurrentDepthFunc = func;
    }
}

inline void Pipeline::setBlendMode(BlendMode mode) const noexcept
{
    if (mode != sCurrentBlendMode) {
        setBlendMode_Internal(mode);
        sCurrentBlendMode = mode;
    }
}

inline void Pipeline::setCullMode(CullMode mode) const noexcept
{
    if (mode != sCurrentCullMode) {
        setCullMode_Internal(mode);
        sCurrentCullMode = mode;
    }
}

inline void Pipeline::bindFramebuffer(const Framebuffer& framebuffer) const noexcept
{
    if (&framebuffer != sBindFramebuffer) {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.renderId());
        sBindFramebuffer = &framebuffer;
    }
}

inline void Pipeline::bindVertexArray(const VertexArray& vertexArray) const noexcept
{
    if (&vertexArray != sBindVertexArray) {
        glBindVertexArray(vertexArray.id());
        sBindVertexArray = &vertexArray;
    }
}

inline void Pipeline::bindTexture(int slot, const Texture& texture) const noexcept
{
    SDL_assert(slot < sBindTexture.size());

    if (&texture == sBindTexture[slot]) {
        return;
    }

    glActiveTexture(GL_TEXTURE0 + slot);

    if (sBindTexture[slot] != nullptr && sBindTexture[slot]->target() != texture.target()) {
        glBindTexture(sBindTexture[slot]->target(), 0);
    }

    glBindTexture(texture.target(), texture.id());
    sBindTexture[slot] = &texture;
}

inline void Pipeline::bindStorage(int slot, const Buffer& storage) const noexcept
{
    SDL_assert(storage.target() == GL_SHADER_STORAGE_BUFFER);
    SDL_assert(slot < sBindStorage.size());

    BufferRange range(0, storage.size());
    if (&storage == sBindStorage[slot] && range == sStorageRange[slot]) {
        return;
    }

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot, storage.id());
    sBindStorage[slot] = &storage;
    sStorageRange[slot] = range;
}

inline void Pipeline::bindStorage(int slot, const Buffer& storage, size_t offset, size_t size) const noexcept
{
    SDL_assert((offset % storageBufferOffsetAlignment()) == 0);
    SDL_assert(storage.target() == GL_SHADER_STORAGE_BUFFER);
    SDL_assert(size > 0 && size <= storage.size());
    SDL_assert(slot < sBindStorage.size());

    BufferRange range(offset, size);
    if (&storage == sBindStorage[slot] && range == sStorageRange[slot]) {
        return;
    }

    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, slot, storage.id(), offset, size);
    sBindStorage[slot] = &storage;
    sStorageRange[slot] = range;
}

inline void Pipeline::bindUniform(int slot, const Buffer& uniform) const noexcept
{
    SDL_assert(uniform.target() == GL_UNIFORM_BUFFER);
    SDL_assert(slot < sBindUniform.size());

    BufferRange range(0, uniform.size());
    if (&uniform == sBindUniform[slot] && range == sUniformRange[slot]) {
        return;
    }

    glBindBufferBase(GL_UNIFORM_BUFFER, slot, uniform.id());
    sBindUniform[slot] = &uniform;
    sUniformRange[slot] = range;
}

inline void Pipeline::bindUniform(int slot, const Buffer& uniform, size_t offset, size_t size) const noexcept
{
    SDL_assert((offset % uniformBufferOffsetAlignment()) == 0);
    SDL_assert(uniform.target() == GL_UNIFORM_BUFFER);
    SDL_assert(size > 0 && size <= uniform.size());
    SDL_assert(slot < sBindUniform.size());

    BufferRange range(offset, size);
    if (&uniform == sBindUniform[slot] && range == sUniformRange[slot]) {
        return;
    }

    glBindBufferRange(GL_UNIFORM_BUFFER, slot, uniform.id(), offset, size);
    sBindUniform[slot] = &uniform;
    sUniformRange[slot] = range;
}

inline void Pipeline::unbindFramebuffer() const noexcept
{
    if (sBindFramebuffer != nullptr) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        sBindFramebuffer = nullptr;
    }
}

inline void Pipeline::unbindVertexArray() const noexcept
{
    if (sBindVertexArray != nullptr) {
        glBindVertexArray(sDummyVAO);
        sBindVertexArray = nullptr;
    }
}

inline void Pipeline::unbindTexture(int slot) const noexcept
{
    if (sBindTexture[slot] != nullptr) {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(sBindTexture[slot]->target(), 0);
        sBindTexture[slot] = nullptr;
    }
}

inline void Pipeline::unbindStorage(int slot) const noexcept
{
    if (sBindStorage[slot] != nullptr) {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot, 0);
        sStorageRange[slot] = BufferRange();
        sBindStorage[slot] = nullptr;
    }
}

inline void Pipeline::unbindUniform(int slot) const noexcept
{
    if (sBindUniform[slot] != nullptr) {
        glBindBufferBase(GL_UNIFORM_BUFFER, slot, 0);
        sUniformRange[slot] = BufferRange();
        sBindUniform[slot] = nullptr;
    }
}

inline void Pipeline::useProgram(const Program& program) const noexcept
{
    if (&program != sUsedProgram) {
        glUseProgram(program.id());
        sUsedProgram = &program;
    }
}

inline void Pipeline::setUniformUint1(int location, uint32_t value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->setUint1(location, value);
}

inline void Pipeline::setUniformUint2(int location, const NX_IVec2& value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->setUint2(location, value);
}

inline void Pipeline::setUniformUint3(int location, const NX_IVec3& value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->setUint3(location, value);
}

inline void Pipeline::setUniformUint4(int location, const NX_IVec4& value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->setUint4(location, value);
}

inline void Pipeline::setUniformInt1(int location, int value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->setInt1(location, value);
}

inline void Pipeline::setUniformInt2(int location, const NX_IVec2& value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->setInt2(location, value);
}

inline void Pipeline::setUniformInt3(int location, const NX_IVec3& value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->setInt3(location, value);
}

inline void Pipeline::setUniformInt4(int location, const NX_IVec4& value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->setInt4(location, value);
}

inline void Pipeline::setUniformFloat1(int location, float value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->setFloat1(location, value);
}

inline void Pipeline::setUniformFloat2(int location, const NX_Vec2& value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->setFloat2(location, value);
}

inline void Pipeline::setUniformFloat3(int location, const NX_Vec3& value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->setFloat3(location, value);
}

inline void Pipeline::setUniformFloat3(int location, const NX_Color& value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->setFloat3(location, value);
}

inline void Pipeline::setUniformFloat4(int location, const NX_Vec4& value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->setFloat4(location, value);
}

inline void Pipeline::setUniformFloat4(int location, const NX_Quat& value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->setFloat4(location, value);
}

inline void Pipeline::setUniformFloat4(int location, const NX_Color& value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->setFloat4(location, value);
}

inline void Pipeline::setUniformMat3(int location, const NX_Mat3& value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->setMat3(location, value);
}

inline void Pipeline::setUniformMat3(int location, const NX_Mat4& value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->setMat3(location, value);
}

inline void Pipeline::setUniformMat4(int location, const NX_Mat4& value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->setMat4(location, value);
}

inline void Pipeline::setViewport(NX_IVec2 size) const noexcept
{
    glViewport(0, 0, size.x, size.y);
}

inline void Pipeline::setViewport(int x, int y, int w, int h) const noexcept
{
    glViewport(x, y, w, h);
}

inline void Pipeline::setViewport(const gpu::Framebuffer& dst) const noexcept
{
    SDL_assert(sBindFramebuffer == &dst && "Likely framebuffer management error");
    glViewport(0, 0, dst.width(), dst.height());
}

inline void Pipeline::clear(const gpu::Framebuffer& framebuffer, NX_Color color, float depth) noexcept
{
    SDL_assert(sBindFramebuffer == &framebuffer && "Likely framebuffer management error");

    for (int i = 0; i < framebuffer.colorAttachmentCount(); i++) {
        glClearBufferfv(GL_COLOR, i, reinterpret_cast<const float*>(&color));
    }

    if (framebuffer.getDepthAttachment().isValid()) {
        glClearBufferfv(GL_DEPTH, 0, &depth);
    }
}

inline void Pipeline::clearColor(std::initializer_list<std::pair<int, NX_Color>> attachments) const noexcept
{
    for (const auto& attachment : attachments) {
        glClearBufferfv(GL_COLOR, attachment.first, reinterpret_cast<const float*>(&attachment.second));
    }
}

inline void Pipeline::clearColor(int attachment, NX_Color color) const noexcept
{
    glClearBufferfv(GL_COLOR, attachment, reinterpret_cast<const float*>(&color));
}

inline void Pipeline::clearDepth(float depth) const noexcept
{
    glClearBufferfv(GL_DEPTH, 0, &depth);
}

inline void Pipeline::draw(GLenum mode, GLsizei count) const noexcept
{
    glDrawArrays(mode, 0, count);
}

inline void Pipeline::draw(GLenum mode, GLint first, GLsizei count) const noexcept
{
    glDrawArrays(mode, first, count);
}

inline void Pipeline::drawInstanced(GLenum mode, GLsizei count, GLsizei instanceCount) const noexcept
{
    glDrawArraysInstanced(mode, 0, count, instanceCount);
}

inline void Pipeline::drawInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instanceCount) const noexcept
{
    glDrawArraysInstanced(mode, first, count, instanceCount);
}

inline void Pipeline::drawElements(GLenum mode, GLenum type, GLsizei count) const noexcept
{
    glDrawElements(mode, count, type, nullptr);
}

inline void Pipeline::drawElements(GLenum mode, GLenum type, GLint first, GLsizei count) const noexcept
{
    size_t typeSize = 0;
    switch (type) {
        case GL_UNSIGNED_BYTE:  typeSize = 1; break;
        case GL_UNSIGNED_SHORT: typeSize = 2; break;
        case GL_UNSIGNED_INT:   typeSize = 4; break;
        default: break;
    }
    glDrawElements(mode, count, type, reinterpret_cast<const void*>(first * typeSize));
}

inline void Pipeline::drawElementsInstanced(GLenum mode, GLenum type, GLsizei count, GLsizei instanceCount) const noexcept
{
    glDrawElementsInstanced(mode, count, type, nullptr, instanceCount);
}

inline void Pipeline::drawElementsInstanced(GLenum mode, GLenum type, GLint first, GLsizei count, GLsizei instanceCount) const noexcept
{
    size_t typeSize = 0;
    switch (type) {
        case GL_UNSIGNED_BYTE:  typeSize = 1; break;
        case GL_UNSIGNED_SHORT: typeSize = 2; break;
        case GL_UNSIGNED_INT:   typeSize = 4; break;
        default: break;
    }
    glDrawElementsInstanced(mode, count, type, reinterpret_cast<const void*>(first * typeSize), instanceCount);
}

inline void Pipeline::drawArraysIndirect(GLenum mode, const void* indirect) const noexcept
{
    glDrawArraysIndirect(mode, indirect);
}

inline void Pipeline::drawElementsIndirect(GLenum mode, GLenum type, const void* indirect) const noexcept
{
    glDrawElementsIndirect(mode, type, indirect);
}

inline void Pipeline::dispatchCompute(GLuint numGroupsX, GLuint numGroupsY, GLuint numGroupsZ) const noexcept
{
    glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
}

inline void Pipeline::dispatchComputeIndirect(GLintptr indirect) const noexcept
{
    glDispatchComputeIndirect(indirect);
}

inline void Pipeline::blitToBackBuffer(const gpu::Framebuffer& src, int xDst, int yDst, int wDst, int hDst, bool linear) noexcept
{
    NX_IVec2 srcSize = src.dimensions();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, src.resolveId());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glReadBuffer(GL_COLOR_ATTACHMENT0);

    glBlitFramebuffer(
        0, 0, srcSize.x, srcSize.y,
        xDst, yDst, xDst + wDst, yDst + hDst,
        GL_COLOR_BUFFER_BIT, linear ? GL_NEAREST : GL_LINEAR
    );

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glReadBuffer(GL_BACK);
}

inline void Pipeline::memoryBarrier(GLbitfield barriers) noexcept
{
    glMemoryBarrier(barriers);
}

inline int Pipeline::uniformBufferOffsetAlignment() noexcept
{
    static int value{-1};

    if (value < 0) {
        glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &value);
    }

    return value;
}

inline int Pipeline::storageBufferOffsetAlignment() noexcept
{
    static int value{-1};

    if (value < 0) {
        glGetIntegerv(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &value);
    }

    return value;
}

inline int Pipeline::maxUniformBufferSize() noexcept
{
    static int value{-1};

    if (value < 0) {
        glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &value);
    }

    return value;
}

inline int Pipeline::maxStorageBufferSize() noexcept
{
    static int value{-1};

    if (value < 0) {
        glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &value);
    }

    return value;
}

/* === Private Implementation === */

template <typename F>
    requires std::invocable<F>
inline void Pipeline::withFramebufferBind(GLuint id, F&& func) noexcept
{
#ifndef NDEBUG
    DebugExclusiveBindGuard<GL_FRAMEBUFFER> guard;
#endif

    GLuint prevFBO = 0;
    if (sCurrentlyInstanced && sBindFramebuffer != nullptr) {
        prevFBO = sBindFramebuffer->renderId();
    }

    if (prevFBO != id) {
        glBindFramebuffer(GL_FRAMEBUFFER, id);
    }

    func();

    if (prevFBO != id) {
        glBindFramebuffer(GL_FRAMEBUFFER, prevFBO);
    }
}

template <typename F>
    requires std::invocable<F>
inline void Pipeline::withVertexArrayBind(GLuint id, F&& func) noexcept
{
#ifndef NDEBUG
    DebugExclusiveBindGuard<GL_VERTEX_ARRAY> guard;
#endif

    GLuint prevVAO = sDummyVAO;
    if (sCurrentlyInstanced && sBindVertexArray != nullptr) {
        prevVAO = sBindVertexArray->id();
    }

    if (prevVAO != id) {
        glBindVertexArray(id);
    }

    func();

    if (prevVAO != id) {
        glBindVertexArray(prevVAO);
    }
}

template <typename F>
    requires std::invocable<F>
inline void Pipeline::withTextureBind(GLenum target, GLuint id, F&& func) noexcept
{
#ifndef NDEBUG
    DebugExclusiveBindGuard<GL_TEXTURE> guard;
#endif

    bool freeSlotFound = false;

    if (sCurrentlyInstanced) {
        for (int slot = 0; slot < sBindTexture.size(); ++slot) {
            if (sBindTexture[slot] == nullptr) {
                glActiveTexture(GL_TEXTURE0 + slot);
                freeSlotFound = true;
                break;
            }
        }
        if (!freeSlotFound) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(sBindTexture[0]->target(), 0);
        }
    }

    glBindTexture(target, id);
    func();
    glBindTexture(target, 0);

    if (sCurrentlyInstanced && !freeSlotFound) {
        glBindTexture(sBindTexture[0]->target(), sBindTexture[0]->id());
    }
}

template <typename F>
    requires std::invocable<F>
inline void Pipeline::withBufferBind(GLenum target, GLuint id, F&& func) noexcept
{
#ifndef NDEBUG
    DebugExclusiveBindGuard<GL_BUFFER> guard;
#endif

    GLuint prevVAO = 0;
    if (sCurrentlyInstanced && (target == GL_ARRAY_BUFFER || target == GL_ELEMENT_ARRAY_BUFFER)) {
        prevVAO = (sBindVertexArray != nullptr) ? sBindVertexArray->id() : sDummyVAO;
        glBindVertexArray(0);
    }

    glBindBuffer(target, id);
    func();
    glBindBuffer(target, 0);

    if (prevVAO > 0) {
        glBindVertexArray(prevVAO);
    }
}

inline void Pipeline::setColorWrite_Internal(ColorWrite mode) const noexcept
{
    switch (mode) {
    case ColorWrite::Disabled:
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        break;
    case ColorWrite::RGB:
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
        break;
    case ColorWrite::RGBA:
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        break;
    }
}

inline void Pipeline::setDepthMode_Internal(DepthMode mode) const noexcept
{
    switch (mode) {
    case DepthMode::Disabled:
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        break;
    case DepthMode::TestOnly:
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        break;
    case DepthMode::WriteOnly:
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        break;
    case DepthMode::TestAndWrite:
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        break;
    }
}

inline void Pipeline::setDepthFunc_Internal(DepthFunc func) const noexcept
{
    GLenum glFunc;
    switch (func) {
    case DepthFunc::Never:        glFunc = GL_NEVER; break;
    case DepthFunc::Less:         glFunc = GL_LESS; break;
    case DepthFunc::Equal:        glFunc = GL_EQUAL; break;
    case DepthFunc::LessEqual:    glFunc = GL_LEQUAL; break;
    case DepthFunc::Greater:      glFunc = GL_GREATER; break;
    case DepthFunc::NotEqual:     glFunc = GL_NOTEQUAL; break;
    case DepthFunc::GreaterEqual: glFunc = GL_GEQUAL; break;
    case DepthFunc::Always:       glFunc = GL_ALWAYS; break;
    }
    glDepthFunc(glFunc);
}

inline void Pipeline::setBlendMode_Internal(BlendMode mode) const noexcept
{
    switch (mode) {
    case BlendMode::Disabled:
        glDisable(GL_BLEND);
        break;
    case BlendMode::Alpha:
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);
        break;
    case BlendMode::Premultiplied:
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);
        break;
    case BlendMode::AddAlpha:
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glBlendEquation(GL_FUNC_ADD);
        break;
    case BlendMode::Additive:
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glBlendEquation(GL_FUNC_ADD);
        break;
    case BlendMode::Multiply:
        glEnable(GL_BLEND);
        glBlendFunc(GL_DST_COLOR, GL_ZERO);
        glBlendEquation(GL_FUNC_ADD);
        break;
    case BlendMode::Subtract:
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
        break;
    case BlendMode::Minimum:
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glBlendEquation(GL_MIN);
        break;
    }
}

inline void Pipeline::setCullMode_Internal(CullMode mode) const noexcept
{
    switch (mode) {
    case CullMode::Disabled:
        glDisable(GL_CULL_FACE);
        break;
    case CullMode::Front:
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        break;
    case CullMode::Back:
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        break;
    case CullMode::FrontAndBack:
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT_AND_BACK);
        break;
    }
}

} // namespace gpu

#endif // NX_GPU_PIPELINE_HPP
