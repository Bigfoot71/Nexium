/* NX_Core.h -- API declaration for Nexium's core module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_CORE_H
#define NX_CORE_H

#include "./NX_Image.h"
#include "./NX_Math.h"
#include "./NX_API.h"

#include <stdint.h>
#include <stdarg.h>

/**
 * @defgroup Core Core Module
 * @brief Core functions for application control, timing, and system utilities.
 * @{
 */

/* === Enums === */

/**
 * @defgroup Enums Core Enums
 * @brief Enumerations and constants used by Core module functions.
 * @{
 */

/**
 * @brief Keyboard key scancodes.
 *
 * Represents all possible keys on the keyboard. Values are platform-independent
 * scancodes used by the input system functions such as `NX_IsKeyPressed`.
 */
typedef enum NX_Key {
    NX_KEY_UNKNOWN = 0,
    NX_KEY_A = 4,
    NX_KEY_B = 5,
    NX_KEY_C = 6,
    NX_KEY_D = 7,
    NX_KEY_E = 8,
    NX_KEY_F = 9,
    NX_KEY_G = 10,
    NX_KEY_H = 11,
    NX_KEY_I = 12,
    NX_KEY_J = 13,
    NX_KEY_K = 14,
    NX_KEY_L = 15,
    NX_KEY_M = 16,
    NX_KEY_N = 17,
    NX_KEY_O = 18,
    NX_KEY_P = 19,
    NX_KEY_Q = 20,
    NX_KEY_R = 21,
    NX_KEY_S = 22,
    NX_KEY_T = 23,
    NX_KEY_U = 24,
    NX_KEY_V = 25,
    NX_KEY_W = 26,
    NX_KEY_X = 27,
    NX_KEY_Y = 28,
    NX_KEY_Z = 29,
    NX_KEY_1 = 30,
    NX_KEY_2 = 31,
    NX_KEY_3 = 32,
    NX_KEY_4 = 33,
    NX_KEY_5 = 34,
    NX_KEY_6 = 35,
    NX_KEY_7 = 36,
    NX_KEY_8 = 37,
    NX_KEY_9 = 38,
    NX_KEY_0 = 39,
    NX_KEY_RETURN = 40,
    NX_KEY_ESCAPE = 41,
    NX_KEY_BACKSPACE = 42,
    NX_KEY_TAB = 43,
    NX_KEY_SPACE = 44,
    NX_KEY_MINUS = 45,
    NX_KEY_EQUALS = 46,
    NX_KEY_LEFTBRACKET = 47,
    NX_KEY_RIGHTBRACKET = 48,
    NX_KEY_BACKSLASH = 49,
    NX_KEY_NONUSHASH = 50,
    NX_KEY_SEMICOLON = 51,
    NX_KEY_APOSTROPHE = 52,
    NX_KEY_GRAVE = 53,
    NX_KEY_COMMA = 54,
    NX_KEY_PERIOD = 55,
    NX_KEY_SLASH = 56,
    NX_KEY_CAPSLOCK = 57,
    NX_KEY_F1 = 58,
    NX_KEY_F2 = 59,
    NX_KEY_F3 = 60,
    NX_KEY_F4 = 61,
    NX_KEY_F5 = 62,
    NX_KEY_F6 = 63,
    NX_KEY_F7 = 64,
    NX_KEY_F8 = 65,
    NX_KEY_F9 = 66,
    NX_KEY_F10 = 67,
    NX_KEY_F11 = 68,
    NX_KEY_F12 = 69,
    NX_KEY_PRINTSCREEN = 70,
    NX_KEY_SCROLLLOCK = 71,
    NX_KEY_PAUSE = 72,
    NX_KEY_INSERT = 73,
    NX_KEY_HOME = 74,
    NX_KEY_PAGEUP = 75,
    NX_KEY_DELETE = 76,
    NX_KEY_END = 77,
    NX_KEY_PAGEDOWN = 78,
    NX_KEY_RIGHT = 79,
    NX_KEY_LEFT = 80,
    NX_KEY_DOWN = 81,
    NX_KEY_UP = 82,
    NX_KEY_NUMLOCKCLEAR = 83,
    NX_KEY_KP_DIVIDE = 84,
    NX_KEY_KP_MULTIPLY = 85,
    NX_KEY_KP_MINUS = 86,
    NX_KEY_KP_PLUS = 87,
    NX_KEY_KP_ENTER = 88,
    NX_KEY_KP_1 = 89,
    NX_KEY_KP_2 = 90,
    NX_KEY_KP_3 = 91,
    NX_KEY_KP_4 = 92,
    NX_KEY_KP_5 = 93,
    NX_KEY_KP_6 = 94,
    NX_KEY_KP_7 = 95,
    NX_KEY_KP_8 = 96,
    NX_KEY_KP_9 = 97,
    NX_KEY_KP_0 = 98,
    NX_KEY_KP_PERIOD = 99,
    NX_KEY_NONUSBACKSLASH = 100,
    NX_KEY_APPLICATION = 101,
    NX_KEY_POWER = 102,
    NX_KEY_KP_EQUALS = 103,
    NX_KEY_F13 = 104,
    NX_KEY_F14 = 105,
    NX_KEY_F15 = 106,
    NX_KEY_F16 = 107,
    NX_KEY_F17 = 108,
    NX_KEY_F18 = 109,
    NX_KEY_F19 = 110,
    NX_KEY_F20 = 111,
    NX_KEY_F21 = 112,
    NX_KEY_F22 = 113,
    NX_KEY_F23 = 114,
    NX_KEY_F24 = 115,
    NX_KEY_EXECUTE = 116,
    NX_KEY_HELP = 117,
    NX_KEY_MENU = 118,
    NX_KEY_SELECT = 119,
    NX_KEY_STOP = 120,
    NX_KEY_AGAIN = 121,
    NX_KEY_UNDO = 122,
    NX_KEY_CUT = 123,
    NX_KEY_COPY = 124,
    NX_KEY_PASTE = 125,
    NX_KEY_FIND = 126,
    NX_KEY_MUTE = 127,
    NX_KEY_VOLUMEUP = 128,
    NX_KEY_VOLUMEDOWN = 129,
    NX_KEY_KP_COMMA = 133,
    NX_KEY_KP_EQUALSAS400 = 134,
    NX_KEY_INTERNATIONAL1 = 135,
    NX_KEY_INTERNATIONAL2 = 136,
    NX_KEY_INTERNATIONAL3 = 137,
    NX_KEY_INTERNATIONAL4 = 138,
    NX_KEY_INTERNATIONAL5 = 139,
    NX_KEY_INTERNATIONAL6 = 140,
    NX_KEY_INTERNATIONAL7 = 141,
    NX_KEY_INTERNATIONAL8 = 142,
    NX_KEY_INTERNATIONAL9 = 143,
    NX_KEY_LANG1 = 144,
    NX_KEY_LANG2 = 145,
    NX_KEY_LANG3 = 146,
    NX_KEY_LANG4 = 147,
    NX_KEY_LANG5 = 148,
    NX_KEY_LANG6 = 149,
    NX_KEY_LANG7 = 150,
    NX_KEY_LANG8 = 151,
    NX_KEY_LANG9 = 152,
    NX_KEY_ALTERASE = 153,
    NX_KEY_SYSREQ = 154,
    NX_KEY_CANCEL = 155,
    NX_KEY_CLEAR = 156,
    NX_KEY_PRIOR = 157,
    NX_KEY_RETURN2 = 158,
    NX_KEY_SEPARATOR = 159,
    NX_KEY_OUT = 160,
    NX_KEY_OPER = 161,
    NX_KEY_CLEARAGAIN = 162,
    NX_KEY_CRSEL = 163,
    NX_KEY_EXSEL = 164,
    NX_KEY_KP_00 = 176,
    NX_KEY_KP_000 = 177,
    NX_KEY_THOUSANDSSEPARATOR = 178,
    NX_KEY_DECIMALSEPARATOR = 179,
    NX_KEY_CURRENCYUNIT = 180,
    NX_KEY_CURRENCYSUBUNIT = 181,
    NX_KEY_KP_LEFTPAREN = 182,
    NX_KEY_KP_RIGHTPAREN = 183,
    NX_KEY_KP_LEFTBRACE = 184,
    NX_KEY_KP_RIGHTBRACE = 185,
    NX_KEY_KP_TAB = 186,
    NX_KEY_KP_BACKSPACE = 187,
    NX_KEY_KP_A = 188,
    NX_KEY_KP_B = 189,
    NX_KEY_KP_C = 190,
    NX_KEY_KP_D = 191,
    NX_KEY_KP_E = 192,
    NX_KEY_KP_F = 193,
    NX_KEY_KP_XOR = 194,
    NX_KEY_KP_POWER = 195,
    NX_KEY_KP_PERCENT = 196,
    NX_KEY_KP_LESS = 197,
    NX_KEY_KP_GREATER = 198,
    NX_KEY_KP_AMPERSAND = 199,
    NX_KEY_KP_DBLAMPERSAND = 200,
    NX_KEY_KP_VERTICALBAR = 201,
    NX_KEY_KP_DBLVERTICALBAR = 202,
    NX_KEY_KP_COLON = 203,
    NX_KEY_KP_HASH = 204,
    NX_KEY_KP_SPACE = 205,
    NX_KEY_KP_AT = 206,
    NX_KEY_KP_EXCLAM = 207,
    NX_KEY_KP_MEMSTORE = 208,
    NX_KEY_KP_MEMRECALL = 209,
    NX_KEY_KP_MEMCLEAR = 210,
    NX_KEY_KP_MEMADD = 211,
    NX_KEY_KP_MEMSUBTRACT = 212,
    NX_KEY_KP_MEMMULTIPLY = 213,
    NX_KEY_KP_MEMDIVIDE = 214,
    NX_KEY_KP_PLUSMINUS = 215,
    NX_KEY_KP_CLEAR = 216,
    NX_KEY_KP_CLEARENTRY = 217,
    NX_KEY_KP_BINARY = 218,
    NX_KEY_KP_OCTAL = 219,
    NX_KEY_KP_DECIMAL = 220,
    NX_KEY_KP_HEXADECIMAL = 221,
    NX_KEY_LCTRL = 224,
    NX_KEY_LSHIFT = 225,
    NX_KEY_LALT = 226,
    NX_KEY_LGUI = 227,
    NX_KEY_RCTRL = 228,
    NX_KEY_RSHIFT = 229,
    NX_KEY_RALT = 230,
    NX_KEY_RGUI = 231,
    NX_KEY_MODE = 257,
    NX_KEY_SLEEP = 258,
    NX_KEY_WAKE = 259,
    NX_KEY_CHANNEL_INCREMENT = 260,
    NX_KEY_CHANNEL_DECREMENT = 261,
    NX_KEY_MEDIA_PLAY = 262,
    NX_KEY_MEDIA_PAUSE = 263,
    NX_KEY_MEDIA_RECORD = 264,
    NX_KEY_MEDIA_FAST_FORWARD = 265,
    NX_KEY_MEDIA_REWIND = 266,
    NX_KEY_MEDIA_NEXT_TRACK = 267,
    NX_KEY_MEDIA_PREVIOUS_TRACK = 268,
    NX_KEY_MEDIA_STOP = 269,
    NX_KEY_MEDIA_EJECT = 270,
    NX_KEY_MEDIA_PLAY_PAUSE = 271,
    NX_KEY_MEDIA_SELECT = 272,
    NX_KEY_AC_NEW = 273,
    NX_KEY_AC_OPEN = 274,
    NX_KEY_AC_CLOSE = 275,
    NX_KEY_AC_EXIT = 276,
    NX_KEY_AC_SAVE = 277,
    NX_KEY_AC_PRINT = 278,
    NX_KEY_AC_PROPERTIES = 279,
    NX_KEY_AC_SEARCH = 280,
    NX_KEY_AC_HOME = 281,
    NX_KEY_AC_BACK = 282,
    NX_KEY_AC_FORWARD = 283,
    NX_KEY_AC_STOP = 284,
    NX_KEY_AC_REFRESH = 285,
    NX_KEY_AC_BOOKMARKS = 286,
    NX_KEY_SOFTLEFT = 287,
    NX_KEY_SOFTRIGHT = 288,
    NX_KEY_CALL = 289,
    NX_KEY_ENDCALL = 290,
    NX_KEY_RESERVED = 400,
    NX_KEY_COUNT = 512
} NX_Key;

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

