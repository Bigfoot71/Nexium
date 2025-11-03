/* INX_BuildInfo.hpp -- Contains build information
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef INX_BUILD_INFO_HPP
#define INX_BUILD_INFO_HPP

struct INX_BuildInfo {
#ifdef NDEBUG
static constexpr bool release = true;
static constexpr bool debug = false;
#else
static constexpr bool release = false;
static constexpr bool debug = true;
#endif
};

#endif // INX_BUILD_INFO_HPP
