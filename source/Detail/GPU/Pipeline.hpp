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

    void SetColorWrite(ColorWrite mode) const noexcept;
    void SetDepthMode(DepthMode mode) const noexcept;
    void SetDepthFunc(DepthFunc func) const noexcept;
    void SetBlendMode(BlendMode mode) const noexcept;
    void SetCullMode(CullMode mode) const noexcept;

    void BindFramebuffer(const Framebuffer& framebuffer) const noexcept;
    void BindVertexArray(const VertexArray& vertexArray) const noexcept;
    void BindTexture(int slot, const Texture& texture) const noexcept;
    void BindStorage(int slot, const Buffer& storage) const noexcept;
    void BindStorage(int slot, const Buffer& storage, size_t offset, size_t size) const noexcept;
    void BindUniform(int slot, const Buffer& uniform) const noexcept;
    void BindUniform(int slot, const Buffer& uniform, size_t offset, size_t size) const noexcept;

    void UnbindFramebuffer() const noexcept;
    void UnbindVertexArray() const noexcept;
    void UnbindTexture(int slot) const noexcept;
    void UnbindStorage(int slot) const noexcept;
    void UnbindUniform(int slot) const noexcept;

    void UseProgram(const Program& program) const noexcept;

    void SetUniformUint1(int location, uint32_t value) const noexcept;
    void SetUniformUint2(int location, const NX_IVec2& value) const noexcept;
    void SetUniformUint3(int location, const NX_IVec3& value) const noexcept;
    void SetUniformUint4(int location, const NX_IVec4& value) const noexcept;
    void SetUniformInt1(int location, int value) const noexcept;
    void SetUniformInt2(int location, const NX_IVec2& value) const noexcept;
    void SetUniformInt3(int location, const NX_IVec3& value) const noexcept;
    void SetUniformInt4(int location, const NX_IVec4& value) const noexcept;
    void SetUniformFloat1(int location, float value) const noexcept;
    void SetUniformFloat2(int location, const NX_Vec2& value) const noexcept;
    void SetUniformFloat3(int location, const NX_Vec3& value) const noexcept;
    void SetUniformFloat3(int location, const NX_Color& value) const noexcept;
    void SetUniformFloat4(int location, const NX_Vec4& value) const noexcept;
    void SetUniformFloat4(int location, const NX_Quat& value) const noexcept;
    void SetUniformFloat4(int location, const NX_Color& value) const noexcept;
    void SetUniformMat3(int location, const NX_Mat3& value) const noexcept;
    void SetUniformMat3(int location, const NX_Mat4& value) const noexcept;
    void SetUniformMat4(int location, const NX_Mat4& value) const noexcept;

    void SetViewport(NX_IVec2 size) const noexcept;
    void SetViewport(int x, int y, int w, int h) const noexcept;
    void SetViewport(const gpu::Framebuffer& dst) const noexcept;

    void Clear(const gpu::Framebuffer& framebuffer, NX_Color color = NX_BLACK, float depth = 1.0) noexcept;
    void ClearColor(std::initializer_list<std::pair<int, NX_Color>> attachments) const noexcept;
    void ClearColor(int attachment, NX_Color color) const noexcept;
    void ClearDepth(float depth) const noexcept;

    void Draw(GLenum mode, GLsizei count) const noexcept;
    void Draw(GLenum mode, GLint first, GLsizei count) const noexcept;

    void DrawInstanced(GLenum mode, GLsizei count, GLsizei instanceCount) const noexcept;
    void DrawInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instanceCount) const noexcept;

    void DrawElements(GLenum mode, GLenum type, GLsizei count) const noexcept;
    void DrawElements(GLenum mode, GLenum type, GLint first, GLsizei count) const noexcept;

    void DrawElementsInstanced(GLenum mode, GLenum type, GLsizei count, GLsizei instanceCount) const noexcept;
    void DrawElementsInstanced(GLenum mode, GLenum type, GLint first, GLsizei count, GLsizei instanceCount) const noexcept;

    void DrawArraysIndirect(GLenum mode, const void* indirect) const noexcept;
    void DrawElementsIndirect(GLenum mode, GLenum type, const void* indirect) const noexcept;

    void DispatchCompute(GLuint numGroupsX, GLuint numGroupsY, GLuint numGroupsZ) const noexcept;
    void DispatchComputeIndirect(GLintptr indirect) const noexcept;

