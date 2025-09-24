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

#ifndef HP_CORE_INTERNAL_LOG_HPP
#define HP_CORE_INTERNAL_LOG_HPP

#include <Hyperion/HP_Core.h>

#ifndef HP_NO_INTERNAL_LOGS
#   define HP_INTERNAL_LOG(level, fmt, ...) HP_Log##level(fmt __VA_OPT__(,) __VA_ARGS__)
#else
#   define HP_INTERNAL_LOG(level, fmt, ...) do { } while (0)
#endif

#endif // HP_CORE_INTERNAL_LOG_HPP
