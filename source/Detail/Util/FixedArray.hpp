#ifndef HP_UTIL_FIXED_ARRAY_HPP
#define HP_UTIL_FIXED_ARRAY_HPP

#include <SDL3/SDL_stdinc.h>
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
    bool assign(size_type count, const T& value) noexcept;

    template<typename InputIt>
    bool assign(InputIt first, InputIt last) noexcept;

    bool assign(std::initializer_list<T> init) noexcept;

    /** Element access */
    [[nodiscard]] pointer at(size_type pos) noexcept;
    [[nodiscard]] const_pointer at(size_type pos) const noexcept;
    [[nodiscard]] reference operator[](size_type index) noexcept;
    [[nodiscard]] const_reference operator[](size_type index) const noexcept;
    [[nodiscard]] pointer front() noexcept;
    [[nodiscard]] const_pointer front() const noexcept;
    [[nodiscard]] pointer back() noexcept;
    [[nodiscard]] const_pointer back() const noexcept;
    [[nodiscard]] pointer data() noexcept;
    [[nodiscard]] const_pointer data() const noexcept;

    /** Iterators */
    [[nodiscard]] iterator begin() noexcept;
    [[nodiscard]] const_iterator begin() const noexcept;
    [[nodiscard]] const_iterator cbegin() const noexcept;
    [[nodiscard]] iterator end() noexcept;
    [[nodiscard]] const_iterator end() const noexcept;
    [[nodiscard]] const_iterator cend() const noexcept;
    [[nodiscard]] reverse_iterator rbegin() noexcept;
    [[nodiscard]] const_reverse_iterator rbegin() const noexcept;
    [[nodiscard]] const_reverse_iterator crbegin() const noexcept;
    [[nodiscard]] reverse_iterator rend() noexcept;
    [[nodiscard]] const_reverse_iterator rend() const noexcept;
    [[nodiscard]] const_reverse_iterator crend() const noexcept;

    /** Capacity */
    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] size_type size() const noexcept;
    [[nodiscard]] size_type capacity() const noexcept;
    [[nodiscard]] size_type max_size() const noexcept;

    /** Modifiers */
    void clear() noexcept;
    bool push_back(const T& value) noexcept;
    bool push_back(T&& value) noexcept;

    template<typename... Args>
    pointer emplace_back(Args&&... args) noexcept;

    void pop_back() noexcept;
    bool resize(size_type count) noexcept;
    bool resize(size_type count, const T& value) noexcept;
    void swap(FixedArray& other) noexcept;

private:
    /** Helpers for construction/destruction */
    template<typename... Args>
    [[nodiscard]] pointer construct_at(pointer p, Args&&... args) noexcept;
    void destroy_at(pointer p) noexcept;
    void destroy_range(pointer first, pointer last) noexcept;

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
        mData = static_cast<T*>(SDL_malloc(max_capacity * sizeof(T)));
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
        resize(count);
    }
}

template<typename T>
FixedArray<T>::FixedArray(size_type max_capacity, size_type count, const T& value) noexcept
    : FixedArray(max_capacity)
{
    if (mData) {
        assign(count, value);
    }
}

template<typename T>
template<typename InputIt>
FixedArray<T>::FixedArray(size_type max_capacity, InputIt first, InputIt last) noexcept
    : FixedArray(max_capacity)
{
    if (mData) {
        assign(first, last);
    }
}

template<typename T>
FixedArray<T>::FixedArray(size_type max_capacity, std::initializer_list<T> init) noexcept
    : FixedArray(max_capacity, init.begin(), init.end())
{ }

