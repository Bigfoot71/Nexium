/* NX_Clipboard.h -- API declaration for Nexium's clipboard module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_CLIPBOARD_H
#define NX_CLIPBOARD_H

#include "./NX_API.h"
#include <stdbool.h>

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

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

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_CLIPBOARD_H
