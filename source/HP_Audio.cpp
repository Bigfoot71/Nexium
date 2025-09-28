/* HP_Audio.cpp -- API definition for Hyperion's audio module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <Hyperion/HP_Audio.h>
#include "./Audio/HP_AudioState.hpp"

/* === Public API === */

float HP_GetMasterVolume(void)
{
    return gAudio->getMasterVolume();
}

float HP_GetAudioClipVolume(void)
{
    return gAudio->getClipVolume();
}

float HP_GetAudioStreamVolume(void)
{
    return gAudio->getStreamVolume();
}

void HP_SetMasterVolume(float volume)
{
    gAudio->setMasterVolume(volume);
}

void HP_SetAudioClipVolume(float volume)
{
    gAudio->setClipVolume(volume);
}

void HP_SetAudioStreamVolume(float volume)
{
    gAudio->setStreamVolume(volume);
}

HP_AudioClip* HP_LoadAudioClip(const char* filePath, int channelCount)
{
    return gAudio->createClip(filePath, channelCount);
}

void HP_DestroyAudioClip(HP_AudioClip* clip)
{
    gAudio->destroyClip(clip);
}

int HP_PlayAudioClip(HP_AudioClip* clip, int channel)
{
    return clip->play(channel);
}

void HP_PauseAudioClip(HP_AudioClip* clip, int channel)
{
    clip->pause(channel);
}

void HP_StopAudioClip(HP_AudioClip* clip, int channel)
{
    clip->stop(channel);
}

void HP_RewindAudioStream(HP_AudioClip* clip, int channel)
{
    clip->rewind(channel);
}

bool HP_IsAudioClipPlaying(HP_AudioClip* clip, int channel)
{
    return clip->isPlaying(channel);
}

int HP_GetAudioClipChannelCount(HP_AudioClip* clip)
{
    return clip->getChannelCount();
}

HP_AudioStream* HP_LoadAudioStream(const char* filePath)
{
    return gAudio->createStream(filePath);
}

void HP_DestroyAudioStream(HP_AudioStream* stream)
{
    gAudio->destroyStream(stream);
}

void HP_PlayAudioStream(HP_AudioStream* stream)
{
    stream->play();
}

void HP_PauseAudioStream(HP_AudioStream* stream)
{
    stream->pause();
}

void HP_StopAudioStream(HP_AudioStream* stream)
{
    stream->stop();
}

void HP_RewindAudioStream(HP_AudioStream* stream)
{
    stream->rewind();
}

bool HP_IsAudioStreamPlaying(HP_AudioStream* stream)
{
    return stream->isPlaying();
}

bool HP_GetAudioStreamLoop(HP_AudioStream* stream)
{
    return stream->getLoop();
}

void HP_SetAudioStreamLoop(HP_AudioStream* stream, bool loop)
{
    stream->setLoop(loop);
}

float HP_GetAudioStreamDuration(const HP_AudioStream* stream)
{
    return stream->getDuration();
}
