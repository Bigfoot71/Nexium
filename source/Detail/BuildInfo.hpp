/* BuildInfo.hpp -- Contains build information
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_DETAIL_BUILD_INFO_HPP
#define NX_DETAIL_BUILD_INFO_HPP

namespace detail {

struct BuildInfo {
#ifdef NDEBUG
static constexpr bool release = true;
static constexpr bool debug = false;
#else
static constexpr bool release = false;
static constexpr bool debug = true;
#endif
};

} // namespace detail

#endif // NX_DETAIL_BUILD_INFO_HPP
