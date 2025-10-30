/* NX_API.h — Symbol export/import definitions for Nexium
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_API_H
#define NX_API_H

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

#endif // NX_API_H
