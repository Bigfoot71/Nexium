/* NX_Window.h -- API declaration for Nexium's window module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_WINDOW_H
#define NX_WINDOW_H

#include "./NX_Image.h"
#include "./NX_Math.h"
#include "./NX_API.h"

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Gets the current window title.
 * @return A pointer to a null-terminated string containing the window title.
 */
NXAPI const char* NX_GetWindowTitle(void);

/**
 * @brief Sets the window title.
 * @param title The new window title as a null-terminated string.
 */
NXAPI void NX_SetWindowTitle(const char* title);

/**
 * @brief Sets the window icon.
 * @param icon Image to use as icon. Must be in RGB8, RGBA8, RGB16F, RGBA16F, RGB32F, or RGBA32F format.
 * @note The icon data is copied, the original image can be freed after this call.
 */
NXAPI void NX_SetWindowIcon(const NX_Image* icon);

/**
 * @brief Gets the window width in pixels.
 */
NXAPI int NX_GetWindowWidth(void);

/**
 * @brief Gets the window height in pixels.
 */
NXAPI int NX_GetWindowHeight(void);

/**
 * @brief Gets the window size in pixels.
 * @return Integer 2D vector (width, height).
 */
NXAPI NX_IVec2 NX_GetWindowSize(void);

/**
 * @brief Gets the window size in pixels as floats.
 * @return Float 2D vector (width, height).
 */
NXAPI NX_Vec2 NX_GetWindowSizeF(void);

/**
 * @brief Sets the window size in pixels.
 * @param w Width in pixels.
 * @param h Height in pixels.
 */
NXAPI void NX_SetWindowSize(int w, int h);

/**
 * @brief Sets the minimum window size in pixels.
 * @param w Minimum width in pixels.
 * @param h Minimum height in pixels.
 */
NXAPI void NX_SetWindowMinSize(int w, int h);

/**
 * @brief Sets the maximum window size in pixels.
 * @param w Maximum width in pixels.
 * @param h Maximum height in pixels.
 */
NXAPI void NX_SetWindowMaxSize(int w, int h);

/**
 * @brief Gets the window position on the screen.
 * @return Integer 2D vector (x, y).
 */
NXAPI NX_IVec2 NX_GetWindowPosition(void);

/**
 * @brief Sets the window position on the screen.
 * @param x X coordinate of the top-left corner.
 * @param y Y coordinate of the top-left corner.
 */
NXAPI void NX_SetWindowPosition(int x, int y);

/**
 * @brief Checks if the window is currently fullscreen.
 * @return true if fullscreen, false otherwise.
 */
NXAPI bool NX_IsWindowFullscreen(void);

/**
 * @brief Enables or disables fullscreen mode.
 * @param enabled true to enable fullscreen, false to restore windowed mode.
 */
NXAPI void NX_SetWindowFullscreen(bool enabled);

/**
 * @brief Checks if the window is resizable by the user.
 * @return true if resizable, false otherwise.
 */
NXAPI bool NX_IsWindowResizable(void);

/**
 * @brief Enables or disables window resizing by the user.
 * @param resizable true to allow resizing, false to fix window size.
 */
NXAPI void NX_SetWindowResizable(bool resizable);

/**
 * @brief Checks if the window is currently visible.
 * @return true if visible, false otherwise.
 */
NXAPI bool NX_IsWindowVisible(void);

/**
 * @brief Minimizes the window to the taskbar/dock.
 */
NXAPI void NX_MinimizeWindow(void);

/**
 * @brief Maximizes the window to occupy the available screen space.
 */
NXAPI void NX_MaximizeWindow(void);

/**
 * @brief Restores the window from minimized or maximized state.
 */
NXAPI void NX_RestoreWindow(void);

/**
 * @brief Shows the window if it is hidden.
 */
NXAPI void NX_ShowWindow(void);

/**
 * @brief Hides the window from the screen without closing it.
 */
NXAPI void NX_HideWindow(void);

/**
 * @brief Checks if the window currently has input focus.
 * @return true if focused, false otherwise.
 */
NXAPI bool NX_IsWindowFocused(void);

/**
 * @brief Brings the window to the foreground and gives it focus.
 */
NXAPI void NX_FocusWindow(void);

/**
 * @brief Checks if the window has borders.
 * @return true if bordered, false otherwise.
 */
NXAPI bool NX_IsWindowBordered(void);

/**
 * @brief Enables or disables window borders.
 * @param bordered true to show borders, false to remove them.
 */
NXAPI void NX_SetWindowBordered(bool bordered);

/**
 * @brief Checks if the cursor is currently grabbed.
 *
 * When the cursor is grabbed, it is confined to the window and cannot leave it.
 *
 * @return true if the cursor is grabbed, false otherwise.
 */
NXAPI bool NX_IsCursorGrabbed(void);

/**
 * @brief Grabs or releases the cursor.
 *
 * Grabbing the cursor confines it to the window and is often used in
 * first-person camera controls or games requiring locked mouse input.
 *
 * @param grab true to grab the cursor, false to release it.
 */
NXAPI void NX_GrabCursor(bool grab);

/**
 * @brief Shows the cursor.
 */
NXAPI void NX_ShowCursor(void);

/**
 * @brief Hides the cursor.
 */
NXAPI void NX_HideCursor(void);

/**
 * @brief Checks if the cursor is currently visible.
 * @return true if visible, false otherwise.
 */
NXAPI bool NX_IsCursorVisible(void);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_WINDOW_H
