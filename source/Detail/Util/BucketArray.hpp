/* BucketArray.hpp -- Category-based container with bucketed indices
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_UTIL_BUCKET_ARRAY_HPP
#define NX_UTIL_BUCKET_ARRAY_HPP

#include <NX/NX_Math.h>

#include "./DynamicArray.hpp"

#include <SDL3/SDL_assert.h>
#include <type_traits>
#include <algorithm>
#include <utility>
#include <cstddef>
#include <array>

namespace util {

/**
 * @brief A container that organizes elements of type T into categories
 *
 * Each category manages its own bucket of indices that reference elements
 * stored in a shared data array. This allows iteration over individual
 * categories or combinations of categories.
 *
 * In short:
 * - Category-based iteration using compile-time templates
 * - A single data array with per-category index buckets
 * - Support for multi-category iteration with a defined order
 * - All operations are noexcept
 *
 * @tparam T The type of elements stored
 * @tparam Category An enum type defining the categories
 * @tparam N The number of categories (must match the enum size)
 */
template<typename T, typename Category, size_t N>
class BucketArray {
public:
    /**
     * @brief Iterator for a single category
     */
    class CategoryIterator {
    public:
        CategoryIterator(const DynamicArray<T>& data, const DynamicArray<size_t>& indices, size_t pos) noexcept;

        const T& operator*() const noexcept;
        const T* operator->() const noexcept;

        CategoryIterator& operator++() noexcept;
        CategoryIterator operator++(int) noexcept;

        bool operator==(const CategoryIterator& other) const noexcept;
        bool operator!=(const CategoryIterator& other) const noexcept;

    private:
        const DynamicArray<T>& mObjects;
        const DynamicArray<size_t>& mIndices;
        size_t mPos;
    };

    /**
     * @brief Iterator for multiple categories in specified order
     * @tparam CatCount Number of categories to iterate through
     */
    template<size_t CatCount>
    class MultiCategoryIterator {
    public:
        MultiCategoryIterator(const BucketArray& parent, std::array<Category, CatCount>&& categories, size_t catIdx, size_t elemIdx) noexcept;

        std::pair<Category, const T&> operator*() const noexcept;

        MultiCategoryIterator& operator++() noexcept;
        MultiCategoryIterator operator++(int) noexcept;

        bool operator==(const MultiCategoryIterator& other) const noexcept;
        bool operator!=(const MultiCategoryIterator& other) const noexcept;

        Category GetCategory() const noexcept;
        size_t GetIndex() const noexcept;
        const T& GetValue() const noexcept;

    private:
        void FindNextValidPosition() noexcept;

    private:
        const BucketArray& mParent;
        std::array<Category, CatCount> mCategories;
        size_t mCatIdx;   // Current category index
        size_t mElemIdx;  // Current element index within category
    };

    /**
     * @brief View for iterating over a single category
     */
    class CategoryView {
    public:
        CategoryView(const DynamicArray<T>& data, const DynamicArray<size_t>& indices) noexcept;

        CategoryIterator Begin() const noexcept;
        CategoryIterator End() const noexcept;

        size_t GetSize() const noexcept;
        bool IsEmpty() const noexcept;

        const T& operator[](size_t idx) const noexcept;

    private:
        const DynamicArray<T>& mObjects;
        const DynamicArray<size_t>& mIndices;
    };

    /**
     * @brief View for iterating over multiple categories
     * @tparam CatCount Number of categories in this view
     */
    template<size_t CatCount>
    class MultiCategoryView {
    public:
        MultiCategoryView(const BucketArray& parent, std::array<Category, CatCount>&& categories) noexcept;

        MultiCategoryIterator<CatCount> Begin() const noexcept;
        MultiCategoryIterator<CatCount> End() const noexcept;

        size_t GetSize() const noexcept;
        bool IsEmpty() const noexcept;

    private:
        const BucketArray& mParent;
        std::array<Category, CatCount> mCategories;
    };

    /* --- Main Operations --- */

    void Clear() noexcept;
    bool Reserve(size_t cap) noexcept;
    size_t Push(Category cat, const T& value) noexcept;

    template<typename... Args>
    size_t Emplace(Category cat, Args&&... args) noexcept;

