#ifndef HP_UTIL_DYNAMIC_ARRAY_HPP
#define HP_UTIL_DYNAMIC_ARRAY_HPP

#include <SDL3/SDL_stdinc.h>
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
    [[nodiscard]] bool reserve(size_type cap) noexcept;
    void shrink_to_fit() noexcept;

    /** Modifiers */
    void clear() noexcept;

    /** Insert operations */
    iterator insert(const_iterator pos, const T& value) noexcept;
    iterator insert(const_iterator pos, T&& value) noexcept;
    iterator insert(const_iterator pos, size_type count, const T& value) noexcept;

    template<typename InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last) noexcept;
    iterator insert(const_iterator pos, std::initializer_list<T> init) noexcept;

    template<typename... Args>
    iterator emplace(const_iterator pos, Args&&... args) noexcept;

    /** Erase operations */
    iterator erase(const_iterator pos) noexcept;
    iterator erase(const_iterator first, const_iterator last) noexcept;

    /** Push/pop operations */
    bool push_back(const T& value) noexcept;
    bool push_back(T&& value) noexcept;

    template<typename... Args>
    pointer emplace_back(Args&&... args) noexcept;

    void pop_back() noexcept;
    bool resize(size_type count) noexcept;
    bool resize(size_type count, const T& value) noexcept;
    void swap(DynamicArray& other) noexcept;

private:
    /** Helper to validate iterator */
    [[nodiscard]] bool is_valid_iterator(const_iterator it) const noexcept;
    [[nodiscard]] size_type iterator_to_index(const_iterator it) const noexcept;

    /** Helpers for construction/destruction */
    template<typename... Args>
    [[nodiscard]] pointer construct_at(pointer p, Args&&... args) noexcept;
    void destroy_at(pointer p) noexcept;
    void destroy_range(pointer first, pointer last) noexcept;

    /** Memory management */
    [[nodiscard]] size_type calculate_growth(size_type min_size) const noexcept;
    [[nodiscard]] bool grow() noexcept;
    [[nodiscard]] bool reallocate(size_type new_capacity) noexcept;

    /** Helper for moving elements */
    void move_elements_right(pointer pos, size_type count) noexcept;
    void move_elements_left(pointer pos, size_type count) noexcept;

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
        resize(count);
    }
}

template<typename T>
DynamicArray<T>::DynamicArray(size_type count, const T& value) noexcept
    : DynamicArray()
{
    assign(count, value);
}

template<typename T>
template<typename InputIt>
DynamicArray<T>::DynamicArray(InputIt first, InputIt last) noexcept
    : DynamicArray()
{
    assign(first, last);
}

template<typename T>
DynamicArray<T>::DynamicArray(std::initializer_list<T> init) noexcept
    : DynamicArray(init.begin(), init.end())
{ }

