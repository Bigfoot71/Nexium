/* FixedArray.hpp -- STL-like fixed array with noexcept operations
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_UTIL_FIXED_ARRAY_HPP
#define NX_UTIL_FIXED_ARRAY_HPP

#include <SDL3/SDL_assert.h>
#include <NX/NX_Memory.h>
#include <type_traits>
#include <algorithm>
#include <iterator>
#include <utility>
#include <cstdlib>
#include <cstddef>

namespace util {

/* === Declaration === */

/**
 * @brief A simple, non-resizable dynamically allocated array, 100% noexcept.
 *
 * Provides a fixed-size array with some adapted behaviors and uses SDL's C allocation
 * functions directly for compatibility with custom C allocators without hassle.
 * The coding style is kept close to the STL for compatibility.
 *
 * @tparam T Type of the elements stored in the array.
 */
template <typename T>
class FixedArray {
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
    FixedArray() noexcept;
    explicit FixedArray(size_type max_capacity) noexcept;
    FixedArray(size_type max_capacity, size_type count) noexcept;
    FixedArray(size_type max_capacity, size_type count, const T& value) noexcept;

    template<typename InputIt>
    FixedArray(size_type max_capacity, InputIt first, InputIt last) noexcept;

    FixedArray(size_type max_capacity, std::initializer_list<T> init) noexcept;

    ~FixedArray() noexcept;

    /** Non-copyable, move-only */
    FixedArray(const FixedArray&) = delete;
    FixedArray& operator=(const FixedArray&) = delete;

    FixedArray(FixedArray&& other) noexcept;
    FixedArray& operator=(FixedArray&& other) noexcept;

    /** Assignment */
    bool Assign(size_type count, const T& value) noexcept;

    template<typename InputIt>
    bool Assign(InputIt first, InputIt last) noexcept;

    bool Assign(std::initializer_list<T> init) noexcept;

    /** Element access */
    [[nodiscard]] pointer GetAt(size_type pos) noexcept;
    [[nodiscard]] const_pointer GetAt(size_type pos) const noexcept;
    [[nodiscard]] reference operator[](size_type index) noexcept;
    [[nodiscard]] const_reference operator[](size_type index) const noexcept;
    [[nodiscard]] pointer GetFront() noexcept;
    [[nodiscard]] const_pointer GetFront() const noexcept;
    [[nodiscard]] pointer GetBack() noexcept;
    [[nodiscard]] const_pointer GetBack() const noexcept;
    [[nodiscard]] pointer GetData() noexcept;
    [[nodiscard]] const_pointer GetData() const noexcept;

    /** Iterators */
    [[nodiscard]] iterator Begin() noexcept;
    [[nodiscard]] const_iterator Begin() const noexcept;
    [[nodiscard]] iterator End() noexcept;
    [[nodiscard]] const_iterator End() const noexcept;
    [[nodiscard]] reverse_iterator ReverseBegin() noexcept;
    [[nodiscard]] const_reverse_iterator ReverseBegin() const noexcept;
    [[nodiscard]] reverse_iterator ReverseEnd() noexcept;
    [[nodiscard]] const_reverse_iterator ReverseEnd() const noexcept;

    /** Capacity */
    [[nodiscard]] bool IsEmpty() const noexcept;
    [[nodiscard]] size_type GetSize() const noexcept;
    [[nodiscard]] size_type GetCapacity() const noexcept;
    [[nodiscard]] size_type GetMaxSize() const noexcept;

    /** Modifiers */
    void Clear() noexcept;
    bool PushBack(const T& value) noexcept;
    bool PushBack(T&& value) noexcept;

    template<typename... Args>
    pointer EmplaceBack(Args&&... args) noexcept;

    void PopBack() noexcept;
    bool Resize(size_type count) noexcept;
    bool Resize(size_type count, const T& value) noexcept;
    void Swap(FixedArray& other) noexcept;