    /** Sort elements within a category using custom comparator */
    template<typename Compare>
    void Sort(Category cat, Compare&& comp) noexcept;

    /** 
     * Removes all objects for which the given condition returns true.
     * Updates both the object storage and the category buckets.
     */
    template<typename Condition>
    void RemoveIf(Condition&& cond) noexcept;

    /* --- Data Access --- */

    /** Get direct access to underlying data array */
    const DynamicArray<T>& GetAll() const noexcept;

    /** Get view for iterating over single category */
    CategoryView GetCategory(Category cat) const noexcept;

    /** Get view for iterating over multiple categories in specified order */
    template<typename... Categories>
    auto GetCategories(Categories... cats) const noexcept -> MultiCategoryView<sizeof...(Categories)>;

    /* --- Size Information --- */

    size_t GetSize() const noexcept;
    size_t GetSize(Category cat) const noexcept;

    bool IsEmpty() const noexcept;
    bool IsEmpty(Category cat) const noexcept;

    /* --- Direct Element Access --- */

    const T& operator[](size_t idx) const noexcept;
    T& operator[](size_t idx) noexcept;

private:
    DynamicArray<T> mObjects;                                       // Object storage
    DynamicArray<std::pair<Category, size_t>> mObjectCategoryMap;   // Object to bucket map
    std::array<DynamicArray<size_t>, N> mBuckets;                   // Index buckets per category
};

/* === Public Implementation === */

template<typename T, typename Category, size_t N>
void BucketArray<T, Category, N>::Clear() noexcept
{
    for (auto& bucket : mBuckets) {
        bucket.Clear();
    }
    mObjectCategoryMap.Clear();
    mObjects.Clear();
}

template<typename T, typename Category, size_t N>
bool BucketArray<T, Category, N>::Reserve(size_t cap) noexcept
{
    if (!mObjects.Reserve(cap) || !mObjectCategoryMap.Reserve(cap)) {
        return false;
    }

    const size_t bucketCap = NX_DIV_CEIL(cap, N);
    for (auto& bucket : mBuckets) {
        if (!bucket.Reserve(bucketCap)) {
            return false;
        }
    }
    return true;
}

template<typename T, typename Category, size_t N>
size_t BucketArray<T, Category, N>::Push(Category cat, const T& value) noexcept
{
    SDL_assert(static_cast<size_t>(cat) < N);

    auto& bucket = mBuckets[static_cast<size_t>(cat)];
    const size_t idx = mObjects.GetSize();

    mObjectCategoryMap.EmplaceBack(cat, bucket.GetSize());
    mObjects.PushBack(value);
    bucket.EmplaceBack(idx);

    return idx;
}

template<typename T, typename Category, size_t N>
template<typename... Args>
size_t BucketArray<T, Category, N>::Emplace(Category cat, Args&&... args) noexcept
{
    SDL_assert(static_cast<size_t>(cat) < N);

    auto& bucket = mBuckets[static_cast<size_t>(cat)];
    const size_t idx = mObjects.GetSize();

    mObjectCategoryMap.EmplaceBack(cat, bucket.GetSize());
    mObjects.EmplaceBack(std::forward<Args>(args)...);
    bucket.EmplaceBack(idx);

    return idx;
}

template<typename T, typename Category, size_t N>
template<typename Compare>
void BucketArray<T, Category, N>::Sort(Category cat, Compare&& comp) noexcept
{
    auto& bucket = mBuckets[static_cast<size_t>(cat)];
    std::sort(bucket.Begin(), bucket.End(), [this, &comp](size_t idx1, size_t idx2) {
        return comp(mObjects[idx1], mObjects[idx2]);
    });
}

