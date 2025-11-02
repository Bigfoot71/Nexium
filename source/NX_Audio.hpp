#ifndef NX_AUDIO_HPP
#define NX_AUDIO_HPP

#include <NX/NX_Audio.h>
#include <NX/NX_Init.h>

// ============================================================================
// INTERNAL FUNCTIONS
// ============================================================================

/** Should be called in NX_Init() */
bool INX_AudioState_Init(NX_AppDesc* desc);

/** Should be call in NX_Quit() */
void INX_AudioState_Quit();

#endif // NX_AUDIO_HPP
