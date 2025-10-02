/* HP_Init.h -- API declaration for Hyperion initialization
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef HP_INIT_H
#define HP_INIT_H

#include "./HP_Math.h"
#include <stdint.h>

/* === Enums === */

/**
 * @brief Configuration flags type for Hyperion library
 *
 * Bitfield type used to configure window and library behavior.
 * Multiple flags can be combined using bitwise OR operations.
 */
typedef uint64_t HP_Flags;

/** @defgroup InitFlags Initialization Flags
 *  @brief Configuration flags for library and window initialization
 *  @{
 */

#define HP_FLAG_VSYNC_HINT              (1 <<  0)  ///< Enable vertical synchronization hint
#define HP_FLAG_FULLSCREEN              (1 <<  1)  ///< Create window in fullscreen mode
#define HP_FLAG_WINDOW_OCCLUDED         (1 <<  2)  ///< Window is occluded by other windows
#define HP_FLAG_WINDOW_HIDDEN           (1 <<  3)  ///< Create window initially hidden
#define HP_FLAG_WINDOW_BORDERLESS       (1 <<  4)  ///< Create borderless window
#define HP_FLAG_WINDOW_RESIZABLE        (1 <<  5)  ///< Window can be resized by user
#define HP_FLAG_WINDOW_MINIMIZED        (1 <<  6)  ///< Window starts minimized
#define HP_FLAG_WINDOW_MAXIMIZED        (1 <<  7)  ///< Window starts maximized
#define HP_FLAG_WINDOW_TOPMOST          (1 <<  8)  ///< Window stays on top of others
#define HP_FLAG_WINDOW_TRANSPARENT      (1 <<  9)  ///< Enable window transparency
#define HP_FLAG_WINDOW_NOT_FOCUSABLE    (1 << 10)  ///< Window cannot receive focus
#define HP_FLAG_MOUSE_GRABBED           (1 << 11)  ///< Mouse cursor is grabbed to window
#define HP_FLAG_MOUSE_CAPTURE           (1 << 12)  ///< Mouse input is captured
#define HP_FLAG_MOUSE_RELATIVE          (1 << 13)  ///< Mouse reports relative motion
#define HP_FLAG_MOUSE_FOCUS             (1 << 14)  ///< Mouse focus is active
#define HP_FLAG_INPUT_FOCUS             (1 << 15)  ///< Input focus is active
#define HP_FLAG_KEYBOARD_GRABBED        (1 << 16)  ///< Keyboard input is grabbed
#define HP_FLAG_HIGH_PIXEL_DENSITY      (1 << 17)  ///< Enable high DPI support

/** @} */ // end of InitFlags

/* === Structures === */

/**
 * @brief Application description structure for extended initialization
 *
 * Contains detailed configuration options for the Hyperion library,
 * including rendering settings, application metadata, and custom memory allocators.
 */
typedef struct HP_AppDesc {

    HP_Flags flags;             ///< Combination of HP_FLAG_XXX values
    int targetFPS;              ///< Target framerate for CPU limiting, if <= 0 no limit is applied

    const char* name;           ///< Application name
    const char* version;        ///< Application version string
    const char* identifier;     ///< Unique application identifier

    struct {
        HP_IVec2 resolution;    ///< Internal framebuffer dimensions, if component <= 0 uses primary monitor resolution
        int sampleCount;        ///< MSAA sample count for 3D rendering, if <= 1 disables MSAA
        int shadowRes;          ///< Shadow map resolution, if <= 0 defaults to 2048x2048
        bool shadowCubeMip;     ///< Indicates whether to generate mipmaps for shadow maps (omni lights)
        bool shadow2DMip;       ///< Indicates whether to generate mipmaps for shadow maps (dir/spot lights)
    } render3D;

    struct {
        HP_IVec2 resolution;    ///< Internal framebuffer dimensions, if component <= 0 uses primary monitor resolution
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

} HP_AppDesc;

/* === API Functions === */

#if defined(__cplusplus)
extern "C" {
#endif

/** @defgroup Init Initialization Functions
 *  @brief Functions for library initialization and shutdown
 *  @{
 */

/**
 * @brief Initialize the Hyperion library with a window.
 *
 * This function initializes the Hyperion library, creating a window with
 * the specified title, width, height, and configuration flags. Must be
 * called before any other Hyperion functions.
 *
 * @param title The title of the window. Must not be NULL.
 * @param w Width of the window in pixels.
 * @param h Height of the window in pixels.
 * @param flags Configuration flags for initialization (HP_Flags). Can be zero.
 * @return Returns true if the library initialized successfully, false otherwise.
 *         If initialization fails, the reason will be logged.
 */
HPAPI bool HP_Init(const char* title, int w, int h, HP_Flags flags);

/**
 * @brief Initialize the Hyperion library with extended application description.
 *
 * This function initializes the Hyperion library with more detailed
 * information about your application. It creates a window and configures
 * the library according to the provided application description.
 *
 * @param title The title of the window. Must not be NULL.
 * @param w Width of the window in pixels.
 * @param h Height of the window in pixels.
 * @param desc Pointer to an HP_AppDesc structure describing your application.
 *             Must not be NULL, but individual fields can be zero if not needed.
 * @return Returns true if Hyperion initialized successfully, false otherwise.
 *         If initialization fails, the reason will be written to the logs.
 *
 * @note The HP_AppDesc structure itself must be valid (non-NULL).
 *       Fields that are not used can be set to zero.
 * @note The HP_AppDesc pointer is not const: some fields may be adjusted
 *       automatically (e.g. defaults or compatibility), allowing you to
 *       inspect the updated values afterwards.
 */
HPAPI bool HP_InitEx(const char* title, int w, int h, HP_AppDesc* desc);

/**
 * @brief Shut down the library and cleanup resources
 *
 * Properly shuts down Hyperion, closes the window, and frees all allocated resources.
 * Should be called before program termination.
 */
HPAPI void HP_Quit(void);

/** @} */ // end of Init

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // HP_INIT_H