template<typename T, typename Category, size_t N>
template<typename Condition>
void BucketArray<T, Category, N>::RemoveIf(Condition&& cond) noexcept
{
    for (int64_t idx = mObjects.GetSize() - 1; idx >= 0; --idx)
    {
        // Keep the elements that do NOT satisfy the condition
        if (!cond(mObjects[idx])) continue;

        size_t lastIdx = mObjects.GetSize() - 1;
        
        // Get category info BEFORE modifying anything
        auto [cat, pos] = mObjectCategoryMap[idx];
        auto& bucket = mBuckets[static_cast<size_t>(cat)];

        if (idx != lastIdx)
        {
            // Move the last object to the position of the one being removed
            if constexpr (std::is_move_assignable_v<T>) {
                mObjects[idx] = std::move(mObjects[lastIdx]);
            } else {
                mObjects[idx].~T();
                new (&mObjects[idx]) T(std::move(mObjects[lastIdx]));
            }

            // Update the bucket of the last element
            auto [lastCat, lastPos] = mObjectCategoryMap[lastIdx];
            auto& lastBucket = mBuckets[static_cast<size_t>(lastCat)];
            lastBucket[lastPos] = idx;                          // update the index in the bucket
            mObjectCategoryMap[idx] = { lastCat, lastPos };     // update the pair
        }

        // Remove the last element
        mObjects.pop_back();
        mObjectCategoryMap.pop_back();

        // Remove the object's index from its bucket
        // Use the bucket info we saved earlier
        size_t lastBucketPos = bucket.GetSize() - 1;
        if (pos != lastBucketPos) {
            bucket[pos] = bucket[lastBucketPos];                    // move the last index of the bucket here
            mObjectCategoryMap[bucket[pos]].second = pos;           // update the index_in_bucket of the moved object
        }
        bucket.pop_back();
    }
}

template<typename T, typename Category, size_t N>
const DynamicArray<T>& BucketArray<T, Category, N>::GetAll() const noexcept
{
    return mObjects;
}

template<typename T, typename Category, size_t N>
typename BucketArray<T, Category, N>::CategoryView 
BucketArray<T, Category, N>::GetCategory(Category cat) const noexcept
{
    return CategoryView(mObjects, mBuckets[static_cast<size_t>(cat)]);
}

template<typename T, typename Category, size_t N>
template<typename... Categories>
auto BucketArray<T, Category, N>::GetCategories(Categories... cats) const noexcept -> MultiCategoryView<sizeof...(Categories)>
{
    static_assert(sizeof...(Categories) > 0, "At least one category must be provided");
    return MultiCategoryView<sizeof...(Categories)>(*this, std::array<Category, sizeof...(Categories)>{cats...});
}

template<typename T, typename Category, size_t N>
size_t BucketArray<T, Category, N>::GetSize() const noexcept
{
    return mObjects.GetSize();
}

template<typename T, typename Category, size_t N>
size_t BucketArray<T, Category, N>::GetSize(Category cat) const noexcept
{
    return mBuckets[static_cast<size_t>(cat)].GetSize();
}

template<typename T, typename Category, size_t N>
bool BucketArray<T, Category, N>::IsEmpty() const noexcept
{
    return mObjects.IsEmpty();
}

template<typename T, typename Category, size_t N>
bool BucketArray<T, Category, N>::IsEmpty(Category cat) const noexcept
{
    return mBuckets[static_cast<size_t>(cat)].IsEmpty();
}

template<typename T, typename Category, size_t N>
const T& BucketArray<T, Category, N>::operator[](size_t idx) const noexcept
{
    return mObjects[idx];
}

template<typename T, typename Category, size_t N>
T& BucketArray<T, Category, N>::operator[](size_t idx) noexcept
{
    return mObjects[idx];
}

/* === Category Iterator Implementation === */

template<typename T, typename Category, size_t N>
BucketArray<T, Category, N>::CategoryIterator::CategoryIterator(
    const DynamicArray<T>& data, const DynamicArray<size_t>& indices, size_t pos) noexcept
    : mObjects(data), mIndices(indices), mPos(pos)
{ }

template<typename T, typename Category, size_t N>
const T& BucketArray<T, Category, N>::CategoryIterator::operator*() const noexcept
{
    return mObjects[mIndices[mPos]];
}

template<typename T, typename Category, size_t N>
const T* BucketArray<T, Category, N>::CategoryIterator::operator->() const noexcept
{
    return &mObjects[mIndices[mPos]];
}

template<typename T, typename Category, size_t N>
typename BucketArray<T, Category, N>::CategoryIterator& 
BucketArray<T, Category, N>::CategoryIterator::operator++() noexcept
{
    ++mPos;
    return *this;
}

