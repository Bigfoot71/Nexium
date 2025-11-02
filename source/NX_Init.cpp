/* NX_Init.cpp -- API definition for Nexium initialization
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/NX_Init.h>
#include <NX/NX_Rand.h>
#include <NX/NX_Log.h>

#include "./INX_GPUProgramCache.hpp"
#include "./INX_GlobalAssets.hpp"
#include "./INX_GlobalState.hpp"
#include "./INX_PoolAssets.hpp"

#include "./NX_Render2D.hpp"
#include "./NX_Audio.hpp"

#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <glad/gles2.h>
#include <physfs.h>
#include <alc.h>

#include "./Render/NX_RenderState.hpp"

#include <memory>

// ============================================================================
// GLOBAL STATES (NOTE: Will be withdrawn eventually)
// ============================================================================

util::UniquePtr<NX_RenderState> gRender;

// ============================================================================
// PHYSFS COMPATIBILITY
// ============================================================================

static void* INX_PhysFS_malloc(PHYSFS_uint64 size)
{
    return SDL_malloc(static_cast<size_t>(size));
}

static void* INX_PhysFS_realloc(void* ptr, PHYSFS_uint64 size)
{
    return SDL_realloc(ptr, static_cast<size_t>(size));
}

// ============================================================================
// LOCAL FUNCTIONS
// ============================================================================

static bool INX_SDL_Init(const NX_AppDesc& desc)
{
    SDL_SetLogPriorityPrefix(SDL_LOG_PRIORITY_TRACE,    "[T] ");
    SDL_SetLogPriorityPrefix(SDL_LOG_PRIORITY_VERBOSE,  "[V] ");
    SDL_SetLogPriorityPrefix(SDL_LOG_PRIORITY_DEBUG,    "[D] ");
    SDL_SetLogPriorityPrefix(SDL_LOG_PRIORITY_INFO,     "[I] ");
    SDL_SetLogPriorityPrefix(SDL_LOG_PRIORITY_WARN,     "[W] ");
    SDL_SetLogPriorityPrefix(SDL_LOG_PRIORITY_ERROR,    "[E] ");
    SDL_SetLogPriorityPrefix(SDL_LOG_PRIORITY_CRITICAL, "[F] ");

    if (desc.memory.malloc && desc.memory.calloc && desc.memory.realloc && desc.memory.free) {
        if (!SDL_SetMemoryFunctions(desc.memory.malloc, desc.memory.calloc, desc.memory.realloc, desc.memory.free)) {
            NX_LOG(W, "CORE: Failed to set custom memory functions; %s", SDL_GetError());
        }
    }
    else if (desc.memory.malloc || desc.memory.calloc || desc.memory.realloc || desc.memory.free) {
        NX_LOG(W, "CORE: Failed to set custom memory functions; If you define at least one memory function, they must all be defined", SDL_GetError());
    }

    SDL_SetAppMetadata(desc.name, desc.version, desc.identifier);

    if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
        NX_LogF("CORE: Failed to init video subsystem; ", SDL_GetError());
        return false;
    }

#ifndef NDEBUG
    SDL_SetLogPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE);
#endif

    return true;
}

static bool INX_PhysFS_Init()
{
    static constexpr PHYSFS_Allocator Allocator = {
        .Init = nullptr,
        .Deinit = nullptr,
        .Malloc = INX_PhysFS_malloc,
        .Realloc = INX_PhysFS_realloc,
        .Free = SDL_free
    };

    if (PHYSFS_setAllocator(&Allocator) == 0) {
        return false;
    }

    if (PHYSFS_init(nullptr) == 0) {
        return false;
    }

    if (PHYSFS_mount(SDL_GetBasePath(), "/", 1) == 0) {
        return false;
    }

    return true;
}

static SDL_WindowFlags INX_GetWindowFlags(NX_Flags flags)
{
    SDL_WindowFlags windowFlags = 0;
    if (flags & NX_FLAG_FULLSCREEN) windowFlags |= SDL_WINDOW_FULLSCREEN;
    if (flags & NX_FLAG_WINDOW_OCCLUDED) windowFlags |= SDL_WINDOW_OCCLUDED;
    if (flags & NX_FLAG_WINDOW_HIDDEN) windowFlags |= SDL_WINDOW_HIDDEN;
    if (flags & NX_FLAG_WINDOW_BORDERLESS) windowFlags |= SDL_WINDOW_BORDERLESS;
    if (flags & NX_FLAG_WINDOW_RESIZABLE) windowFlags |= SDL_WINDOW_RESIZABLE;
    if (flags & NX_FLAG_WINDOW_MINIMIZED) windowFlags |= SDL_WINDOW_MINIMIZED;
    if (flags & NX_FLAG_WINDOW_MAXIMIZED) windowFlags |= SDL_WINDOW_MAXIMIZED;
    if (flags & NX_FLAG_WINDOW_TOPMOST) windowFlags |= SDL_WINDOW_ALWAYS_ON_TOP;
    if (flags & NX_FLAG_WINDOW_TRANSPARENT) windowFlags |= SDL_WINDOW_TRANSPARENT;
    if (flags & NX_FLAG_WINDOW_NOT_FOCUSABLE) windowFlags |= SDL_WINDOW_NOT_FOCUSABLE;
    if (flags & NX_FLAG_MOUSE_GRABBED) windowFlags |= SDL_WINDOW_MOUSE_GRABBED;
    if (flags & NX_FLAG_MOUSE_CAPTURE) windowFlags |= SDL_WINDOW_MOUSE_CAPTURE;
    if (flags & NX_FLAG_MOUSE_RELATIVE) windowFlags |= SDL_WINDOW_MOUSE_RELATIVE_MODE;
    if (flags & NX_FLAG_MOUSE_FOCUS) windowFlags |= SDL_WINDOW_MOUSE_FOCUS;
    if (flags & NX_FLAG_INPUT_FOCUS) windowFlags |= SDL_WINDOW_INPUT_FOCUS;
    if (flags & NX_FLAG_KEYBOARD_GRABBED) windowFlags |= SDL_WINDOW_KEYBOARD_GRABBED;
    if (flags & NX_FLAG_HIGH_PIXEL_DENSITY) windowFlags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;
    return windowFlags;
}

static bool INX_DisplayState_Init(const char* title, int w, int h, const NX_AppDesc& desc)
{
    SDL_memset(&INX_Display, 0, sizeof(INX_Display));

    /* --- Test OpenGL support and define attributes --- */

    bool useOpenGLES = false;

    // Common OpenGL attributes
    auto setCommonGLAttributes = []() {
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 0);
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
    };

