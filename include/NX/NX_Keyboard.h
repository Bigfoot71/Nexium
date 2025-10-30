/* NX_Keyboard.h -- API declaration for Nexium's keyboard module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_KEYBOARD_H
#define NX_KEYBOARD_H

#include "./NX_Math.h"
#include "./NX_API.h"

// ============================================================================
// TYPES DEFINITIONS
// ============================================================================

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

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

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

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_KEYBOARD_H
