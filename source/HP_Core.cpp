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

#include <Hyperion/HP_Core.h>
#include <Hyperion/HP_Rand.h>

#include "./Render/HP_RenderState.hpp"
#include "./Audio/HP_AudioState.hpp"
#include "./Core/HP_CoreState.hpp"
#include "Hyperion/HP_Math.h"
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

bool HP_FrameStep(void)
{
    bool shouldRun = true;

    /* --- Buffer swap --- */

    // NOTE: The buffer swap happens inside HP_FrameStep at the start of each frame.
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
    gCore->mMouseDelta = HP_VEC2_ZERO;
    gCore->mMouseWheel = HP_VEC2_ZERO;

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

int64_t HP_GetCurrentTimeNS(void)
{
    SDL_Time time{};
    SDL_GetCurrentTime(&time);
    return time;
}

double HP_GetCurrentTime(void)
{
    int64_t ns = HP_GetCurrentTimeNS();
    return static_cast<double>(ns) / 1e9;
}

double HP_GetElapsedTime(void)
{
    return gCore->elapsedTime();
}

double HP_GetFrameTime(void)
{
    return gCore->frameTime();
}

void HP_SetTargetFPS(int fps)
{
    gCore->setTargetFrameRate(fps);
}

int HP_GetFPS(void)
{
    return static_cast<int>(gCore->frameRate() + 0.5);
}

bool HP_SetVSync(int mode)
{
    return SDL_GL_SetSwapInterval(mode);
}

float HP_GetDisplayScale(void)
{
    return SDL_GetWindowDisplayScale(gCore->window());
}

float HP_GetDisplayGetDPI(void)
{
    float displayScale = SDL_GetWindowDisplayScale(gCore->window());

#if defined(__ANDROID__) || defined(__IPHONEOS__)
    return displayScale * 160.0f;
#else
    return displayScale * 96.0f;
#endif
}

int HP_GetDisplayIndex(void)
{
    return SDL_GetDisplayForWindow(gCore->window());
}

HP_IVec2 HP_GetDisplaySize(void)
{
    int displayIndex = SDL_GetDisplayForWindow(gCore->window());

    SDL_Rect bounds;
    SDL_GetDisplayBounds(displayIndex, &bounds);

    return HP_IVEC2(bounds.w, bounds.h);
}

HP_Vec2 HP_GetDisplaySizeF(void)
{
    int displayIndex = SDL_GetDisplayForWindow(gCore->window());

    SDL_Rect bounds;
    SDL_GetDisplayBounds(displayIndex, &bounds);

    return HP_VEC2(bounds.w, bounds.h);
}

const char* HP_GetWindowTitle(void)
{
    return SDL_GetWindowTitle(gCore->window());
}

void HP_SetWindowTitle(const char* title)
{
    SDL_SetWindowTitle(gCore->window(), title);
}

int HP_GetWindowWidth(void)
{
    int w = 0;
    SDL_GetWindowSize(gCore->window(), &w, NULL);
    return w;
}

int HP_GetWindowHeight(void)
{
    int h = 0;
    SDL_GetWindowSize(gCore->window(), NULL, &h);
    return h;
}

HP_IVec2 HP_GetWindowSize(void)
{
    HP_IVec2 result{};
    SDL_GetWindowSize(gCore->window(), &result.x, &result.y);
    return result;
}

HP_Vec2 HP_GetWindowSizeF(void)
{
    HP_IVec2 result{};
    SDL_GetWindowSize(gCore->window(), &result.x, &result.y);
    return HP_VEC2(result.x, result.y);
}

void HP_SetWindowSize(int w, int h)
{
    SDL_SetWindowSize(gCore->window(), w, h);
}

void HP_SetWindowMinSize(int w, int h)
{
    SDL_SetWindowMinimumSize(gCore->window(), w, h);
}

void HP_SetWindowMaxSize(int w, int h)
{
    SDL_SetWindowMaximumSize(gCore->window(), w, h);
}

HP_IVec2 HP_GetWindowPosition(void)
{
    HP_IVec2 result{};
    SDL_GetWindowPosition(gCore->window(), &result.x, &result.y);
    return result;
}

void HP_SetWindowPosition(int x, int y)
{
    SDL_SetWindowPosition(gCore->window(), x, y);
}

bool HP_IsWindowFullscreen(void)
{
    Uint64 flags = SDL_GetWindowFlags(gCore->window());
    return (flags & SDL_WINDOW_FULLSCREEN) != 0;
}

void HP_SetWindowFullscreen(bool enabled)
{
    SDL_SetWindowFullscreen(gCore->window(), enabled);
}

bool HP_IsWindowResizable(void)
{
    Uint64 flags = SDL_GetWindowFlags(gCore->window());
    return (flags & SDL_WINDOW_RESIZABLE) != 0;
}

void HP_SetWindowResizable(bool resizable)
{
    SDL_SetWindowResizable(gCore->window(), resizable);
}

bool HP_IsWindowVisible(void)
{
    Uint64 flags = SDL_GetWindowFlags(gCore->window());
    return (flags & SDL_WINDOW_HIDDEN) == 0;
}

void HP_MinimizeWindow(void)
{
    SDL_MinimizeWindow(gCore->window());
}

void HP_MaximizeWindow(void)
{
    SDL_MaximizeWindow(gCore->window());
}

void HP_RestoreWindow(void)
{
    SDL_RestoreWindow(gCore->window());
}

void HP_ShowWindow(void)
{
    SDL_ShowWindow(gCore->window());
}

void HP_HideWindow(void)
{
    SDL_HideWindow(gCore->window());
}

bool HP_IsWindowFocused(void)
{
    Uint64 flags = SDL_GetWindowFlags(gCore->window());
    return (flags & SDL_WINDOW_INPUT_FOCUS) != 0;
}

void HP_FocusWindow(void)
{
    SDL_RaiseWindow(gCore->window());
}

bool HP_IsWindowBordered(void)
{
    Uint64 flags = SDL_GetWindowFlags(gCore->window());
    return (flags & SDL_WINDOW_BORDERLESS) == 0;
}

void HP_SetWindowBordered(bool bordered)
{
    SDL_SetWindowBordered(gCore->window(), bordered);
}

bool HP_IsCursorGrabbed(void)
{
    return SDL_GetWindowMouseGrab(gCore->window());
}

void HP_GrabCursor(bool grab)
{
    SDL_SetWindowMouseGrab(gCore->window(), grab);
}

void HP_ShowCursor(void)
{
    SDL_ShowCursor();
}

void HP_HideCursor(void)
{
    SDL_HideCursor();
}

bool HP_IsCursorVisible(void)
{
    return SDL_CursorVisible();
}

void HP_CaptureMouse(bool enabled)
{
    SDL_SetWindowRelativeMouseMode(gCore->window(), enabled);
}

bool HP_IsMouseButtonPressed(HP_MouseButton button)
{
    return gCore->currentMouseButton(button);
}

bool HP_IsMouseButtonReleased(HP_MouseButton button)
{
    return !gCore->currentMouseButton(button);
}

bool HP_IsMouseButtonJustPressed(HP_MouseButton button)
{
    return gCore->currentMouseButton(button) &&
        !gCore->previousMouseButton(button);
}

bool HP_IsMouseButtonJustReleased(HP_MouseButton button)
{
    return gCore->previousMouseButton(button) &&
        !gCore->currentMouseButton(button);
}

HP_Vec2 HP_GetMousePosition(void)
{
    return gCore->mousePosition();
}

void HP_SetMousePosition(HP_Vec2 p)
{
    SDL_WarpMouseInWindow(gCore->window(), p.x, p.y);
    gCore->mMousePosition = p;
}

HP_Vec2 HP_GetMouseDelta(void)
{
    return gCore->mouseDelta();
}

HP_Vec2 HP_GetMouseWheel(void)
{
    return gCore->mouseWheel();
}

bool HP_IsKeyPressed(HP_Key key)
{
    if (static_cast<uint32_t>(key) >= SDL_SCANCODE_COUNT) {
        return false;
    }
    return gCore->currentKey(key);
}

bool HP_IsKeyReleased(HP_Key key)
{
    if (static_cast<uint32_t>(key) >= SDL_SCANCODE_COUNT) {
        return false;
    }
    return !gCore->currentKey(key);
}

bool HP_IsKeyJustPressed(HP_Key key)
{
    if (static_cast<uint32_t>(key) >= SDL_SCANCODE_COUNT) {
        return false;
    }
    return gCore->currentKey(key) && !gCore->previousKey(key);
}

bool HP_IsKeyJustReleased(HP_Key key)
{
    if (static_cast<uint32_t>(key) >= SDL_SCANCODE_COUNT) {
        return false;
    }
    return gCore->previousKey(key) && !gCore->currentKey(key);
}

HP_Vec2 HP_GetKeyVec2(HP_Key up, HP_Key down, HP_Key left, HP_Key right)
{
    int x = gCore->currentKey(right) - gCore->currentKey(left);
    int y = gCore->currentKey(down) - gCore->currentKey(up);

    return HP_Vec2Normalize(HP_VEC2(x, y));
}

HP_Vec3 HP_GetKeyVec3(HP_Key forward, HP_Key backward, HP_Key left, HP_Key right)
{
    int x = gCore->currentKey(right) - gCore->currentKey(left);
    int z = gCore->currentKey(forward) - gCore->currentKey(backward);

    return HP_Vec3Normalize(HP_VEC3(x, 0, z));
}

bool HP_AddSearchPath(const char* path, bool appendToEnd)
{
    return PHYSFS_mount(path, NULL, appendToEnd ? 1 : 0) != 0;
}

bool HP_RemoveSearchPath(const char* path)
{
    return PHYSFS_unmount(path) != 0;
}

char** HP_GetSearchPaths(void)
{
    return PHYSFS_getSearchPath();
}

void HP_FreeSearchPaths(char** paths)
{
    PHYSFS_freeList(paths);
}

bool HP_MountArchive(const char* archivePath, const char* mountPoint, bool appendToEnd)
{
    return PHYSFS_mount(archivePath, mountPoint, appendToEnd ? 1 : 0) != 0;
}

bool HP_UnmountArchive(const char* archivePath)
{
    return PHYSFS_unmount(archivePath) != 0;
}

const char* HP_GetWriteDir(void)
{
    return PHYSFS_getWriteDir();
}

bool HP_SetWriteDir(const char* path)
{
    return PHYSFS_setWriteDir(path) != 0;
}

const char* HP_GetBaseDir(void)
{
    return PHYSFS_getBaseDir();
}

const char* HP_GetPrefDir(const char* org, const char* app)
{
    return PHYSFS_getPrefDir(org, app);
}

bool HP_FileExists(const char* filePath)
{
    return PHYSFS_exists(filePath) != 0;
}

bool HP_IsDirectory(const char* path)
{
    PHYSFS_Stat stat;
    if (PHYSFS_stat(path, &stat) == 0) {
        return false;
    }
    return stat.filetype == PHYSFS_FILETYPE_DIRECTORY;
}

bool HP_IsFile(const char* path)
{
    PHYSFS_Stat stat;
    if (PHYSFS_stat(path, &stat) == 0) {
        return false;
    }
    return stat.filetype == PHYSFS_FILETYPE_REGULAR;
}

size_t HP_GetFileSize(const char* filePath)
{
    PHYSFS_Stat stat;
    if (PHYSFS_stat(filePath, &stat) == 0) {
        return -1;
    }
    return stat.filesize;
}

const char* HP_GetRealPath(const char* filePath)
{
    return PHYSFS_getRealDir(filePath);
}

char** HP_ListDirectory(const char* dirPath)
{
    return PHYSFS_enumerateFiles(dirPath);
}

void HP_FreeDirectoryList(char** list)
{
    PHYSFS_freeList(list);
}

bool HP_CreateDirectory(const char* dirPath)
{
    return PHYSFS_mkdir(dirPath) != 0;
}

bool HP_DeleteFile(const char* filePath)
{
    return PHYSFS_delete(filePath) != 0;
}

void* HP_LoadFile(const char* filePath, size_t* size)
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

char* HP_LoadFileText(const char* filePath)
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

bool HP_WriteFile(const char* filePath, const void* data, size_t size)
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

bool HP_WriteFileText(const char* filePath, const char* data, size_t size)
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

bool HP_SetClipboardText(const char* text)
{
    return SDL_SetClipboardText(text);
}

const char* HP_GetClipboardText(void)
{
    return SDL_GetClipboardText();
}

bool HP_HasClipboardText(void)
{
    return SDL_HasClipboardText();
}

void HP_SetLogPriority(HP_LogLevel log)
{
    SDL_SetLogPriority(SDL_LOG_CATEGORY_APPLICATION, (SDL_LogPriority)log);
}

void HP_Log(HP_LogLevel log, const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, (SDL_LogPriority)log, msg, args);
    va_end(args);
}

void HP_LogVA(HP_LogLevel log, const char* msg, va_list args)
{
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, (SDL_LogPriority)log, msg, args);
}

void HP_LogT(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE, msg, args);
    va_end(args);
}

void HP_LogV(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE, msg, args);
    va_end(args);
}

void HP_LogD(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG, msg, args);
    va_end(args);
}

void HP_LogI(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, msg, args);
    va_end(args);
}

void HP_LogW(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_WARN, msg, args);
    va_end(args);
}

void HP_LogE(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, msg, args);
    va_end(args);
}

void HP_LogF(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_CRITICAL, msg, args);
    va_end(args);
}

void* HP_Malloc(size_t size)
{
    return SDL_malloc(size);
}

void* HP_Calloc(size_t nmemb, size_t size)
{
    return SDL_calloc(nmemb, size);
}

void* HP_Realloc(void* ptr, size_t size)
{
    return SDL_realloc(ptr, size);
}

void HP_Free(void* ptr)
{
    SDL_free(ptr);
}
