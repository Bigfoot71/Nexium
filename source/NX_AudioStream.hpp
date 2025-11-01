#ifndef NX_AUDIO_STREAM_HPP
#define NX_AUDIO_STREAM_HPP

#include "./Detail/Util/Memory.hpp"
#include "./INX_AudioFormat.hpp"

#include <stb_vorbis.h>
#include <dr_flac.h>
#include <dr_wav.h>
#include <dr_mp3.h>
#include <array>
#include <al.h>

struct NX_AudioStream {
    static constexpr size_t BufferCount = 3;
    static constexpr size_t BufferSize = 256 * 32 * 2 * 2; // frames * blocks * channels * bytes

    // OpenAL resources
    std::array<ALuint, BufferCount> buffers{};
    ALuint source{};
    ALenum format{};

    // Audio data and decoder
    util::UniquePtr<uint8_t> audioData{};
    INX_AudioFormat audioFormat{};
    
    union Decoder {
        drwav* wav;
        drflac* flac;
        drmp3* mp3;
        stb_vorbis* ogg;
    } decoder{};

    // State flags
    bool shouldLoop{};
    bool isPaused{};
    bool isPlaying{};

    ~NX_AudioStream();
};

#endif // NX_AUDIO_STREAM_HPP
