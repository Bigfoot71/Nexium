/* NX_InternalLog.hpp -- Contains internal log helper that can be disabled
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_CORE_INTERNAL_LOG_HPP
#define NX_CORE_INTERNAL_LOG_HPP

#include <NX/NX_Core.h>

#ifndef NX_NO_INTERNAL_LOGS
#	define NX_INTERNAL_LOG(level, fmt, ...) NX_Log##level(fmt, ##__VA_ARGS__)
#else
#   define NX_INTERNAL_LOG(level, fmt, ...) do { } while (0)
#endif

#endif // NX_CORE_INTERNAL_LOG_HPP
