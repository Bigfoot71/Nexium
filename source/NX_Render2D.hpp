#ifndef NX_RENDER_2D_H
#define NX_RENDER_2D_H

#include <NX/NX_Init.h>

// ============================================================================
// INTERNAL FUNCTIONS
// ============================================================================

/** Should be called in NX_Init() */
bool INX_Render2DState_Init(NX_AppDesc* desc);

/** Should be call in NX_Quit() */
void INX_Render2DState_Quit();

#endif // NX_RENDER_2D_H
