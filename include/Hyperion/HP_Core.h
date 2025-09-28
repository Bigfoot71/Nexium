/* HP_Core.h -- API declaration for Hyperion's core module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef HP_CORE_H
#define HP_CORE_H

#include "./HP_Platform.h"
#include "./HP_Math.h"

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
 * scancodes used by the input system functions such as `HP_IsKeyPressed`.
 */
typedef enum HP_Key {
    HP_KEY_UNKNOWN = 0,
    HP_KEY_A = 4,
    HP_KEY_B = 5,
    HP_KEY_C = 6,
    HP_KEY_D = 7,
    HP_KEY_E = 8,
    HP_KEY_F = 9,
    HP_KEY_G = 10,
    HP_KEY_H = 11,
    HP_KEY_I = 12,
    HP_KEY_J = 13,
    HP_KEY_K = 14,
    HP_KEY_L = 15,
    HP_KEY_M = 16,
    HP_KEY_N = 17,
    HP_KEY_O = 18,
    HP_KEY_P = 19,
    HP_KEY_Q = 20,
    HP_KEY_R = 21,
    HP_KEY_S = 22,
    HP_KEY_T = 23,
    HP_KEY_U = 24,
    HP_KEY_V = 25,
    HP_KEY_W = 26,
    HP_KEY_X = 27,
    HP_KEY_Y = 28,
    HP_KEY_Z = 29,
    HP_KEY_1 = 30,
    HP_KEY_2 = 31,
    HP_KEY_3 = 32,
    HP_KEY_4 = 33,
    HP_KEY_5 = 34,
    HP_KEY_6 = 35,
    HP_KEY_7 = 36,
    HP_KEY_8 = 37,
    HP_KEY_9 = 38,
    HP_KEY_0 = 39,
    HP_KEY_RETURN = 40,
    HP_KEY_ESCAPE = 41,
    HP_KEY_BACKSPACE = 42,
    HP_KEY_TAB = 43,
    HP_KEY_SPACE = 44,
    HP_KEY_MINUS = 45,
    HP_KEY_EQUALS = 46,
    HP_KEY_LEFTBRACKET = 47,
    HP_KEY_RIGHTBRACKET = 48,
    HP_KEY_BACKSLASH = 49,
    HP_KEY_NONUSHASH = 50,
    HP_KEY_SEMICOLON = 51,
    HP_KEY_APOSTROPHE = 52,
    HP_KEY_GRAVE = 53,
    HP_KEY_COMMA = 54,
    HP_KEY_PERIOD = 55,
    HP_KEY_SLASH = 56,
    HP_KEY_CAPSLOCK = 57,
    HP_KEY_F1 = 58,
    HP_KEY_F2 = 59,
    HP_KEY_F3 = 60,
    HP_KEY_F4 = 61,
    HP_KEY_F5 = 62,
    HP_KEY_F6 = 63,
    HP_KEY_F7 = 64,
    HP_KEY_F8 = 65,
    HP_KEY_F9 = 66,
    HP_KEY_F10 = 67,
    HP_KEY_F11 = 68,
    HP_KEY_F12 = 69,
    HP_KEY_PRINTSCREEN = 70,
    HP_KEY_SCROLLLOCK = 71,
    HP_KEY_PAUSE = 72,
    HP_KEY_INSERT = 73,
    HP_KEY_HOME = 74,
    HP_KEY_PAGEUP = 75,
    HP_KEY_DELETE = 76,
    HP_KEY_END = 77,
    HP_KEY_PAGEDOWN = 78,
    HP_KEY_RIGHT = 79,
    HP_KEY_LEFT = 80,
    HP_KEY_DOWN = 81,
    HP_KEY_UP = 82,
    HP_KEY_NUMLOCKCLEAR = 83,
    HP_KEY_KP_DIVIDE = 84,
    HP_KEY_KP_MULTIPLY = 85,
    HP_KEY_KP_MINUS = 86,
    HP_KEY_KP_PLUS = 87,
    HP_KEY_KP_ENTER = 88,
    HP_KEY_KP_1 = 89,
    HP_KEY_KP_2 = 90,
    HP_KEY_KP_3 = 91,
    HP_KEY_KP_4 = 92,
    HP_KEY_KP_5 = 93,
    HP_KEY_KP_6 = 94,
    HP_KEY_KP_7 = 95,
    HP_KEY_KP_8 = 96,
    HP_KEY_KP_9 = 97,
    HP_KEY_KP_0 = 98,
    HP_KEY_KP_PERIOD = 99,
    HP_KEY_NONUSBACKSLASH = 100,
    HP_KEY_APPLICATION = 101,
    HP_KEY_POWER = 102,
    HP_KEY_KP_EQUALS = 103,
    HP_KEY_F13 = 104,
    HP_KEY_F14 = 105,
    HP_KEY_F15 = 106,
    HP_KEY_F16 = 107,
    HP_KEY_F17 = 108,
    HP_KEY_F18 = 109,
    HP_KEY_F19 = 110,
    HP_KEY_F20 = 111,
    HP_KEY_F21 = 112,
    HP_KEY_F22 = 113,
    HP_KEY_F23 = 114,
    HP_KEY_F24 = 115,
    HP_KEY_EXECUTE = 116,
    HP_KEY_HELP = 117,
    HP_KEY_MENU = 118,
    HP_KEY_SELECT = 119,
    HP_KEY_STOP = 120,
    HP_KEY_AGAIN = 121,
    HP_KEY_UNDO = 122,
    HP_KEY_CUT = 123,
    HP_KEY_COPY = 124,
    HP_KEY_PASTE = 125,
    HP_KEY_FIND = 126,
    HP_KEY_MUTE = 127,
    HP_KEY_VOLUMEUP = 128,
    HP_KEY_VOLUMEDOWN = 129,
    HP_KEY_KP_COMMA = 133,
    HP_KEY_KP_EQUALSAS400 = 134,
    HP_KEY_INTERNATIONAL1 = 135,
    HP_KEY_INTERNATIONAL2 = 136,
    HP_KEY_INTERNATIONAL3 = 137,
    HP_KEY_INTERNATIONAL4 = 138,
    HP_KEY_INTERNATIONAL5 = 139,
    HP_KEY_INTERNATIONAL6 = 140,
    HP_KEY_INTERNATIONAL7 = 141,
    HP_KEY_INTERNATIONAL8 = 142,
    HP_KEY_INTERNATIONAL9 = 143,
    HP_KEY_LANG1 = 144,
    HP_KEY_LANG2 = 145,
    HP_KEY_LANG3 = 146,
    HP_KEY_LANG4 = 147,
    HP_KEY_LANG5 = 148,
    HP_KEY_LANG6 = 149,
    HP_KEY_LANG7 = 150,
    HP_KEY_LANG8 = 151,
    HP_KEY_LANG9 = 152,
    HP_KEY_ALTERASE = 153,
    HP_KEY_SYSREQ = 154,
    HP_KEY_CANCEL = 155,
    HP_KEY_CLEAR = 156,
    HP_KEY_PRIOR = 157,
    HP_KEY_RETURN2 = 158,
    HP_KEY_SEPARATOR = 159,
    HP_KEY_OUT = 160,
    HP_KEY_OPER = 161,
    HP_KEY_CLEARAGAIN = 162,
    HP_KEY_CRSEL = 163,
    HP_KEY_EXSEL = 164,
    HP_KEY_KP_00 = 176,
    HP_KEY_KP_000 = 177,
    HP_KEY_THOUSANDSSEPARATOR = 178,
    HP_KEY_DECIMALSEPARATOR = 179,
    HP_KEY_CURRENCYUNIT = 180,
    HP_KEY_CURRENCYSUBUNIT = 181,
    HP_KEY_KP_LEFTPAREN = 182,
    HP_KEY_KP_RIGHTPAREN = 183,
    HP_KEY_KP_LEFTBRACE = 184,
    HP_KEY_KP_RIGHTBRACE = 185,
    HP_KEY_KP_TAB = 186,
    HP_KEY_KP_BACKSPACE = 187,
    HP_KEY_KP_A = 188,
    HP_KEY_KP_B = 189,
    HP_KEY_KP_C = 190,
    HP_KEY_KP_D = 191,
    HP_KEY_KP_E = 192,
    HP_KEY_KP_F = 193,
    HP_KEY_KP_XOR = 194,
    HP_KEY_KP_POWER = 195,
    HP_KEY_KP_PERCENT = 196,
    HP_KEY_KP_LESS = 197,
    HP_KEY_KP_GREATER = 198,
    HP_KEY_KP_AMPERSAND = 199,
    HP_KEY_KP_DBLAMPERSAND = 200,
    HP_KEY_KP_VERTICALBAR = 201,
    HP_KEY_KP_DBLVERTICALBAR = 202,
    HP_KEY_KP_COLON = 203,
    HP_KEY_KP_HASH = 204,
    HP_KEY_KP_SPACE = 205,
    HP_KEY_KP_AT = 206,
    HP_KEY_KP_EXCLAM = 207,
    HP_KEY_KP_MEMSTORE = 208,
    HP_KEY_KP_MEMRECALL = 209,
    HP_KEY_KP_MEMCLEAR = 210,
    HP_KEY_KP_MEMADD = 211,
    HP_KEY_KP_MEMSUBTRACT = 212,
    HP_KEY_KP_MEMMULTIPLY = 213,
    HP_KEY_KP_MEMDIVIDE = 214,
    HP_KEY_KP_PLUSMINUS = 215,
    HP_KEY_KP_CLEAR = 216,
    HP_KEY_KP_CLEARENTRY = 217,
    HP_KEY_KP_BINARY = 218,
    HP_KEY_KP_OCTAL = 219,
    HP_KEY_KP_DECIMAL = 220,
    HP_KEY_KP_HEXADECIMAL = 221,
    HP_KEY_LCTRL = 224,
    HP_KEY_LSHIFT = 225,
    HP_KEY_LALT = 226,
    HP_KEY_LGUI = 227,
    HP_KEY_RCTRL = 228,
    HP_KEY_RSHIFT = 229,
    HP_KEY_RALT = 230,
    HP_KEY_RGUI = 231,
    HP_KEY_MODE = 257,
    HP_KEY_SLEEP = 258,
    HP_KEY_WAKE = 259,
    HP_KEY_CHANNEL_INCREMENT = 260,
    HP_KEY_CHANNEL_DECREMENT = 261,
    HP_KEY_MEDIA_PLAY = 262,
    HP_KEY_MEDIA_PAUSE = 263,
    HP_KEY_MEDIA_RECORD = 264,
    HP_KEY_MEDIA_FAST_FORWARD = 265,
    HP_KEY_MEDIA_REWIND = 266,
    HP_KEY_MEDIA_NEXT_TRACK = 267,
    HP_KEY_MEDIA_PREVIOUS_TRACK = 268,
    HP_KEY_MEDIA_STOP = 269,
    HP_KEY_MEDIA_EJECT = 270,
    HP_KEY_MEDIA_PLAY_PAUSE = 271,
    HP_KEY_MEDIA_SELECT = 272,
    HP_KEY_AC_NEW = 273,
    HP_KEY_AC_OPEN = 274,
    HP_KEY_AC_CLOSE = 275,
    HP_KEY_AC_EXIT = 276,
    HP_KEY_AC_SAVE = 277,
    HP_KEY_AC_PRINT = 278,
    HP_KEY_AC_PROPERTIES = 279,
    HP_KEY_AC_SEARCH = 280,
    HP_KEY_AC_HOME = 281,
    HP_KEY_AC_BACK = 282,
    HP_KEY_AC_FORWARD = 283,
    HP_KEY_AC_STOP = 284,
    HP_KEY_AC_REFRESH = 285,
    HP_KEY_AC_BOOKMARKS = 286,
    HP_KEY_SOFTLEFT = 287,
    HP_KEY_SOFTRIGHT = 288,
    HP_KEY_CALL = 289,
    HP_KEY_ENDCALL = 290,
    HP_KEY_RESERVED = 400,
    HP_KEY_COUNT = 512
} HP_Key;

