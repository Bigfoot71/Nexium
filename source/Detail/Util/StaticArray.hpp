/* StaticArray.hpp -- STL-like static array with noexcept operations
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_UTIL_STATIC_ARRAY_HPP
#define NX_UTIL_STATIC_ARRAY_HPP

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
    constexpr bool Assign(size_type count, const T& value) noexcept;
    template<typename InputIt>
    constexpr bool Assign(InputIt first, InputIt last) noexcept;
    constexpr bool Assign(std::initializer_list<T> init) noexcept;

    /** Element access */
    [[nodiscard]] constexpr pointer GetAt(size_type pos) noexcept;
    [[nodiscard]] constexpr const_pointer GetAt(size_type pos) const noexcept;
    [[nodiscard]] constexpr reference operator[](size_type index) noexcept;
    [[nodiscard]] constexpr const_reference operator[](size_type index) const noexcept;
    [[nodiscard]] constexpr pointer GetFront() noexcept;
    [[nodiscard]] constexpr const_pointer GetFront() const noexcept;
    [[nodiscard]] constexpr pointer GetBack() noexcept;
    [[nodiscard]] constexpr const_pointer GetBack() const noexcept;
    [[nodiscard]] constexpr pointer GetData() noexcept;
    [[nodiscard]] constexpr const_pointer GetData() const noexcept;

    /** Iterators */
    [[nodiscard]] constexpr iterator Begin() noexcept;
    [[nodiscard]] constexpr const_iterator Begin() const noexcept;
    [[nodiscard]] constexpr iterator End() noexcept;
    [[nodiscard]] constexpr const_iterator End() const noexcept;
    [[nodiscard]] constexpr reverse_iterator ReverseBegin() noexcept;
    [[nodiscard]] constexpr const_reverse_iterator ReverseBegin() const noexcept;
    [[nodiscard]] constexpr reverse_iterator ReverseEnd() noexcept;
    [[nodiscard]] constexpr const_reverse_iterator ReverseEnd() const noexcept;

    /** Capacity */
    [[nodiscard]] constexpr bool IsEmpty() const noexcept;
    [[nodiscard]] constexpr size_type GetSize() const noexcept;
    [[nodiscard]] static constexpr size_type GetCapacity() noexcept;
    [[nodiscard]] static constexpr size_type GetMaxSize() noexcept;

    /** Modifiers */
    constexpr void Clear() noexcept;
    constexpr bool PushBack(const T& value) noexcept;
    constexpr bool PushBack(T&& value) noexcept;

    template<typename... Args>
    constexpr pointer EmplaceBack(Args&&... args) noexcept;

    constexpr void PopBack() noexcept;
    constexpr bool Resize(size_type count) noexcept;
    constexpr bool Resize(size_type count, const T& value) noexcept;
    constexpr void Swap(StaticArray& other) noexcept;

private:
    /** Helpers for construction/destruction */
    template<typename... Args>
    [[nodiscard]] constexpr pointer ConstructAt(pointer p, Args&&... args) noexcept;
    constexpr void DestroyAt(pointer p) noexcept;
    constexpr void DestroyRange(pointer first, pointer last) noexcept;

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
        Resize(count);
    }
}

template<typename T, size_t N>
constexpr StaticArray<T, N>::StaticArray(size_type count, const T& value) noexcept
    : StaticArray()
{
    Assign(count, value);
}

template<typename T, size_t N>
template<typename InputIt>
constexpr StaticArray<T, N>::StaticArray(InputIt first, InputIt last) noexcept
    : StaticArray()
{
    Assign(first, last);
}

template<typename T, size_t N>
constexpr StaticArray<T, N>::StaticArray(std::initializer_list<T> init) noexcept
    : StaticArray(init.Begin(), init.End())
{ }

template<typename T, size_t N>
constexpr StaticArray<T, N>::StaticArray(StaticArray&& other) noexcept
    : mData{}, mSize(other.mSize)
{
    for (size_type i = 0; i < mSize; ++i) {
        ConstructAt(mData + i, std::move(other.mData[i]));
        DestroyAt(other.mData + i);
    }
    other.mSize = 0;
}

