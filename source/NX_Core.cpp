/* NX_Core.cpp -- API definition for Nexium's core module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/NX_Core.h>
#include <NX/NX_Rand.h>

#include "./Render/NX_RenderState.hpp"
#include "./Audio/NX_AudioState.hpp"
#include "./Core/NX_CoreState.hpp"
#include "NX/NX_Math.h"
#include "SDL3/SDL_clipboard.h"

#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_time.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <physfs.h>

bool NX_FrameStep(void)
{
    bool shouldRun = true;

    /* --- Buffer swap --- */

    // NOTE: The buffer swap happens inside NX_FrameStep at the start of each frame.
    //       This is fine because there’s no latency between the end and start
    //       of the loop, and it spares the user from calling swap/present manually.
    //       The only minor drawback is an extra swap on the very first frame,
    //       but everything works normally afterwards.

    SDL_GL_SwapWindow(gCore->mWindow);

    /* --- Calculate delta time and sleep if enough time remains --- */

    Uint64 ticksNow = SDL_GetPerformanceCounter();
    gCore->mCurrentFrameTime = static_cast<double>(ticksNow - gCore->mTicksLast) / gCore->mPerfFrequency;

    constexpr double sleepSafetyMargin = 0.002;
    if (gCore->mCurrentFrameTime < gCore->mTargetFrameTime - sleepSafetyMargin) {
        SDL_DelayNS(1e9 * static_cast<Uint64>((gCore->mTargetFrameTime - gCore->mCurrentFrameTime - sleepSafetyMargin)));
    }

    /* --- Get accurate delta time after sleep and busy-wait remaining time if needed --- */

    do {
        ticksNow = SDL_GetPerformanceCounter();
        gCore->mCurrentFrameTime = static_cast<double>(ticksNow - gCore->mTicksLast) / gCore->mPerfFrequency;
    } while (gCore->mCurrentFrameTime < gCore->mTargetFrameTime);

    gCore->mElapsedTime += gCore->mCurrentFrameTime;
    gCore->mTicksLast = ticksNow;

    /* --- FPS smoothing using exponential moving average --- */

    constexpr double smoothingFactor = 0.1;
    double currentFPS = 1.0 / gCore->mCurrentFrameTime;
    gCore->mFpsAverage = gCore->mFpsAverage * (1.0 - smoothingFactor) + currentFPS * smoothingFactor;

    /* --- Update input state --- */

    // Shift current >> previous state
    for (int i = 0; i < SDL_SCANCODE_COUNT; i++) {
        gCore->mKeys[i] = (gCore->mKeys[i] & 0xF0) | (gCore->mKeys[i] >> 4);
    }

    gCore->mMouseButtons[1] = gCore->mMouseButtons[0];
    gCore->mMouseDelta = NX_VEC2_ZERO;
    gCore->mMouseWheel = NX_VEC2_ZERO;

    /* --- Update system events --- */

    SDL_Event ev;
    while (SDL_PollEvent(&ev))
    {
        switch (ev.type)
        {
        case SDL_EVENT_QUIT:
            shouldRun = false;
            break;
        case SDL_EVENT_KEY_DOWN:
            gCore->mKeys[ev.key.scancode] |= 0xF0;
            break;
        case SDL_EVENT_KEY_UP:
            gCore->mKeys[ev.key.scancode] &= 0x0F;
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            gCore->mMouseButtons[0] |= SDL_BUTTON_MASK(ev.button.button);
            break;
        case SDL_EVENT_MOUSE_BUTTON_UP:
            gCore->mMouseButtons[0] &= ~SDL_BUTTON_MASK(ev.button.button);
            break;
        case SDL_EVENT_MOUSE_MOTION:
            gCore->mMousePosition.x = ev.motion.x;
            gCore->mMousePosition.y = ev.motion.y;
            gCore->mMouseDelta.x = ev.motion.xrel;
            gCore->mMouseDelta.y = ev.motion.yrel;
            break;
        case SDL_EVENT_MOUSE_WHEEL:
            gCore->mMouseWheel.x = ev.wheel.x;
            gCore->mMouseWheel.y = ev.wheel.y;
            break;
        default:
            break;
        }
    }

    return shouldRun;
}