/**
 * @brief Mouse buttons.
 *
 * Each constant represents a mouse button that can be queried with
 * `HP_IsMouseButtonPressed` and similar functions.
 */
typedef uint8_t HP_MouseButton;

#define HP_MOUSE_BUTTON_LEFT    0x01  /**< Left mouse button */
#define HP_MOUSE_BUTTON_MIDDLE  0x02  /**< Middle mouse button (wheel click) */
#define HP_MOUSE_BUTTON_RIGHT   0x04  /**< Right mouse button */
#define HP_MOUSE_BUTTON_X1      0x08  /**< Extra button 1 (usually back) */
#define HP_MOUSE_BUTTON_X2      0x10  /**< Extra button 2 (usually forward) */

/**
 * @brief Logging levels.
 *
 * Used to control verbosity and priority of log messages.
 * Functions like `HP_Log` and `HP_SetLogPriority` use these levels.
 */
typedef enum HP_LogLevel {
    HP_LOG_INVALID, /**< Invalid log level */
    HP_LOG_TRACE,   /**< Trace-level messages for detailed debugging */
    HP_LOG_VERBOSE, /**< Verbose messages */
    HP_LOG_DEBUG,   /**< Debug messages */
    HP_LOG_INFO,    /**< Informational messages */
    HP_LOG_WARN,    /**< Warning messages */
    HP_LOG_ERROR,   /**< Error messages */
    HP_LOG_FATAL,   /**< Fatal error messages */
    HP_LOG_COUNT    /**< Number of log levels */
} HP_LogLevel;

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
 * @note Can be used directly in the main loop: `while (HP_FrameStep())`.
 */
