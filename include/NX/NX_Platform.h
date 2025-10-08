/* NX_Platform.h -- Contains target platform specific definitions
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_PLATFORM_H
#define NX_PLATFORM_H

#if defined(_WIN32)
#   if defined(__TINYC__)
#       define __declspec(x) __attribute__((x))
#   endif
#   if defined(NX_BUILD_SHARED)
#       define NXAPI __declspec(dllexport)
#   elif defined(NX_USE_SHARED)
#       define NXAPI __declspec(dllimport)
#   endif
#else
#   if defined(NX_BUILD_SHARED)
#       define NXAPI __attribute__((visibility("default")))
#   endif
#endif

#ifndef NXAPI
#   define NXAPI extern
#endif

#ifndef NX_RESTRICT
#   if defined(__cplusplus) || defined(_MSC_VER)
#       define NX_RESTRICT __restrict
#   else
#       define NX_RESTRICT restrict
#   endif
#endif

#if defined(__FMA__) && defined(__AVX2__)
#   define NX_HAS_FMA_AVX2
#   include <immintrin.h>
#endif

#if defined(__FMA__) && defined(__AVX__)
#    define NX_HAS_FMA_AVX
#    include <immintrin.h>
#endif

#if defined(__AVX2__)
#    define NX_HAS_AVX2
#    include <immintrin.h>
#endif

#if defined(__AVX__)
#    define NX_HAS_AVX
#    include <immintrin.h>
#endif

#if defined(__SSE4_2__)
#    define NX_HAS_SSE42
#    include <nmmintrin.h>
#endif

#if defined(__SSE4_1__)
#    define NX_HAS_SSE41
#    include <smmintrin.h>
#endif

#if defined(__SSSE3__)
#    define NX_HAS_SSSE3
#    include <tmmintrin.h>
#endif

#if defined(__SSE3__)
#    define NX_HAS_SSE3
#    include <pmmintrin.h>
#endif

#if defined(__SSE2__)
#    define NX_HAS_SSE2
#    include <emmintrin.h>
#endif

#if defined(__SSE__)
#    define NX_HAS_SSE
#    include <xmmintrin.h>
#endif

#if defined(__ARM_NEON) || defined(__aarch64__)
#    if defined(__ARM_FEATURE_FMA)
#        define NX_HAS_NEON_FMA
#    else
#        define NX_HAS_NEON
#    endif
#    include <arm_neon.h>
#endif

#endif // NX_PLATFORM_H
