#ifndef NX_GAMEPAD_H
#define NX_GAMEPAD_H

#include "./NX_Math.h"
#include "./NX_API.h"

// ============================================================================
// TYPES DEFINITIONS
// ============================================================================

/**
 * @brief Represents the type of a connected gamepad.
 *
 * This value describes the general layout or style of the controller,
 * not its actual brand or manufacturer. For example, a third-party
 * controller might report as an Xbox or PlayStation type.
 */
typedef enum NX_GamepadType {
    NX_GAMEPAD_TYPE_UNKNOWN = 0,                    ///< Unknown or unsupported controller.
    NX_GAMEPAD_TYPE_STANDARD,                       ///< Generic standard gamepad layout.
    NX_GAMEPAD_TYPE_XBOX360,                        ///< Xbox 360 style controller.
    NX_GAMEPAD_TYPE_XBOXONE,                        ///< Xbox One / Series style controller.
    NX_GAMEPAD_TYPE_PS3,                            ///< PlayStation 3 controller.
    NX_GAMEPAD_TYPE_PS4,                            ///< PlayStation 4 controller.
    NX_GAMEPAD_TYPE_PS5,                            ///< PlayStation 5 controller.
    NX_GAMEPAD_TYPE_NINTENDO_SWITCH_PRO,            ///< Nintendo Switch Pro controller.
    NX_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_LEFT,    ///< Left Joy-Con.
    NX_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT,   ///< Right Joy-Con.
    NX_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_PAIR,    ///< Paired Joy-Cons.
    NX_GAMEPAD_TYPE_GAMECUBE                        ///< Nintendo GameCube controller.
} NX_GamepadType;

/**
 * @brief Represents all available buttons on a standard gamepad.
 *
 * The face buttons (South/East/West/North) refer to their physical position on the pad,
 * not their label, for example, the "South" button corresponds to the lower face button
 * (A on Xbox, Cross on PlayStation, B on Nintendo).
 */
typedef enum NX_GamepadButton {
    NX_GAMEPAD_BUTTON_INVALID = -1,            ///< Invalid button.
    NX_GAMEPAD_BUTTON_SOUTH,                   ///< Bottom face button.
    NX_GAMEPAD_BUTTON_EAST,                    ///< Right face button.
    NX_GAMEPAD_BUTTON_WEST,                    ///< Left face button.
    NX_GAMEPAD_BUTTON_NORTH,                   ///< Top face button.
    NX_GAMEPAD_BUTTON_BACK,                    ///< Back or Select button.
    NX_GAMEPAD_BUTTON_GUIDE,                   ///< Guide or Home button.
    NX_GAMEPAD_BUTTON_START,                   ///< Start or Options button.
    NX_GAMEPAD_BUTTON_LEFT_STICK,              ///< Left stick press.
    NX_GAMEPAD_BUTTON_RIGHT_STICK,             ///< Right stick press.
    NX_GAMEPAD_BUTTON_LEFT_SHOULDER,           ///< Left bumper.
    NX_GAMEPAD_BUTTON_RIGHT_SHOULDER,          ///< Right bumper.
    NX_GAMEPAD_BUTTON_DPAD_UP,                 ///< D-pad up.
    NX_GAMEPAD_BUTTON_DPAD_DOWN,               ///< D-pad down.
    NX_GAMEPAD_BUTTON_DPAD_LEFT,               ///< D-pad left.
    NX_GAMEPAD_BUTTON_DPAD_RIGHT,              ///< D-pad right.
    NX_GAMEPAD_BUTTON_MISC1,                   ///< Misc button (e.g. Share, Capture, etc.).
    NX_GAMEPAD_BUTTON_RIGHT_PADDLE1,           ///< Upper right paddle or SR button.
    NX_GAMEPAD_BUTTON_LEFT_PADDLE1,            ///< Upper left paddle or SL button.
    NX_GAMEPAD_BUTTON_RIGHT_PADDLE2,           ///< Lower right paddle or function button.
    NX_GAMEPAD_BUTTON_LEFT_PADDLE2,            ///< Lower left paddle or function button.
    NX_GAMEPAD_BUTTON_TOUCHPAD,                ///< Touchpad click (PS4/PS5).
    NX_GAMEPAD_BUTTON_MISC2,                   ///< Additional custom button.
    NX_GAMEPAD_BUTTON_MISC3,                   ///< Additional custom button.
    NX_GAMEPAD_BUTTON_MISC4,                   ///< Additional custom button.
    NX_GAMEPAD_BUTTON_MISC5,                   ///< Additional custom button.
    NX_GAMEPAD_BUTTON_MISC6,                   ///< Additional custom button.
} NX_GamepadButton;

