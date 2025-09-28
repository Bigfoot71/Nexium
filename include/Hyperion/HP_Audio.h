/* HP_Audio.h -- API declaration for Hyperion's render module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef HP_AUDIO_H
#define HP_AUDIO_H

#include "./HP_Platform.h"
#include <stdbool.h>

/* === Types === */

typedef struct HP_AudioClip HP_AudioClip;
typedef struct HP_AudioStream HP_AudioStream;

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
HPAPI float HP_GetMasterVolume(void);

/**
 * @brief Get the audio clips volume
 * @return Clip volume (0.0 = mute, 1.0 = max)
 */
HPAPI float HP_GetAudioClipVolume(void);

/**
 * @brief Get the audio streams volume
 * @return Stream volume (0.0 = mute, 1.0 = max)
 */
HPAPI float HP_GetAudioStreamVolume(void);

/**
 * @brief Set the master volume
 * @param volume Volume value (0.0 = mute, 1.0 = max)
 */
HPAPI void HP_SetMasterVolume(float volume);

/**
 * @brief Set the audio clips volume
 * @param volume Volume value (0.0 = mute, 1.0 = max)
 */
HPAPI void HP_SetAudioClipVolume(float volume);

/**
 * @brief Set the audio streams volume
 * @param volume Volume value (0.0 = mute, 1.0 = max)
 */
HPAPI void HP_SetAudioStreamVolume(float volume);

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
HPAPI HP_AudioClip* HP_LoadAudioClip(const char* filePath, int channelCount);

/**
 * @brief Destroy a loaded clip and free all associated resources
 * @param clip Clip pointer to destroy
 */
HPAPI void HP_DestroyAudioClip(HP_AudioClip* clip);

/**
 * @brief Play a clip on a specific channel or automatically find a free one
 * @param clip Clip pointer to play
 * @param channel Channel index (>= 0) or negative value to auto-select first available channel
 * @return Channel index where the clip is played, or -1 if no channel is available
 */
HPAPI int HP_PlayAudioClip(HP_AudioClip* clip, int channel);

/**
 * @brief Pause a clip on a specific channel or all channels
 * @param clip Clip pointer to pause
 * @param channel Channel index (>= 0) to pause specific channel, or negative value to pause all channels
 */
HPAPI void HP_PauseAudioClip(HP_AudioClip* clip, int channel);

/**
 * @brief Stop a clip on a specific channel or all channels
 * @param clip Clip pointer to stop
 * @param channel Channel index (>= 0) to stop specific channel, or negative value to stop all channels
 */
HPAPI void HP_StopAudioClip(HP_AudioClip* clip, int channel);

/**
 * @brief Rewind a clip on a specific channel or all channels
 * @param clip Clip pointer to rewind
 * @param channel Channel index (>= 0) to rewind specific channel, or negative value to rewind all channels
 */
HPAPI void HP_RewindAudioClip(HP_AudioClip* clip, int channel);

/**
 * @brief Check if a clip is playing on a specific channel or any channel
 * @param clip Clip pointer to check
 * @param channel Channel index (>= 0) to check specific channel, or negative value to check if playing on any channel
 * @return true if clip is playing on the specified channel(s), false otherwise
 */
HPAPI bool HP_IsAudioClipPlaying(HP_AudioClip* clip, int channel);

/**
 * @brief Get the number of channels assigned to a clip
 * @param clip Clip pointer
 * @return Number of channels, or 0 if clip pointer is invalid
 */
HPAPI int HP_GetAudioClipChannelCount(HP_AudioClip* clip);

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
HPAPI HP_AudioStream* HP_LoadAudioStream(const char* filePath);

/**
 * @brief Destroy loaded stream
 * @param stream Stream pointer
 */
HPAPI void HP_DestroyAudioStream(HP_AudioStream* stream);

/**
 * @brief Play stream
 * @param stream Stream pointer
 */
HPAPI void HP_PlayAudioStream(HP_AudioStream* stream);

/**
 * @brief Pause stream
 * @param stream Stream pointer
 */
HPAPI void HP_PauseAudioStream(HP_AudioStream* stream);

/**
 * @brief Stop stream
 * @param stream Stream pointer
 */
HPAPI void HP_StopAudioStream(HP_AudioStream* stream);

/**
 * @brief Rewind stream to the beginning
 * @param stream Stream pointer
 */
HPAPI void HP_RewindAudioStream(HP_AudioStream* stream);

/**
 * @brief Check if stream is currently playing
 * @param stream Stream pointer
 * @return true if playing, false otherwise
 */
HPAPI bool HP_IsAudioStreamPlaying(HP_AudioStream* stream);

/**
 * @brief Get current looping state of stream
 * @param stream Stream pointer
 * @return true if looping is enabled, false otherwise
 */
HPAPI bool HP_GetAudioStreamLoop(HP_AudioStream* stream);

/**
 * @brief Enable or disable looping for stream
 * @param stream Stream pointer
 * @param loop true to loop continuously, false to stop at the end
 */
HPAPI void HP_SetAudioStreamLoop(HP_AudioStream* stream, bool loop);

/**
 * @brief Get the total duration of the audio stream in seconds.
 * @param stream Pointer to the audio stream.
 * @return Duration of the stream in seconds.
 */
HPAPI float HP_GetAudioStreamDuration(const HP_AudioStream* stream);

/** @} */ // Stream

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // HP_AUDIO_H