public:
    /** Non-instantiated operations */
    static void BlitToBackBuffer(const gpu::Framebuffer& src, int xDst, int yDst, int wDst, int hDst, bool linear) noexcept;
    static void MemoryBarrier(GLbitfield barriers) noexcept;

    /** Hardware info getters */
    static int GetUniformBufferOffsetAlignment() noexcept;
    static int GetStorageBufferOffsetAlignment() noexcept;
    static int GetMaxUniformBufferSize() noexcept;
    static int GetMaxStorageBufferSize() noexcept;

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
    static void WithFramebufferBind(GLuint id, F&& func) noexcept;

    template <typename F>
        requires std::invocable<F>
    static void WithVertexArrayBind(GLuint id, F&& func) noexcept;

    template <typename F>
        requires std::invocable<F>
    static void WithTextureBind(GLenum target, GLuint id, F&& func) noexcept;

    template <typename F>
        requires std::invocable<F>
    static void WithBufferBind(GLenum target, GLuint id, F&& func) noexcept;

private:
    void SetColorWrite_Internal(ColorWrite mode) const noexcept;
    void SetDepthMode_Internal(DepthMode mode) const noexcept;
    void SetDepthFunc_Internal(DepthFunc func) const noexcept;
    void SetBlendMode_Internal(BlendMode mode) const noexcept;
    void SetCullMode_Internal(CullMode mode) const noexcept;

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

        SetColorWrite_Internal(InitialColorWrite);
        SetDepthMode_Internal(InitialDepthMode);
        SetDepthFunc_Internal(InitialDepthFunc);
        SetBlendMode_Internal(InitialBlendMode);
        SetCullMode_Internal(InitialCullMode);

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
        SetColorWrite_Internal(InitialColorWrite);
        sCurrentColorWrite = InitialColorWrite;
    }
    if (sCurrentDepthMode != InitialDepthMode) {
        SetDepthMode_Internal(InitialDepthMode);
        sCurrentDepthMode = InitialDepthMode;
    }
    if (sCurrentDepthFunc != InitialDepthFunc) {
        SetDepthFunc_Internal(InitialDepthFunc);
        sCurrentDepthFunc = InitialDepthFunc;
    }
    if (sCurrentBlendMode != InitialBlendMode) {
        SetBlendMode_Internal(InitialBlendMode);
        sCurrentBlendMode = InitialBlendMode;
    }
    if (sCurrentCullMode != InitialCullMode) {
        SetCullMode_Internal(InitialCullMode);
        sCurrentCullMode = InitialCullMode;
    }
    for (int slot = 0; slot < sBindTexture.size(); ++slot) {
        if (sBindTexture[slot] != nullptr) {
            glActiveTexture(GL_TEXTURE0 + slot);
            glBindTexture(sBindTexture[slot]->GetTarget(), 0);
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

inline void Pipeline::SetColorWrite(ColorWrite mode) const noexcept
{
    if (mode != sCurrentColorWrite) {
        SetColorWrite_Internal(mode);
        sCurrentColorWrite = mode;
    }
}

inline void Pipeline::SetDepthMode(DepthMode mode) const noexcept
{
    if (mode != sCurrentDepthMode) {
        SetDepthMode_Internal(mode);
        sCurrentDepthMode = mode;
    }
}

inline void Pipeline::SetDepthFunc(DepthFunc func) const noexcept
{
    if (func != sCurrentDepthFunc) {
        SetDepthFunc_Internal(func);
        sCurrentDepthFunc = func;
    }
}

inline void Pipeline::SetBlendMode(BlendMode mode) const noexcept
{
    if (mode != sCurrentBlendMode) {
        SetBlendMode_Internal(mode);
        sCurrentBlendMode = mode;
    }
}

inline void Pipeline::SetCullMode(CullMode mode) const noexcept
{
    if (mode != sCurrentCullMode) {
        SetCullMode_Internal(mode);
        sCurrentCullMode = mode;
    }
}

inline void Pipeline::BindFramebuffer(const Framebuffer& framebuffer) const noexcept
{
    if (&framebuffer != sBindFramebuffer) {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.GetRenderId());
        sBindFramebuffer = &framebuffer;
    }
}

inline void Pipeline::BindVertexArray(const VertexArray& vertexArray) const noexcept
{
    if (&vertexArray != sBindVertexArray) {
        glBindVertexArray(vertexArray.GetID());
        sBindVertexArray = &vertexArray;
    }
}

inline void Pipeline::BindTexture(int slot, const Texture& texture) const noexcept
{
    SDL_assert(slot < sBindTexture.size());

    if (&texture == sBindTexture[slot]) {
        return;
    }

    glActiveTexture(GL_TEXTURE0 + slot);

    if (sBindTexture[slot] != nullptr && sBindTexture[slot]->GetTarget() != texture.GetTarget()) {
        glBindTexture(sBindTexture[slot]->GetTarget(), 0);
    }

    glBindTexture(texture.GetTarget(), texture.GetID());
    sBindTexture[slot] = &texture;
}

inline void Pipeline::BindStorage(int slot, const Buffer& storage) const noexcept
{
    SDL_assert(storage.GetTarget() == GL_SHADER_STORAGE_BUFFER);
    SDL_assert(slot < sBindStorage.size());

    BufferRange range(0, storage.GetSize());
    if (&storage == sBindStorage[slot] && range == sStorageRange[slot]) {
        return;
    }

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot, storage.GetID());
    sBindStorage[slot] = &storage;
    sStorageRange[slot] = range;
}

