/* NX_Clipboard.cpp -- API definition for Nexium's clipboard module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/NX_Clipboard.h>
#include <SDL3/SDL_clipboard.h>

// ============================================================================
// PUBLIC API
// ============================================================================

bool NX_SetClipboardText(const char* text)
{
    return SDL_SetClipboardText(text);
}

const char* NX_GetClipboardText()
{
    return SDL_GetClipboardText();
}

bool NX_HasClipboardText()
{
    return SDL_HasClipboardText();
}
