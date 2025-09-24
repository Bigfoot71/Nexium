/**
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided "as-is", without any express or implied warranty. In no event
 * will the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including commercial
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not claim that you
 *   wrote the original software. If you use this software in a product, an acknowledgment
 *   in the product documentation would be appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *   as being the original software.
 *
 *   3. This notice may not be removed or altered from any source distribution.
 */

#include "./HP_AudioClip.hpp"

#include "../Core/HP_InternalLog.hpp"
#include "./HP_AudioFormat.hpp"
#include "./HP_AudioState.hpp"

/* === Public Implementation === */

HP_AudioClip::HP_AudioClip(const char* filePath, int channelCount)
    : mSources(channelCount, channelCount)
{
    if (channelCount <= 0) {
        HP_INTERNAL_LOG(E, "AUDIO: Invalid channel count %d", channelCount);
        return;
    }

    if (!filePath) {
        HP_INTERNAL_LOG(E, "AUDIO: Null file path");
        return;
    }

    /* --- Load file data --- */

    size_t fileSize = 0;
    void* fileData = HP_LoadFile(filePath, &fileSize);
    if (!fileData) {
        HP_INTERNAL_LOG(E, "AUDIO: Unable to load file '%s'", filePath);
        return;
    }

    /* --- Decode according to format --- */

    RawData audioData;
    auto format = getAudioFormat(static_cast<const uint8_t*>(fileData), fileSize);

    switch (format) {
        case HP_AudioFormat::WAV:
            audioData = loadWAV(fileData, fileSize);
            break;
        case HP_AudioFormat::FLAC:
            audioData = loadFLAC(fileData, fileSize);
            break;
        case HP_AudioFormat::MP3:
            audioData = loadMP3(fileData, fileSize);
            break;
        case HP_AudioFormat::OGG:
            audioData = loadOGG(fileData, fileSize);
            break;
        default:
            HP_INTERNAL_LOG(E, "AUDIO: Unknown audio format for '%s'", filePath);
            SDL_free(fileData);
            return;
    }

    SDL_free(fileData);

    if (!audioData.pcmData) {
        HP_INTERNAL_LOG(E, "AUDIO: Failed to decode audio file '%s'", filePath);
        return;
    }

    /* --- Create the OpenAL buffer --- */

    alGenBuffers(1, &mBuffer);
    if (alGetError() != AL_NO_ERROR) {
        HP_INTERNAL_LOG(E, "AUDIO: Could not generate OpenAL buffer");
        cleanupRawData(audioData);
        return;
    }

    /* --- Load data into the buffer --- */

    alBufferData(mBuffer, audioData.format, audioData.pcmData, audioData.pcmDataSize, audioData.sampleRate);
    if (alGetError() != AL_NO_ERROR) {
        HP_INTERNAL_LOG(E, "AUDIO: Could not buffer data to OpenAL");
        alDeleteBuffers(1, &mBuffer);
        cleanupRawData(audioData);
        return;
    }

    /* --- Cleanup PCM data (no longer needed now) --- */

    cleanupRawData(audioData);

    /* --- Create the OpenAL sources --- */

    alGenSources(channelCount, mSources.data());
    if (alGetError() != AL_NO_ERROR) {
        HP_INTERNAL_LOG(E, "AUDIO: Could not generate OpenAL sources");
        alDeleteBuffers(1, &mBuffer);
        return;
    }

    /* --- Attach buffer to all sources --- */

    for (int i = 0; i < channelCount; i++) {
        alSourcei(mSources[i], AL_BUFFER, mBuffer);
        if (alGetError() != AL_NO_ERROR) {
            HP_INTERNAL_LOG(E, "AUDIO: Could not attach buffer to source %d", i);
            alDeleteSources(channelCount, mSources.data());
            alDeleteBuffers(1, &mBuffer);
            return;
        }
    }

    /* --- Set initial volume --- */

    for (ALuint source : mSources) {
        alSourcef(source, AL_GAIN, gAudio->getStreamGain());
    }
}
