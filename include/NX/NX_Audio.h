/* NX_Audio.h -- API declaration for Nexium's render module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_AUDIO_H
#define NX_AUDIO_H

#include "./NX_Platform.h"
#include <stdbool.h>

/* === Types === */

typedef struct NX_AudioClip NX_AudioClip;
typedef struct NX_AudioStream NX_AudioStream;

/* === API Functions === */

#if defined(__cplusplus)
extern "C" {
#endif

/** @defgroup Audio Audio Functions
 *  Global audio parameters and volume control.
 *  @{
 */

/**
 * @brief Get the master volume
 * @return Master volume (0.0 = mute, 1.0 = max)
 */
HPAPI float NX_GetMasterVolume(void);

/**
 * @brief Get the audio clips volume
 * @return Clip volume (0.0 = mute, 1.0 = max)
 */
HPAPI float NX_GetAudioClipVolume(void);

/**
 * @brief Get the audio streams volume
 * @return Stream volume (0.0 = mute, 1.0 = max)
 */
HPAPI float NX_GetAudioStreamVolume(void);

/**
 * @brief Set the master volume
 * @param volume Volume value (0.0 = mute, 1.0 = max)
 */
HPAPI void NX_SetMasterVolume(float volume);

/**
 * @brief Set the audio clips volume
 * @param volume Volume value (0.0 = mute, 1.0 = max)
 */
HPAPI void NX_SetAudioClipVolume(float volume);

/**
 * @brief Set the audio streams volume
 * @param volume Volume value (0.0 = mute, 1.0 = max)
 */
HPAPI void NX_SetAudioStreamVolume(float volume);

/** @} */ // Audio

/* === Clip Functions === */

/** @defgroup Clip Clip Functions
 *  Clip effects with channel-based polyphony.
 *  @{
 */

/**
 * @brief Load a clip from a file
 * @param filePath Path to the clip file (supports WAV, FLAC, MP3, OGG)
 * @param channelCount Number of channels for polyphony (must be > 0)
 * @return Clip pointer on success, NULL on failure
 */
HPAPI NX_AudioClip* NX_LoadAudioClip(const char* filePath, int channelCount);

/**
 * @brief Destroy a loaded clip and free all associated resources
 * @param clip Clip pointer to destroy
 */
HPAPI void NX_DestroyAudioClip(NX_AudioClip* clip);

/**
 * @brief Play a clip on a specific channel or automatically find a free one
 * @param clip Clip pointer to play
 * @param channel Channel index (>= 0) or negative value to auto-select first available channel
 * @return Channel index where the clip is played, or -1 if no channel is available
 */
HPAPI int NX_PlayAudioClip(NX_AudioClip* clip, int channel);

/**
 * @brief Pause a clip on a specific channel or all channels
 * @param clip Clip pointer to pause
 * @param channel Channel index (>= 0) to pause specific channel, or negative value to pause all channels
 */
HPAPI void NX_PauseAudioClip(NX_AudioClip* clip, int channel);

/**
 * @brief Stop a clip on a specific channel or all channels
 * @param clip Clip pointer to stop
 * @param channel Channel index (>= 0) to stop specific channel, or negative value to stop all channels
 */
HPAPI void NX_StopAudioClip(NX_AudioClip* clip, int channel);

/**
 * @brief Rewind a clip on a specific channel or all channels
 * @param clip Clip pointer to rewind
 * @param channel Channel index (>= 0) to rewind specific channel, or negative value to rewind all channels
 */
HPAPI void NX_RewindAudioClip(NX_AudioClip* clip, int channel);

/**
 * @brief Check if a clip is playing on a specific channel or any channel
 * @param clip Clip pointer to check
 * @param channel Channel index (>= 0) to check specific channel, or negative value to check if playing on any channel
 * @return true if clip is playing on the specified channel(s), false otherwise
 */
HPAPI bool NX_IsAudioClipPlaying(NX_AudioClip* clip, int channel);

/**
 * @brief Get the number of channels assigned to a clip
 * @param clip Clip pointer
 * @return Number of channels, or 0 if clip pointer is invalid
 */
HPAPI int NX_GetAudioClipChannelCount(NX_AudioClip* clip);

/** @} */ // Clip

/* === Stream Functions === */

/** @defgroup Stream Stream Functions
 *  Background stream playback and control.
 *  @{
 */

/**
 * @brief Load stream from a file
 * @param filePath Path to the stream file
 * @return Stream pointer on success, NULL on failure
 */
HPAPI NX_AudioStream* NX_LoadAudioStream(const char* filePath);

/**
 * @brief Destroy loaded stream
 * @param stream Stream pointer
 */
HPAPI void NX_DestroyAudioStream(NX_AudioStream* stream);

/**
 * @brief Play stream
 * @param stream Stream pointer
 */
HPAPI void NX_PlayAudioStream(NX_AudioStream* stream);

/**
 * @brief Pause stream
 * @param stream Stream pointer
 */
HPAPI void NX_PauseAudioStream(NX_AudioStream* stream);

/**
 * @brief Stop stream
 * @param stream Stream pointer
 */
HPAPI void NX_StopAudioStream(NX_AudioStream* stream);

/**
 * @brief Rewind stream to the beginning
 * @param stream Stream pointer
 */
HPAPI void NX_RewindAudioStream(NX_AudioStream* stream);

/**
 * @brief Check if stream is currently playing
 * @param stream Stream pointer
 * @return true if playing, false otherwise
 */
HPAPI bool NX_IsAudioStreamPlaying(NX_AudioStream* stream);

/**
 * @brief Get current looping state of stream
 * @param stream Stream pointer
 * @return true if looping is enabled, false otherwise
 */
HPAPI bool NX_GetAudioStreamLoop(NX_AudioStream* stream);

/**
 * @brief Enable or disable looping for stream
 * @param stream Stream pointer
 * @param loop true to loop continuously, false to stop at the end
 */
HPAPI void NX_SetAudioStreamLoop(NX_AudioStream* stream, bool loop);

/**
 * @brief Get the total duration of the audio stream in seconds.
 * @param stream Pointer to the audio stream.
 * @return Duration of the stream in seconds.
 */
HPAPI float NX_GetAudioStreamDuration(const NX_AudioStream* stream);

/** @} */ // Stream

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_AUDIO_H