HPAPI bool HP_FrameStep(void);

/** @} */ // end of Update

/**
 * @defgroup Time Time Management
 * @brief Functions for time measurement and frame control.
 * @{
 */

/**
 * @brief Gets the current system time in nanoseconds since Jan 1, 1970 (UTC).
 */
int64_t HP_GetCurrentTimeNS(void);

/**
 * @brief Gets the current system time in seconds since Jan 1, 1970 (UTC).
 */
double HP_GetCurrentTime(void);

/**
 * @brief Gets the elapsed time since library initialization.
 * @return Elapsed time in seconds.
 */
HPAPI double HP_GetElapsedTime(void);

/**
 * @brief Gets the time taken by the last frame.
 * @return Frame time in seconds.
 */
HPAPI double HP_GetFrameTime(void);

/**
 * @brief Sets the target frame rate (FPS).
 * @param fps Desired target FPS.
 */
HPAPI void HP_SetTargetFPS(int fps);

/**
 * @brief Gets the current frame rate (FPS).
 */
HPAPI int HP_GetFPS(void);

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
HPAPI bool HP_SetVSync(int mode);

/**
 * @brief Gets the display scaling factor.
 * @return Display scale (e.g., 1.0 for standard DPI, >1.0 for HiDPI/Retina).
 */
