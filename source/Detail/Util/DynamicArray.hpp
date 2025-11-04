/* DynamicArray.hpp -- STL-like dynamic array with noexcept operations
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_UTIL_DYNAMIC_ARRAY_HPP
#define NX_UTIL_DYNAMIC_ARRAY_HPP

#include <NX/NX_Memory.h>
#include <type_traits>
#include <algorithm>
#include <iterator>
#include <utility>
#include <cstdlib>
#include <cstddef>
#include <limits>

namespace util {

/* === Declaration === */

/**
 * @brief Equivalent of std::vector but 100% noexcept.
 *
 * Behaves like a standard vector with some adapted behaviors and uses SDL's C allocation
 * functions directly for compatibility with custom C allocators without hassle.
 * The coding style is kept as close as possible to the STL for compatibility
 * and is kept internally for consistency
 *
 * @tparam T Type of the elements stored in the array.
 */
template <typename T>
class DynamicArray {
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
    DynamicArray() noexcept;
    explicit DynamicArray(size_type count) noexcept;
    DynamicArray(size_type count, const T& value) noexcept;

    template<typename InputIt>
    DynamicArray(InputIt first, InputIt last) noexcept;

    DynamicArray(std::initializer_list<T> init) noexcept;

    ~DynamicArray() noexcept;

    /** Non-copyable, move-only */
    DynamicArray(const DynamicArray&) = delete;
    DynamicArray& operator=(const DynamicArray&) = delete;

    DynamicArray(DynamicArray&& other) noexcept;
    DynamicArray& operator=(DynamicArray&& other) noexcept;

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
    [[nodiscard]] bool Reserve(size_type cap) noexcept;
    void ShrinkToFit() noexcept;

    /** Modifiers */
    void Clear() noexcept;

    /** Insert operations */
    iterator Insert(const_iterator pos, const T& value) noexcept;
    iterator Insert(const_iterator pos, T&& value) noexcept;
    iterator Insert(const_iterator pos, size_type count, const T& value) noexcept;

    template<typename InputIt>
    iterator Insert(const_iterator pos, InputIt first, InputIt last) noexcept;
    iterator Insert(const_iterator pos, std::initializer_list<T> init) noexcept;

    template<typename... Args>
    iterator Emplace(const_iterator pos, Args&&... args) noexcept;

    /** Erase operations */
    iterator Erase(const_iterator pos) noexcept;
    iterator Erase(const_iterator first, const_iterator last) noexcept;

    /** Push/pop operations */
    bool PushBack(const T& value) noexcept;
    bool PushBack(T&& value) noexcept;

    template<typename... Args>
    pointer EmplaceBack(Args&&... args) noexcept;

    void PopBack() noexcept;
    bool Resize(size_type count) noexcept;
    bool Resize(size_type count, const T& value) noexcept;
    void Swap(DynamicArray& other) noexcept;

private:
    /** Helper to validate iterator */
    [[nodiscard]] bool IsValidIterator(const_iterator it) const noexcept;
    [[nodiscard]] size_type IteratorToIndex(const_iterator it) const noexcept;

    /** Helpers for construction/destruction */
    template<typename... Args>
    [[nodiscard]] pointer ConstructAt(pointer p, Args&&... args) noexcept;
    void DestroyAt(pointer p) noexcept;
    void DestroyRange(pointer first, pointer last) noexcept;

    /** Memory management */
    [[nodiscard]] size_type CalculateGrowth(size_type min_size) const noexcept;
    [[nodiscard]] bool Grow() noexcept;
    [[nodiscard]] bool Reallocate(size_type new_capacity) noexcept;

    /** Helper for moving elements */
    void MoveElementsRight(pointer pos, size_type count) noexcept;
    void MoveElementsLeft(pointer pos, size_type count) noexcept;

private:
    T* mData;
    size_type mSize;
    size_type mCapacity;
};

/* === Public Implementation === */

template<typename T>
DynamicArray<T>::DynamicArray() noexcept
    : mData(nullptr), mSize(0), mCapacity(0)
{ }

template<typename T>
DynamicArray<T>::DynamicArray(size_type count) noexcept
    : DynamicArray()
{
    if (count > 0) {
        Resize(count);
    }
}