int64_t NX_GetCurrentTimeNS(void)
{
    SDL_Time time{};
    SDL_GetCurrentTime(&time);
    return time;
}

double NX_GetCurrentTime(void)
{
    int64_t ns = NX_GetCurrentTimeNS();
    return static_cast<double>(ns) / 1e9;
}

double NX_GetElapsedTime(void)
{
    return gCore->elapsedTime();
}

double NX_GetFrameTime(void)
{
    return gCore->frameTime();
}

void NX_SetTargetFPS(int fps)
{
    gCore->setTargetFrameRate(fps);
}

int NX_GetFPS(void)
{
    return static_cast<int>(gCore->frameRate() + 0.5);
}

bool NX_SetVSync(int mode)
{
    return SDL_GL_SetSwapInterval(mode);
}

float NX_GetDisplayScale(void)
{
    return SDL_GetWindowDisplayScale(gCore->window());
}

float NX_GetDisplayGetDPI(void)
{
    float displayScale = SDL_GetWindowDisplayScale(gCore->window());

#if defined(__ANDROID__) || defined(__IPHONEOS__)
    return displayScale * 160.0f;
#else
    return displayScale * 96.0f;
#endif
}

int NX_GetDisplayIndex(void)
{
    return SDL_GetDisplayForWindow(gCore->window());
}

NX_IVec2 NX_GetDisplaySize(void)
{
    int displayIndex = SDL_GetDisplayForWindow(gCore->window());

    SDL_Rect bounds;
    SDL_GetDisplayBounds(displayIndex, &bounds);

    return NX_IVEC2(bounds.w, bounds.h);
}

NX_Vec2 NX_GetDisplaySizeF(void)
{
    int displayIndex = SDL_GetDisplayForWindow(gCore->window());

    SDL_Rect bounds;
    SDL_GetDisplayBounds(displayIndex, &bounds);

    return NX_VEC2(bounds.w, bounds.h);
}

const char* NX_GetWindowTitle(void)
{
    return SDL_GetWindowTitle(gCore->window());
}

void NX_SetWindowTitle(const char* title)
{
    SDL_SetWindowTitle(gCore->window(), title);
}

int NX_GetWindowWidth(void)
{
    int w = 0;
    SDL_GetWindowSize(gCore->window(), &w, NULL);
    return w;
}

int NX_GetWindowHeight(void)
{
    int h = 0;
    SDL_GetWindowSize(gCore->window(), NULL, &h);
    return h;
}

NX_IVec2 NX_GetWindowSize(void)
{
    NX_IVec2 result{};
    SDL_GetWindowSize(gCore->window(), &result.x, &result.y);
    return result;
}

NX_Vec2 NX_GetWindowSizeF(void)
{
    NX_IVec2 result{};
    SDL_GetWindowSize(gCore->window(), &result.x, &result.y);
    return NX_VEC2(result.x, result.y);
}

void NX_SetWindowSize(int w, int h)
{
    SDL_SetWindowSize(gCore->window(), w, h);
}

void NX_SetWindowMinSize(int w, int h)
{
    SDL_SetWindowMinimumSize(gCore->window(), w, h);
}

void NX_SetWindowMaxSize(int w, int h)
{
    SDL_SetWindowMaximumSize(gCore->window(), w, h);
}

NX_IVec2 NX_GetWindowPosition(void)
{
    NX_IVec2 result{};
    SDL_GetWindowPosition(gCore->window(), &result.x, &result.y);
    return result;
}

void NX_SetWindowPosition(int x, int y)
{
    SDL_SetWindowPosition(gCore->window(), x, y);
}

bool NX_IsWindowFullscreen(void)
{
    Uint64 flags = SDL_GetWindowFlags(gCore->window());
    return (flags & SDL_WINDOW_FULLSCREEN) != 0;
}

void NX_SetWindowFullscreen(bool enabled)
{
    SDL_SetWindowFullscreen(gCore->window(), enabled);
}