HPAPI float HP_GetDisplayScale(void);

/**
 * @brief Gets the display DPI (dots per inch).
 */
HPAPI float HP_GetDisplayGetDPI(void);

/**
 * @brief Gets the current display index.
 * @return Display index, useful in multi-monitor setups.
 */
HPAPI int HP_GetDisplayIndex(void);

/**
 * @brief Gets the display size in pixels.
 * @return Integer 2D vector (width, height).
 */
HPAPI HP_IVec2 HP_GetDisplaySize(void);

/**
 * @brief Gets the display size in pixels as floats.
 * @return Float 2D vector (width, height).
 */
HPAPI HP_Vec2 HP_GetDisplaySizeF(void);

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
HPAPI const char* HP_GetWindowTitle(void);

/**
 * @brief Sets the window title.
 * @param title The new window title as a null-terminated string.
 */
HPAPI void HP_SetWindowTitle(const char* title);

/**
 * @brief Gets the window width in pixels.
 */
HPAPI int HP_GetWindowWidth(void);

/**
 * @brief Gets the window height in pixels.
 */
HPAPI int HP_GetWindowHeight(void);

/**
 * @brief Gets the window size in pixels.
 * @return Integer 2D vector (width, height).
 */
HPAPI HP_IVec2 HP_GetWindowSize(void);

/**
 * @brief Gets the window size in pixels as floats.
 * @return Float 2D vector (width, height).
 */
HPAPI HP_Vec2 HP_GetWindowSizeF(void);

/**
 * @brief Sets the window size in pixels.
 * @param w Width in pixels.
 * @param h Height in pixels.
 */
HPAPI void HP_SetWindowSize(int w, int h);

/**
 * @brief Sets the minimum window size in pixels.
 * @param w Minimum width in pixels.
 * @param h Minimum height in pixels.
 */
HPAPI void HP_SetWindowMinSize(int w, int h);

/**
 * @brief Sets the maximum window size in pixels.
 * @param w Maximum width in pixels.
 * @param h Maximum height in pixels.
 */
HPAPI void HP_SetWindowMaxSize(int w, int h);

/**
 * @brief Gets the window position on the screen.
 * @return Integer 2D vector (x, y).
 */
HPAPI HP_IVec2 HP_GetWindowPosition(void);

/**
 * @brief Sets the window position on the screen.
 * @param x X coordinate of the top-left corner.
 * @param y Y coordinate of the top-left corner.
 */
