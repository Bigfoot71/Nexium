#ifndef NX_GPU_CHECK_ERROR_HPP
#define NX_GPU_CHECK_ERROR_HPP

#include <NX/NX_Log.h>
#include <glad/gles2.h>

namespace gpu {

/* === Declaration === */

class CheckError {
public:
    CheckError(const char* msg);

private:
    static const char* GetName(GLenum err);
};

/* === Public Implementation === */

inline CheckError::CheckError(const char* msg)
{
    GLenum err;
    int count = 0;
    while ((err = glGetError()) != GL_NO_ERROR) {
        if (count == 0) {
            NX_LogE("GPU: OpenGL error(s) detected: %s", msg);
        }
        NX_LogE("GPU: [%d] Error 0x%04X: %s", count, err, GetName(err));
        count++;
    }
}

/* === Private Implementation === */

inline const char* CheckError::GetName(GLenum err)
{
    switch (err) {
    case GL_NO_ERROR: return "GL_NO_ERROR";
    case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
    case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
    case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
    case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW";
    case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW";
    default: return "Unknown OpenGL error";
    }
}

} // namespace gpu

#endif // NX_GPU_CHECK_ERROR_HPP
