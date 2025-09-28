/* StaticArray.hpp -- STL-like static array with noexcept operations
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef HP_UTIL_STATIC_ARRAY_HPP
#define HP_UTIL_STATIC_ARRAY_HPP

#include <algorithm>
#include <iterator>
#include <utility>
#include <cstddef>

namespace util {

/* === Declaration === */

/**
 * @brief A simple fixed-size array with size known at compile time, 100% noexcept and constexpr.
 *
 * Provides a fully compile-time sized array with some adapted behaviors.
 * The coding style is kept close to the STL for compatibility.
 *
 * @tparam T Type of the elements stored in the array.
 * @tparam N Number of elements in the array, known at compile time.
 */
template <typename T, size_t N>
class StaticArray {
    static_assert(N > 0, "StaticArray size must be greater than 0");
public:
    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    /** Constructors */
    constexpr StaticArray() noexcept;
    constexpr explicit StaticArray(size_type count) noexcept;
    constexpr StaticArray(size_type count, const T& value) noexcept;

    template<typename InputIt>
    constexpr StaticArray(InputIt first, InputIt last) noexcept;

    constexpr StaticArray(std::initializer_list<T> init) noexcept;

    constexpr ~StaticArray() noexcept = default;

    /** Non-copyable, move-only */
    StaticArray(const StaticArray&) = delete;
    StaticArray& operator=(const StaticArray&) = delete;

    constexpr StaticArray(StaticArray&& other) noexcept;
    constexpr StaticArray& operator=(StaticArray&& other) noexcept;

    /** Assignment */
    constexpr bool assign(size_type count, const T& value) noexcept;
    template<typename InputIt>
    constexpr bool assign(InputIt first, InputIt last) noexcept;
    constexpr bool assign(std::initializer_list<T> init) noexcept;

    /** Element access */
    [[nodiscard]] constexpr pointer at(size_type pos) noexcept;
    [[nodiscard]] constexpr const_pointer at(size_type pos) const noexcept;
    [[nodiscard]] constexpr reference operator[](size_type index) noexcept;
    [[nodiscard]] constexpr const_reference operator[](size_type index) const noexcept;
    [[nodiscard]] constexpr pointer front() noexcept;
    [[nodiscard]] constexpr const_pointer front() const noexcept;
    [[nodiscard]] constexpr pointer back() noexcept;
    [[nodiscard]] constexpr const_pointer back() const noexcept;
    [[nodiscard]] constexpr pointer data() noexcept;
    [[nodiscard]] constexpr const_pointer data() const noexcept;