    /** @warning This function recreates the array and discards existing data */
    bool Reset(size_t size) noexcept;

private:
    /** Helpers for construction/destruction */
    template<typename... Args>
    [[nodiscard]] pointer ConstructAt(pointer p, Args&&... args) noexcept;
    void DestroyAt(pointer p) noexcept;
    void DestroyRange(pointer first, pointer last) noexcept;

private:
    T* mData;
    size_type mSize;
    size_type mCapacity;
};

/* === Public Implementation === */

template<typename T>
FixedArray<T>::FixedArray() noexcept
    : mData(nullptr), mSize(0), mCapacity(0)
{ }

template<typename T>
FixedArray<T>::FixedArray(size_type max_capacity) noexcept
    : mData(nullptr), mSize(0), mCapacity(0)
{
    if (max_capacity > 0) {
        mData = NX_Malloc<T>(max_capacity);
        if (mData) {
            mCapacity = max_capacity;
        }
    }
}

template<typename T>
FixedArray<T>::FixedArray(size_type max_capacity, size_type count) noexcept
    : FixedArray(max_capacity)
{
    if (count > 0 && mData) {
        Resize(count);
    }
}

template<typename T>
FixedArray<T>::FixedArray(size_type max_capacity, size_type count, const T& value) noexcept
    : FixedArray(max_capacity)
{
    if (mData) {
        Assign(count, value);
    }
}

template<typename T>
template<typename InputIt>
FixedArray<T>::FixedArray(size_type max_capacity, InputIt first, InputIt last) noexcept
    : FixedArray(max_capacity)
{
    if (mData) {
        Assign(first, last);
    }
}

template<typename T>
FixedArray<T>::FixedArray(size_type max_capacity, std::initializer_list<T> init) noexcept
    : FixedArray(max_capacity, init.Begin(), init.End())
{ }

template<typename T>
FixedArray<T>::~FixedArray() noexcept
{
    Clear();
    NX_Free(mData);
}

template<typename T>
FixedArray<T>::FixedArray(FixedArray&& other) noexcept
    : mData(other.mData), mSize(other.mSize), mCapacity(other.mCapacity)
{
    other.mData = nullptr;
    other.mSize = 0;
    other.mCapacity = 0;
}

template<typename T>
FixedArray<T>& FixedArray<T>::operator=(FixedArray&& other) noexcept
{
    if (this != &other) {
        Clear();
        NX_Free(mData);
        mData = other.mData;
        mSize = other.mSize;
        mCapacity = other.mCapacity;
        other.mData = nullptr;
        other.mSize = 0;
        other.mCapacity = 0;
    }
    return *this;
}

template<typename T>
bool FixedArray<T>::Assign(size_type count, const T& value) noexcept
{
    Clear();
    return Resize(count, value);
}

template<typename T>
template<typename InputIt>
bool FixedArray<T>::Assign(InputIt first, InputIt last) noexcept
{
    Clear();
    for (auto it = first; it != last; ++it) {
        if (!PushBack(*it)) {
            return false;
        }
    }
    return true;
}

