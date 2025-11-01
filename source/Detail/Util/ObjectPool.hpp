/* ObjectPool.hpp -- Pool-based container with stable object pointers
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_UTIL_OBJECT_POOL_HPP
#define NX_UTIL_OBJECT_POOL_HPP

#include "Memory.hpp"
#include <type_traits>
#include <cstddef>
#include <utility>

namespace util {

/* === Declaration === */

/**
 * @brief Pool of objects of type T.
 *
 * This class provides a pool-based object allocation mechanism that allows:
 * - keeping valid pointers to objects even if the total container capacity increases
 *   (each pool remains fixed once allocated),
 * - storing objects contiguously at least within a single pool,
 * - creating and destroying objects dynamically without moving already allocated ones,
 * - iterating over objects using iterators and reverse iterators.
 *
 * @tparam T Type of object stored.
 * @tparam PoolSize Number of objects per pool.
 *
 * @note When creating an object, the class searches for the first pool with a free slot.
 * If all existing pools are full, a new pool is allocated.
 *
 * @note Pointers returned by create() remain valid as long as the object is not destroyed
 * using destroy() or clear(), even if new pools are allocated.
 */
template<typename T, std::size_t PoolSize>
class ObjectPool {
public:
    /** Forward declarations */
    class Iterator;
    class ReverseIterator;

    /** Constructors/Destructors */
    ObjectPool() noexcept;
    ~ObjectPool() noexcept = default;

    /** Move operator (only) */
    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;
    ObjectPool(ObjectPool&& other) noexcept = default;
    ObjectPool& operator=(ObjectPool&& other) noexcept;

    /** Object management */
    template<typename... Args>
    T* create(Args&&... args) noexcept;
    bool destroy(T* ptr) noexcept;
    void clear() noexcept;

    /** Accessors */
    std::size_t size() const noexcept;
    std::size_t poolCount() const noexcept;
    bool empty() const noexcept;

    /** Iterators */
    Iterator begin() noexcept;
    Iterator end() noexcept;
    ReverseIterator rbegin() noexcept;
    ReverseIterator rend() noexcept;

private:
    struct Slot {
        alignas(T) char mStorage[sizeof(T)];
        bool mOccupied = false;
        std::size_t mNextFree = 0; // Index of the next free slot in this pool
    };

    struct Pool {
        Slot mSlots[PoolSize];
        std::size_t mFirstFree = 0; // Index of the first free slot
        std::size_t mFreeCount = PoolSize;
        UniquePtr<Pool> mNext = nullptr;
    };

    struct PoolLocation {
        Pool* pool = nullptr;
        std::size_t index = 0;
        bool found = false;
    };

    UniquePtr<Pool> mFirstPool = nullptr;
    Pool* mLastPool = nullptr;
    std::size_t mTotalCount = 0;
    std::size_t mPoolCount = 0;

    // Private methods
    Pool* allocateNewPool() noexcept;
    PoolLocation findObjectLocation(T* ptr) const noexcept;
    void destroyObjectNoexcept(T* obj) noexcept;
};

/* === Iterator Declaration === */

template<typename T, std::size_t PoolSize>
class ObjectPool<T, PoolSize>::Iterator
{
public:
    Iterator() noexcept = default;
    Iterator(typename ObjectPool<T, PoolSize>::Pool* pool, std::size_t index) noexcept;

    T& operator*() const noexcept;
    T* operator->() const noexcept;
    Iterator& operator++() noexcept;
    Iterator operator++(int) noexcept;
    bool operator==(const Iterator& other) const noexcept;
    bool operator!=(const Iterator& other) const noexcept;

private:
    typename ObjectPool<T, PoolSize>::Pool* mCurrentPool = nullptr;
    std::size_t mCurrentIndex = 0;

    void findNext() noexcept;
};

/* === ReverseIterator Declaration === */

template<typename T, std::size_t PoolSize>
class ObjectPool<T, PoolSize>::ReverseIterator
{
public:
    ReverseIterator() noexcept = default;
    ReverseIterator(typename ObjectPool<T, PoolSize>::Pool* lastPool,
                    typename ObjectPool<T, PoolSize>::Pool* firstPool) noexcept;