template<typename T, typename Category, size_t N>
typename BucketArray<T, Category, N>::CategoryIterator 
BucketArray<T, Category, N>::CategoryIterator::operator++(int) noexcept
{
    CategoryIterator tmp = *this;
    ++mPos;
    return tmp;
}

template<typename T, typename Category, size_t N>
bool BucketArray<T, Category, N>::CategoryIterator::operator==(const CategoryIterator& other) const noexcept
{
    return mPos == other.mPos;
}

template<typename T, typename Category, size_t N>
bool BucketArray<T, Category, N>::CategoryIterator::operator!=(const CategoryIterator& other) const noexcept
{
    return mPos != other.mPos;
}

/* === Multi Category Iterator Implementation === */

template<typename T, typename Category, size_t N>
template<size_t CatCount>
BucketArray<T, Category, N>::MultiCategoryIterator<CatCount>::MultiCategoryIterator(
    const BucketArray& parent, std::array<Category, CatCount>&& categories,
    size_t catIdx, size_t elemIdx) noexcept
    : mParent(parent), mCategories(std::move(categories)), mCatIdx(catIdx), mElemIdx(elemIdx)
{
    // Only search for valid position if we're not creating an End() iterator
    if (mCatIdx < CatCount) {
        FindNextValidPosition();
    }
}

template<typename T, typename Category, size_t N>
template<size_t CatCount>
void BucketArray<T, Category, N>::MultiCategoryIterator<CatCount>::FindNextValidPosition() noexcept
{
    while (mCatIdx < CatCount)
    {
        const size_t bucketIdx = static_cast<size_t>(mCategories[mCatIdx]);

        // Skip invalid bucket indices
        if (bucketIdx >= N) {
            ++mCatIdx;
            mElemIdx = 0;
            continue;
        }

        const auto& bucket = mParent.mBuckets[bucketIdx];

        // If we have a valid position in current bucket, stop here
        if (mElemIdx < bucket.GetSize()) {
            return;
        }

        // Otherwise, move to next category
        ++mCatIdx;
        mElemIdx = 0;
    }
}

template<typename T, typename Category, size_t N>
template<size_t CatCount>
std::pair<Category, const T&> BucketArray<T, Category, N>::MultiCategoryIterator<CatCount>::operator*() const noexcept
{
    const size_t bucketIdx = static_cast<size_t>(mCategories[mCatIdx]);
    const auto& bucket = mParent.mBuckets[bucketIdx];
    const size_t dataIdx = bucket[mElemIdx];
    return { mCategories[mCatIdx], mParent.mObjects[dataIdx] };
}

template<typename T, typename Category, size_t N>
template<size_t CatCount>
typename BucketArray<T, Category, N>::template MultiCategoryIterator<CatCount>& 
BucketArray<T, Category, N>::MultiCategoryIterator<CatCount>::operator++() noexcept
{
    if (mCatIdx >= CatCount) {
        return *this;
    }

    ++mElemIdx;
    FindNextValidPosition();

    return *this;
}

template<typename T, typename Category, size_t N>
template<size_t CatCount>
typename BucketArray<T, Category, N>::template MultiCategoryIterator<CatCount> 
BucketArray<T, Category, N>::MultiCategoryIterator<CatCount>::operator++(int) noexcept
{
    MultiCategoryIterator tmp = *this;
    ++(*this);
    return tmp;
}

template<typename T, typename Category, size_t N>
template<size_t CatCount>
bool BucketArray<T, Category, N>::MultiCategoryIterator<CatCount>::operator==(const MultiCategoryIterator& other) const noexcept
{
    return mCatIdx == other.mCatIdx && mElemIdx == other.mElemIdx;
}

template<typename T, typename Category, size_t N>
template<size_t CatCount>
bool BucketArray<T, Category, N>::MultiCategoryIterator<CatCount>::operator!=(const MultiCategoryIterator& other) const noexcept
{
    return !(*this == other);
}

template<typename T, typename Category, size_t N>
template<size_t CatCount>
Category BucketArray<T, Category, N>::MultiCategoryIterator<CatCount>::GetCategory() const noexcept
{
    return mCategories[mCatIdx];
}

