/* NX_Core.cpp -- API definition for Nexium's core module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/NX_Core.h>
#include <NX/NX_Math.h>

#include "./Render/NX_RenderState.hpp"
#include "./Audio/NX_AudioState.hpp"
#include "./Core/NX_CoreState.hpp"

#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_clipboard.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_time.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <physfs.h>
#include <zlib.h>

bool NX_FrameStep(void)
{
    bool shouldRun = true;

    /* --- Buffer swap --- */

    static bool firstFrame = true;

    if (!firstFrame) {
        SDL_GL_SwapWindow(gCore->mWindow);
    }
    else {
        firstFrame = false;
    }

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

void NX_SetWindowIcon(const NX_Image* icon)
{
    if (icon == nullptr || icon->pixels == nullptr) {
        NX_INTERNAL_LOG(E, "CORE: Failed to set window icon; Invalid icon data");
        return;
    }

    SDL_PixelFormat format;
    int bpp;

    switch (icon->format) {
    case NX_PIXEL_FORMAT_RGB8:
        format = SDL_PIXELFORMAT_RGB24, bpp = 3;
        break;
    case NX_PIXEL_FORMAT_RGBA8:
        format = SDL_PIXELFORMAT_RGBA32, bpp = 4;
        break;
    case NX_PIXEL_FORMAT_RGB16F:
        format = SDL_PIXELFORMAT_RGB48_FLOAT, bpp = 6;
        break;
    case NX_PIXEL_FORMAT_RGBA16F:
        format = SDL_PIXELFORMAT_RGBA64_FLOAT, bpp = 8;
        break;
    case NX_PIXEL_FORMAT_RGB32F:
        format = SDL_PIXELFORMAT_RGB96_FLOAT, bpp = 12;
        break;
    case NX_PIXEL_FORMAT_RGBA32F:
        format = SDL_PIXELFORMAT_RGBA128_FLOAT, bpp = 16;
        break;
    default:
        NX_INTERNAL_LOG(E, "CORE: Failed to set window icon; Unsupported format");
        return;
    }

    SDL_Surface* surface = SDL_CreateSurfaceFrom(icon->w, icon->h, format, 
                                                 icon->pixels, bpp * icon->w);
    if (surface == nullptr) {
        NX_INTERNAL_LOG(E, "CORE: Failed to set window icon; %s", SDL_GetError());
        return;
    }

    if (!SDL_SetWindowIcon(gCore->window(), surface)) {
        NX_INTERNAL_LOG(E, "CORE: Failed to set window icon; %s", SDL_GetError());
    }

    SDL_DestroySurface(surface);
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
    int z = gCore->currentKey(backward) - gCore->currentKey(forward);

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

    void* buffer = util::malloc(static_cast<size_t>(fileSize));
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

    char* buffer = util::malloc<char>(static_cast<size_t>(fileSize) + 1);
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

void* NX_CompressData(const void* data, size_t dataSize, size_t* outputSize)
{
    /* --- Validate input parameters --- */

    if (!data || dataSize == 0 || !outputSize) {
        if (outputSize) *outputSize = 0;
        return NULL;
    }

    /* --- Allocate buffer for header and compressed data --- */

    uLongf maxCompressedSize = compressBound(static_cast<uLongf>(dataSize));
    size_t totalSize = sizeof(uint64_t) + maxCompressedSize;
    void* buffer = util::malloc(totalSize);
    if (!buffer) {
        *outputSize = 0;
        return NULL;
    }

    /* --- Compress data after header --- */

    void* compressedData = static_cast<uint8_t*>(buffer) + sizeof(uint64_t);
    uLongf compressedSize = maxCompressedSize;
    int result = compress(
        static_cast<Bytef*>(compressedData),
        &compressedSize,
        static_cast<const Bytef*>(data),
        static_cast<uLongf>(dataSize)
    );

    if (result != Z_OK) {
        util::free(buffer);
        *outputSize = 0;
        return NULL;
    }

    /* --- Write uncompressed size to header --- */

    uint64_t uncompressedSizeHeader = static_cast<uint64_t>(dataSize);
    SDL_memcpy(buffer, &uncompressedSizeHeader, sizeof(uint64_t));

    /* --- Set output size and return buffer --- */

    *outputSize = sizeof(uint64_t) + static_cast<size_t>(compressedSize);

    return buffer;
}

void* NX_DecompressData(const void* data, size_t dataSize, size_t* outputSize)
{
    /* --- Validate input parameters --- */

    if (!data || dataSize < sizeof(uint64_t) || !outputSize) {
        return nullptr;
    }

    /* --- Read uncompressed size from header --- */

    uint64_t uncompressedSize;
    SDL_memcpy(&uncompressedSize, data, sizeof(uint64_t));

    /* --- Allocate decompression buffer --- */

    void* decompressedData = util::malloc(static_cast<size_t>(uncompressedSize));
    if (!decompressedData) {
        return nullptr;
    }

    /* --- Decompress data after header --- */

    const void* compressedData = static_cast<const uint8_t*>(data) + sizeof(uint64_t);
    size_t compressedSize = dataSize - sizeof(uint64_t);
    uLongf destLen = static_cast<uLongf>(uncompressedSize);
    int result = uncompress(
        static_cast<Bytef*>(decompressedData),
        &destLen,
        static_cast<const Bytef*>(compressedData),
        static_cast<uLongf>(compressedSize)
    );

    if (result != Z_OK) {
        util::free(decompressedData);
        return nullptr;
    }

    /* --- Set output size and return buffer --- */

    *outputSize = static_cast<size_t>(destLen);

    return decompressedData;
}

void* NX_CompressText(const char* text, size_t* outputSize)
{
    if (!text || !outputSize) {
        if (outputSize) *outputSize = 0;
        return NULL;
    }

    return NX_CompressData(text, strlen(text), outputSize);
}

char* NX_DecompressText(const void* data, size_t dataSize)
{
    /* --- Validate input parameters --- */

    if (!data || dataSize < sizeof(uint64_t)) {
        return nullptr;
    }

    /* --- Read uncompressed size from header --- */

    uint64_t uncompressedSize;
    SDL_memcpy(&uncompressedSize, data, sizeof(uint64_t));

    /* --- Allocate decompression buffer with extra byte for null terminator --- */

    char* decompressedText = util::malloc<char>(static_cast<size_t>(uncompressedSize) + 1);
    if (!decompressedText) {
        return nullptr;
    }

    /* --- Decompress data after header --- */

    const void* compressedData = static_cast<const uint8_t*>(data) + sizeof(uint64_t);
    size_t compressedSize = dataSize - sizeof(uint64_t);
    uLongf destLen = static_cast<uLongf>(uncompressedSize);
    int result = uncompress(
        reinterpret_cast<Bytef*>(decompressedText),
        &destLen,
        static_cast<const Bytef*>(compressedData),
        static_cast<uLongf>(compressedSize)
    );

    if (result != Z_OK) {
        util::free(decompressedText);
        return nullptr;
    }

    /* --- Add null terminator --- */

    decompressedText[destLen] = '\0';

    return decompressedText;
}

char* NX_EncodeBase64(const void* data, size_t dataSize, size_t* outputSize)
{
    // Base64 conversion table according to RFC 4648
    constexpr char b64Table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    /* --- Validation of input parameters --- */

    if (data == nullptr || dataSize < 0 || outputSize == nullptr) {
        return nullptr;
    }

    /* --- Special case: empty data --- */

    if (dataSize == 0) {
        char* empty = util::malloc<char>(1);
        if (empty != nullptr) {
            empty[0] = '\0';
            *outputSize = 1;
        }
        return empty;
    }

    /* --- Calculation of output size (4 characters per group of 3 bytes + null terminator) --- */

    const int completeGroups = dataSize / 3;
    const int remainingBytes = dataSize % 3;
    const int paddingCount = (remainingBytes > 0) ? (3 - remainingBytes) : 0;
    const int totalOutputSize = (completeGroups + (remainingBytes > 0 ? 1 : 0)) * 4 + 1;

    /* --- Memory allocation --- */

    char* encodedData = util::malloc<char>(totalOutputSize);
    if (encodedData == nullptr) {
        return nullptr;
    }

    auto bData = static_cast<const uint8_t*>(data);
    int outputIndex = 0;
    int inputIndex = 0;

    /* --- Processing complete 3-byte groups --- */

    for (int group = 0; group < completeGroups; ++group, inputIndex += 3) {
        const uint32_t triplet = (static_cast<uint32_t>(bData[inputIndex]) << 16) |
                                 (static_cast<uint32_t>(bData[inputIndex + 1]) << 8) |
                                  static_cast<uint32_t>(bData[inputIndex + 2]);

        encodedData[outputIndex++] = b64Table[(triplet >> 18) & 0x3F];
        encodedData[outputIndex++] = b64Table[(triplet >> 12) & 0x3F];
        encodedData[outputIndex++] = b64Table[(triplet >> 6) & 0x3F];
        encodedData[outputIndex++] = b64Table[triplet & 0x3F];
    }

    /* --- Processing remaining bytes (with padding) --- */

    if (remainingBytes > 0) {
        uint32_t triplet = static_cast<uint32_t>(bData[inputIndex]) << 16;
        if (remainingBytes == 2) {
            triplet |= static_cast<uint32_t>(bData[inputIndex + 1]) << 8;
        }
        encodedData[outputIndex++] = b64Table[(triplet >> 18) & 0x3F];
        encodedData[outputIndex++] = b64Table[(triplet >> 12) & 0x3F];
        encodedData[outputIndex++] = (remainingBytes == 2) ? b64Table[(triplet >> 6) & 0x3F] : '=';
        encodedData[outputIndex++] = '=';
    }

    /* --- Null Terminator --- */

    encodedData[outputIndex] = '\0';
    *outputSize = totalOutputSize;

    return encodedData;
}

void* NX_DecodeBase64(const char* text, size_t* outputSize)
{
    /* --- Validation of input parameters --- */

    if (text == nullptr || outputSize == nullptr) {
        return nullptr;
    }

    /* --- Base64 decoding table (255 = invalid character) --- */

    constexpr uint8_t base64DecodeTable[256] = {
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 0-15
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 16-31
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  62, 255, 255, 255,  63, // 32-47  ('+' = 62, '/' = 63)
         52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255, 255, 254, 255, 255, // 48-63  ('0'-'9' = 52-61, '=' = 254)
        255,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14, // 64-79  ('A'-'O' = 0-14)
         15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255, // 80-95  ('P'-'Z' = 15-25)
        255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40, // 96-111 ('a'-'o' = 26-40)
         41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51, 255, 255, 255, 255, 255, // 112-127 ('p'-'z' = 41-51)
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 128-143
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 144-159
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 160-175
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 176-191
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 192-207
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 208-223
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 224-239
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255  // 240-255
    };

    const int textLength = static_cast<int>(strlen(text));

    /* --- Length check (must be a multiple of 4) --- */

    if (textLength == 0) {
        *outputSize = 0;
        return nullptr;
    }

    if (textLength % 4 != 0) {
        NX_INTERNAL_LOG(W, "CORE: Invalid Base64 string length (not multiple of 4)");
        return nullptr;
    }

    /* --- Padding count --- */

    int paddingCount = 0;
    if (text[textLength - 1] == '=') {
        paddingCount++;
        if (textLength > 1 && text[textLength - 2] == '=') {
            paddingCount++;
        }
    }

    /* --- Validation of padding (maximum 2 '=' characters) --- */

    if (paddingCount > 2) {
        NX_INTERNAL_LOG(W, "CORE: Invalid Base64 padding");
        return nullptr;
    }

    /* --- Calculating the output size --- */

    const int decodedSize = (textLength / 4) * 3 - paddingCount;
    if (decodedSize < 0) {
        NX_INTERNAL_LOG(W, "CORE: Invalid Base64 string");
        return nullptr;
    }

    /* --- Memory allocation --- */

    uint8_t* decodedData = util::malloc<uint8_t>(decodedSize);
    if (decodedData == nullptr) {
        return nullptr;
    }

    int outputIndex = 0;
    int inputIndex = 0;

    /* --- Processing in blocks of 4 characters --- */

    const int numCompleteGroups = textLength / 4;

    for (int group = 0; group < numCompleteGroups; ++group, inputIndex += 4)
    {
        /* --- Reading the 4 Base64 characters --- */

        const uint8_t c0 = static_cast<uint8_t>(text[inputIndex]);
        const uint8_t c1 = static_cast<uint8_t>(text[inputIndex + 1]);
        const uint8_t c2 = static_cast<uint8_t>(text[inputIndex + 2]);
        const uint8_t c3 = static_cast<uint8_t>(text[inputIndex + 3]);

        /* --- Decoding via the lookup table --- */

        const uint8_t v0 = base64DecodeTable[c0];
        const uint8_t v1 = base64DecodeTable[c1];
        const uint8_t v2 = base64DecodeTable[c2];
        const uint8_t v3 = base64DecodeTable[c3];

        /* --- Character validation (254 = padding '=', 255 = invalid) --- */

        if (v0 == 255 || v1 == 255 || 
            (v2 == 255 && c2 != '=') || 
            (v3 == 255 && c3 != '=')) {
            NX_INTERNAL_LOG(W, "CORE: Invalid Base64 character detected");
            util::free(decodedData);
            return nullptr;
        }

        /* --- Rebuild the sixtets (replace padding with 0) --- */

        const uint32_t sextet0 = (v0 != 254) ? v0 : 0;
        const uint32_t sextet1 = (v1 != 254) ? v1 : 0;
        const uint32_t sextet2 = (v2 != 254) ? v2 : 0;
        const uint32_t sextet3 = (v3 != 254) ? v3 : 0;

        /* --- 24-bit triplet combination --- */

        const uint32_t triplet = (sextet0 << 18) | (sextet1 << 12) | (sextet2 << 6) | sextet3;

        /* --- Extracting bytes --- */

        decodedData[outputIndex++] = static_cast<uint8_t>((triplet >> 16) & 0xFF);

        if (outputIndex < decodedSize) {
            decodedData[outputIndex++] = static_cast<uint8_t>((triplet >> 8) & 0xFF);
        }

        if (outputIndex < decodedSize) {
            decodedData[outputIndex++] = static_cast<uint8_t>(triplet & 0xFF);
        }
    }

    *outputSize = decodedSize;

    return decodedData;
}

uint32_t NX_ComputeCRC32(void* data, size_t dataSize)
{
    return crc32(0L, (const uint8_t*)data, dataSize);
}

const uint32_t* NX_ComputeMD5(void* data, size_t dataSize)
{
    /* --- MD5 constants --- */

    // Per-round shift amounts (4 rounds of 16 operations each)
    constexpr uint32_t shiftAmounts[64] = {
        7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  // Round 1
        5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  // Round 2
        4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  // Round 3
        6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21   // Round 4
    };

    // Binary integer parts of the sines of integers (radians) as constants
    constexpr uint32_t sineConstants[64] = {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
        0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
        0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
        0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
        0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
        0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
        0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
        0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
        0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
        0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
        0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
        0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
        0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
    };

    /* --- Initialize hash state (MD5 magic numbers) --- */

    static uint32_t hash[4];
    hash[0] = 0x67452301;
    hash[1] = 0xefcdab89;
    hash[2] = 0x98badcfe;
    hash[3] = 0x10325476;

    /* --- Calculate padded message size --- */

    // Message must be padded to 448 bits (mod 512), then append 64-bit length
    // This ensures total size is a multiple of 512 bits (64 bytes)

    const size_t paddedSize = ((((dataSize + 8) / 64) + 1) * 64) - 8;
    const uint32_t bitLength = static_cast<uint32_t>(dataSize * 8);

    /* --- Allocate and prepare padded message --- */

    uint8_t* message = util::calloc<uint8_t>(paddedSize + 64);
    if (!message) {
        return hash;
    }

    // Copy original data
    SDL_memcpy(message, data, dataSize);

    // Append '1' bit (0x80 = 10000000 in binary)
    message[dataSize] = 0x80;

    // Append original message length in bits (little-endian, lower 32 bits only)
    SDL_memcpy(message + paddedSize, &bitLength, sizeof(uint32_t));

    /* --- Process message in 512-bit (64-byte) chunks --- */

    for (size_t offset = 0; offset < paddedSize; offset += 64)
    {
        // Break chunk into sixteen 32-bit words (little-endian)
        const uint32_t* words = reinterpret_cast<const uint32_t*>(message + offset);

        // Initialize working variables with current hash state
        uint32_t a = hash[0];
        uint32_t b = hash[1];
        uint32_t c = hash[2];
        uint32_t d = hash[3];

        /* --- Perform 64 operations (4 rounds of 16 operations) --- */

        for (int i = 0; i < 64; ++i)
        {
            uint32_t f, g;

            // Select auxiliary function and word index based on round
            if (i < 16) {
                f = (b & c) | ((~b) & d);
                g = i;
            }
            else if (i < 32) {
                f = (d & b) | ((~d) & c);
                g = (5 * i + 1) % 16;
            }
            else if (i < 48) {
                f = b ^ c ^ d;
                g = (3 * i + 5) % 16;
            }
            else {
                f = c ^ (b | (~d));
                g = (7 * i) % 16;
            }

            // Rotate left
            const uint32_t temp = d;
            d = c;
            c = b;
            const uint32_t rotated = (a + f + sineConstants[i] + words[g]);
            b = b + ((rotated << shiftAmounts[i]) | (rotated >> (32 - shiftAmounts[i])));
            a = temp;
        }

        /* --- Add this chunk's hash to result --- */

        hash[0] += a;
        hash[1] += b;
        hash[2] += c;
        hash[3] += d;
    }

    /* --- Cleanup and return --- */

    util::free(message);

    return hash;
}

const uint32_t* NX_ComputeSHA1(void* data, size_t dataSize)
{
    /* --- SHA-1 constants --- */

    // Round constants (used in different phases of compression)
    constexpr uint32_t roundConstants[4] = {
        0x5A827999,  // Rounds 0-19
        0x6ED9EBA1,  // Rounds 20-39
        0x8F1BBCDC,  // Rounds 40-59
        0xCA62C1D6   // Rounds 60-79
    };

    /* --- Initialize hash state (SHA-1 magic numbers) --- */

    static uint32_t hash[5];
    hash[0] = 0x67452301;
    hash[1] = 0xEFCDAB89;
    hash[2] = 0x98BADCFE;
    hash[3] = 0x10325476;
    hash[4] = 0xC3D2E1F0;

    /* --- Calculate padded message size --- */

    // Message must be padded to 448 bits (mod 512), then append 64-bit length
    // This ensures total size is a multiple of 512 bits (64 bytes)

    const size_t paddedSize = ((((dataSize + 8) / 64) + 1) * 64);

    /* --- Allocate and prepare padded message --- */

    uint8_t* message = util::calloc<uint8_t>(paddedSize);
    if (!message) {
        return hash;
    }

    // Copy original data
    SDL_memcpy(message, data, dataSize);

    // Append '1' bit (0x80 = 10000000 in binary)
    message[dataSize] = 0x80;

    // Append original message length in bits (big-endian, as 64-bit value)
    // SHA-1 uses big-endian format unlike MD5
    const uint64_t bitLength = dataSize * 8;
    for (int i = 0; i < 8; ++i) {
        message[paddedSize - 1 - i] = static_cast<uint8_t>((bitLength >> (i * 8)) & 0xFF);
    }

    /* --- Process message in 512-bit (64-byte) chunks --- */

    for (size_t offset = 0; offset < paddedSize; offset += 64)
    {
        /* --- Prepare message schedule (80 words) --- */

        uint32_t w[80] = { 0 };

        // Break chunk into sixteen 32-bit words (big-endian)
        for (int i = 0; i < 16; ++i) {
            w[i] = (static_cast<uint32_t>(message[offset + (i * 4) + 0]) << 24) |
                   (static_cast<uint32_t>(message[offset + (i * 4) + 1]) << 16) |
                   (static_cast<uint32_t>(message[offset + (i * 4) + 2]) << 8)  |
                   (static_cast<uint32_t>(message[offset + (i * 4) + 3]));
        }

        // Extend the sixteen 32-bit words into eighty 32-bit words
        for (int i = 16; i < 80; ++i) {
            const uint32_t temp = w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16];
            w[i] = (temp << 1) | (temp >> 31);  // Rotate left by 1
        }

        /* --- Initialize working variables with current hash state --- */

        uint32_t a = hash[0];
        uint32_t b = hash[1];
        uint32_t c = hash[2];
        uint32_t d = hash[3];
        uint32_t e = hash[4];

        /* --- Perform 80 operations (4 rounds of 20 operations) --- */

        for (int i = 0; i < 80; ++i)
        {
            uint32_t f, k;

            if (i < 20) {
                f = (b & c) | ((~b) & d);
                k = roundConstants[0];
            }
            else if (i < 40) {
                f = b ^ c ^ d;
                k = roundConstants[1];
            }
            else if (i < 60) {
                f = (b & c) | (b & d) | (c & d);
                k = roundConstants[2];
            }
            else {
                f = b ^ c ^ d;
                k = roundConstants[3];
            }

            const uint32_t rotatedA = (a << 5) | (a >> 27);  // Rotate left by 5
            const uint32_t temp = rotatedA + f + e + k + w[i];

            e = d;
            d = c;
            c = (b << 30) | (b >> 2);  // Rotate left by 30
            b = a;
            a = temp;
        }

        /* --- Add this chunk's hash to result --- */

        hash[0] += a;
        hash[1] += b;
        hash[2] += c;
        hash[3] += d;
        hash[4] += e;
    }

    /* --- Cleanup and return --- */

    util::free(message);

    return hash;
}

