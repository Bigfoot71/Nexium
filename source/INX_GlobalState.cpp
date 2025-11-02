/* INX_GlobalState.cpp -- Internal implementation details for managing global states
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "./INX_GlobalState.hpp"

// ============================================================================
// GLOBAL STATES
// ============================================================================

INX_DisplayState  INX_Display{};
INX_KeyboardState INX_Keyboard{};
INX_MouseState    INX_Mouse{};
INX_FrameState    INX_Frame{};