template<typename T>
bool FixedArray<T>::Assign(std::initializer_list<T> init) noexcept
{
    return Assign(init.Begin(), init.End());
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::pointer FixedArray<T>::GetAt(size_type pos) noexcept
{
    return (pos < mSize) ? mData + pos : nullptr;
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::const_pointer FixedArray<T>::GetAt(size_type pos) const noexcept
{
    return (pos < mSize) ? mData + pos : nullptr;
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::reference FixedArray<T>::operator[](size_type index) noexcept
{
    return *(mData + index);
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::const_reference FixedArray<T>::operator[](size_type index) const noexcept
{
    return *(mData + index);
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::pointer FixedArray<T>::GetFront() noexcept
{
    return mSize > 0 ? mData : nullptr;
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::const_pointer FixedArray<T>::GetFront() const noexcept
{
    return mSize > 0 ? mData : nullptr;
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::pointer FixedArray<T>::GetBack() noexcept
{
    return mSize > 0 ? mData + mSize - 1 : nullptr;
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::const_pointer FixedArray<T>::GetBack() const noexcept
{
    return mSize > 0 ? mData + mSize - 1 : nullptr;
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::pointer FixedArray<T>::GetData() noexcept
{
    return mData;
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::const_pointer FixedArray<T>::GetData() const noexcept
{
    return mData;
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::iterator FixedArray<T>::Begin() noexcept
{
    return mData;
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::const_iterator FixedArray<T>::Begin() const noexcept
{
    return mData;
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::iterator FixedArray<T>::End() noexcept
{
    return mData + mSize;
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::const_iterator FixedArray<T>::End() const noexcept
{
    return mData + mSize;
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::reverse_iterator FixedArray<T>::ReverseBegin() noexcept
{
    return reverse_iterator(End());
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::const_reverse_iterator FixedArray<T>::ReverseBegin() const noexcept
{
    return const_reverse_iterator(End());
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::reverse_iterator FixedArray<T>::ReverseEnd() noexcept
{
    return reverse_iterator(Begin());
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::const_reverse_iterator FixedArray<T>::ReverseEnd() const noexcept
{
    return const_reverse_iterator(Begin());
}

template<typename T>
[[nodiscard]] bool FixedArray<T>::IsEmpty() const noexcept
{
    return mSize == 0;
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::size_type FixedArray<T>::GetSize() const noexcept
{
    return mSize;
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::size_type FixedArray<T>::GetCapacity() const noexcept
{
    return mCapacity;
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::size_type FixedArray<T>::GetMaxSize() const noexcept
{
    return mCapacity;
}

template<typename T>
void FixedArray<T>::Clear() noexcept
{
    DestroyRange(mData, mData + mSize);
    mSize = 0;
}

template<typename T>
bool FixedArray<T>::PushBack(const T& value) noexcept
{
    return EmplaceBack(value) != nullptr;
}

template<typename T>
bool FixedArray<T>::PushBack(T&& value) noexcept
{
    return EmplaceBack(std::move(value)) != nullptr;
}

template<typename T>
template<typename... Args>
typename FixedArray<T>::pointer FixedArray<T>::EmplaceBack(Args&&... args) noexcept
{
    if (mSize >= mCapacity) {
        return nullptr;
    }
    pointer result = ConstructAt(mData + mSize, std::forward<Args>(args)...);
    if (result) {
        ++mSize;
    }
    return result;
}

template<typename T>
void FixedArray<T>::PopBack() noexcept
{
    if (mSize > 0) {
        --mSize;
        DestroyAt(mData + mSize);
    }
}

template<typename T>
bool FixedArray<T>::Resize(size_type count) noexcept
{
    if (count > mCapacity) {
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

template<typename T>
bool FixedArray<T>::Resize(size_type count, const T& value) noexcept
{
    if (count > mCapacity) {
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

template<typename T>
void FixedArray<T>::Swap(FixedArray& other) noexcept
{
    std::swap(mData, other.mData);
    std::swap(mSize, other.mSize);
    std::swap(mCapacity, other.mCapacity);
}

template<typename T>
bool FixedArray<T>::Reset(size_t size) noexcept
{
    SDL_assert(size > 0);

    if (mData != nullptr) {
        NX_Free(mData);
        mCapacity = 0;
        mSize = 0;
    }

    mData = NX_Malloc<T>(size);
    if (mData == nullptr) {
        return false;
    }

    mCapacity = size;
    return true;
}

/* === Private Implementation === */

template<typename T>
template<typename... Args>
[[nodiscard]] typename FixedArray<T>::pointer FixedArray<T>::ConstructAt(pointer p, Args&&... args) noexcept
{
    if constexpr (std::is_nothrow_constructible_v<T, Args...>) {
        new (p) T(std::forward<Args>(args)...);
        return p;
    }
    else {
        try {
            new (p) T(std::forward<Args>(args)...);
            return p;
        } catch (...) {
            return nullptr;
        }
    }
}

template<typename T>
void FixedArray<T>::DestroyAt(pointer p) noexcept
{
    p->~T();
}

template<typename T>
void FixedArray<T>::DestroyRange(pointer first, pointer last) noexcept
{
    for (; first != last; ++first) {
        DestroyAt(first);
    }
}

/* === Non-member functions === */

template<typename T>
[[nodiscard]] bool operator==(const FixedArray<T>& lhs, const FixedArray<T>& rhs) noexcept
{
    return lhs.GetSize() == rhs.GetSize() && std::equal(lhs.Begin(), lhs.End(), rhs.Begin());
}

template<typename T>
[[nodiscard]] bool operator!=(const FixedArray<T>& lhs, const FixedArray<T>& rhs) noexcept
{
    return !(lhs == rhs);
}

template<typename T>
[[nodiscard]] bool operator<(const FixedArray<T>& lhs, const FixedArray<T>& rhs) noexcept
{
    return std::lexicographical_compare(lhs.Begin(), lhs.End(), rhs.Begin(), rhs.End());
}

template<typename T>
[[nodiscard]] bool operator<=(const FixedArray<T>& lhs, const FixedArray<T>& rhs) noexcept
{
    return !(rhs < lhs);
}

template<typename T>
[[nodiscard]] bool operator>(const FixedArray<T>& lhs, const FixedArray<T>& rhs) noexcept
{
    return rhs < lhs;
}

template<typename T>
[[nodiscard]] bool operator>=(const FixedArray<T>& lhs, const FixedArray<T>& rhs) noexcept
{
    return !(lhs < rhs);
}

template <typename T>
constexpr typename FixedArray<T>::iterator begin(FixedArray<T>& arr) noexcept
{
    return arr.Begin();
}

template <typename T>
constexpr typename FixedArray<T>::const_iterator begin(const FixedArray<T>& arr) noexcept
{
    return arr.Begin();
}

template <typename T>
constexpr typename FixedArray<T>::iterator end(FixedArray<T>& arr) noexcept
{
    return arr.End();
}

template <typename T>
constexpr typename FixedArray<T>::const_iterator end(const FixedArray<T>& arr) noexcept
{
    return arr.End();
}

template <typename T>
constexpr typename FixedArray<T>::reverse_iterator rbegin(FixedArray<T>& arr) noexcept
{
    return arr.ReverseBegin();
}

template <typename T>
constexpr typename FixedArray<T>::const_reverse_iterator rbegin(const FixedArray<T>& arr) noexcept
{
    return arr.ReverseBegin();
}

template <typename T>
constexpr typename FixedArray<T>::reverse_iterator rend(FixedArray<T>& arr) noexcept
{
    return arr.ReverseEnd();
}

template <typename T>
constexpr typename FixedArray<T>::const_reverse_iterator rend(const FixedArray<T>& arr) noexcept
{
    return arr.ReverseEnd();
}

template <typename T>
constexpr typename FixedArray<T>::const_iterator cbegin(const FixedArray<T>& arr) noexcept
{
    return arr.Begin();
}

template <typename T>
constexpr typename FixedArray<T>::const_iterator cend(const FixedArray<T>& arr) noexcept
{
    return arr.End();
}

template <typename T>
constexpr typename FixedArray<T>::const_reverse_iterator crbegin(const FixedArray<T>& arr) noexcept
{
    return arr.ReverseBegin();
}

template <typename T>
constexpr typename FixedArray<T>::const_reverse_iterator crend(const FixedArray<T>& arr) noexcept
{
    return arr.ReverseEnd();
}

template<typename T>
void swap(FixedArray<T>& lhs, FixedArray<T>& rhs) noexcept
{
    lhs.Swap(rhs);
}

} // namespace util

#endif // NX_UTIL_FIXED_ARRAY_HPP