    T& operator*() const noexcept;
    T* operator->() const noexcept;
    ReverseIterator& operator++() noexcept;
    bool operator==(const ReverseIterator& other) const noexcept;
    bool operator!=(const ReverseIterator& other) const noexcept;

private:
    typename ObjectPool<T, PoolSize>::Pool* mCurrentPool = nullptr;
    typename ObjectPool<T, PoolSize>::Pool* mFirstPool = nullptr;
    std::size_t mCurrentIndex = PoolSize;

    void findPrevious() noexcept;
};

/* === Public Implementation === */

template<typename T, std::size_t PoolSize>
ObjectPool<T, PoolSize>::ObjectPool() noexcept = default;

template<typename T, std::size_t PoolSize>
ObjectPool<T, PoolSize>& ObjectPool<T, PoolSize>::operator=(ObjectPool&& other) noexcept
{
    if (this != &other) {
        clear();
        mFirstPool = std::move(other.mFirstPool);
        mLastPool = std::exchange(other.mLastPool, nullptr);
        mTotalCount = std::exchange(other.mTotalCount, 0);
        mPoolCount = std::exchange(other.mPoolCount, 0);
    }
    return *this;
}

template<typename T, std::size_t PoolSize>
template<typename... Args>
T* ObjectPool<T, PoolSize>::create(Args&&... args) noexcept
{
    // Find a pool with space
    Pool* targetPool = nullptr;
    for (Pool* pool = mFirstPool.get(); pool; pool = pool->mNext.get()) {
        if (pool->mFreeCount > 0) {
            targetPool = pool;
            break;
        }
    }

    // If no pool available, create a new one
    if (!targetPool) {
        targetPool = allocateNewPool();
        if (!targetPool) {
            return nullptr; // Allocation failure
        }
    }

    // Take the first free slot
    std::size_t slotIndex = targetPool->mFirstFree;
    Slot& slot = targetPool->mSlots[slotIndex];

    // Updates the list of free slots
    targetPool->mFirstFree = slot.mNextFree;
    --targetPool->mFreeCount;

    // Constructs the object with noexcept handling
    T* obj = reinterpret_cast<T*>(slot.mStorage);
    if constexpr (std::is_nothrow_constructible_v<T, Args...>) {
        new(obj) T(static_cast<Args&&>(args)...);
        slot.mOccupied = true;
        ++mTotalCount;
        return obj;
    }
    else {
        // If the constructor can throw an exception, we use try/catch
        try {
            new(obj) T(static_cast<Args&&>(args)...);
            slot.mOccupied = true;
            ++mTotalCount;
            return obj;
        } catch (...) {
            // Restores state on failure
            targetPool->mFirstFree = slotIndex;
            slot.mNextFree = targetPool->mFirstFree;
            ++targetPool->mFreeCount;
            return nullptr;
        }
    }
}

template<typename T, std::size_t PoolSize>
bool ObjectPool<T, PoolSize>::destroy(T* ptr) noexcept
{
    if (!ptr) return false;

    PoolLocation loc = findObjectLocation(ptr);
    if (!loc.found) return false;

    // Destroys the object safely
    destroyObjectNoexcept(ptr);

    // Marks the slot as free and adds it to the top of the free list
    Slot& slot = loc.pool->mSlots[loc.index];
    slot.mOccupied = false;
    slot.mNextFree = loc.pool->mFirstFree;
    loc.pool->mFirstFree = loc.index;
    ++loc.pool->mFreeCount;

    --mTotalCount;
    return true;
}

template<typename T, std::size_t PoolSize>
void ObjectPool<T, PoolSize>::clear() noexcept
{
    for (Pool* pool = mFirstPool.get(); pool; pool = pool->mNext.get()) {
        for (std::size_t i = 0; i < PoolSize; ++i) {
            if (pool->mSlots[i].mOccupied) {
                T* obj = reinterpret_cast<T*>(pool->mSlots[i].mStorage);
                destroyObjectNoexcept(obj);
                pool->mSlots[i].mOccupied = false;
            }
            pool->mSlots[i].mNextFree = (i < PoolSize - 1) ? i + 1 : PoolSize;
        }
        pool->mFirstFree = 0;
        pool->mFreeCount = PoolSize;
    }
    mTotalCount = 0;
}

