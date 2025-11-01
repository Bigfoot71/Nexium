#ifndef NX_AUDIO_CLIP_HPP
#define NX_AUDIO_CLIP_HPP

#include "./Detail/Util/FixedArray.hpp"
#include <al.h>

struct NX_AudioClip {
    util::FixedArray<ALuint> sources{};
    ALuint buffer{};
    ~NX_AudioClip();
};

#endif // NX_AUDIO_CLIP_HPP