template<typename T>
DynamicArray<T>::DynamicArray(size_type count, const T& value) noexcept
    : DynamicArray()
{
    Assign(count, value);
}

template<typename T>
template<typename InputIt>
DynamicArray<T>::DynamicArray(InputIt first, InputIt last) noexcept
    : DynamicArray()
{
    Assign(first, last);
}

template<typename T>
DynamicArray<T>::DynamicArray(std::initializer_list<T> init) noexcept
    : DynamicArray(init.Begin(), init.End())
{ }

template<typename T>
DynamicArray<T>::~DynamicArray() noexcept
{
    Clear();
    NX_Free(mData);
}

template<typename T>
DynamicArray<T>::DynamicArray(DynamicArray&& other) noexcept
    : mData(other.mData), mSize(other.mSize), mCapacity(other.mCapacity)
{
    other.mData = nullptr;
    other.mSize = 0;
    other.mCapacity = 0;
}

template<typename T>
DynamicArray<T>& DynamicArray<T>::operator=(DynamicArray&& other) noexcept
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
bool DynamicArray<T>::Assign(size_type count, const T& value) noexcept
{
    Clear();
    return Resize(count, value);
}

template<typename T>
template<typename InputIt>
bool DynamicArray<T>::Assign(InputIt first, InputIt last) noexcept
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
bool DynamicArray<T>::Assign(std::initializer_list<T> init) noexcept
{
    return Assign(init.Begin(), init.End());
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::pointer DynamicArray<T>::GetAt(size_type pos) noexcept
{
    return (pos < mSize) ? mData + pos : nullptr;
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::const_pointer DynamicArray<T>::GetAt(size_type pos) const noexcept
{
    return (pos < mSize) ? mData + pos : nullptr;
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::reference DynamicArray<T>::operator[](size_type index) noexcept
{
    return *(mData + index);
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::const_reference DynamicArray<T>::operator[](size_type index) const noexcept
{
    return *(mData + index);
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::pointer DynamicArray<T>::GetFront() noexcept
{
    return mSize > 0 ? mData : nullptr;
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::const_pointer DynamicArray<T>::GetFront() const noexcept
{
    return mSize > 0 ? mData : nullptr;
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::pointer DynamicArray<T>::GetBack() noexcept
{
    return mSize > 0 ? mData + mSize - 1 : nullptr;
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::const_pointer DynamicArray<T>::GetBack() const noexcept
{
    return mSize > 0 ? mData + mSize - 1 : nullptr;
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::pointer DynamicArray<T>::GetData() noexcept
{
    return mData;
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::const_pointer DynamicArray<T>::GetData() const noexcept
{
    return mData;
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::iterator DynamicArray<T>::Begin() noexcept
{
    return mData;
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::const_iterator DynamicArray<T>::Begin() const noexcept
{
    return mData;
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::iterator DynamicArray<T>::End() noexcept
{
    return mData + mSize;
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::const_iterator DynamicArray<T>::End() const noexcept
{
    return mData + mSize;
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::reverse_iterator DynamicArray<T>::ReverseBegin() noexcept
{
    return reverse_iterator(End());
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::const_reverse_iterator DynamicArray<T>::ReverseBegin() const noexcept
{
    return const_reverse_iterator(End());
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::reverse_iterator DynamicArray<T>::ReverseEnd() noexcept
{
    return reverse_iterator(Begin());
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::const_reverse_iterator DynamicArray<T>::ReverseEnd() const noexcept
{
    return const_reverse_iterator(Begin());
}

template<typename T>
[[nodiscard]] bool DynamicArray<T>::IsEmpty() const noexcept
{
    return mSize == 0;
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::size_type DynamicArray<T>::GetSize() const noexcept
{
    return mSize;
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::size_type DynamicArray<T>::GetCapacity() const noexcept
{
    return mCapacity;
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::size_type DynamicArray<T>::GetMaxSize() const noexcept
{
    return std::numeric_limits<size_type>::max() / sizeof(T);
}

template<typename T>
[[nodiscard]] bool DynamicArray<T>::Reserve(size_type cap) noexcept
{
    return (cap <= mCapacity) ? true : Reallocate(cap);
}

template<typename T>
void DynamicArray<T>::ShrinkToFit() noexcept
{
    if (mSize < mCapacity) {
        if (mSize == 0) {
            NX_Free(mData);
            mData = nullptr;
            mCapacity = 0;
        }
        else {
            Reallocate(mSize); // Ignore failures here
        }
    }
}

template<typename T>
void DynamicArray<T>::Clear() noexcept
{
    DestroyRange(mData, mData + mSize);
    mSize = 0;
}

// Insert operations
template<typename T>
typename DynamicArray<T>::iterator DynamicArray<T>::Insert(const_iterator pos, const T& value) noexcept
{
    return Emplace(pos, value);
}

template<typename T>
typename DynamicArray<T>::iterator DynamicArray<T>::Insert(const_iterator pos, T&& value) noexcept
{
    return Emplace(pos, std::move(value));
}

template<typename T>
typename DynamicArray<T>::iterator DynamicArray<T>::Insert(const_iterator pos, size_type count, const T& value) noexcept
{
    if (!IsValidIterator(pos) || count == 0) {
        return const_cast<iterator>(pos);
    }

    size_type index = IteratorToIndex(pos);
    size_type requiredCapacity = mSize + count;

    if (requiredCapacity > mCapacity && !Reallocate(CalculateGrowth(requiredCapacity))) {
        return End(); // Failure
    }

    iterator insertPos = mData + index;

    // Move items to the right
    if (index < mSize) {
        MoveElementsRight(insertPos, count);
    }

    // Insert new elements
    for (size_type i = 0; i < count; ++i) {
        if (!ConstructAt(insertPos + i, value)) {
            // Failure, cleaning up what was built
            for (size_type j = 0; j < i; ++j) {
                DestroyAt(insertPos + j);
            }
            // Put the moved items back in place
            if (index < mSize) {
                MoveElementsLeft(insertPos, count);
            }
            return End();
        }
    }

    mSize += count;
    return insertPos;
}

template<typename T>
template<typename InputIt>
typename DynamicArray<T>::iterator DynamicArray<T>::Insert(const_iterator pos, InputIt first, InputIt last) noexcept
{
    if (!IsValidIterator(pos) || first == last) {
        return const_cast<iterator>(pos);
    }

    // For non-random-access iterators, one must insert one by one
    size_type index = IteratorToIndex(pos);
    size_type inserted = 0;

    for (auto it = first; it != last; ++it, ++inserted) {
        iterator insert_result = Emplace(Begin() + index + inserted, *it);
        if (insert_result == End()) {
            return End(); // Failure
        }
    }

    return mData + index;
}

template<typename T>
typename DynamicArray<T>::iterator DynamicArray<T>::Insert(const_iterator pos, std::initializer_list<T> init) noexcept
{
    return Insert(pos, init.Begin(), init.End());
}

template<typename T>
template<typename... Args>
typename DynamicArray<T>::iterator DynamicArray<T>::Emplace(const_iterator pos, Args&&... args) noexcept
{
    if (!IsValidIterator(pos)) {
        return End();
    }

    size_type index = IteratorToIndex(pos);

    if (mSize >= mCapacity && !Grow()) {
        return End(); // Failure
    }

    iterator insertPos = mData + index;

    // Move items to the right if necessary
    if (index < mSize) {
        MoveElementsRight(insertPos, 1);
    }

    // Build the new element
    if (!ConstructAt(insertPos, std::forward<Args>(args)...)) {
        // Failure, put the elements back in their place
        if (index < mSize) {
            MoveElementsLeft(insertPos, 1);
        }
        return End();
    }

    ++mSize;
    return insertPos;
}

// Erase operations
template<typename T>
typename DynamicArray<T>::iterator DynamicArray<T>::Erase(const_iterator pos) noexcept
{
    if (!IsValidIterator(pos) || pos == End()) {
        return End();
    }

    size_type index = IteratorToIndex(pos);
    iterator erase_pos = mData + index;

    // Destroy the element
    DestroyAt(erase_pos);

    // Move items to the left
    if (index < mSize - 1) {
        MoveElementsLeft(erase_pos + 1, 1);
    }

    --mSize;
    return erase_pos;
}

template<typename T>
typename DynamicArray<T>::iterator DynamicArray<T>::Erase(const_iterator first, const_iterator last) noexcept
{
    if (!IsValidIterator(first) || !IsValidIterator(last) || first >= last) {
        return const_cast<iterator>(first);
    }

    size_type first_index = IteratorToIndex(first);
    size_type last_index = IteratorToIndex(last);
    size_type count = last_index - first_index;

    iterator erase_first = mData + first_index;
    iterator erase_last = mData + last_index;

    // Destroy elements in range
    DestroyRange(erase_first, erase_last);

    // Move the remaining items to the left
    if (last_index < mSize) {
        MoveElementsLeft(erase_last, count);
    }

    mSize -= count;
    return erase_first;
}

template<typename T>
bool DynamicArray<T>::PushBack(const T& value) noexcept
{
    return EmplaceBack(value) != nullptr;
}

template<typename T>
bool DynamicArray<T>::PushBack(T&& value) noexcept
{
    return EmplaceBack(std::move(value)) != nullptr;
}

template<typename T>
template<typename... Args>
typename DynamicArray<T>::pointer DynamicArray<T>::EmplaceBack(Args&&... args) noexcept
{
    if (mSize >= mCapacity && !Grow()) {
        return nullptr;
    }
    pointer result = ConstructAt(mData + mSize, std::forward<Args>(args)...);
    if (result) {
        ++mSize;
    }
    return result;
}

template<typename T>
void DynamicArray<T>::PopBack() noexcept
{
    if (mSize > 0) {
        --mSize;
        DestroyAt(mData + mSize);
    }
}

template<typename T>
bool DynamicArray<T>::Resize(size_type count) noexcept
{
    if (count < mSize) {
        DestroyRange(mData + count, mData + mSize);
        mSize = count;
        return true;
    }
    else if (count > mSize) {
        if (count > mCapacity && !Reallocate(count)) {
            return false;
        }
        size_type oldSize = mSize;
        mSize = count; // Optimistic, we go back to the old one if it fails
        for (size_type i = oldSize; i < count; ++i) {
            if (!ConstructAt(mData + i)) {
                // Fail, clean up and go back
                DestroyRange(mData + oldSize, mData + i);
                mSize = oldSize;
                return false;
            }
        }
    }
    return true;
}

template<typename T>
bool DynamicArray<T>::Resize(size_type count, const T& value) noexcept
{
    if (count < mSize) {
        DestroyRange(mData + count, mData + mSize);
        mSize = count;
        return true;
    }
    else if (count > mSize) {
        if (count > mCapacity && !Reallocate(count)) {
            return false;
        }
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
void DynamicArray<T>::Swap(DynamicArray& other) noexcept
{
    std::swap(mData, other.mData);
    std::swap(mSize, other.mSize);
    std::swap(mCapacity, other.mCapacity);
}

/* === Private Implementation === */

template<typename T>
[[nodiscard]] bool DynamicArray<T>::IsValidIterator(const_iterator it) const noexcept
{
    return it >= Begin() && it <= End();
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::size_type DynamicArray<T>::IteratorToIndex(const_iterator it) const noexcept
{
    return static_cast<size_type>(it - Begin());
}

template<typename T>
template<typename... Args>
[[nodiscard]] typename DynamicArray<T>::pointer DynamicArray<T>::ConstructAt(pointer p, Args&&... args) noexcept
{
    if constexpr (std::is_nothrow_constructible_v<T, Args...>) {
        new (p) T(std::forward<Args>(args)...);
        return p;
    }
    else {
        try {
            new (p) T(std::forward<Args>(args)...);
            return p;
        }
        catch (...) {
            return nullptr;
        }
    }
}

template<typename T>
void DynamicArray<T>::DestroyAt(pointer p) noexcept
{
    p->~T();
}

template<typename T>
void DynamicArray<T>::DestroyRange(pointer first, pointer last) noexcept
{
    for (; first != last; ++first) {
        DestroyAt(first);
    }
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::size_type DynamicArray<T>::CalculateGrowth(size_type min_size) const noexcept
{
    const size_type maxSz = GetMaxSize();

    // Classic geometric growth: 1.5x
    if (mCapacity > maxSz - mCapacity / 2) {
        return maxSz;
    }

    const size_type geometric = mCapacity + mCapacity / 2;
    return geometric > min_size ? geometric : min_size;
}

template<typename T>
[[nodiscard]] bool DynamicArray<T>::Grow() noexcept
{
    size_type new_cap = mCapacity == 0 ? 1 : CalculateGrowth(mSize + 1);
    return Reallocate(new_cap);
}

template<typename T>
[[nodiscard]] bool DynamicArray<T>::Reallocate(size_type new_capacity) noexcept
{
    if (new_capacity == 0) {
        NX_Free(mData);
        mData = nullptr;
        mCapacity = 0;
        return true;
    }

    if (new_capacity > GetMaxSize()) {
        return false;
    }

    T* new_data = NX_Malloc<T>(new_capacity);
    if (!new_data) {
        return false;
    }

    // Move
    if constexpr (std::is_nothrow_move_constructible_v<T>) {
        for (size_type i = 0; i < mSize; ++i) {
            new (new_data + i) T(std::move(mData[i]));
            DestroyAt(mData + i);
        }
    }
    else {
        size_type moved = 0;
        for (size_type i = 0; i < mSize; ++i) {
            if (!ConstructAt(new_data + i, std::move(mData[i]))) {
                // Fail, clean up and abandon
                DestroyRange(new_data, new_data + moved);
                NX_Free(new_data);
                return false;
            }
            DestroyAt(mData + i);
            ++moved;
        }
    }

    NX_Free(mData);
    mData = new_data;
    mCapacity = new_capacity;
    return true;
}

template<typename T>
void DynamicArray<T>::MoveElementsRight(pointer pos, size_type count) noexcept
{
    if (pos == mData + mSize) return;

    if constexpr (std::is_trivially_copyable_v<T>) {
        SDL_memmove(pos + count, pos, (mData + mSize - pos) * sizeof(T));
    }
    else {
        pointer src = mData + mSize - 1;
        pointer dest = src + count;
        while (src >= pos) {
            new (dest) T(std::move(*src));
            src->~T();
            --src;
            --dest;
        }
    }
}

template<typename T>
void DynamicArray<T>::MoveElementsLeft(pointer pos, size_type count) noexcept
{
    if (pos == mData + mSize) return;

    if constexpr (std::is_trivially_copyable_v<T>) {
        SDL_memmove(pos - count, pos, (mData + mSize - pos) * sizeof(T));
    }
    else {
        pointer src = pos;
        pointer dest = pos - count;
        pointer end_src = mData + mSize;
        while (src != end_src) {
            new (dest) T(std::move(*src));
            src->~T();
            ++src;
            ++dest;
        }
    }
}

/* === Non-member functions === */

template<typename T>
[[nodiscard]] bool operator==(const DynamicArray<T>& lhs, const DynamicArray<T>& rhs) noexcept
{
    return lhs.GetSize() == rhs.GetSize() && std::equal(lhs.Begin(), lhs.End(), rhs.Begin());
}

template<typename T>
[[nodiscard]] bool operator!=(const DynamicArray<T>& lhs, const DynamicArray<T>& rhs) noexcept
{
    return !(lhs == rhs);
}

template<typename T>
[[nodiscard]] bool operator<(const DynamicArray<T>& lhs, const DynamicArray<T>& rhs) noexcept
{
    return std::lexicographical_compare(lhs.Begin(), lhs.End(), rhs.Begin(), rhs.End());
}

template<typename T>
[[nodiscard]] bool operator<=(const DynamicArray<T>& lhs, const DynamicArray<T>& rhs) noexcept
{
    return !(rhs < lhs);
}

template<typename T>
[[nodiscard]] bool operator>(const DynamicArray<T>& lhs, const DynamicArray<T>& rhs) noexcept
{
    return rhs < lhs;
}

template<typename T>
[[nodiscard]] bool operator>=(const DynamicArray<T>& lhs, const DynamicArray<T>& rhs) noexcept
{
    return !(lhs < rhs);
}

template<typename T>
void swap(DynamicArray<T>& lhs, DynamicArray<T>& rhs) noexcept
{
    lhs.Swap(rhs);
}

} // namespace util

#endif // NX_UTIL_DYNAMIC_ARRAY_HPP