bool NX_IsWindowResizable(void)
{
    Uint64 flags = SDL_GetWindowFlags(gCore->window());
    return (flags & SDL_WINDOW_RESIZABLE) != 0;
}

void NX_SetWindowResizable(bool resizable)
{
    SDL_SetWindowResizable(gCore->window(), resizable);
}

bool NX_IsWindowVisible(void)
{
    Uint64 flags = SDL_GetWindowFlags(gCore->window());
    return (flags & SDL_WINDOW_HIDDEN) == 0;
}

void NX_MinimizeWindow(void)
{
    SDL_MinimizeWindow(gCore->window());
}

void NX_MaximizeWindow(void)
{
    SDL_MaximizeWindow(gCore->window());
}

void NX_RestoreWindow(void)
{
    SDL_RestoreWindow(gCore->window());
}

void NX_ShowWindow(void)
{
    SDL_ShowWindow(gCore->window());
}

void NX_HideWindow(void)
{
    SDL_HideWindow(gCore->window());
}

bool NX_IsWindowFocused(void)
{
    Uint64 flags = SDL_GetWindowFlags(gCore->window());
    return (flags & SDL_WINDOW_INPUT_FOCUS) != 0;
}

void NX_FocusWindow(void)
{
    SDL_RaiseWindow(gCore->window());
}

bool NX_IsWindowBordered(void)
{
    Uint64 flags = SDL_GetWindowFlags(gCore->window());
    return (flags & SDL_WINDOW_BORDERLESS) == 0;
}

void NX_SetWindowBordered(bool bordered)
{
    SDL_SetWindowBordered(gCore->window(), bordered);
}

bool NX_IsCursorGrabbed(void)
{
    return SDL_GetWindowMouseGrab(gCore->window());
}

void NX_GrabCursor(bool grab)
{
    SDL_SetWindowMouseGrab(gCore->window(), grab);
}

void NX_ShowCursor(void)
{
    SDL_ShowCursor();
}

void NX_HideCursor(void)
{
    SDL_HideCursor();
}

bool NX_IsCursorVisible(void)
{
    return SDL_CursorVisible();
}

void NX_CaptureMouse(bool enabled)
{
    SDL_SetWindowRelativeMouseMode(gCore->window(), enabled);
}

bool NX_IsMouseButtonPressed(NX_MouseButton button)
{
    return gCore->currentMouseButton(button);
}

bool NX_IsMouseButtonReleased(NX_MouseButton button)
{
    return !gCore->currentMouseButton(button);
}

bool NX_IsMouseButtonJustPressed(NX_MouseButton button)
{
    return gCore->currentMouseButton(button) &&
        !gCore->previousMouseButton(button);
}

bool NX_IsMouseButtonJustReleased(NX_MouseButton button)
{
    return gCore->previousMouseButton(button) &&
        !gCore->currentMouseButton(button);
}

NX_Vec2 NX_GetMousePosition(void)
{
    return gCore->mousePosition();
}

void NX_SetMousePosition(NX_Vec2 p)
{
    SDL_WarpMouseInWindow(gCore->window(), p.x, p.y);
    gCore->mMousePosition = p;
}

NX_Vec2 NX_GetMouseDelta(void)
{
    return gCore->mouseDelta();
}

NX_Vec2 NX_GetMouseWheel(void)
{
    return gCore->mouseWheel();
}

bool NX_IsKeyPressed(NX_Key key)
{
    if (static_cast<uint32_t>(key) >= SDL_SCANCODE_COUNT) {
        return false;
    }
    return gCore->currentKey(key);
}

bool NX_IsKeyReleased(NX_Key key)
{
    if (static_cast<uint32_t>(key) >= SDL_SCANCODE_COUNT) {
        return false;
    }
    return !gCore->currentKey(key);
}

bool NX_IsKeyJustPressed(NX_Key key)
{
    if (static_cast<uint32_t>(key) >= SDL_SCANCODE_COUNT) {
        return false;
    }
    return gCore->currentKey(key) && !gCore->previousKey(key);
}