template<typename T>
DynamicArray<T>::~DynamicArray() noexcept
{
    clear();
    SDL_free(mData);
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
bool DynamicArray<T>::assign(size_type count, const T& value) noexcept
{
    clear();
    return resize(count, value);
}

template<typename T>
template<typename InputIt>
bool DynamicArray<T>::assign(InputIt first, InputIt last) noexcept
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
bool DynamicArray<T>::assign(std::initializer_list<T> init) noexcept
{
    return assign(init.begin(), init.end());
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::pointer DynamicArray<T>::at(size_type pos) noexcept
{
    return (pos < mSize) ? mData + pos : nullptr;
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::const_pointer DynamicArray<T>::at(size_type pos) const noexcept
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
[[nodiscard]] typename DynamicArray<T>::pointer DynamicArray<T>::front() noexcept
{
    return mSize > 0 ? mData : nullptr;
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::const_pointer DynamicArray<T>::front() const noexcept
{
    return mSize > 0 ? mData : nullptr;
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::pointer DynamicArray<T>::back() noexcept
{
    return mSize > 0 ? mData + mSize - 1 : nullptr;
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::const_pointer DynamicArray<T>::back() const noexcept
{
    return mSize > 0 ? mData + mSize - 1 : nullptr;
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::pointer DynamicArray<T>::data() noexcept
{
    return mData;
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::const_pointer DynamicArray<T>::data() const noexcept
{
    return mData;
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::iterator DynamicArray<T>::begin() noexcept
{
    return mData;
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::const_iterator DynamicArray<T>::begin() const noexcept
{
    return mData;
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::const_iterator DynamicArray<T>::cbegin() const noexcept
{
    return mData;
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::iterator DynamicArray<T>::end() noexcept
{
    return mData + mSize;
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::const_iterator DynamicArray<T>::end() const noexcept
{
    return mData + mSize;
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::const_iterator DynamicArray<T>::cend() const noexcept
{
    return mData + mSize;
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::reverse_iterator DynamicArray<T>::rbegin() noexcept
{
    return reverse_iterator(end());
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::const_reverse_iterator DynamicArray<T>::rbegin() const noexcept
{
    return const_reverse_iterator(end());
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::const_reverse_iterator DynamicArray<T>::crbegin() const noexcept
{
    return const_reverse_iterator(end());
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::reverse_iterator DynamicArray<T>::rend() noexcept
{
    return reverse_iterator(begin());
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::const_reverse_iterator DynamicArray<T>::rend() const noexcept
{
    return const_reverse_iterator(begin());
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::const_reverse_iterator DynamicArray<T>::crend() const noexcept
{
    return const_reverse_iterator(begin());
}

template<typename T>
[[nodiscard]] bool DynamicArray<T>::empty() const noexcept
{
    return mSize == 0;
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::size_type DynamicArray<T>::size() const noexcept
{
    return mSize;
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::size_type DynamicArray<T>::capacity() const noexcept
{
    return mCapacity;
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::size_type DynamicArray<T>::max_size() const noexcept
{
    return std::numeric_limits<size_type>::max() / sizeof(T);
}

template<typename T>
[[nodiscard]] bool DynamicArray<T>::reserve(size_type cap) noexcept
{
    return (cap <= mCapacity) ? true : reallocate(cap);
}

template<typename T>
void DynamicArray<T>::shrink_to_fit() noexcept
{
    if (mSize < mCapacity) {
        if (mSize == 0) {
            SDL_free(mData);
            mData = nullptr;
            mCapacity = 0;
        }
        else {
            reallocate(mSize); // Ignore les échecs ici
        }
    }
}

template<typename T>
void DynamicArray<T>::clear() noexcept
{
    destroy_range(mData, mData + mSize);
    mSize = 0;
}

// Insert operations
template<typename T>
typename DynamicArray<T>::iterator DynamicArray<T>::insert(const_iterator pos, const T& value) noexcept
{
    return emplace(pos, value);
}

template<typename T>
typename DynamicArray<T>::iterator DynamicArray<T>::insert(const_iterator pos, T&& value) noexcept
{
    return emplace(pos, std::move(value));
}

template<typename T>
typename DynamicArray<T>::iterator DynamicArray<T>::insert(const_iterator pos, size_type count, const T& value) noexcept
{
    if (!is_valid_iterator(pos) || count == 0) {
        return const_cast<iterator>(pos);
    }

    size_type index = iterator_to_index(pos);
    size_type required_capacity = mSize + count;

    if (required_capacity > mCapacity && !reallocate(calculate_growth(required_capacity))) {
        return end(); // Failure
    }

    iterator insert_pos = mData + index;

    // Move items to the right
    if (index < mSize) {
        move_elements_right(insert_pos, count);
    }

    // Insert new elements
    for (size_type i = 0; i < count; ++i) {
        if (!construct_at(insert_pos + i, value)) {
            // Failure, cleaning up what was built
            for (size_type j = 0; j < i; ++j) {
                destroy_at(insert_pos + j);
            }
            // Put the moved items back in place
            if (index < mSize) {
                move_elements_left(insert_pos, count);
            }
            return end();
        }
    }

    mSize += count;
    return insert_pos;
}

template<typename T>
template<typename InputIt>
typename DynamicArray<T>::iterator DynamicArray<T>::insert(const_iterator pos, InputIt first, InputIt last) noexcept
{
    if (!is_valid_iterator(pos) || first == last) {
        return const_cast<iterator>(pos);
    }

    // For non-random-access iterators, one must insert one by one
    size_type index = iterator_to_index(pos);
    size_type inserted = 0;

    for (auto it = first; it != last; ++it, ++inserted) {
        iterator insert_result = emplace(begin() + index + inserted, *it);
        if (insert_result == end()) {
            return end(); // Failure
        }
    }

    return mData + index;
}

template<typename T>
typename DynamicArray<T>::iterator DynamicArray<T>::insert(const_iterator pos, std::initializer_list<T> init) noexcept
{
    return insert(pos, init.begin(), init.end());
}

template<typename T>
template<typename... Args>
typename DynamicArray<T>::iterator DynamicArray<T>::emplace(const_iterator pos, Args&&... args) noexcept
{
    if (!is_valid_iterator(pos)) {
        return end();
    }

    size_type index = iterator_to_index(pos);

    if (mSize >= mCapacity && !grow()) {
        return end(); // Failure
    }

    iterator insert_pos = mData + index;

    // Move items to the right if necessary
    if (index < mSize) {
        move_elements_right(insert_pos, 1);
    }

    // Build the new element
    if (!construct_at(insert_pos, std::forward<Args>(args)...)) {
        // Failure, put the elements back in their place
        if (index < mSize) {
            move_elements_left(insert_pos, 1);
        }
        return end();
    }

    ++mSize;
    return insert_pos;
}

// Erase operations
template<typename T>
typename DynamicArray<T>::iterator DynamicArray<T>::erase(const_iterator pos) noexcept
{
    if (!is_valid_iterator(pos) || pos == end()) {
        return end();
    }

    size_type index = iterator_to_index(pos);
    iterator erase_pos = mData + index;

    // Destroy the element
    destroy_at(erase_pos);

    // Move items to the left
    if (index < mSize - 1) {
        move_elements_left(erase_pos + 1, 1);
    }

    --mSize;
    return erase_pos;
}

template<typename T>
typename DynamicArray<T>::iterator DynamicArray<T>::erase(const_iterator first, const_iterator last) noexcept
{
    if (!is_valid_iterator(first) || !is_valid_iterator(last) || first >= last) {
        return const_cast<iterator>(first);
    }

    size_type first_index = iterator_to_index(first);
    size_type last_index = iterator_to_index(last);
    size_type count = last_index - first_index;

    iterator erase_first = mData + first_index;
    iterator erase_last = mData + last_index;

    // Destroy elements in range
    destroy_range(erase_first, erase_last);

    // Move the remaining items to the left
    if (last_index < mSize) {
        move_elements_left(erase_last, count);
    }

    mSize -= count;
    return erase_first;
}

template<typename T>
bool DynamicArray<T>::push_back(const T& value) noexcept
{
    return emplace_back(value) != nullptr;
}

template<typename T>
bool DynamicArray<T>::push_back(T&& value) noexcept
{
    return emplace_back(std::move(value)) != nullptr;
}

template<typename T>
template<typename... Args>
typename DynamicArray<T>::pointer DynamicArray<T>::emplace_back(Args&&... args) noexcept
{
    if (mSize >= mCapacity && !grow()) {
        return nullptr;
    }
    pointer result = construct_at(mData + mSize, std::forward<Args>(args)...);
    if (result) {
        ++mSize;
    }
    return result;
}

template<typename T>
void DynamicArray<T>::pop_back() noexcept
{
    if (mSize > 0) {
        --mSize;
        destroy_at(mData + mSize);
    }
}

template<typename T>
bool DynamicArray<T>::resize(size_type count) noexcept
{
    if (count < mSize) {
        destroy_range(mData + count, mData + mSize);
        mSize = count;
        return true;
    }
    else if (count > mSize) {
        if (count > mCapacity && !reallocate(count)) {
            return false;
        }
        size_type old_size = mSize;
        mSize = count; // Optimistic, we go back to the old one if it fails
        for (size_type i = old_size; i < count; ++i) {
            if (!construct_at(mData + i)) {
                // Fail, clean up and go back
                destroy_range(mData + old_size, mData + i);
                mSize = old_size;
                return false;
            }
        }
    }
    return true;
}

template<typename T>
bool DynamicArray<T>::resize(size_type count, const T& value) noexcept
{
    if (count < mSize) {
        destroy_range(mData + count, mData + mSize);
        mSize = count;
        return true;
    }
    else if (count > mSize) {
        if (count > mCapacity && !reallocate(count)) {
            return false;
        }
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
void DynamicArray<T>::swap(DynamicArray& other) noexcept
{
    std::swap(mData, other.mData);
    std::swap(mSize, other.mSize);
    std::swap(mCapacity, other.mCapacity);
}

/* === Private Implementation === */

template<typename T>
[[nodiscard]] bool DynamicArray<T>::is_valid_iterator(const_iterator it) const noexcept
{
    return it >= begin() && it <= end();
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::size_type DynamicArray<T>::iterator_to_index(const_iterator it) const noexcept
{
    return static_cast<size_type>(it - begin());
}

template<typename T>
template<typename... Args>
[[nodiscard]] typename DynamicArray<T>::pointer DynamicArray<T>::construct_at(pointer p, Args&&... args) noexcept
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
void DynamicArray<T>::destroy_at(pointer p) noexcept
{
    p->~T();
}

template<typename T>
void DynamicArray<T>::destroy_range(pointer first, pointer last) noexcept
{
    for (; first != last; ++first) {
        destroy_at(first);
    }
}

template<typename T>
[[nodiscard]] typename DynamicArray<T>::size_type DynamicArray<T>::calculate_growth(size_type min_size) const noexcept
{
    const size_type max_sz = max_size();

    // Classic geometric growth: 1.5x
    if (mCapacity > max_sz - mCapacity / 2) {
        return max_sz;
    }

    const size_type geometric = mCapacity + mCapacity / 2;
    return geometric > min_size ? geometric : min_size;
}

template<typename T>
[[nodiscard]] bool DynamicArray<T>::grow() noexcept
{
    size_type new_cap = mCapacity == 0 ? 1 : calculate_growth(mSize + 1);
    return reallocate(new_cap);
}

template<typename T>
[[nodiscard]] bool DynamicArray<T>::reallocate(size_type new_capacity) noexcept
{
    if (new_capacity == 0) {
        SDL_free(mData);
        mData = nullptr;
        mCapacity = 0;
        return true;
    }

    if (new_capacity > max_size()) {
        return false;
    }

    T* new_data = static_cast<T*>(SDL_malloc(new_capacity * sizeof(T)));
    if (!new_data) {
        return false;
    }

    // Move
    if constexpr (std::is_nothrow_move_constructible_v<T>) {
        for (size_type i = 0; i < mSize; ++i) {
            new (new_data + i) T(std::move(mData[i]));
            destroy_at(mData + i);
        }
    }
    else {
        size_type moved = 0;
        for (size_type i = 0; i < mSize; ++i) {
            if (!construct_at(new_data + i, std::move(mData[i]))) {
                // Fail, clean up and abandon
                destroy_range(new_data, new_data + moved);
                SDL_free(new_data);
                return false;
            }
            destroy_at(mData + i);
            ++moved;
        }
    }

    SDL_free(mData);
    mData = new_data;
    mCapacity = new_capacity;
    return true;
}

template<typename T>
void DynamicArray<T>::move_elements_right(pointer pos, size_type count) noexcept
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
void DynamicArray<T>::move_elements_left(pointer pos, size_type count) noexcept
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
    return lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template<typename T>
[[nodiscard]] bool operator!=(const DynamicArray<T>& lhs, const DynamicArray<T>& rhs) noexcept
{
    return !(lhs == rhs);
}

template<typename T>
[[nodiscard]] bool operator<(const DynamicArray<T>& lhs, const DynamicArray<T>& rhs) noexcept
{
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
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
    lhs.swap(rhs);
}

} // namespace util

#endif // HP_UTIL_DYNAMIC_ARRAY_HPP
