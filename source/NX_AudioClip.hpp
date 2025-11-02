/* NX_AudioClip.hpp -- API definition for Nexium's audio clip module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

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
