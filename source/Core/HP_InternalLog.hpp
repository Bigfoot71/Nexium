/* HP_InternalLog.hpp -- Contains internal log helper that can be disabled
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef HP_CORE_INTERNAL_LOG_HPP
#define HP_CORE_INTERNAL_LOG_HPP

#include <Hyperion/HP_Core.h>

#ifndef HP_NO_INTERNAL_LOGS
#	define HP_INTERNAL_LOG(level, fmt, ...) HP_Log##level(fmt, ##__VA_ARGS__)
#else
#   define HP_INTERNAL_LOG(level, fmt, ...) do { } while (0)
#endif

#endif // HP_CORE_INTERNAL_LOG_HPP