inline void Pipeline::BindStorage(int slot, const Buffer& storage, size_t offset, size_t size) const noexcept
{
    SDL_assert((offset % GetStorageBufferOffsetAlignment()) == 0);
    SDL_assert(storage.GetTarget() == GL_SHADER_STORAGE_BUFFER);
    SDL_assert(size > 0 && size <= storage.GetSize());
    SDL_assert(slot < sBindStorage.size());

    BufferRange range(offset, size);
    if (&storage == sBindStorage[slot] && range == sStorageRange[slot]) {
        return;
    }

    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, slot, storage.GetID(), offset, size);
    sBindStorage[slot] = &storage;
    sStorageRange[slot] = range;
}

inline void Pipeline::BindUniform(int slot, const Buffer& uniform) const noexcept
{
    SDL_assert(uniform.GetTarget() == GL_UNIFORM_BUFFER);
    SDL_assert(slot < sBindUniform.size());

    BufferRange range(0, uniform.GetSize());
    if (&uniform == sBindUniform[slot] && range == sUniformRange[slot]) {
        return;
    }

    glBindBufferBase(GL_UNIFORM_BUFFER, slot, uniform.GetID());
    sBindUniform[slot] = &uniform;
    sUniformRange[slot] = range;
}

inline void Pipeline::BindUniform(int slot, const Buffer& uniform, size_t offset, size_t size) const noexcept
{
    SDL_assert((offset % GetUniformBufferOffsetAlignment()) == 0);
    SDL_assert(uniform.GetTarget() == GL_UNIFORM_BUFFER);
    SDL_assert(size > 0 && size <= uniform.GetSize());
    SDL_assert(slot < sBindUniform.size());

    BufferRange range(offset, size);
    if (&uniform == sBindUniform[slot] && range == sUniformRange[slot]) {
        return;
    }

    glBindBufferRange(GL_UNIFORM_BUFFER, slot, uniform.GetID(), offset, size);
    sBindUniform[slot] = &uniform;
    sUniformRange[slot] = range;
}

inline void Pipeline::UnbindFramebuffer() const noexcept
{
    if (sBindFramebuffer != nullptr) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        sBindFramebuffer = nullptr;
    }
}

inline void Pipeline::UnbindVertexArray() const noexcept
{
    if (sBindVertexArray != nullptr) {
        glBindVertexArray(sDummyVAO);
        sBindVertexArray = nullptr;
    }
}

inline void Pipeline::UnbindTexture(int slot) const noexcept
{
    if (sBindTexture[slot] != nullptr) {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(sBindTexture[slot]->GetTarget(), 0);
        sBindTexture[slot] = nullptr;
    }
}

inline void Pipeline::UnbindStorage(int slot) const noexcept
{
    if (sBindStorage[slot] != nullptr) {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot, 0);
        sStorageRange[slot] = BufferRange();
        sBindStorage[slot] = nullptr;
    }
}

inline void Pipeline::UnbindUniform(int slot) const noexcept
{
    if (sBindUniform[slot] != nullptr) {
        glBindBufferBase(GL_UNIFORM_BUFFER, slot, 0);
        sUniformRange[slot] = BufferRange();
        sBindUniform[slot] = nullptr;
    }
}

inline void Pipeline::UseProgram(const Program& program) const noexcept
{
    if (&program != sUsedProgram) {
        glUseProgram(program.GetID());
        sUsedProgram = &program;
    }
}

inline void Pipeline::SetUniformUint1(int location, uint32_t value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->SetUint1(location, value);
}

inline void Pipeline::SetUniformUint2(int location, const NX_IVec2& value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->SetUint2(location, value);
}

inline void Pipeline::SetUniformUint3(int location, const NX_IVec3& value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->SetUint3(location, value);
}