bool NX_IsKeyJustReleased(NX_Key key)
{
    if (static_cast<uint32_t>(key) >= SDL_SCANCODE_COUNT) {
        return false;
    }
    return gCore->previousKey(key) && !gCore->currentKey(key);
}

NX_Vec2 NX_GetKeyVec2(NX_Key up, NX_Key down, NX_Key left, NX_Key right)
{
    int x = gCore->currentKey(right) - gCore->currentKey(left);
    int y = gCore->currentKey(down) - gCore->currentKey(up);

    return NX_Vec2Normalize(NX_VEC2(x, y));
}

NX_Vec3 NX_GetKeyVec3(NX_Key forward, NX_Key backward, NX_Key left, NX_Key right)
{
    int x = gCore->currentKey(right) - gCore->currentKey(left);
    int z = gCore->currentKey(forward) - gCore->currentKey(backward);

    return NX_Vec3Normalize(NX_VEC3(x, 0, z));
}

bool NX_AddSearchPath(const char* path, bool appendToEnd)
{
    return PHYSFS_mount(path, NULL, appendToEnd ? 1 : 0) != 0;
}

bool NX_RemoveSearchPath(const char* path)
{
    return PHYSFS_unmount(path) != 0;
}

char** NX_GetSearchPaths(void)
{
    return PHYSFS_getSearchPath();
}

void NX_FreeSearchPaths(char** paths)
{
    PHYSFS_freeList(paths);
}

bool NX_MountArchive(const char* archivePath, const char* mountPoint, bool appendToEnd)
{
    return PHYSFS_mount(archivePath, mountPoint, appendToEnd ? 1 : 0) != 0;
}

bool NX_UnmountArchive(const char* archivePath)
{
    return PHYSFS_unmount(archivePath) != 0;
}

const char* NX_GetWriteDir(void)
{
    return PHYSFS_getWriteDir();
}

bool NX_SetWriteDir(const char* path)
{
    return PHYSFS_setWriteDir(path) != 0;
}

const char* NX_GetBaseDir(void)
{
    return PHYSFS_getBaseDir();
}

const char* NX_GetPrefDir(const char* org, const char* app)
{
    return PHYSFS_getPrefDir(org, app);
}

bool NX_FileExists(const char* filePath)
{
    return PHYSFS_exists(filePath) != 0;
}

bool NX_IsDirectory(const char* path)
{
    PHYSFS_Stat stat;
    if (PHYSFS_stat(path, &stat) == 0) {
        return false;
    }
    return stat.filetype == PHYSFS_FILETYPE_DIRECTORY;
}

bool NX_IsFile(const char* path)
{
    PHYSFS_Stat stat;
    if (PHYSFS_stat(path, &stat) == 0) {
        return false;
    }
    return stat.filetype == PHYSFS_FILETYPE_REGULAR;
}

size_t NX_GetFileSize(const char* filePath)
{
    PHYSFS_Stat stat;
    if (PHYSFS_stat(filePath, &stat) == 0) {
        return -1;
    }
    return stat.filesize;
}

const char* NX_GetRealPath(const char* filePath)
{
    return PHYSFS_getRealDir(filePath);
}

char** NX_ListDirectory(const char* dirPath)
{
    return PHYSFS_enumerateFiles(dirPath);
}

void NX_FreeDirectoryList(char** list)
{
    PHYSFS_freeList(list);
}

bool NX_CreateDirectory(const char* dirPath)
{
    return PHYSFS_mkdir(dirPath) != 0;
}

bool NX_DeleteFile(const char* filePath)
{
    return PHYSFS_delete(filePath) != 0;
}

