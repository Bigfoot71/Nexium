/* NX_Audio.cpp -- API definition for Nexium's audio module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/NX_Audio.h>
#include "./Audio/NX_AudioState.hpp"

/* === Public API === */

float NX_GetMasterVolume(void)
{
    return gAudio->getMasterVolume();
}

float NX_GetAudioClipVolume(void)
{
    return gAudio->getClipVolume();
}

float NX_GetAudioStreamVolume(void)
{
    return gAudio->getStreamVolume();
}

void NX_SetMasterVolume(float volume)
{
    gAudio->setMasterVolume(volume);
}

void NX_SetAudioClipVolume(float volume)
{
    gAudio->setClipVolume(volume);
}

void NX_SetAudioStreamVolume(float volume)
{
    gAudio->setStreamVolume(volume);
}

NX_AudioClip* NX_LoadAudioClip(const char* filePath, int channelCount)
{
    return gAudio->createClip(filePath, channelCount);
}

void NX_DestroyAudioClip(NX_AudioClip* clip)
{
    gAudio->destroyClip(clip);
}

int NX_PlayAudioClip(NX_AudioClip* clip, int channel)
{
    return clip->play(channel);
}

void NX_PauseAudioClip(NX_AudioClip* clip, int channel)
{
    clip->pause(channel);
}

void NX_StopAudioClip(NX_AudioClip* clip, int channel)
{
    clip->stop(channel);
}

void NX_RewindAudioStream(NX_AudioClip* clip, int channel)
{
    clip->rewind(channel);
}

bool NX_IsAudioClipPlaying(NX_AudioClip* clip, int channel)
{
    return clip->isPlaying(channel);
}

int NX_GetAudioClipChannelCount(NX_AudioClip* clip)
{
    return clip->getChannelCount();
}

NX_AudioStream* NX_LoadAudioStream(const char* filePath)
{
    return gAudio->createStream(filePath);
}

void NX_DestroyAudioStream(NX_AudioStream* stream)
{
    gAudio->destroyStream(stream);
}

void NX_PlayAudioStream(NX_AudioStream* stream)
{
    stream->play();
}

void NX_PauseAudioStream(NX_AudioStream* stream)
{
    stream->pause();
}

void NX_StopAudioStream(NX_AudioStream* stream)
{
    stream->stop();
}

void NX_RewindAudioStream(NX_AudioStream* stream)
{
    stream->rewind();
}

bool NX_IsAudioStreamPlaying(NX_AudioStream* stream)
{
    return stream->isPlaying();
}

bool NX_GetAudioStreamLoop(NX_AudioStream* stream)
{
    return stream->getLoop();
}

void NX_SetAudioStreamLoop(NX_AudioStream* stream, bool loop)
{
    stream->setLoop(loop);
}

float NX_GetAudioStreamDuration(const NX_AudioStream* stream)
{
    return stream->getDuration();
}