inline void Pipeline::SetUniformUint4(int location, const NX_IVec4& value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->SetUint4(location, value);
}

inline void Pipeline::SetUniformInt1(int location, int value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->SetInt1(location, value);
}

inline void Pipeline::SetUniformInt2(int location, const NX_IVec2& value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->SetInt2(location, value);
}

inline void Pipeline::SetUniformInt3(int location, const NX_IVec3& value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->SetInt3(location, value);
}

inline void Pipeline::SetUniformInt4(int location, const NX_IVec4& value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->SetInt4(location, value);
}

inline void Pipeline::SetUniformFloat1(int location, float value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->SetFloat1(location, value);
}

inline void Pipeline::SetUniformFloat2(int location, const NX_Vec2& value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->SetFloat2(location, value);
}

inline void Pipeline::SetUniformFloat3(int location, const NX_Vec3& value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->SetFloat3(location, value);
}

inline void Pipeline::SetUniformFloat3(int location, const NX_Color& value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->SetFloat3(location, value);
}

inline void Pipeline::SetUniformFloat4(int location, const NX_Vec4& value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->SetFloat4(location, value);
}

inline void Pipeline::SetUniformFloat4(int location, const NX_Quat& value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->SetFloat4(location, value);
}

inline void Pipeline::SetUniformFloat4(int location, const NX_Color& value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->SetFloat4(location, value);
}

inline void Pipeline::SetUniformMat3(int location, const NX_Mat3& value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->SetMat3(location, value);
}

inline void Pipeline::SetUniformMat3(int location, const NX_Mat4& value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->SetMat3(location, value);
}

inline void Pipeline::SetUniformMat4(int location, const NX_Mat4& value) const noexcept
{
    SDL_assert(sUsedProgram != nullptr);
    sUsedProgram->SetMat4(location, value);
}

inline void Pipeline::SetViewport(NX_IVec2 size) const noexcept
{
    glViewport(0, 0, size.x, size.y);
}

inline void Pipeline::SetViewport(int x, int y, int w, int h) const noexcept
{
    glViewport(x, y, w, h);
}

inline void Pipeline::SetViewport(const gpu::Framebuffer& dst) const noexcept
{
    SDL_assert(sBindFramebuffer == &dst && "Likely framebuffer management error"); // NOLINT
    glViewport(0, 0, dst.GetWidth(), dst.GetHeight());
}

inline void Pipeline::Clear(const gpu::Framebuffer& framebuffer, NX_Color color, float depth) noexcept
{
    SDL_assert(sBindFramebuffer == &framebuffer && "Likely framebuffer management error"); // NOLINT

    for (int i = 0; i < framebuffer.GetColorAttachmentCount(); i++) {
        glClearBufferfv(GL_COLOR, i, reinterpret_cast<const float*>(&color));
    }

    if (framebuffer.GetDepthAttachment().IsValid()) {
        glClearBufferfv(GL_DEPTH, 0, &depth);
    }
}

inline void Pipeline::ClearColor(std::initializer_list<std::pair<int, NX_Color>> attachments) const noexcept
{
    for (const auto& attachment : attachments) {
        glClearBufferfv(GL_COLOR, attachment.first, reinterpret_cast<const float*>(&attachment.second));
    }
}

inline void Pipeline::ClearColor(int attachment, NX_Color color) const noexcept
{
    glClearBufferfv(GL_COLOR, attachment, reinterpret_cast<const float*>(&color));
}

inline void Pipeline::ClearDepth(float depth) const noexcept
{
    glClearBufferfv(GL_DEPTH, 0, &depth);
}

inline void Pipeline::Draw(GLenum mode, GLsizei count) const noexcept
{
    glDrawArrays(mode, 0, count);
}

inline void Pipeline::Draw(GLenum mode, GLint first, GLsizei count) const noexcept
{
    glDrawArrays(mode, first, count);
}

inline void Pipeline::DrawInstanced(GLenum mode, GLsizei count, GLsizei instanceCount) const noexcept
{
    glDrawArraysInstanced(mode, 0, count, instanceCount);
}

inline void Pipeline::DrawInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instanceCount) const noexcept
{
    glDrawArraysInstanced(mode, first, count, instanceCount);
}

inline void Pipeline::DrawElements(GLenum mode, GLenum type, GLsizei count) const noexcept
{
    glDrawElements(mode, count, type, nullptr);
}

inline void Pipeline::DrawElements(GLenum mode, GLenum type, GLint first, GLsizei count) const noexcept
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