template<typename T, size_t N>
constexpr StaticArray<T, N>& StaticArray<T, N>::operator=(StaticArray&& other) noexcept
{
    if (this != &other) {
        Clear();
        mSize = other.mSize;
        for (size_type i = 0; i < mSize; ++i) {
            ConstructAt(mData + i, std::move(other.mData[i]));
            DestroyAt(other.mData + i);
        }
        other.mSize = 0;
    }
    return *this;
}

template<typename T, size_t N>
constexpr bool StaticArray<T, N>::Assign(size_type count, const T& value) noexcept
{
    Clear();
    return Resize(count, value);
}

template<typename T, size_t N>
template<typename InputIt>
constexpr bool StaticArray<T, N>::Assign(InputIt first, InputIt last) noexcept
{
    Clear();
    for (auto it = first; it != last; ++it) {
        if (!PushBack(*it)) {
            return false;
        }
    }
    return true;
}

template<typename T, size_t N>
constexpr bool StaticArray<T, N>::Assign(std::initializer_list<T> init) noexcept
{
    return Assign(init.Begin(), init.End());
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::pointer StaticArray<T, N>::GetAt(size_type pos) noexcept
{
    return (pos < mSize) ? mData + pos : nullptr;
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::const_pointer StaticArray<T, N>::GetAt(size_type pos) const noexcept
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
[[nodiscard]] constexpr typename StaticArray<T, N>::pointer StaticArray<T, N>::GetFront() noexcept
{
    return mSize > 0 ? mData : nullptr;
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::const_pointer StaticArray<T, N>::GetFront() const noexcept
{
    return mSize > 0 ? mData : nullptr;
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::pointer StaticArray<T, N>::GetBack() noexcept
{
    return mSize > 0 ? mData + mSize - 1 : nullptr;
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::const_pointer StaticArray<T, N>::GetBack() const noexcept
{
    return mSize > 0 ? mData + mSize - 1 : nullptr;
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::pointer StaticArray<T, N>::GetData() noexcept
{
    return mData;
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::const_pointer StaticArray<T, N>::GetData() const noexcept
{
    return mData;
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::iterator StaticArray<T, N>::Begin() noexcept
{
    return mData;
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::const_iterator StaticArray<T, N>::Begin() const noexcept
{
    return mData;
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::iterator StaticArray<T, N>::End() noexcept
{
    return mData + mSize;
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::const_iterator StaticArray<T, N>::End() const noexcept
{
    return mData + mSize;
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::reverse_iterator StaticArray<T, N>::ReverseBegin() noexcept
{
    return reverse_iterator(End());
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::const_reverse_iterator StaticArray<T, N>::ReverseBegin() const noexcept
{
    return const_reverse_iterator(End());
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::reverse_iterator StaticArray<T, N>::ReverseEnd() noexcept
{
    return reverse_iterator(Begin());
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::const_reverse_iterator StaticArray<T, N>::ReverseEnd() const noexcept
{
    return const_reverse_iterator(Begin());
}

template<typename T, size_t N>
[[nodiscard]] constexpr bool StaticArray<T, N>::IsEmpty() const noexcept
{
    return mSize == 0;
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::size_type StaticArray<T, N>::GetSize() const noexcept
{
    return mSize;
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::size_type StaticArray<T, N>::GetCapacity() noexcept
{
    return N;
}

template<typename T, size_t N>
[[nodiscard]] constexpr typename StaticArray<T, N>::size_type StaticArray<T, N>::GetMaxSize() noexcept
{
    return N; // For StaticArray, maxSize = capacity = N
}

template<typename T, size_t N>
constexpr void StaticArray<T, N>::Clear() noexcept
{
    DestroyRange(mData, mData + mSize);
    mSize = 0;
}

template<typename T, size_t N>
constexpr bool StaticArray<T, N>::PushBack(const T& value) noexcept
{
    return EmplaceBack(value) != nullptr;
}

template<typename T, size_t N>
constexpr bool StaticArray<T, N>::PushBack(T&& value) noexcept
{
    return EmplaceBack(std::move(value)) != nullptr;
}

template<typename T, size_t N>
template<typename... Args>
constexpr typename StaticArray<T, N>::pointer StaticArray<T, N>::EmplaceBack(Args&&... args) noexcept
{
    if (mSize >= N) {
        return nullptr;
    }
    pointer result = ConstructAt(mData + mSize, std::forward<Args>(args)...);
    if (result) {
        ++mSize;
    }
    return result;
}

template<typename T, size_t N>
constexpr void StaticArray<T, N>::PopBack() noexcept
{
    if (mSize > 0) {
        --mSize;
        DestroyAt(mData + mSize);
    }
}

template<typename T, size_t N>
constexpr bool StaticArray<T, N>::Resize(size_type count) noexcept
{
    if (count > N) {
        return false;
    }

    if (count < mSize) {
        DestroyRange(mData + count, mData + mSize);
        mSize = count;
        return true;
    }
    else if (count > mSize) {
        size_type oldSize = mSize;
        mSize = count; // Optimistic, we go back to the old one if it fails
        for (size_type i = oldSize; i < count; ++i) {
            if (!ConstructAt(mData + i)) {
                // Failure, cleaning and return
                DestroyRange(mData + oldSize, mData + i);
                mSize = oldSize;
                return false;
            }
        }
    }
    return true;
}

template<typename T, size_t N>
constexpr bool StaticArray<T, N>::Resize(size_type count, const T& value) noexcept
{
    if (count > N) {
        return false;
    }

    if (count < mSize) {
        DestroyRange(mData + count, mData + mSize);
        mSize = count;
        return true;
    }
    else if (count > mSize) {
        size_type oldSize = mSize;
        mSize = count;
        for (size_type i = oldSize; i < count; ++i) {
            if (!ConstructAt(mData + i, value)) {
                DestroyRange(mData + oldSize, mData + i);
                mSize = oldSize;
                return false;
            }
        }
    }
    return true;
}

template<typename T, size_t N>
constexpr void StaticArray<T, N>::Swap(StaticArray& other) noexcept
{
    size_type maxSize = mSize > other.mSize ? mSize : other.mSize;

    // Swap existing elements
    size_type minSize = mSize < other.mSize ? mSize : other.mSize;
    for (size_type i = 0; i < minSize; ++i) {
        std::swap(mData[i], other.mData[i]);
    }

    // Move the extra items
    if (mSize > other.mSize) {
        for (size_type i = minSize; i < mSize; ++i) {
            ConstructAt(other.mData + i, std::move(mData[i]));
            DestroyAt(mData + i);
        }
    }
    else if (other.mSize > mSize) {
        for (size_type i = minSize; i < other.mSize; ++i) {
            ConstructAt(mData + i, std::move(other.mData[i]));
            DestroyAt(other.mData + i);
        }
    }

    std::swap(mSize, other.mSize);
}

/* === Private Implementation === */

template<typename T, size_t N>
template<typename... Args>
[[nodiscard]] constexpr typename StaticArray<T, N>::pointer StaticArray<T, N>::ConstructAt(pointer p, Args&&... args) noexcept
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
constexpr void StaticArray<T, N>::DestroyAt(pointer p) noexcept
{
    if constexpr (!std::is_trivially_destructible_v<T>) {
        p->~T();
    }
}

template<typename T, size_t N>
constexpr void StaticArray<T, N>::DestroyRange(pointer first, pointer last) noexcept
{
    if constexpr (!std::is_trivially_destructible_v<T>) {
        for (; first != last; ++first) {
            DestroyAt(first);
        }
    }
}

/* === Non-member functions === */

template<typename T, size_t N>
[[nodiscard]] constexpr bool operator==(const StaticArray<T, N>& lhs, const StaticArray<T, N>& rhs) noexcept
{
    if (lhs.GetSize() != rhs.GetSize()) {
        return false;
    }
    for (typename StaticArray<T, N>::size_type i = 0; i < lhs.GetSize(); ++i) {
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
    auto lhsSize = lhs.GetSize();
    auto rhsSize = rhs.GetSize();
    auto minSize = lhsSize < rhsSize ? lhsSize : rhsSize;

    for (typename StaticArray<T, N>::size_type i = 0; i < minSize; ++i) {
        if (lhs[i] < rhs[i]) return true;
        if (rhs[i] < lhs[i]) return false;
    }
    return lhsSize < rhsSize;
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
    lhs.Swap(rhs);
}

} // namespace util

#endif // NX_UTIL_STATIC_ARRAY_HPP
