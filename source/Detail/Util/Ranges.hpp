/* Ranges.hpp -- Range based loops support
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_UTIL_RANGES_HPP
#define NX_UTIL_RANGES_HPP

namespace util {

template<typename Container>
concept has_begin_end = requires(Container c) {
    { c.Begin() };
    { c.End() };
};

template<typename Container>
concept has_reverse_begin_end = requires(Container c) {
    { c.ReverseBegin() };
    { c.ReverseEnd() };
};

template<has_begin_end Container>
constexpr auto begin(Container&& c) noexcept
{
    return c.Begin();
}

template<has_begin_end Container>
constexpr auto end(Container&& c) noexcept
{
    return c.End();
}

template<has_begin_end Container>
constexpr auto begin(const Container& c) noexcept
{
    return c.Begin();
}

template<has_begin_end Container>
constexpr auto end(const Container& c) noexcept
{
    return c.End();
}

template<has_reverse_begin_end Container>
constexpr auto rbegin(Container&& c) noexcept
{
    return c.ReverseBegin();
}

template<has_reverse_begin_end Container>
constexpr auto rend(Container&& c) noexcept
{
    return c.ReverseEnd();
}

template<has_reverse_begin_end Container>
constexpr auto rbegin(const Container& c) noexcept
{
    return c.ReverseBegin();
}

template<has_reverse_begin_end Container>
constexpr auto rend(const Container& c) noexcept
{
    return c.ReverseEnd();
}

template<has_begin_end Container>
constexpr auto cbegin(const Container& c) noexcept
{
    return c.Begin();
}

template<has_begin_end Container>
constexpr auto cend(const Container& c) noexcept
{
    return c.End();
}

template<has_reverse_begin_end Container>
constexpr auto crbegin(const Container& c) noexcept
{
    return c.ReverseBegin();
}

template<has_reverse_begin_end Container>
constexpr auto crend(const Container& c) noexcept
{
    return c.ReverseEnd();
}

} // namespace util

#endif // NX_UTIL_RANGES_HPP