const uint32_t* NX_ComputeSHA256(void* data, size_t dataSize)
{
    /* --- SHA-256 constants --- */

    // Round constants (first 32 bits of the fractional parts of the cube roots of the first 64 primes)
    constexpr uint32_t roundConstants[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
        0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
        0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
        0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
        0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
        0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
        0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
        0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
        0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    /* --- Initialize hash state (SHA-256 magic numbers) --- */

    // First 32 bits of the fractional parts of the square roots of the first 8 primes
    static uint32_t hash[8];
    hash[0] = 0x6a09e667;
    hash[1] = 0xbb67ae85;
    hash[2] = 0x3c6ef372;
    hash[3] = 0xa54ff53a;
    hash[4] = 0x510e527f;
    hash[5] = 0x9b05688c;
    hash[6] = 0x1f83d9ab;
    hash[7] = 0x5be0cd19;

    /* --- Calculate padded message size --- */

    // Message must be padded to 448 bits (mod 512), then append 64-bit length
    // This ensures total size is a multiple of 512 bits (64 bytes)

    const uint64_t bitLength = static_cast<uint64_t>(dataSize) * 8;
    size_t paddedSize = dataSize + sizeof(uint64_t);
    paddedSize += (64 - (paddedSize % 64)) % 64;
    if (paddedSize < dataSize + sizeof(uint64_t) + 1) {
        paddedSize += 64;
    }

    /* --- Allocate and prepare padded message --- */

    uint8_t* message = util::calloc<uint8_t>(paddedSize);
    if (!message) {
        return hash;
    }

    // Copy original data
    SDL_memcpy(message, data, dataSize);

    // Append '1' bit (0x80 = 10000000 in binary)
    message[dataSize] = 0x80;

    // Append original message length in bits (big-endian, as 64-bit value)
    for (int i = 0; i < 8; ++i) {
        message[paddedSize - 8 + i] = static_cast<uint8_t>((bitLength >> (8 * (7 - i))) & 0xFF);
    }

    /* --- Process message in 512-bit (64-byte) chunks --- */

    for (size_t offset = 0; offset < paddedSize; offset += 64)
    {
        /* --- Prepare message schedule (64 words) --- */

        uint32_t w[64];

        // Break chunk into sixteen 32-bit words (big-endian)
        for (int i = 0; i < 16; ++i) {
            w[i] = (static_cast<uint32_t>(message[offset + (i * 4) + 0]) << 24) |
                   (static_cast<uint32_t>(message[offset + (i * 4) + 1]) << 16) |
                   (static_cast<uint32_t>(message[offset + (i * 4) + 2]) << 8)  |
                   (static_cast<uint32_t>(message[offset + (i * 4) + 3]));
        }

        // Extend the sixteen 32-bit words into sixty-four 32-bit words
        for (int i = 16; i < 64; ++i)
        {
            const uint32_t s0_x = w[i - 15];
            const uint32_t s0 = ((s0_x >> 7) | (s0_x << 25)) ^ ((s0_x >> 18) | (s0_x << 14)) ^ (s0_x >> 3);

            const uint32_t s1_x = w[i - 2];
            const uint32_t s1 = ((s1_x >> 17) | (s1_x << 15)) ^ ((s1_x >> 19) | (s1_x << 13)) ^ (s1_x >> 10);

            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }

        /* --- Initialize working variables with current hash state --- */

        uint32_t a = hash[0];
        uint32_t b = hash[1];
        uint32_t c = hash[2];
        uint32_t d = hash[3];
        uint32_t e = hash[4];
        uint32_t f = hash[5];
        uint32_t g = hash[6];
        uint32_t h = hash[7];

        /* --- Perform 64 rounds of compression --- */

        for (int i = 0; i < 64; ++i)
        {
            const uint32_t sum1 = ((e >> 6) | (e << 26)) ^ ((e >> 11) | (e << 21)) ^ ((e >> 25) | (e << 7));

            const uint32_t ch = (e & f) ^ ((~e) & g);
            const uint32_t t1 = h + sum1 + ch + roundConstants[i] + w[i];

            const uint32_t sum0 = ((a >> 2) | (a << 30)) ^ ((a >> 13) | (a << 19)) ^ ((a >> 22) | (a << 10));

            const uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            const uint32_t t2 = sum0 + maj;

            h = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }

        /* --- Add this chunk's hash to result --- */

        hash[0] += a;
        hash[1] += b;
        hash[2] += c;
        hash[3] += d;
        hash[4] += e;
        hash[5] += f;
        hash[6] += g;
        hash[7] += h;
    }

    /* --- Cleanup and return --- */

    util::free(message);

    return hash;
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
    return util::malloc(size);
}

void* NX_Calloc(size_t nmemb, size_t size)
{
    return util::calloc(nmemb, size);
}

void* NX_Realloc(void* ptr, size_t size)
{
    return util::realloc(ptr, size);
}

void NX_Free(void* ptr)
{
    SDL_free(ptr);
}
