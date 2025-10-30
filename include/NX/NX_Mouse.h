/* NX_Mouse.h -- API declaration for Nexium's mouse module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_MOUSE_H
#define NX_MOUSE_H

#include "./NX_Math.h"
#include "./NX_API.h"

// ============================================================================
// TYPES DEFINITIONS
// ============================================================================

/**
 * @brief Mouse buttons.
 *
 * Each constant represents a mouse button that can be queried with
 * `NX_IsMouseButtonPressed` and similar functions.
 */
typedef uint8_t NX_MouseButton;

#define NX_MOUSE_BUTTON_LEFT    0x01  ///< Left mouse button
#define NX_MOUSE_BUTTON_MIDDLE  0x02  ///< Middle mouse button (wheel click)
#define NX_MOUSE_BUTTON_RIGHT   0x04  ///< Right mouse button
#define NX_MOUSE_BUTTON_X1      0x08  ///< Extra button 1 (usually back)
#define NX_MOUSE_BUTTON_X2      0x10  ///< Extra button 2 (usually forward)

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Enables or disables mouse capture.
 *
 * When capture is enabled, the mouse is locked to the window and its
 * movement is reported as relative instead of absolute coordinates.
 *
 * @param enabled true to enable capture, false to disable.
 */
NXAPI void NX_CaptureMouse(bool enabled);

/**
 * @brief Checks if a mouse button is currently pressed.
 *
 * @param button Mouse button to query.
 * @return true if the button is pressed, false otherwise.
 */
NXAPI bool NX_IsMouseButtonPressed(NX_MouseButton buttons);

/**
 * @brief Checks if a mouse button is currently released.
 *
 * @param button Mouse button to query.
 * @return true if the button is released, false otherwise.
 */
NXAPI bool NX_IsMouseButtonReleased(NX_MouseButton buttons);

/**
 * @brief Checks if a mouse button was just pressed this frame.
 *
 * @param button Mouse button to query.
 * @return true if the button was pressed this frame, false otherwise.
 */
NXAPI bool NX_IsMouseButtonJustPressed(NX_MouseButton buttons);

/**
 * @brief Checks if a mouse button was just released this frame.
 *
 * @param button Mouse button to query.
 * @return true if the button was released this frame, false otherwise.
 */
NXAPI bool NX_IsMouseButtonJustReleased(NX_MouseButton buttons);

/**
 * @brief Gets the current mouse position in window coordinates.
 *
 * @return Mouse position as a 2D vector (x, y).
 */
NXAPI NX_Vec2 NX_GetMousePosition(void);

/**
 * @brief Sets the mouse position in window coordinates.
 *
 * @param p Target position as a 2D vector (x, y).
 */
NXAPI void NX_SetMousePosition(NX_Vec2 p);

/**
 * @brief Gets the mouse movement delta since the last frame.
 *
 * @return Movement delta as a 2D vector (x, y).
 */
NXAPI NX_Vec2 NX_GetMouseDelta(void);

/**
 * @brief Gets the mouse wheel movement for the current frame.
 *
 * Typically, the y component represents vertical scrolling and the x
 * component represents horizontal scrolling (if supported).
 *
 * @return Mouse wheel movement as a 2D vector.
 */
NXAPI NX_Vec2 NX_GetMouseWheel(void);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_MOUSE_H