#if defined(NX_PLATFORM_MACOS) || defined(NX_PLATFORM_ANDROID)
    // Android always uses OpenGL ES
    // Hopefully MacOS via ANGLE
    useOpenGLES = true;
#else
    // Test OpenGL 4.5 support on other platforms
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    setCommonGLAttributes();

    SDL_Window* testWindow = SDL_CreateWindow(nullptr, 1, 1, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    if (testWindow != nullptr) {
        SDL_GLContext testContext = SDL_GL_CreateContext(testWindow);
        if (testContext == nullptr) {
            NX_LOG(W, "CORE: OpenGL 4.5 not supported, falling back to OpenGL ES 3.2");
            useOpenGLES = true;
        }
        else {
            SDL_GL_DestroyContext(testContext);
        }
        SDL_DestroyWindow(testWindow);
    }
    else {
        useOpenGLES = true;
    }
#endif

    // Set final OpenGL attributes
    if (useOpenGLES) {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    }
    else {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    }
    setCommonGLAttributes();

    /* --- Create the SDL window --- */

    INX_Display.window = SDL_CreateWindow(title, w, h, SDL_WINDOW_OPENGL | INX_GetWindowFlags(desc.flags));
    if (INX_Display.window == nullptr) {
        NX_LogF("CORE: Failed to create window; ", SDL_GetError());
        return false;
    }

    SDL_SetWindowPosition(INX_Display.window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    /* --- Create OpenGL context --- */

    INX_Display.glContext = SDL_GL_CreateContext(INX_Display.window);
    if (INX_Display.glContext == nullptr) {
        NX_LogF("CORE: Failed to create OpenGL context; ", SDL_GetError());
        return false;
    }

    /* --- Load OpenGL functions --- */

    if (gladLoadGLES2(SDL_GL_GetProcAddress) < 0) {
        NX_LogF("CORE: Failed to load OpenGL functions");
        return false;
    }

    /* --- Store GL context type --- */

    INX_Display.glProfile = useOpenGLES ? SDL_GL_CONTEXT_PROFILE_ES : SDL_GL_CONTEXT_PROFILE_CORE;

    /* --- Set VSync --- */

    if (desc.flags & NX_FLAG_VSYNC_HINT) {
        if (!SDL_GL_SetSwapInterval(-1)) {
            SDL_GL_SetSwapInterval(1);
        }
    }
    else {
        SDL_GL_SetSwapInterval(0);
    }

    /* --- Print debug infos --- */

    NX_LOG(D, "CORE: GL Vendor     : %s", glGetString(GL_VENDOR));
    NX_LOG(D, "CORE: GL Renderer   : %s", glGetString(GL_RENDERER));
    NX_LOG(D, "CORE: GL Version    : %s", glGetString(GL_VERSION));
    NX_LOG(D, "CORE: GLSL Version  : %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

    return true;
}

static bool INX_KeyboardState_Init()
{
    SDL_memset(&INX_Keyboard, 0, sizeof(INX_Keyboard));

    return true;
}

static bool INX_MouseState_Init()
{
    SDL_memset(&INX_Mouse, 0, sizeof(INX_Mouse));

    return true;
}

static bool INX_FrameState_Init(const NX_AppDesc& desc)
{
    SDL_memset(&INX_Frame, 0, sizeof(INX_Frame));

    INX_Frame.targetDeltaTime = (desc.targetFPS > 0) ? 1.0f / desc.targetFPS : 0.0f;
    INX_Frame.perfFrequency = SDL_GetPerformanceFrequency();
    INX_Frame.ticksLast = SDL_GetPerformanceCounter();

    return true;
}

// ============================================================================
// PUBLIC API
// ============================================================================

bool NX_Init(const char* title, int w, int h, NX_Flags flags)
{
    NX_AppDesc desc{
        .flags = flags,
        .name = nullptr,
        .version = nullptr,
        .identifier = nullptr,
        .memory = {}
    };

    return NX_InitEx(title, w, h, &desc);
}

bool NX_InitEx(const char* title, int w, int h, NX_AppDesc* desc)
{
    /* --- Ensures that the application description is valid --- */

    if (desc == nullptr) {
        NX_LOG(E, "CORE: Failed to initialize Nexium; App description cannot be null");
        return false;
    }

    /* --- Init dependencies --- */

    if (!INX_SDL_Init(*desc)) {
        return false;
    }

    if (!INX_PhysFS_Init()) {
        return false;
    }

    /* --- Init each modules --- */

    if (!INX_DisplayState_Init(title, w, h, *desc)) {
        return false;
    }

    if (!INX_Render2DState_Init(desc)) {
        return false;
    }

    if (!INX_AudioState_Init(desc)) {
        return false;
    }

    try {
        gRender = util::MakeUnique<NX_RenderState>(*desc);
    }
    catch (const std::exception& e) {
        NX_LOG(E, e.what());
        gRender.reset();
        return false;
    }

    if (!INX_KeyboardState_Init()) {
        return false;
    }

    if (!INX_MouseState_Init()) {
        return false;
    }

    if (!INX_FrameState_Init(*desc)) {
        return false;
    }

    /* --- Oh yeaaaah :3 --- */

    return true;
}

void NX_Quit()
{
    gRender.reset();

    INX_Programs.UnloadAll();
    INX_Assets.UnloadAll();
    INX_Pool.UnloadAll();

    INX_Render2DState_Quit();
    INX_AudioState_Quit();

    SDL_GL_DestroyContext(INX_Display.glContext);
    INX_Display.glContext = nullptr;

    SDL_DestroyWindow(INX_Display.window);
    INX_Display.window = nullptr;

    SDL_Quit();
}