inline void Pipeline::DrawElementsInstanced(GLenum mode, GLenum type, GLsizei count, GLsizei instanceCount) const noexcept
{
    glDrawElementsInstanced(mode, count, type, nullptr, instanceCount);
}

inline void Pipeline::DrawElementsInstanced(GLenum mode, GLenum type, GLint first, GLsizei count, GLsizei instanceCount) const noexcept
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

inline void Pipeline::DrawArraysIndirect(GLenum mode, const void* indirect) const noexcept
{
    glDrawArraysIndirect(mode, indirect);
}

inline void Pipeline::DrawElementsIndirect(GLenum mode, GLenum type, const void* indirect) const noexcept
{
    glDrawElementsIndirect(mode, type, indirect);
}

inline void Pipeline::DispatchCompute(GLuint numGroupsX, GLuint numGroupsY, GLuint numGroupsZ) const noexcept
{
    glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
}

inline void Pipeline::DispatchComputeIndirect(GLintptr indirect) const noexcept
{
    glDispatchComputeIndirect(indirect);
}

inline void Pipeline::BlitToBackBuffer(const gpu::Framebuffer& src, int xDst, int yDst, int wDst, int hDst, bool linear) noexcept
{
    NX_IVec2 srcSize = src.GetDimensions();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, src.GetResolveId());
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

inline void Pipeline::MemoryBarrier(GLbitfield barriers) noexcept
{
    glMemoryBarrier(barriers);
}

inline int Pipeline::GetUniformBufferOffsetAlignment() noexcept
{
    static int value{-1};

    if (value < 0) {
        glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &value);
    }

    return value;
}

inline int Pipeline::GetStorageBufferOffsetAlignment() noexcept
{
    static int value{-1};

    if (value < 0) {
        glGetIntegerv(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &value);
    }

    return value;
}

inline int Pipeline::GetMaxUniformBufferSize() noexcept
{
    static int value{-1};

    if (value < 0) {
        glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &value);
    }

    return value;
}

inline int Pipeline::GetMaxStorageBufferSize() noexcept
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
inline void Pipeline::WithFramebufferBind(GLuint id, F&& func) noexcept
{
#ifndef NDEBUG
    DebugExclusiveBindGuard<GL_FRAMEBUFFER> guard;
#endif

    GLuint prevFBO = 0;
    if (sCurrentlyInstanced && sBindFramebuffer != nullptr) {
        prevFBO = sBindFramebuffer->GetRenderId();
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
inline void Pipeline::WithVertexArrayBind(GLuint id, F&& func) noexcept
{
#ifndef NDEBUG
    DebugExclusiveBindGuard<GL_VERTEX_ARRAY> guard;
#endif

    GLuint prevVAO = sDummyVAO;
    if (sCurrentlyInstanced && sBindVertexArray != nullptr) {
        prevVAO = sBindVertexArray->GetID();
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
inline void Pipeline::WithTextureBind(GLenum target, GLuint id, F&& func) noexcept
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
            glBindTexture(sBindTexture[0]->GetTarget(), 0);
        }
    }

    glBindTexture(target, id);
    func();
    glBindTexture(target, 0);

    if (sCurrentlyInstanced && !freeSlotFound) {
        glBindTexture(sBindTexture[0]->GetTarget(), sBindTexture[0]->GetID());
    }
}

template <typename F>
    requires std::invocable<F>
inline void Pipeline::WithBufferBind(GLenum target, GLuint id, F&& func) noexcept
{
#ifndef NDEBUG
    DebugExclusiveBindGuard<GL_BUFFER> guard;
#endif

    GLuint prevVAO = 0;
    if (sCurrentlyInstanced && (target == GL_ARRAY_BUFFER || target == GL_ELEMENT_ARRAY_BUFFER)) {
        prevVAO = (sBindVertexArray != nullptr) ? sBindVertexArray->GetID() : sDummyVAO;
        glBindVertexArray(0);
    }

    glBindBuffer(target, id);
    func();
    glBindBuffer(target, 0);

    if (prevVAO > 0) {
        glBindVertexArray(prevVAO);
    }
}

inline void Pipeline::SetColorWrite_Internal(ColorWrite mode) const noexcept
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

inline void Pipeline::SetDepthMode_Internal(DepthMode mode) const noexcept
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

inline void Pipeline::SetDepthFunc_Internal(DepthFunc func) const noexcept
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

inline void Pipeline::SetBlendMode_Internal(BlendMode mode) const noexcept
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

inline void Pipeline::SetCullMode_Internal(CullMode mode) const noexcept
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