void* NX_LoadFile(const char* filePath, size_t* size)
{
    if (!filePath || !size) {
        return NULL;
    }

    PHYSFS_File* file = PHYSFS_openRead(filePath);
    if (!file) {
        *size = 0;
        return NULL;
    }

    PHYSFS_sint64 fileSize = PHYSFS_fileLength(file);
    if (fileSize < 0) {
        PHYSFS_close(file);
        *size = 0;
        return NULL;
    }

    void* buffer = SDL_malloc((size_t)fileSize);
    if (!buffer) {
        PHYSFS_close(file);
        *size = 0;
        return NULL;
    }

    PHYSFS_sint64 bytesRead = PHYSFS_readBytes(file, buffer, (PHYSFS_uint64)fileSize);
    PHYSFS_close(file);

    if (bytesRead != fileSize) {
        SDL_free(buffer);
        *size = 0;
        return NULL;
    }

    *size = (size_t)fileSize;
    return buffer;
}

char* NX_LoadFileText(const char* filePath)
{
    if (!filePath) {
        return NULL;
    }

    PHYSFS_File* file = PHYSFS_openRead(filePath);
    if (!file) {
        return NULL;
    }

    PHYSFS_sint64 fileSize = PHYSFS_fileLength(file);
    if (fileSize < 0) {
        PHYSFS_close(file);
        return NULL;
    }

    char* buffer = (char*)SDL_malloc((size_t)fileSize + 1);
    if (!buffer) {
        PHYSFS_close(file);
        return NULL;
    }

    PHYSFS_sint64 bytesRead = PHYSFS_readBytes(file, buffer, (PHYSFS_uint64)fileSize);
    PHYSFS_close(file);

    if (bytesRead != fileSize) {
        SDL_free(buffer);
        return NULL;
    }

    buffer[fileSize] = '\0';

    return buffer;
}

bool NX_WriteFile(const char* filePath, const void* data, size_t size)
{
    if (!filePath || !data || size == 0) {
        return false;
    }

    PHYSFS_File* file = PHYSFS_openWrite(filePath);
    if (!file) {
        return false;
    }

    PHYSFS_sint64 bytesWritten = PHYSFS_writeBytes(file, data, (PHYSFS_uint64)size);
    PHYSFS_close(file);

    return bytesWritten == (PHYSFS_sint64)size;
}

bool NX_WriteFileText(const char* filePath, const char* data, size_t size)
{
    if (!filePath || !data || size == 0) {
        return false;
    }

    PHYSFS_File* file = PHYSFS_openWrite(filePath);
    if (!file) {
        return false;
    }

    PHYSFS_sint64 bytesWritten = PHYSFS_writeBytes(file, data, (PHYSFS_uint64)size);
    PHYSFS_close(file);

    return bytesWritten == (PHYSFS_sint64)size;
}

bool NX_SetClipboardText(const char* text)
{
    return SDL_SetClipboardText(text);
}

const char* NX_GetClipboardText(void)
{
    return SDL_GetClipboardText();
}

bool NX_HasClipboardText(void)
{
    return SDL_HasClipboardText();
}

void NX_SetLogPriority(NX_LogLevel log)
{
    SDL_SetLogPriority(SDL_LOG_CATEGORY_APPLICATION, (SDL_LogPriority)log);
}

void NX_Log(NX_LogLevel log, const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, (SDL_LogPriority)log, msg, args);
    va_end(args);
}

void NX_LogVA(NX_LogLevel log, const char* msg, va_list args)
{
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, (SDL_LogPriority)log, msg, args);
}

void NX_LogT(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE, msg, args);
    va_end(args);
}

void NX_LogV(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE, msg, args);
    va_end(args);
}

void NX_LogD(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG, msg, args);
    va_end(args);
}

void NX_LogI(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, msg, args);
    va_end(args);
}

void NX_LogW(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_WARN, msg, args);
    va_end(args);
}

void NX_LogE(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, msg, args);
    va_end(args);
}

void NX_LogF(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_CRITICAL, msg, args);
    va_end(args);
}

void* NX_Malloc(size_t size)
{
    return SDL_malloc(size);
}

void* NX_Calloc(size_t nmemb, size_t size)
{
    return SDL_calloc(nmemb, size);
}

void* NX_Realloc(void* ptr, size_t size)
{
    return SDL_realloc(ptr, size);
}

void NX_Free(void* ptr)
{
    SDL_free(ptr);
}
