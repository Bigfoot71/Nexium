/* HP_Platform.h -- Contains target platform specific definitions
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef HP_PLATFORM_H
#define HP_PLATFORM_H

#if defined(_WIN32)
#   if defined(__TINYC__)
#       define __declspec(x) __attribute__((x))
#   endif
#   if defined(HP_BUILD_SHARED)
#       define HPAPI __declspec(dllexport)
#   elif defined(HP_USE_SHARED)
#       define HPAPI __declspec(dllimport)
#   endif
#else
#   if defined(HP_BUILD_SHARED)
#       define HPAPI __attribute__((visibility("default")))
#   endif
#endif

#ifndef HPAPI
#   define HPAPI extern
#endif

#ifndef HP_RESTRICT
#   if defined(__cplusplus) || defined(_MSC_VER)
#       define HP_RESTRICT __restrict
#   else
#       define HP_RESTRICT restrict
#   endif
#endif

#if defined(__FMA__) && defined(__AVX2__)
#   define HP_HAS_FMA_AVX2
#   include <immintrin.h>
#endif

#if defined(__FMA__) && defined(__AVX__)
#    define HP_HAS_FMA_AVX
#    include <immintrin.h>
#endif

#if defined(__AVX2__)
#    define HP_HAS_AVX2
#    include <immintrin.h>
#endif

#if defined(__AVX__)
#    define HP_HAS_AVX
#    include <immintrin.h>
#endif

#if defined(__SSE4_2__)
#    define HP_HAS_SSE42
#    include <nmmintrin.h>
#endif

#if defined(__SSE4_1__)
#    define HP_HAS_SSE41
#    include <smmintrin.h>
#endif

#if defined(__SSSE3__)
#    define HP_HAS_SSSE3
#    include <tmmintrin.h>
#endif

#if defined(__SSE3__)
#    define HP_HAS_SSE3
#    include <pmmintrin.h>
#endif

#if defined(__SSE2__)
#    define HP_HAS_SSE2
#    include <emmintrin.h>
#endif

#if defined(__SSE__)
#    define HP_HAS_SSE
#    include <xmmintrin.h>
#endif

#if defined(__ARM_NEON) || defined(__aarch64__)
#    if defined(__ARM_FEATURE_FMA)
#        define HP_HAS_NEON_FMA
#    else
#        define HP_HAS_NEON
#    endif
#    include <arm_neon.h>
#endif

#endif // HP_PLATFORM_H