template<typename T, typename Category, size_t N>
template<size_t CatCount>
size_t BucketArray<T, Category, N>::MultiCategoryIterator<CatCount>::GetIndex() const noexcept
{
    const size_t bucketIdx = static_cast<size_t>(mCategories[mCatIdx]);
    return mParent.mBuckets[bucketIdx][mElemIdx];
}

template<typename T, typename Category, size_t N>
template<size_t CatCount>
const T& BucketArray<T, Category, N>::MultiCategoryIterator<CatCount>::GetValue() const noexcept
{
    const size_t dataIdx = GetIndex();
    return mParent.mObjects[dataIdx];
}

/* === Category View Implementation === */

template<typename T, typename Category, size_t N>
BucketArray<T, Category, N>::CategoryView::CategoryView(
    const DynamicArray<T>& data, const DynamicArray<size_t>& indices) noexcept
    : mObjects(data), mIndices(indices)
{ }

template<typename T, typename Category, size_t N>
typename BucketArray<T, Category, N>::CategoryIterator 
BucketArray<T, Category, N>::CategoryView::Begin() const noexcept
{
    return CategoryIterator(mObjects, mIndices, 0);
}

template<typename T, typename Category, size_t N>
typename BucketArray<T, Category, N>::CategoryIterator 
BucketArray<T, Category, N>::CategoryView::End() const noexcept
{
    return CategoryIterator(mObjects, mIndices, mIndices.GetSize());
}

template<typename T, typename Category, size_t N>
size_t BucketArray<T, Category, N>::CategoryView::GetSize() const noexcept
{
    return mIndices.GetSize();
}

template<typename T, typename Category, size_t N>
bool BucketArray<T, Category, N>::CategoryView::IsEmpty() const noexcept
{
    return mIndices.IsEmpty();
}

template<typename T, typename Category, size_t N>
const T& BucketArray<T, Category, N>::CategoryView::operator[](size_t idx) const noexcept
{
    return mObjects[mIndices[idx]];
}

/* === Multi Category View Implementation === */

template<typename T, typename Category, size_t N>
template<size_t CatCount>
BucketArray<T, Category, N>::MultiCategoryView<CatCount>::MultiCategoryView(
    const BucketArray& parent, std::array<Category, CatCount>&& categories) noexcept
    : mParent(parent), mCategories(std::move(categories))
{ }

template<typename T, typename Category, size_t N>
template<size_t CatCount>
typename BucketArray<T, Category, N>::template MultiCategoryIterator<CatCount> 
BucketArray<T, Category, N>::MultiCategoryView<CatCount>::Begin() const noexcept
{
    return MultiCategoryIterator<CatCount>(mParent, std::array<Category, CatCount>(mCategories), 0, 0);
}

template<typename T, typename Category, size_t N>
template<size_t CatCount>
typename BucketArray<T, Category, N>::template MultiCategoryIterator<CatCount> 
BucketArray<T, Category, N>::MultiCategoryView<CatCount>::End() const noexcept
{
    return MultiCategoryIterator<CatCount>(mParent, std::array<Category, CatCount>(mCategories), CatCount, 0);
}

template<typename T, typename Category, size_t N>
template<size_t CatCount>
size_t BucketArray<T, Category, N>::MultiCategoryView<CatCount>::GetSize() const noexcept
{
    size_t total = 0;
    for (const auto& cat : mCategories) {
        const size_t bucketIdx = static_cast<size_t>(cat);
        if (bucketIdx < N) {
            total += mParent.mBuckets[bucketIdx].GetSize();
        }
    }
    return total;
}

template<typename T, typename Category, size_t N>
template<size_t CatCount>
bool BucketArray<T, Category, N>::MultiCategoryView<CatCount>::IsEmpty() const noexcept
{
    for (const auto& cat : mCategories) {
        const size_t bucketIdx = static_cast<size_t>(cat);
        if (bucketIdx < N && !mParent.mBuckets[bucketIdx].IsEmpty()) {
            return false;
        }
    }
    return true;
}

} // namespace util

#endif // NX_UTIL_BUCKET_ARRAY_HPP