/**
 * @brief Represents analog axes available on a gamepad.
 *
 * Axis values are normalized to the range [-1.0, 1.0].
 * Triggers are reported in the range [0.0, 1.0].
 */
typedef enum NX_GamepadAxis {
    NX_GAMEPAD_AXIS_INVALID = -1,              ///< Invalid axis.
    NX_GAMEPAD_AXIS_LEFTX,                     ///< Left stick X axis.
    NX_GAMEPAD_AXIS_LEFTY,                     ///< Left stick Y axis.
    NX_GAMEPAD_AXIS_RIGHTX,                    ///< Right stick X axis.
    NX_GAMEPAD_AXIS_RIGHTY,                    ///< Right stick Y axis.
    NX_GAMEPAD_AXIS_LEFT_TRIGGER,              ///< Left trigger.
    NX_GAMEPAD_AXIS_RIGHT_TRIGGER,             ///< Right trigger.
} NX_GamepadAxis;

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Checks if the specified gamepad is currently connected and available.
 *
 * @param gamepad Index of the gamepad to query.
 * @return true if the gamepad is available, false otherwise.
 */
NXAPI bool NX_IsGamepadAvailable(int gamepad);

/**
 * @brief Retrieves the type of the specified gamepad.
 *
 * @param gamepad Index of the gamepad to query.
 * @return The gamepad type (e.g., Xbox, PlayStation, etc).
 */
NXAPI NX_GamepadType NX_GetGamepadType(int gamepad);

/**
 * @brief Gets the display name of the specified gamepad.
 *
 * @param gamepad Index of the gamepad to query.
 * @return A pointer to the gamepad name string, or NULL if unavailable.
 */
NXAPI const char* NX_GetGamepadName(int gamepad);

/**
 * @brief Checks if a gamepad button is currently pressed.
 *
 * @param gamepad Index of the gamepad to query.
 * @param button Button to query.
 * @return true if the button is pressed, false otherwise.
 */
NXAPI bool NX_IsGamepadButtonPressed(int gamepad, NX_GamepadButton button);

/**
 * @brief Checks if a gamepad button is currently released.
 *
 * @param gamepad Index of the gamepad to query.
 * @param button Button to query.
 * @return true if the button is released, false otherwise.
 */
NXAPI bool NX_IsGamepadButtonReleased(int gamepad, NX_GamepadButton button);

/**
 * @brief Checks if a gamepad button was just pressed during the current frame.
 *
 * @param gamepad Index of the gamepad to query.
 * @param button Button to query.
 * @return true if the button was pressed this frame, false otherwise.
 */
NXAPI bool NX_IsGamepadButtonJustPressed(int gamepad, NX_GamepadButton button);

/**
 * @brief Checks if a gamepad button was just released during the current frame.
 *
 * @param gamepad Index of the gamepad to query.
 * @param button Button to query.
 * @return true if the button was released this frame, false otherwise.
 */
NXAPI bool NX_IsGamepadButtonJustReleased(int gamepad, NX_GamepadButton button);

/**
 * @brief Checks if the specified gamepad provides the given axis.
 *
 * @param gamepad Index of the gamepad to query.
 * @param axis Axis to check.
 * @return true if the gamepad supports the axis, false otherwise.
 */
NXAPI bool NX_HasGamepadAxis(int gamepad, NX_GamepadAxis axis);

/**
 * @brief Gets the normalized value of a specific gamepad axis.
 *
 * @param gamepad Index of the gamepad to query.
 * @param axis Axis to read.
 * @return Axis value in the range [-1.0, 1.0] for sticks and [0.0, 1.0] for triggers.
 */
NXAPI float NX_GetGamepadAxis(int gamepad, NX_GamepadAxis axis);

/**
 * @brief Returns the normalized 2D vector of the left analog stick.
 *
 * @param gamepad Index of the gamepad to query.
 * @return A 2D vector representing the left stick position.
 */
NXAPI NX_Vec2 NX_GetGamepadLeftStick(int gamepad);

/**
 * @brief Returns the normalized 2D vector of the right analog stick.
 *
 * @param gamepad Index of the gamepad to query.
 * @return A 2D vector representing the right stick position.
 */
NXAPI NX_Vec2 NX_GetGamepadRightStick(int gamepad);

/**
 * @brief Triggers vibration (rumble) on the specified gamepad.
 *
 * @param gamepad Index of the gamepad to control.
 * @param left Strength of the left motor (0.0-1.0).
 * @param right Strength of the right motor (0.0-1.0).
 * @param duration Duration of the vibration in seconds.
 */
NXAPI void NX_RumbleGamepad(int gamepad, float left, float right, float duration);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_GAMEPAD_H