HPAPI void HP_SetWindowPosition(int x, int y);

/**
 * @brief Checks if the window is currently fullscreen.
 * @return true if fullscreen, false otherwise.
 */
HPAPI bool HP_IsWindowFullscreen(void);

/**
 * @brief Enables or disables fullscreen mode.
 * @param enabled true to enable fullscreen, false to restore windowed mode.
 */
HPAPI void HP_SetWindowFullscreen(bool enabled);

/**
 * @brief Checks if the window is resizable by the user.
 * @return true if resizable, false otherwise.
 */
HPAPI bool HP_IsWindowResizable(void);

/**
 * @brief Enables or disables window resizing by the user.
 * @param resizable true to allow resizing, false to fix window size.
 */
HPAPI void HP_SetWindowResizable(bool resizable);

/**
 * @brief Checks if the window is currently visible.
 * @return true if visible, false otherwise.
 */
HPAPI bool HP_IsWindowVisible(void);

/**
 * @brief Minimizes the window to the taskbar/dock.
 */
HPAPI void HP_MinimizeWindow(void);

/**
 * @brief Maximizes the window to occupy the available screen space.
 */
HPAPI void HP_MaximizeWindow(void);

/**
 * @brief Restores the window from minimized or maximized state.
 */
HPAPI void HP_RestoreWindow(void);

/**
 * @brief Shows the window if it is hidden.
 */
HPAPI void HP_ShowWindow(void);

/**
 * @brief Hides the window from the screen without closing it.
 */
HPAPI void HP_HideWindow(void);

/**
 * @brief Checks if the window currently has input focus.
 * @return true if focused, false otherwise.
 */
HPAPI bool HP_IsWindowFocused(void);

/**
 * @brief Brings the window to the foreground and gives it focus.
 */
HPAPI void HP_FocusWindow(void);

/**
 * @brief Checks if the window has borders.
 * @return true if bordered, false otherwise.
 */
HPAPI bool HP_IsWindowBordered(void);

/**
 * @brief Enables or disables window borders.
 * @param bordered true to show borders, false to remove them.
 */
HPAPI void HP_SetWindowBordered(bool bordered);

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
HPAPI bool HP_IsCursorGrabbed(void);

/**
 * @brief Grabs or releases the cursor.
 *
 * Grabbing the cursor confines it to the window and is often used in
 * first-person camera controls or games requiring locked mouse input.
 *
 * @param grab true to grab the cursor, false to release it.
 */
HPAPI void HP_GrabCursor(bool grab);

/**
 * @brief Shows the cursor.
 */
HPAPI void HP_ShowCursor(void);

/**
 * @brief Hides the cursor.
 */
HPAPI void HP_HideCursor(void);

/**
 * @brief Checks if the cursor is currently visible.
 * @return true if visible, false otherwise.
 */
HPAPI bool HP_IsCursorVisible(void);

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
HPAPI void HP_CaptureMouse(bool enabled);

/**
 * @brief Checks if a mouse button is currently pressed.
 *
 * @param button Mouse button to query.
 * @return true if the button is pressed, false otherwise.
 */
HPAPI bool HP_IsMouseButtonPressed(HP_MouseButton button);

/**
 * @brief Checks if a mouse button is currently released.
 *
 * @param button Mouse button to query.
 * @return true if the button is released, false otherwise.
 */
HPAPI bool HP_IsMouseButtonReleased(HP_MouseButton button);

/**
 * @brief Checks if a mouse button was just pressed this frame.
 *
 * @param button Mouse button to query.
 * @return true if the button was pressed this frame, false otherwise.
 */
HPAPI bool HP_IsMouseButtonJustPressed(HP_MouseButton button);

/**
 * @brief Checks if a mouse button was just released this frame.
 *
 * @param button Mouse button to query.
 * @return true if the button was released this frame, false otherwise.
 */
HPAPI bool HP_IsMouseButtonJustReleased(HP_MouseButton button);

/**
 * @brief Gets the current mouse position in window coordinates.
 *
 * @return Mouse position as a 2D vector (x, y).
 */
HPAPI HP_Vec2 HP_GetMousePosition(void);

/**
 * @brief Sets the mouse position in window coordinates.
 *
 * @param p Target position as a 2D vector (x, y).
 */
HPAPI void HP_SetMousePosition(HP_Vec2 p);

/**
 * @brief Gets the mouse movement delta since the last frame.
 *
 * @return Movement delta as a 2D vector (x, y).
 */