/**
 * @brief Logging levels.
 *
 * Used to control verbosity and priority of log messages.
 * Functions like `NX_Log` and `NX_SetLogPriority` use these levels.
 */
typedef enum NX_LogLevel {
    NX_LOG_INVALID,         ///< Invalid log level
    NX_LOG_TRACE,           ///< Trace-level messages for detailed debugging
    NX_LOG_VERBOSE,         ///< Verbose messages
    NX_LOG_DEBUG,           ///< Debug messages
    NX_LOG_INFO,            ///< Informational messages
    NX_LOG_WARN,            ///< Warning messages
    NX_LOG_ERROR,           ///< Error messages
    NX_LOG_FATAL,           ///< Fatal error messages
    NX_LOG_COUNT            ///< Number of log levels
} NX_LogLevel;

/** @} */ // end of Enums

/* === API Functions === */

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @defgroup Update Frame Update
 * @brief Functions for managing the main loop and frame stepping.
 * @{
 */

/**
 * @brief Run one iteration of the main loop.
 * @return false if the program should close.
 * @note Can be used directly in the main loop: `while (NX_FrameStep())`.
 */
NXAPI bool NX_FrameStep(void);

/** @} */ // end of Update

/**
 * @defgroup Time Time Management
 * @brief Functions for time measurement and frame control.
 * @{
 */

/**
 * @brief Gets the current system time in nanoseconds since Jan 1, 1970 (UTC).
 */
int64_t NX_GetCurrentTimeNS(void);

/**
 * @brief Gets the current system time in seconds since Jan 1, 1970 (UTC).
 */
double NX_GetCurrentTime(void);

/**
 * @brief Gets the elapsed time since library initialization.
 * @return Elapsed time in seconds.
 */
NXAPI double NX_GetElapsedTime(void);

/**
 * @brief Gets the time taken by the last frame.
 * @return Frame time in seconds.
 */
NXAPI double NX_GetFrameTime(void);

/**
 * @brief Sets the target frame rate (FPS).
 * @param fps Desired target FPS.
 */
NXAPI void NX_SetTargetFPS(int fps);

/**
 * @brief Gets the current frame rate (FPS).
 */
NXAPI int NX_GetFPS(void);

/** @} */ // end of Time

/**
 * @defgroup Display Display Management
 * @brief Functions for managing display settings and properties.
 * @{
 */

/**
 * @brief Sets the vertical synchronization mode.
 *
 * VSync synchronizes the frame rate with the display's refresh rate to reduce screen tearing.
 *
 * @param mode The VSync mode:
 *   -  0 : Disabled (unlimited FPS, may cause tearing).
 *   -  1 : Enabled (FPS capped to display refresh rate).
 *   - -1 : Adaptive (enabled only if FPS > refresh rate, if supported).
 *
 * @return true if the VSync mode was successfully applied, false otherwise.
 *
 * @note Adaptive VSync is not supported on all platforms or drivers.
 */
NXAPI bool NX_SetVSync(int mode);

/**
 * @brief Gets the display scaling factor.
 * @return Display scale (e.g., 1.0 for standard DPI, >1.0 for HiDPI/Retina).
 */
NXAPI float NX_GetDisplayScale(void);

/**
 * @brief Gets the display DPI (dots per inch).
 */
NXAPI float NX_GetDisplayGetDPI(void);

/**
 * @brief Gets the current display index.
 * @return Display index, useful in multi-monitor setups.
 */
NXAPI int NX_GetDisplayIndex(void);

/**
 * @brief Gets the display size in pixels.
 * @return Integer 2D vector (width, height).
 */
NXAPI NX_IVec2 NX_GetDisplaySize(void);

/**
 * @brief Gets the display size in pixels as floats.
 * @return Float 2D vector (width, height).
 */
NXAPI NX_Vec2 NX_GetDisplaySizeF(void);

/** @} */ // end of Display

/**
 * @defgroup Window Window Management
 * @brief Functions for managing the main application window.
 * @{
 */

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

/** @} */ // end of Window

/**
 * @defgroup Cursor Cursor Management
 * @brief Functions for controlling the mouse cursor.
 * @{
 */

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

/** @} */ // end of Cursor

/**
 * @defgroup Mouse Mouse Input
 * @brief Functions for handling mouse input and state.
 * @{
 */

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
NXAPI bool NX_IsMouseButtonPressed(NX_MouseButton button);

/**
 * @brief Checks if a mouse button is currently released.
 *
 * @param button Mouse button to query.
 * @return true if the button is released, false otherwise.
 */
NXAPI bool NX_IsMouseButtonReleased(NX_MouseButton button);

/**
 * @brief Checks if a mouse button was just pressed this frame.
 *
 * @param button Mouse button to query.
 * @return true if the button was pressed this frame, false otherwise.
 */
NXAPI bool NX_IsMouseButtonJustPressed(NX_MouseButton button);

/**
 * @brief Checks if a mouse button was just released this frame.
 *
 * @param button Mouse button to query.
 * @return true if the button was released this frame, false otherwise.
 */
NXAPI bool NX_IsMouseButtonJustReleased(NX_MouseButton button);

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

/** @} */ // end of Mouse

/**
 * @defgroup Keyboard Keyboard Input
 * @brief Functions for handling keyboard input and state.
 * @{
 */

/**
 * @brief Checks if a key is currently pressed.
 *
 * @param key Key to query.
 * @return true if the key is pressed, false otherwise.
 */
NXAPI bool NX_IsKeyPressed(NX_Key key);

/**
 * @brief Checks if a key is currently released.
 *
 * @param key Key to query.
 * @return true if the key is released, false otherwise.
 */
NXAPI bool NX_IsKeyReleased(NX_Key key);

/**
 * @brief Checks if a key was just pressed during the current frame.
 *
 * @param key Key to query.
 * @return true if the key was pressed this frame, false otherwise.
 */
NXAPI bool NX_IsKeyJustPressed(NX_Key key);

/**
 * @brief Checks if a key was just released during the current frame.
 *
 * @param key Key to query.
 * @return true if the key was released this frame, false otherwise.
 */
NXAPI bool NX_IsKeyJustReleased(NX_Key key);

/**
 * @brief Generates a 2D vector from four directional keys.
 *
 * This function is useful for creating movement input from the keyboard.
 * For example, it can generate a normalized directional vector based on
 * arrow keys or WASD keys.
 *
 * @param up Key representing upward movement.
 * @param down Key representing downward movement.
 * @param left Key representing left movement.
 * @param right Key representing right movement.
 * @return 2D vector representing the combined input of the four keys.
 */
NXAPI NX_Vec2 NX_GetKeyVec2(NX_Key up, NX_Key down, NX_Key left, NX_Key right);

NXAPI NX_Vec3 NX_GetKeyVec3(NX_Key forward, NX_Key backward, NX_Key left, NX_Key right);

/** @} */ // end of Keyboard

/**
 * @defgroup FileSystem File System
 * @brief Functions for managing virtual and physical files, directories, and archives.
 *
 * This module provides a unified interface to handle:
 *  - Virtual file system (search paths, mounted archives)
 *  - File and directory operations
 *  - File I/O for binary and text files
 *  - Access to base, write, and preferences directories
 *
 * It abstracts platform differences and allows working with files and archives
 * in a consistent way across systems.
 * @{
 */

/* --- Virtual File System --- */

/**
 * @brief Add directory or archive to search path
 * @param path Physical path to directory or archive
 * @param appendToEnd If true, add to end of search order; if false, add to beginning
 * @return True on success, false on failure
 */
NXAPI bool NX_AddSearchPath(const char* path, bool appendToEnd);

/**
 * @brief Remove directory or archive from search path
 * @param path Path that was previously added
 * @return True on success, false on failure
 */
NXAPI bool NX_RemoveSearchPath(const char* path);

/**
 * @brief Get list of all search paths in order
 * @return Null-terminated array of strings, must be freed with NX_FreeSearchPaths()
 */
NXAPI char** NX_GetSearchPaths(void);

/**
 * @brief Free search paths list returned by NX_GetSearchPaths()
 * @param paths Array to free
 */
NXAPI void NX_FreeSearchPaths(char** paths);

/**
 * @brief Mount archive to virtual file system
 * @param archivePath Physical path to archive (ZIP, 7Z, etc.)
 * @param mountPoint Virtual mount point (NULL for root)
 * @param appendToEnd If true, add to end of search order
 * @return True on success, false on failure
 */
NXAPI bool NX_MountArchive(const char* archivePath, const char* mountPoint, bool appendToEnd);

/**
 * @brief Unmount archive from virtual file system
 * @param archivePath Archive path that was previously mounted
 * @return True on success, false on failure
 */
NXAPI bool NX_UnmountArchive(const char* archivePath);

/**
 * @brief Get current write directory
 * @return Write directory path, or NULL if not set
 */
NXAPI const char* NX_GetWriteDir(void);

/**
 * @brief Set write directory for file operations
 * @param path Physical directory path for writing files
 * @return True on success, false on failure
 */
NXAPI bool NX_SetWriteDir(const char* path);

/**
 * @brief Get executable's base directory
 * @return Base directory path (read-only)
 */
NXAPI const char* NX_GetBaseDir(void);

/**
 * @brief Get user preferences directory
 * @param org Organization name
 * @param app Application name
 * @return Preferences directory path (platform-specific)
 */
NXAPI const char* NX_GetPrefDir(const char* org, const char* app);

/* --- File Information --- */

/**
 * @brief Check if file or directory exists in virtual file system
 * @param filePath Virtual file path
 * @return True if exists, false otherwise
 */
NXAPI bool NX_FileExists(const char* filePath);

/**
 * @brief Check if path is a directory
 * @param path Virtual path to check
 * @return True if directory, false otherwise
 */
NXAPI bool NX_IsDirectory(const char* path);

/**
 * @brief Check if path is a regular file
 * @param path Virtual path to check
 * @return True if file, false otherwise
 */
NXAPI bool NX_IsFile(const char* path);

/**
 * @brief Get file size without opening
 * @param filePath Virtual file path
 * @return File size in bytes, or 0 on error
 */
NXAPI size_t NX_GetFileSize(const char* filePath);

/**
 * @brief Get real physical path of virtual file
 * @param filePath Virtual file path
 * @return Physical directory containing the file, or NULL if not found
 */
NXAPI const char* NX_GetRealPath(const char* filePath);

/* --- Directory Operations --- */

/**
 * @brief List directory contents
 * @param dirPath Virtual directory path
 * @return Null-terminated array of filenames, must be freed with NX_FreeDirectoryList()
 */
NXAPI char** NX_ListDirectory(const char* dirPath);

/**
 * @brief Free directory list returned by NX_ListDirectory()
 * @param list Array to free
 */
NXAPI void NX_FreeDirectoryList(char** list);

/**
 * @brief Create directory in write directory
 * @param dirPath Virtual directory path
 * @return True on success, false on failure
 */
NXAPI bool NX_CreateDirectory(const char* dirPath);

/**
 * @brief Delete file from write directory
 * @param filePath Virtual file path
 * @return True on success, false on failure
 */
NXAPI bool NX_DeleteFile(const char* filePath);

/* --- File I/O --- */

/**
 * @brief Load binary file into memory
 * @param filePath Virtual file path
 * @param size Pointer to store file size
 * @return File data buffer (must be freed), or NULL on failure
 */
NXAPI void* NX_LoadFile(const char* filePath, size_t* size);

/**
 * @brief Load text file into memory (null-terminated)
 * @param filePath Virtual file path
 * @return Text buffer (must be freed), or NULL on failure
 */
NXAPI char* NX_LoadFileText(const char* filePath);

/**
 * @brief Write binary data to file in write directory
 * @param filePath Virtual file path
 * @param data Data to write
 * @param size Data size in bytes
 * @return True on success, false on failure
 */
NXAPI bool NX_WriteFile(const char* filePath, const void* data, size_t size);

/**
 * @brief Write text data to file in write directory
 * @param filePath Virtual file path
 * @param data Text data to write
 * @param size Data size in bytes
 * @return True on success, false on failure
 */
NXAPI bool NX_WriteFileText(const char* filePath, const char* data, size_t size);

/** @} */ // end of FileSystem

/**
 * @defgroup DataEncoding Data Encoding and Compression
 * @brief Functions for data compression, encoding, and hashing operations
 * @{
 */

/**
 * @brief Compresses binary data using zlib DEFLATE algorithm
 *
 * Creates a compressed buffer with an 8-byte header containing the uncompressed size
 * followed by zlib-compressed data. The caller must free the returned buffer using NX_Free().
 *
 * Format: [8 bytes: uncompressed size][compressed data with zlib header and checksum]
 *
 * @param data Pointer to the data to compress
 * @param dataSize Size of the input data in bytes
 * @param outputSize Pointer to receive the size of the compressed output (header + compressed data)
 * @return Pointer to the allocated compressed buffer, or NULL on failure
 */
NXAPI void* NX_CompressData(const void* data, size_t dataSize, size_t* outputSize);

/**
 * @brief Decompresses binary data compressed by NX_CompressData()
 * 
 * Reads the 8-byte uncompressed size header, allocates the appropriate buffer, and decompresses
 * the zlib data. The caller must free the returned buffer using NX_Free().
 * 
 * @param data Pointer to the compressed data (with 8-byte header)
 * @param dataSize Size of the compressed data including the header
 * @param outputSize Pointer to receive the size of the decompressed output
 * @return Pointer to the allocated decompressed buffer, or NULL on failure
 */
NXAPI void* NX_DecompressData(const void* data, size_t dataSize, size_t* outputSize);

/**
 * @brief Compresses a null-terminated text string using zlib DEFLATE algorithm
 * 
 * Convenience wrapper around NX_CompressData() for text strings. Uses strlen() to determine
 * the text length and compresses without including the null terminator.
 * The caller must free the returned buffer using NX_Free().
 * 
 * @param text Pointer to the null-terminated text string to compress
 * @param outputSize Pointer to receive the size of the compressed output
 * @return Pointer to the allocated compressed buffer, or NULL on failure
 */
NXAPI void* NX_CompressText(const char* text, size_t* outputSize);

/**
 * @brief Decompresses text data and adds a null terminator
 * 
 * Decompresses data compressed by NX_CompressText() and automatically appends a null terminator,
 * making the result safe to use as a C string. The caller must free the returned buffer using NX_Free().
 * 
 * @param data Pointer to the compressed data (with 8-byte header)
 * @param dataSize Size of the compressed data including the header
 * @return Pointer to the allocated null-terminated string, or NULL on failure
 */
NXAPI char* NX_DecompressText(const void* data, size_t dataSize);

/**
 * @brief Encodes binary data to Base64 ASCII string
 * 
 * Converts binary data to Base64 encoding (RFC 4648). The output is a null-terminated ASCII string.
 * The caller must free the returned buffer using NX_Free().
 * 
 * @param data Pointer to the binary data to encode
 * @param dataSize Size of the input data in bytes
 * @param outputSize Pointer to receive the size of the encoded string (excluding null terminator)
 * @return Pointer to the allocated null-terminated Base64 string, or NULL on failure
 */
NXAPI char* NX_EncodeBase64(const void* data, size_t dataSize, size_t* outputSize);

/**
 * @brief Decodes a Base64 ASCII string to binary data
 * 
 * Converts a Base64-encoded string (RFC 4648) back to binary data. The input string length
 * must be a multiple of 4. The caller must free the returned buffer using NX_Free().
 * 
 * @param text Pointer to the null-terminated Base64 string
 * @param outputSize Pointer to receive the size of the decoded binary data
 * @return Pointer to the allocated binary data buffer, or NULL on failure
 */
NXAPI void* NX_DecodeBase64(const char* text, size_t* outputSize);

/**
 * @brief Computes the CRC32 checksum of data
 * 
 * Calculates a 32-bit cyclic redundancy check (CRC32) value for error detection.
 * 
 * @param data Pointer to the data to checksum
 * @param dataSize Size of the data in bytes
 * @return The CRC32 checksum value
 */
uint32_t NX_ComputeCRC32(void* data, size_t dataSize);

/**
 * @brief Computes the MD5 hash of data
 * 
 * Calculates a 128-bit MD5 hash. Returns a pointer to a static array of 4 uint32_t values (16 bytes total).
 * The returned pointer is valid until the next call to this function.
 * 
 * @param data Pointer to the data to hash
 * @param dataSize Size of the data in bytes
 * @return Pointer to a static uint32_t[4] array containing the MD5 hash
 */
const uint32_t* NX_ComputeMD5(void* data, size_t dataSize);

/**
 * @brief Computes the SHA-1 hash of data
 * 
 * Calculates a 160-bit SHA-1 hash. Returns a pointer to a static array of 5 uint32_t values (20 bytes total).
 * The returned pointer is valid until the next call to this function.
 * 
 * @param data Pointer to the data to hash
 * @param dataSize Size of the data in bytes
 * @return Pointer to a static uint32_t[5] array containing the SHA-1 hash
 */
const uint32_t* NX_ComputeSHA1(void* data, size_t dataSize);

/**
 * @brief Computes the SHA-256 hash of data
 * 
 * Calculates a 256-bit SHA-256 hash. Returns a pointer to a static array of 8 uint32_t values (32 bytes total).
 * The returned pointer is valid until the next call to this function.
 * 
 * @param data Pointer to the data to hash
 * @param dataSize Size of the data in bytes
 * @return Pointer to a static uint32_t[8] array containing the SHA-256 hash
 */
const uint32_t* NX_ComputeSHA256(void* data, size_t dataSize);

/** @} */ // end of DataEncoding

/**
 * @defgroup Clipboard Clipboard Functions
 * @brief Functions to interact with the system clipboard.
 * @{
 */

/**
 * @brief Set the system clipboard content.
 * @param text Null-terminated string to copy to the clipboard.
 * @return True if the text was successfully copied, false otherwise.
 */
NXAPI bool NX_SetClipboardText(const char* text);

/**
 * @brief Get the current content of the system clipboard.
 * @return Null-terminated string from the clipboard, or NULL if empty.
 * @note The returned string may be invalidated on the next clipboard operation.
 */
NXAPI const char* NX_GetClipboardText(void);

/**
 * @brief Check if the clipboard contains any text.
 * @return True if there is text available, false otherwise.
 */
NXAPI bool NX_HasClipboardText(void);

/** @} */ // end of Clipboard

/**
 * @defgroup Logging Logging Functions
 * @brief Functions for logging messages.
 * @{
 */

/**
 * @brief Set the minimum log priority.
 * @param log Log level to set as minimum (messages below this level will be ignored).
 */
NXAPI void NX_SetLogPriority(NX_LogLevel log);

/**
 * @brief Log a message with the specified log level.
 * @param log Log level for this message.
 * @param msg Null-terminated format string.
 */
NXAPI void NX_Log(NX_LogLevel log, const char* msg, ...);

/**
 * @brief Log a message using a va_list.
 * @param log Log level for this message.
 * @param msg Null-terminated format string.
 * @param args Variable argument list.
 */
NXAPI void NX_LogVA(NX_LogLevel log, const char* msg, va_list args);

/** @brief Log a trace message */
NXAPI void NX_LogT(const char* msg, ...);

/** @brief Log a verbose message */
NXAPI void NX_LogV(const char* msg, ...);

/** @brief Log a debug message */
NXAPI void NX_LogD(const char* msg, ...);

/** @brief Log an info message */
NXAPI void NX_LogI(const char* msg, ...);

/** @brief Log a warning message */
NXAPI void NX_LogW(const char* msg, ...);

/** @brief Log an error message */
NXAPI void NX_LogE(const char* msg, ...);

/** @brief Log a fatal error message */
NXAPI void NX_LogF(const char* msg, ...);

/** @} */ // end of Logging

/**
 * @defgroup Memory Memory Functions
 * @brief Functions for memory allocation and deallocation.
 * @{
 */

/**
 * @brief Allocates a memory block of the given size.
 * @param size Number of bytes to allocate.
 * @return Pointer to the allocated memory, or NULL if allocation fails.
 */
NXAPI void* NX_Malloc(size_t size);

/**
 * @brief Allocates and zero-initializes an array.
 * @param nmemb Number of elements.
 * @param size Size of each element in bytes.
 * @return Pointer to the allocated memory, or NULL if allocation fails.
 */
NXAPI void* NX_Calloc(size_t nmemb, size_t size);

/**
 * @brief Resizes a previously allocated memory block.
 * @param ptr Pointer to the memory block to resize.
 * @param size New size in bytes.
 * @return Pointer to the reallocated memory, or NULL if allocation fails.
 */
NXAPI void* NX_Realloc(void* ptr, size_t size);

/**
 * @brief Frees a previously allocated memory block.
 * @param ptr Pointer to the memory block to free.
 */
NXAPI void NX_Free(void* ptr);

/** @} */ // end of Memory

/** @} */ // end of Core

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_CORE_H