    /** Iterators */
    [[nodiscard]] constexpr iterator begin() noexcept;
    [[nodiscard]] constexpr const_iterator begin() const noexcept;
    [[nodiscard]] constexpr const_iterator cbegin() const noexcept;
    [[nodiscard]] constexpr iterator end() noexcept;
    [[nodiscard]] constexpr const_iterator end() const noexcept;
    [[nodiscard]] constexpr const_iterator cend() const noexcept;
    [[nodiscard]] constexpr reverse_iterator rbegin() noexcept;
    [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept;
    [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept;
    [[nodiscard]] constexpr reverse_iterator rend() noexcept;
    [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept;
    [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept;

    /** Capacity */
    [[nodiscard]] constexpr bool empty() const noexcept;
    [[nodiscard]] constexpr size_type size() const noexcept;
    [[nodiscard]] static constexpr size_type capacity() noexcept;
    [[nodiscard]] static constexpr size_type max_size() noexcept;

    /** Modifiers */
    constexpr void clear() noexcept;
    constexpr bool push_back(const T& value) noexcept;
    constexpr bool push_back(T&& value) noexcept;

    template<typename... Args>
    constexpr pointer emplace_back(Args&&... args) noexcept;

    constexpr void pop_back() noexcept;
    constexpr bool resize(size_type count) noexcept;
    constexpr bool resize(size_type count, const T& value) noexcept;
    constexpr void swap(StaticArray& other) noexcept;

private:
    /** Helpers for construction/destruction */
    template<typename... Args>
    [[nodiscard]] constexpr pointer construct_at(pointer p, Args&&... args) noexcept;
    constexpr void destroy_at(pointer p) noexcept;
    constexpr void destroy_range(pointer first, pointer last) noexcept;

private:
    T mData[N];
    size_type mSize;
};

/* === Public Implementation === */

template<typename T, size_t N>
constexpr StaticArray<T, N>::StaticArray() noexcept
    : mData{}, mSize(0)
{ }

template<typename T, size_t N>
constexpr StaticArray<T, N>::StaticArray(size_type count) noexcept
    : StaticArray()
{
    if (count > 0) {
        resize(count);
    }
}

template<typename T, size_t N>
constexpr StaticArray<T, N>::StaticArray(size_type count, const T& value) noexcept
    : StaticArray()
{
    assign(count, value);
}

template<typename T, size_t N>
template<typename InputIt>
constexpr StaticArray<T, N>::StaticArray(InputIt first, InputIt last) noexcept
    : StaticArray()
{
    assign(first, last);
}

template<typename T, size_t N>
constexpr StaticArray<T, N>::StaticArray(std::initializer_list<T> init) noexcept
    : StaticArray(init.begin(), init.end())
{ }

template<typename T, size_t N>
constexpr StaticArray<T, N>::StaticArray(StaticArray&& other) noexcept
    : mData{}, mSize(other.mSize)
{
    for (size_type i = 0; i < mSize; ++i) {
        construct_at(mData + i, std::move(other.mData[i]));
        destroy_at(other.mData + i);
    }
    other.mSize = 0;
}

template<typename T, size_t N>
constexpr StaticArray<T, N>& StaticArray<T, N>::operator=(StaticArray&& other) noexcept
{
    if (this != &other) {
        clear();
        mSize = other.mSize;
        for (size_type i = 0; i < mSize; ++i) {
            construct_at(mData + i, std::move(other.mData[i]));
            destroy_at(other.mData + i);
        }
        other.mSize = 0;
    }
    return *this;
}

template<typename T, size_t N>
constexpr bool StaticArray<T, N>::assign(size_type count, const T& value) noexcept
{
    clear();
    return resize(count, value);
}

template<typename T, size_t N>
template<typename InputIt>
constexpr bool StaticArray<T, N>::assign(InputIt first, InputIt last) noexcept
{
    clear();
    for (auto it = first; it != last; ++it) {
        if (!push_back(*it)) {
            return false;
        }
    }
    return true;
}

template<typename T, size_t N>
constexpr bool StaticArray<T, N>::assign(std::initializer_list<T> init) noexcept
{
    return assign(init.begin(), init.end());
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::pointer StaticArray<T, N>::at(size_type pos) noexcept
{
    return (pos < mSize) ? mData + pos : nullptr;
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::const_pointer StaticArray<T, N>::at(size_type pos) const noexcept
{
    return (pos < mSize) ? mData + pos : nullptr;
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::reference StaticArray<T, N>::operator[](size_type index) noexcept
{
    return *(mData + index);
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::const_reference StaticArray<T, N>::operator[](size_type index) const noexcept
{
    return *(mData + index);
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::pointer StaticArray<T, N>::front() noexcept
{
    return mSize > 0 ? mData : nullptr;
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::const_pointer StaticArray<T, N>::front() const noexcept
{
    return mSize > 0 ? mData : nullptr;
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::pointer StaticArray<T, N>::back() noexcept
{
    return mSize > 0 ? mData + mSize - 1 : nullptr;
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::const_pointer StaticArray<T, N>::back() const noexcept
{
    return mSize > 0 ? mData + mSize - 1 : nullptr;
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::pointer StaticArray<T, N>::data() noexcept
{
    return mData;
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::const_pointer StaticArray<T, N>::data() const noexcept
{
    return mData;
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::iterator StaticArray<T, N>::begin() noexcept
{
    return mData;
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::const_iterator StaticArray<T, N>::begin() const noexcept
{
    return mData;
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::const_iterator StaticArray<T, N>::cbegin() const noexcept
{
    return mData;
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::iterator StaticArray<T, N>::end() noexcept
{
    return mData + mSize;
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::const_iterator StaticArray<T, N>::end() const noexcept
{
    return mData + mSize;
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::const_iterator StaticArray<T, N>::cend() const noexcept
{
    return mData + mSize;
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::reverse_iterator StaticArray<T, N>::rbegin() noexcept
{
    return reverse_iterator(end());
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::const_reverse_iterator StaticArray<T, N>::rbegin() const noexcept
{
    return const_reverse_iterator(end());
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::const_reverse_iterator StaticArray<T, N>::crbegin() const noexcept
{
    return const_reverse_iterator(end());
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::reverse_iterator StaticArray<T, N>::rend() noexcept
{
    return reverse_iterator(begin());
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::const_reverse_iterator StaticArray<T, N>::rend() const noexcept
{
    return const_reverse_iterator(begin());
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::const_reverse_iterator StaticArray<T, N>::crend() const noexcept
{
    return const_reverse_iterator(begin());
}

template<typename T, size_t N>
[[nodiscard]] constexpr bool StaticArray<T, N>::empty() const noexcept
{
    return mSize == 0;
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::size_type StaticArray<T, N>::size() const noexcept
{
    return mSize;
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::size_type StaticArray<T, N>::capacity() noexcept
{
    return N;
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::size_type StaticArray<T, N>::max_size() noexcept
{
    return N; // For StaticArray, max_size = capacity = N
}

template<typename T, size_t N>
constexpr void StaticArray<T, N>::clear() noexcept
{
    destroy_range(mData, mData + mSize);
    mSize = 0;
}

template<typename T, size_t N>
constexpr bool StaticArray<T, N>::push_back(const T& value) noexcept
{
    return emplace_back(value) != nullptr;
}

template<typename T, size_t N>
constexpr bool StaticArray<T, N>::push_back(T&& value) noexcept
{
    return emplace_back(std::move(value)) != nullptr;
}

template<typename T, size_t N>
template<typename... Args>
constexpr typename StaticArray<T, N>::pointer StaticArray<T, N>::emplace_back(Args&&... args) noexcept
{
    if (mSize >= N) {
        return nullptr;
    }
    pointer result = construct_at(mData + mSize, std::forward<Args>(args)...);
    if (result) {
        ++mSize;
    }
    return result;
}

template<typename T, size_t N>
constexpr void StaticArray<T, N>::pop_back() noexcept
{
    if (mSize > 0) {
        --mSize;
        destroy_at(mData + mSize);
    }
}

template<typename T, size_t N>
constexpr bool StaticArray<T, N>::resize(size_type count) noexcept
{
    if (count > N) {
        return false;
    }

    if (count < mSize) {
        destroy_range(mData + count, mData + mSize);
        mSize = count;
        return true;
    }
    else if (count > mSize) {
        size_type old_size = mSize;
        mSize = count; // Optimistic, we go back to the old one if it fails
        for (size_type i = old_size; i < count; ++i) {
            if (!construct_at(mData + i)) {
                // Failure, cleaning and return
                destroy_range(mData + old_size, mData + i);
                mSize = old_size;
                return false;
            }
        }
    }
    return true;
}

template<typename T, size_t N>
constexpr bool StaticArray<T, N>::resize(size_type count, const T& value) noexcept
{
    if (count > N) {
        return false;
    }

    if (count < mSize) {
        destroy_range(mData + count, mData + mSize);
        mSize = count;
        return true;
    }
    else if (count > mSize) {
        size_type old_size = mSize;
        mSize = count;
        for (size_type i = old_size; i < count; ++i) {
            if (!construct_at(mData + i, value)) {
                destroy_range(mData + old_size, mData + i);
                mSize = old_size;
                return false;
            }
        }
    }
    return true;
}

template<typename T, size_t N>
constexpr void StaticArray<T, N>::swap(StaticArray& other) noexcept
{
    size_type max_size = mSize > other.mSize ? mSize : other.mSize;

    // Swap existing elements
    size_type min_size = mSize < other.mSize ? mSize : other.mSize;
    for (size_type i = 0; i < min_size; ++i) {
        auto temp = std::move(mData[i]);
        mData[i] = std::move(other.mData[i]);
        other.mData[i] = std::move(temp);
    }

    // Move the extra items
    if (mSize > other.mSize) {
        for (size_type i = min_size; i < mSize; ++i) {
            construct_at(other.mData + i, std::move(mData[i]));
            destroy_at(mData + i);
        }
    }
    else if (other.mSize > mSize) {
        for (size_type i = min_size; i < other.mSize; ++i) {
            construct_at(mData + i, std::move(other.mData[i]));
            destroy_at(other.mData + i);
        }
    }

    auto temp_size = mSize;
    mSize = other.mSize;
    other.mSize = temp_size;
}

/* === Private Implementation === */

template<typename T, size_t N>
template<typename... Args>
[[nodiscard]] constexpr typename StaticArray<T, N>::pointer StaticArray<T, N>::construct_at(pointer p, Args&&... args) noexcept
{
    if constexpr (std::is_nothrow_constructible_v<T, Args...>) {
        new (p) T(std::forward<Args>(args)...);
        return p;
    }
    else {
        // In a constexpr context, try/catch cannot be used
        // We assume that construction always succeeds
        new (p) T(std::forward<Args>(args)...);
        return p;
    }
}

template<typename T, size_t N>
constexpr void StaticArray<T, N>::destroy_at(pointer p) noexcept
{
    if constexpr (!std::is_trivially_destructible_v<T>) {
        p->~T();
    }
}

template<typename T, size_t N>
constexpr void StaticArray<T, N>::destroy_range(pointer first, pointer last) noexcept
{
    if constexpr (!std::is_trivially_destructible_v<T>) {
        for (; first != last; ++first) {
            destroy_at(first);
        }
    }
}

/* === Non-member functions === */

template<typename T, size_t N>
[[nodiscard]] constexpr bool operator==(const StaticArray<T, N>& lhs, const StaticArray<T, N>& rhs) noexcept
{
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (typename StaticArray<T, N>::size_type i = 0; i < lhs.size(); ++i) {
        if (!(lhs[i] == rhs[i])) {
            return false;
        }
    }
    return true;
}

template<typename T, size_t N>
[[nodiscard]] constexpr bool operator!=(const StaticArray<T, N>& lhs, const StaticArray<T, N>& rhs) noexcept
{
    return !(lhs == rhs);
}

template<typename T, size_t N>
[[nodiscard]] constexpr bool operator<(const StaticArray<T, N>& lhs, const StaticArray<T, N>& rhs) noexcept
{
    auto lhs_size = lhs.size();
    auto rhs_size = rhs.size();
    auto min_size = lhs_size < rhs_size ? lhs_size : rhs_size;

    for (typename StaticArray<T, N>::size_type i = 0; i < min_size; ++i) {
        if (lhs[i] < rhs[i]) return true;
        if (rhs[i] < lhs[i]) return false;
    }
    return lhs_size < rhs_size;
}

template<typename T, size_t N>
[[nodiscard]] constexpr bool operator<=(const StaticArray<T, N>& lhs, const StaticArray<T, N>& rhs) noexcept
{
    return !(rhs < lhs);
}

template<typename T, size_t N>
[[nodiscard]] constexpr bool operator>(const StaticArray<T, N>& lhs, const StaticArray<T, N>& rhs) noexcept
{
    return rhs < lhs;
}

template<typename T, size_t N>
[[nodiscard]] constexpr bool operator>=(const StaticArray<T, N>& lhs, const StaticArray<T, N>& rhs) noexcept
{
    return !(lhs < rhs);
}

template<typename T, size_t N>
constexpr void swap(StaticArray<T, N>& lhs, StaticArray<T, N>& rhs) noexcept
{
    lhs.swap(rhs);
}

} // namespace util

#endif // HP_UTIL_STATIC_ARRAY_HPP
