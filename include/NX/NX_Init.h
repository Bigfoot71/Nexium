/* NX_Init.h -- API declaration for Nexium initialization
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_INIT_H
#define NX_INIT_H

#include "./NX_Math.h"
#include "./NX_API.h"

#include <stdint.h>

/* === Enums === */

/**
 * @brief Configuration flags type for Nexium library
 *
 * Bitfield type used to configure window and library behavior.
 * Multiple flags can be combined using bitwise OR operations.
 */
typedef uint64_t NX_Flags;

/** @defgroup InitFlags Initialization Flags
 *  @brief Configuration flags for library and window initialization
 *  @{
 */

#define NX_FLAG_VSYNC_HINT              (1 <<  0)  ///< Enable vertical synchronization hint
#define NX_FLAG_FULLSCREEN              (1 <<  1)  ///< Create window in fullscreen mode
#define NX_FLAG_WINDOW_OCCLUDED         (1 <<  2)  ///< Window is occluded by other windows
#define NX_FLAG_WINDOW_HIDDEN           (1 <<  3)  ///< Create window initially hidden
#define NX_FLAG_WINDOW_BORDERLESS       (1 <<  4)  ///< Create borderless window
#define NX_FLAG_WINDOW_RESIZABLE        (1 <<  5)  ///< Window can be resized by user
#define NX_FLAG_WINDOW_MINIMIZED        (1 <<  6)  ///< Window starts minimized
#define NX_FLAG_WINDOW_MAXIMIZED        (1 <<  7)  ///< Window starts maximized
#define NX_FLAG_WINDOW_TOPMOST          (1 <<  8)  ///< Window stays on top of others
#define NX_FLAG_WINDOW_TRANSPARENT      (1 <<  9)  ///< Enable window transparency
#define NX_FLAG_WINDOW_NOT_FOCUSABLE    (1 << 10)  ///< Window cannot receive focus
#define NX_FLAG_MOUSE_GRABBED           (1 << 11)  ///< Mouse cursor is grabbed to window
#define NX_FLAG_MOUSE_CAPTURE           (1 << 12)  ///< Mouse input is captured
#define NX_FLAG_MOUSE_RELATIVE          (1 << 13)  ///< Mouse reports relative motion
#define NX_FLAG_MOUSE_FOCUS             (1 << 14)  ///< Mouse focus is active
#define NX_FLAG_INPUT_FOCUS             (1 << 15)  ///< Input focus is active
#define NX_FLAG_KEYBOARD_GRABBED        (1 << 16)  ///< Keyboard input is grabbed
#define NX_FLAG_HIGH_PIXEL_DENSITY      (1 << 17)  ///< Enable high DPI support

/** @} */ // end of InitFlags

/* === Structures === */

/**
 * @brief Application description structure for extended initialization
 *
 * Contains detailed configuration options for the Nexium library,
 * including rendering settings, application metadata, and custom memory allocators.
 */
typedef struct NX_AppDesc {

    NX_Flags flags;             ///< Combination of NX_FLAG_XXX values
    int targetFPS;              ///< Target framerate for CPU limiting, if <= 0 no limit is applied

    const char* name;           ///< Application name
    const char* version;        ///< Application version string
    const char* identifier;     ///< Unique application identifier

    struct {
        NX_IVec2 resolution;    ///< Internal framebuffer dimensions, if component <= 0 uses primary monitor resolution
        int sampleCount;        ///< MSAA sample count for 3D rendering, if <= 1 disables MSAA
        int shadowRes;          ///< Shadow map resolution, if <= 0 defaults to 2048x2048
    } render3D;

    struct {
        NX_IVec2 resolution;    ///< Internal framebuffer dimensions, if component <= 0 uses primary monitor resolution
        int sampleCount;        ///< MSAA sample count for 2D rendering, if <= 1 disables MSAA
    } render2D;

    /**
     * @brief Custom memory allocator functions
     *
     * If provided, these functions will be used instead of standard malloc/free.
     * All four functions must be provided together or all set to NULL.
     */
    struct {
        void*(*malloc)(size_t size);                    ///< Custom malloc function
        void*(*calloc)(size_t nmemb, size_t size);      ///< Custom calloc function
        void*(*realloc)(void *mem, size_t size);        ///< Custom realloc function
        void(*free)(void *mem);                         ///< Custom free function
    } memory;

} NX_AppDesc;

/* === API Functions === */

#if defined(__cplusplus)
extern "C" {
#endif

/** @defgroup Init Initialization Functions
 *  @brief Functions for library initialization and shutdown
 *  @{
 */

/**
 * @brief Initialize the Nexium library with a window.
 *
 * This function initializes the Nexium library, creating a window with
 * the specified title, width, height, and configuration flags. Must be
 * called before any other Nexium functions.
 *
 * @param title The title of the window. Must not be NULL.
 * @param w Width of the window in pixels.
 * @param h Height of the window in pixels.
 * @param flags Configuration flags for initialization (NX_Flags). Can be zero.
 * @return Returns true if the library initialized successfully, false otherwise.
 *         If initialization fails, the reason will be logged.
 */
NXAPI bool NX_Init(const char* title, int w, int h, NX_Flags flags);

/**
 * @brief Initialize the Nexium library with extended application description.
 *
 * This function initializes the Nexium library with more detailed
 * information about your application. It creates a window and configures
 * the library according to the provided application description.
 *
 * @param title The title of the window. Must not be NULL.
 * @param w Width of the window in pixels.
 * @param h Height of the window in pixels.
 * @param desc Pointer to an NX_AppDesc structure describing your application.
 *             Must not be NULL, but individual fields can be zero if not needed.
 * @return Returns true if Nexium initialized successfully, false otherwise.
 *         If initialization fails, the reason will be written to the logs.
 *
 * @note The NX_AppDesc structure itself must be valid (non-NULL).
 *       Fields that are not used can be set to zero.
 * @note The NX_AppDesc pointer is not const: some fields may be adjusted
 *       automatically (e.g. defaults or compatibility), allowing you to
 *       inspect the updated values afterwards.
 */
NXAPI bool NX_InitEx(const char* title, int w, int h, NX_AppDesc* desc);

/**
 * @brief Shut down the library and cleanup resources
 *
 * Properly shuts down Nexium, closes the window, and frees all allocated resources.
 * Should be called before program termination.
 */
NXAPI void NX_Quit(void);

/** @} */ // end of Init

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_INIT_H
