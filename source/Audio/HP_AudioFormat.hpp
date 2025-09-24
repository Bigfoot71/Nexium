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

#ifndef HP_AUDIO_FORMAT_HPP
#define HP_AUDIO_FORMAT_HPP

#include <Hyperion/HP_Core.h>
#include <SDL3/SDL_stdinc.h>
#include <al.h>

#include "../Core/HP_InternalLog.hpp"

/* === Enums === */

enum class HP_AudioFormat {
    Unknown,
    WAV,
    FLAC,
    MP3,
    OGG,
};

/* === Helper Functions === */

inline const char* getAudioFormatName(ALenum format)
{
    switch (format) {
    case AL_FORMAT_MONO8:
        return "Mono 8-Bit";
    case AL_FORMAT_MONO16:
        return "Mono 16-Bit";
    case AL_FORMAT_STEREO8:
        return "Stereo 8-Bit";
    case AL_FORMAT_STEREO16:
        return "Stereo 16-Bit";
    default:
        break;
    }

    return "Unknown";
}

inline HP_AudioFormat getAudioFormat(const uint8_t* data, size_t size)
{
    // Check for WAV format (RIFF + WAVE)
    if (size >= 12 && SDL_memcmp(data, "RIFF", 4) == 0 && SDL_memcmp(data + 8, "WAVE", 4) == 0) {
        return HP_AudioFormat::WAV;
    }

    // Check for FLAC format
    if (size >= 4 && SDL_memcmp(data, "fLaC", 4) == 0) {
        return HP_AudioFormat::FLAC;
    }

    // Check for MP3 format (ID3 tag or sync frame)
    if (size >= 3 && (SDL_memcmp(data, "ID3", 3) == 0 ||
        (size >= 2 && data[0] == 0xFF && (data[1] & 0xE0) == 0xE0))) {
        return HP_AudioFormat::MP3;
    }

    // Check for OGG container format
    if (size >= 4 && SDL_memcmp(data, "OggS", 4) == 0) {
        // Look for Vorbis codec identifier in the first logical stream
        // Vorbis identification header starts with packet type 0x01 followed by "vorbis"
        for (size_t i = 28; i < size - 6; i++) {
            if (data[i] == 0x01 && SDL_memcmp(data + i + 1, "vorbis", 6) == 0) {
                return HP_AudioFormat::OGG;
            }
        }

        // Check for other common OGG codecs and report them as unsupported
        for (size_t i = 28; i < size - 8; i++) {
            if (SDL_memcmp(data + i, "OpusHead", 8) == 0) {
                HP_INTERNAL_LOG(E, "AUDIO: OGG Opus codec detected but not supported (only OGG Vorbis is supported)");
                return HP_AudioFormat::Unknown;
            }
            if (data[i] == 0x80 && SDL_memcmp(data + i + 1, "theora", 6) == 0) {
                HP_INTERNAL_LOG(E, "AUDIO: OGG Theora codec detected but not supported (video codec, only OGG Vorbis audio is supported)");
                return HP_AudioFormat::Unknown;
            }
            if (data[i] == 0x7F && SDL_memcmp(data + i + 1, "FLAC", 4) == 0) {
                HP_INTERNAL_LOG(E, "AUDIO: OGG FLAC codec detected but not supported (use native FLAC format instead)");
                return HP_AudioFormat::Unknown;
            }
            if (SDL_memcmp(data + i, "Speex   ", 8) == 0) {
                HP_INTERNAL_LOG(E, "AUDIO: OGG Speex codec detected but not supported (only OGG Vorbis is supported)");
                return HP_AudioFormat::Unknown;
            }
        }

        HP_INTERNAL_LOG(E, "AUDIO: OGG container detected but codec not recognized or supported (only OGG Vorbis is supported)");
        return HP_AudioFormat::Unknown;
    }

    return HP_AudioFormat::Unknown;
}

#endif // HP_AUDIO_FORMAT_HPP