HPAPI HP_Vec2 HP_GetMouseDelta(void);

/**
 * @brief Gets the mouse wheel movement for the current frame.
 *
 * Typically, the y component represents vertical scrolling and the x
 * component represents horizontal scrolling (if supported).
 *
 * @return Mouse wheel movement as a 2D vector.
 */
HPAPI HP_Vec2 HP_GetMouseWheel(void);

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
HPAPI bool HP_IsKeyPressed(HP_Key key);

/**
 * @brief Checks if a key is currently released.
 *
 * @param key Key to query.
 * @return true if the key is released, false otherwise.
 */
HPAPI bool HP_IsKeyReleased(HP_Key key);

/**
 * @brief Checks if a key was just pressed during the current frame.
 *
 * @param key Key to query.
 * @return true if the key was pressed this frame, false otherwise.
 */
HPAPI bool HP_IsKeyJustPressed(HP_Key key);

/**
 * @brief Checks if a key was just released during the current frame.
 *
 * @param key Key to query.
 * @return true if the key was released this frame, false otherwise.
 */
HPAPI bool HP_IsKeyJustReleased(HP_Key key);

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
HPAPI HP_Vec2 HP_GetKeyVec2(HP_Key up, HP_Key down, HP_Key left, HP_Key right);

HPAPI HP_Vec3 HP_GetKeyVec3(HP_Key forward, HP_Key backward, HP_Key left, HP_Key right);

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
HPAPI bool HP_AddSearchPath(const char* path, bool appendToEnd);

/**
 * @brief Remove directory or archive from search path
 * @param path Path that was previously added
 * @return True on success, false on failure
 */
HPAPI bool HP_RemoveSearchPath(const char* path);

/**
 * @brief Get list of all search paths in order
 * @return Null-terminated array of strings, must be freed with HP_FreeSearchPaths()
 */
HPAPI char** HP_GetSearchPaths(void);

/**
 * @brief Free search paths list returned by HP_GetSearchPaths()
 * @param paths Array to free
 */
HPAPI void HP_FreeSearchPaths(char** paths);

/**
 * @brief Mount archive to virtual file system
 * @param archivePath Physical path to archive (ZIP, 7Z, etc.)
 * @param mountPoint Virtual mount point (NULL for root)
 * @param appendToEnd If true, add to end of search order
 * @return True on success, false on failure
 */
HPAPI bool HP_MountArchive(const char* archivePath, const char* mountPoint, bool appendToEnd);

/**
 * @brief Unmount archive from virtual file system
 * @param archivePath Archive path that was previously mounted
 * @return True on success, false on failure
 */
HPAPI bool HP_UnmountArchive(const char* archivePath);

/**
 * @brief Get current write directory
 * @return Write directory path, or NULL if not set
 */
HPAPI const char* HP_GetWriteDir(void);

/**
 * @brief Set write directory for file operations
 * @param path Physical directory path for writing files
 * @return True on success, false on failure
 */
HPAPI bool HP_SetWriteDir(const char* path);

/**
 * @brief Get executable's base directory
 * @return Base directory path (read-only)
 */
HPAPI const char* HP_GetBaseDir(void);

/**
 * @brief Get user preferences directory
 * @param org Organization name
 * @param app Application name
 * @return Preferences directory path (platform-specific)
 */
HPAPI const char* HP_GetPrefDir(const char* org, const char* app);

/* --- File Information --- */

/**
 * @brief Check if file or directory exists in virtual file system
 * @param filePath Virtual file path
 * @return True if exists, false otherwise
 */
HPAPI bool HP_FileExists(const char* filePath);

/**
 * @brief Check if path is a directory
 * @param path Virtual path to check
 * @return True if directory, false otherwise
 */
HPAPI bool HP_IsDirectory(const char* path);

/**
 * @brief Check if path is a regular file
 * @param path Virtual path to check
 * @return True if file, false otherwise
 */
HPAPI bool HP_IsFile(const char* path);

/**
 * @brief Get file size without opening
 * @param filePath Virtual file path
 * @return File size in bytes, or 0 on error
 */
HPAPI size_t HP_GetFileSize(const char* filePath);

/**
 * @brief Get real physical path of virtual file
 * @param filePath Virtual file path
 * @return Physical directory containing the file, or NULL if not found
 */
HPAPI const char* HP_GetRealPath(const char* filePath);