template<typename T, std::size_t PoolSize>
std::size_t ObjectPool<T, PoolSize>::size() const noexcept
{
    return mTotalCount;
}

template<typename T, std::size_t PoolSize>
std::size_t ObjectPool<T, PoolSize>::poolCount() const noexcept
{
    return mPoolCount;
}

template<typename T, std::size_t PoolSize>
bool ObjectPool<T, PoolSize>::empty() const noexcept
{
    return mTotalCount == 0;
}

template<typename T, std::size_t PoolSize>
typename ObjectPool<T, PoolSize>::Iterator ObjectPool<T, PoolSize>::begin() noexcept
{
    return Iterator(mFirstPool.get(), 0);
}

template<typename T, std::size_t PoolSize>
typename ObjectPool<T, PoolSize>::Iterator ObjectPool<T, PoolSize>::end() noexcept
{
    return Iterator(nullptr, 0);
}

template<typename T, std::size_t PoolSize>
typename ObjectPool<T, PoolSize>::ReverseIterator ObjectPool<T, PoolSize>::rbegin() noexcept
{
    return ReverseIterator(mLastPool, mFirstPool.get());
}

template<typename T, std::size_t PoolSize>
typename ObjectPool<T, PoolSize>::ReverseIterator ObjectPool<T, PoolSize>::rend() noexcept
{
    return ReverseIterator(nullptr, mFirstPool.get());
}

/* === Private Implementation === */

template<typename T, std::size_t PoolSize>
typename ObjectPool<T, PoolSize>::Pool* ObjectPool<T, PoolSize>::allocateNewPool() noexcept
{
    UniquePtr<Pool> newPool = makeUnique<Pool>();
    if (!newPool) {
        return nullptr; // Allocation failure
    }

    new(newPool.get()) Pool();

    // Initializes the linked list of free slots
    for (std::size_t i = 0; i < PoolSize - 1; ++i) {
        newPool->mSlots[i].mNextFree = i + 1;
    }
    newPool->mSlots[PoolSize - 1].mNextFree = PoolSize; // Mark the end

    Pool* rawPtr = newPool.get();

    if (!mFirstPool) {
        mFirstPool = std::move(newPool);
        mLastPool = mFirstPool.get();
    } else {
        mLastPool->mNext = std::move(newPool);
        mLastPool = rawPtr;
    }
    ++mPoolCount;

    return rawPtr;
}

template<typename T, std::size_t PoolSize>
typename ObjectPool<T, PoolSize>::PoolLocation ObjectPool<T, PoolSize>::findObjectLocation(T* ptr) const noexcept
{
    for (Pool* pool = mFirstPool.get(); pool; pool = pool->mNext.get()) {
        char* poolStart = reinterpret_cast<char*>(pool->mSlots);
        char* poolEnd = poolStart + sizeof(pool->mSlots);
        char* objPtr = reinterpret_cast<char*>(ptr);
        if (objPtr >= poolStart && objPtr < poolEnd) {
            std::size_t offset = objPtr - poolStart;
            if (offset % sizeof(Slot) == 0) {
                std::size_t index = offset / sizeof(Slot);
                if (index < PoolSize && pool->mSlots[index].mOccupied) {
                    return {pool, index, true};
                }
            }
        }
    }
    return {};
}

template<typename T, std::size_t PoolSize>
void ObjectPool<T, PoolSize>::destroyObjectNoexcept(T* obj) noexcept
{
    if constexpr (std::is_nothrow_destructible_v<T>) {
        obj->~T();
    }
    else {
        // For destructors that may throw exceptions,
        // we still call them because std::terminate will be invoked
        // if an exception occurs in a noexcept context
        try {
            obj->~T();
        } catch (...) {
            // In a noexcept context, this will trigger std::terminate,
            // which is the expected behavior
        }
    }
}

/* === Iterator Implementation === */

template<typename T, std::size_t PoolSize>
ObjectPool<T, PoolSize>::Iterator::Iterator(typename ObjectPool<T, PoolSize>::Pool* pool, std::size_t index) noexcept
    : mCurrentPool(pool), mCurrentIndex(index)
{
    findNext();
}