template<typename T>
FixedArray<T>::~FixedArray() noexcept
{
    clear();
    SDL_free(mData);
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
        clear();
        SDL_free(mData);
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
bool FixedArray<T>::assign(size_type count, const T& value) noexcept
{
    clear();
    return resize(count, value);
}

template<typename T>
template<typename InputIt>
bool FixedArray<T>::assign(InputIt first, InputIt last) noexcept
{
    clear();
    for (auto it = first; it != last; ++it) {
        if (!push_back(*it)) {
            return false;
        }
    }
    return true;
}

template<typename T>
bool FixedArray<T>::assign(std::initializer_list<T> init) noexcept
{
    return assign(init.begin(), init.end());
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::pointer FixedArray<T>::at(size_type pos) noexcept
{
    return (pos < mSize) ? mData + pos : nullptr;
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::const_pointer FixedArray<T>::at(size_type pos) const noexcept
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
[[nodiscard]] typename FixedArray<T>::pointer FixedArray<T>::front() noexcept
{
    return mSize > 0 ? mData : nullptr;
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::const_pointer FixedArray<T>::front() const noexcept
{
    return mSize > 0 ? mData : nullptr;
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::pointer FixedArray<T>::back() noexcept
{
    return mSize > 0 ? mData + mSize - 1 : nullptr;
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::const_pointer FixedArray<T>::back() const noexcept
{
    return mSize > 0 ? mData + mSize - 1 : nullptr;
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::pointer FixedArray<T>::data() noexcept
{
    return mData;
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::const_pointer FixedArray<T>::data() const noexcept
{
    return mData;
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::iterator FixedArray<T>::begin() noexcept
{
    return mData;
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::const_iterator FixedArray<T>::begin() const noexcept
{
    return mData;
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::const_iterator FixedArray<T>::cbegin() const noexcept
{
    return mData;
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::iterator FixedArray<T>::end() noexcept
{
    return mData + mSize;
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::const_iterator FixedArray<T>::end() const noexcept
{
    return mData + mSize;
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::const_iterator FixedArray<T>::cend() const noexcept
{
    return mData + mSize;
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::reverse_iterator FixedArray<T>::rbegin() noexcept
{
    return reverse_iterator(end());
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::const_reverse_iterator FixedArray<T>::rbegin() const noexcept
{
    return const_reverse_iterator(end());
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::const_reverse_iterator FixedArray<T>::crbegin() const noexcept
{
    return const_reverse_iterator(end());
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::reverse_iterator FixedArray<T>::rend() noexcept
{
    return reverse_iterator(begin());
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::const_reverse_iterator FixedArray<T>::rend() const noexcept
{
    return const_reverse_iterator(begin());
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::const_reverse_iterator FixedArray<T>::crend() const noexcept
{
    return const_reverse_iterator(begin());
}

template<typename T>
[[nodiscard]] bool FixedArray<T>::empty() const noexcept
{
    return mSize == 0;
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::size_type FixedArray<T>::size() const noexcept
{
    return mSize;
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::size_type FixedArray<T>::capacity() const noexcept
{
    return mCapacity;
}

template<typename T>
[[nodiscard]] typename FixedArray<T>::size_type FixedArray<T>::max_size() const noexcept
{
    return mCapacity;
}

template<typename T>
void FixedArray<T>::clear() noexcept
{
    destroy_range(mData, mData + mSize);
    mSize = 0;
}

template<typename T>
bool FixedArray<T>::push_back(const T& value) noexcept
{
    return emplace_back(value) != nullptr;
}

template<typename T>
bool FixedArray<T>::push_back(T&& value) noexcept
{
    return emplace_back(std::move(value)) != nullptr;
}

template<typename T>
template<typename... Args>
typename FixedArray<T>::pointer FixedArray<T>::emplace_back(Args&&... args) noexcept
{
    if (mSize >= mCapacity) {
        return nullptr;
    }
    pointer result = construct_at(mData + mSize, std::forward<Args>(args)...);
    if (result) {
        ++mSize;
    }
    return result;
}

template<typename T>
void FixedArray<T>::pop_back() noexcept
{
    if (mSize > 0) {
        --mSize;
        destroy_at(mData + mSize);
    }
}

template<typename T>
bool FixedArray<T>::resize(size_type count) noexcept
{
    if (count > mCapacity) {
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

template<typename T>
bool FixedArray<T>::resize(size_type count, const T& value) noexcept
{
    if (count > mCapacity) {
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

template<typename T>
void FixedArray<T>::swap(FixedArray& other) noexcept
{
    std::swap(mData, other.mData);
    std::swap(mSize, other.mSize);
    std::swap(mCapacity, other.mCapacity);
}

/* === Private Implementation === */

template<typename T>
template<typename... Args>
[[nodiscard]] typename FixedArray<T>::pointer FixedArray<T>::construct_at(pointer p, Args&&... args) noexcept
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
void FixedArray<T>::destroy_at(pointer p) noexcept
{
    p->~T();
}

template<typename T>
void FixedArray<T>::destroy_range(pointer first, pointer last) noexcept
{
    for (; first != last; ++first) {
        destroy_at(first);
    }
}

/* === Non-member functions === */

template<typename T>
[[nodiscard]] bool operator==(const FixedArray<T>& lhs, const FixedArray<T>& rhs) noexcept
{
    return lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template<typename T>
[[nodiscard]] bool operator!=(const FixedArray<T>& lhs, const FixedArray<T>& rhs) noexcept
{
    return !(lhs == rhs);
}

template<typename T>
[[nodiscard]] bool operator<(const FixedArray<T>& lhs, const FixedArray<T>& rhs) noexcept
{
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
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

template<typename T>
void swap(FixedArray<T>& lhs, FixedArray<T>& rhs) noexcept
{
    lhs.swap(rhs);
}

} // namespace util

#endif // HP_UTIL_FIXED_ARRAY_HPP