/* --- Directory Operations --- */

/**
 * @brief List directory contents
 * @param dirPath Virtual directory path
 * @return Null-terminated array of filenames, must be freed with HP_FreeDirectoryList()
 */
HPAPI char** HP_ListDirectory(const char* dirPath);

/**
 * @brief Free directory list returned by HP_ListDirectory()
 * @param list Array to free
 */
HPAPI void HP_FreeDirectoryList(char** list);

/**
 * @brief Create directory in write directory
 * @param dirPath Virtual directory path
 * @return True on success, false on failure
 */
HPAPI bool HP_CreateDirectory(const char* dirPath);

/**
 * @brief Delete file from write directory
 * @param filePath Virtual file path
 * @return True on success, false on failure
 */
HPAPI bool HP_DeleteFile(const char* filePath);

/* --- File I/O --- */

/**
 * @brief Load binary file into memory
 * @param filePath Virtual file path
 * @param size Pointer to store file size
 * @return File data buffer (must be freed), or NULL on failure
 */
HPAPI void* HP_LoadFile(const char* filePath, size_t* size);

/**
 * @brief Load text file into memory (null-terminated)
 * @param filePath Virtual file path
 * @return Text buffer (must be freed), or NULL on failure
 */
HPAPI char* HP_LoadFileText(const char* filePath);

/**
 * @brief Write binary data to file in write directory
 * @param filePath Virtual file path
 * @param data Data to write
 * @param size Data size in bytes
 * @return True on success, false on failure
 */
HPAPI bool HP_WriteFile(const char* filePath, const void* data, size_t size);

/**
 * @brief Write text data to file in write directory
 * @param filePath Virtual file path
 * @param data Text data to write
 * @param size Data size in bytes
 * @return True on success, false on failure
 */
HPAPI bool HP_WriteFileText(const char* filePath, const char* data, size_t size);

/** @} */ // end of Keyboard

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
HPAPI bool HP_SetClipboardText(const char* text);

/**
 * @brief Get the current content of the system clipboard.
 * @return Null-terminated string from the clipboard, or NULL if empty.
 * @note The returned string may be invalidated on the next clipboard operation.
 */
HPAPI const char* HP_GetClipboardText(void);

/**
 * @brief Check if the clipboard contains any text.
 * @return True if there is text available, false otherwise.
 */
HPAPI bool HP_HasClipboardText(void);

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
HPAPI void HP_SetLogPriority(HP_LogLevel log);

/**
 * @brief Log a message with the specified log level.
 * @param log Log level for this message.
 * @param msg Null-terminated format string.
 */
HPAPI void HP_Log(HP_LogLevel log, const char* msg, ...);

/**
 * @brief Log a message using a va_list.
 * @param log Log level for this message.
 * @param msg Null-terminated format string.
 * @param args Variable argument list.
 */
HPAPI void HP_LogVA(HP_LogLevel log, const char* msg, va_list args);

/** @brief Log a trace message */
HPAPI void HP_LogT(const char* msg, ...);

/** @brief Log a verbose message */
HPAPI void HP_LogV(const char* msg, ...);

/** @brief Log a debug message */
HPAPI void HP_LogD(const char* msg, ...);

/** @brief Log an info message */
HPAPI void HP_LogI(const char* msg, ...);

/** @brief Log a warning message */
HPAPI void HP_LogW(const char* msg, ...);

/** @brief Log an error message */
HPAPI void HP_LogE(const char* msg, ...);

/** @brief Log a fatal error message */
HPAPI void HP_LogF(const char* msg, ...);

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
HPAPI void* HP_Malloc(size_t size);

/**
 * @brief Allocates and zero-initializes an array.
 * @param nmemb Number of elements.
 * @param size Size of each element in bytes.
 * @return Pointer to the allocated memory, or NULL if allocation fails.
 */
HPAPI void* HP_Calloc(size_t nmemb, size_t size);

/**
 * @brief Resizes a previously allocated memory block.
 * @param ptr Pointer to the memory block to resize.
 * @param size New size in bytes.
 * @return Pointer to the reallocated memory, or NULL if allocation fails.
 */
HPAPI void* HP_Realloc(void* ptr, size_t size);

/**
 * @brief Frees a previously allocated memory block.
 * @param ptr Pointer to the memory block to free.
 */
HPAPI void HP_Free(void* ptr);

/** @} */ // end of Memory

/** @} */ // end of Core

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // HP_CORE_H