template<typename T, std::size_t PoolSize>
void ObjectPool<T, PoolSize>::Iterator::findNext() noexcept
{
    while (mCurrentPool) {
        while (mCurrentIndex < PoolSize) {
            if (mCurrentPool->mSlots[mCurrentIndex].mOccupied) {
                return;
            }
            ++mCurrentIndex;
        }
        mCurrentPool = mCurrentPool->mNext.get();
        mCurrentIndex = 0;
    }
}

template<typename T, std::size_t PoolSize>
T& ObjectPool<T, PoolSize>::Iterator::operator*() const noexcept
{
    return *reinterpret_cast<T*>(mCurrentPool->mSlots[mCurrentIndex].mStorage);
}

template<typename T, std::size_t PoolSize>
T* ObjectPool<T, PoolSize>::Iterator::operator->() const noexcept
{
    return reinterpret_cast<T*>(mCurrentPool->mSlots[mCurrentIndex].mStorage);
}

template<typename T, std::size_t PoolSize>
typename ObjectPool<T, PoolSize>::Iterator& ObjectPool<T, PoolSize>::Iterator::operator++() noexcept
{
    ++mCurrentIndex;
    findNext();
    return *this;
}

template<typename T, std::size_t PoolSize>
typename ObjectPool<T, PoolSize>::Iterator ObjectPool<T, PoolSize>::Iterator::operator++(int) noexcept
{
    Iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<typename T, std::size_t PoolSize>
bool ObjectPool<T, PoolSize>::Iterator::operator==(const Iterator& other) const noexcept
{
    return mCurrentPool == other.mCurrentPool && mCurrentIndex == other.mCurrentIndex;
}

template<typename T, std::size_t PoolSize>
bool ObjectPool<T, PoolSize>::Iterator::operator!=(const Iterator& other) const noexcept
{
    return !(*this == other);
}

/* === ReverseIterator Implementation === */

template<typename T, std::size_t PoolSize>
ObjectPool<T, PoolSize>::ReverseIterator::ReverseIterator(
    typename ObjectPool<T, PoolSize>::Pool* lastPool,
    typename ObjectPool<T, PoolSize>::Pool* firstPool) noexcept
    : mCurrentPool(lastPool), mFirstPool(firstPool)
{
    if (mCurrentPool) {
        findPrevious();
    }
}

template<typename T, std::size_t PoolSize>
void ObjectPool<T, PoolSize>::ReverseIterator::findPrevious() noexcept
{
    while (mCurrentPool) {
        while (mCurrentIndex > 0) {
            --mCurrentIndex;
            if (mCurrentPool->mSlots[mCurrentIndex].mOccupied) {
                return;
            }
        }

        // Find the previous pool
        typename ObjectPool<T, PoolSize>::Pool* prevPool = nullptr;
        for (Pool* p = mFirstPool; p && p->mNext.get() != mCurrentPool; p = p->mNext.get()) {
            prevPool = p;
        }
        mCurrentPool = prevPool;
        mCurrentIndex = PoolSize;
    }
}

template<typename T, std::size_t PoolSize>
inline T& ObjectPool<T, PoolSize>::ReverseIterator::operator*() const noexcept
{
    return *reinterpret_cast<T*>(mCurrentPool->mSlots[mCurrentIndex].mStorage);
}

template<typename T, std::size_t PoolSize>
inline T* ObjectPool<T, PoolSize>::ReverseIterator::operator->() const noexcept
{
    return reinterpret_cast<T*>(mCurrentPool->mSlots[mCurrentIndex].mStorage);
}

template<typename T, std::size_t PoolSize>
inline typename ObjectPool<T, PoolSize>::ReverseIterator& ObjectPool<T, PoolSize>::ReverseIterator::operator++() noexcept
{
    findPrevious();
    return *this;
}

template<typename T, std::size_t PoolSize>
inline bool ObjectPool<T, PoolSize>::ReverseIterator::operator==(const ReverseIterator& other) const noexcept
{
    return mCurrentPool == other.mCurrentPool && mCurrentIndex == other.mCurrentIndex;
}

template<typename T, std::size_t PoolSize>
inline bool ObjectPool<T, PoolSize>::ReverseIterator::operator!=(const ReverseIterator& other) const noexcept
{
    return !(*this == other);
}

} // namespace util

#endif // NX_UTIL_OBJECT_POOL_HPP
