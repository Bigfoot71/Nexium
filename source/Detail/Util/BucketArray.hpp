#ifndef HP_UTIL_BUCKET_ARRAY_HPP
#define HP_UTIL_BUCKET_ARRAY_HPP

#include <Hyperion/HP_Macros.h>
#include <SDL3/SDL_assert.h>

#include "./DynamicArray.hpp"

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
        const DynamicArray<T>& mData;
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

        const T& operator*() const noexcept;
        const T* operator->() const noexcept;

        MultiCategoryIterator& operator++() noexcept;
        MultiCategoryIterator operator++(int) noexcept;

        bool operator==(const MultiCategoryIterator& other) const noexcept;
        bool operator!=(const MultiCategoryIterator& other) const noexcept;

    private:
        void findNextValidPosition() noexcept;

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

        CategoryIterator begin() const noexcept;
        CategoryIterator end() const noexcept;

        size_t size() const noexcept;
        bool empty() const noexcept;

        const T& operator[](size_t idx) const noexcept;

    private:
        const DynamicArray<T>& mData;
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

        MultiCategoryIterator<CatCount> begin() const noexcept;
        MultiCategoryIterator<CatCount> end() const noexcept;

        size_t size() const noexcept;
        bool empty() const noexcept;

    private:
        const BucketArray& mParent;
        std::array<Category, CatCount> mCategories;
    };

    /* --- Main Operations --- */

    void clear() noexcept;
    bool reserve(size_t cap) noexcept;
    size_t push(Category cat, const T& value) noexcept;

    template<typename... Args>
    size_t emplace(Category cat, Args&&... args) noexcept;

    // Sort elements within a category using custom comparator
    template<typename Compare>
    void sort(Category cat, Compare&& comp) noexcept;

    /* --- Data Access --- */

    // Get direct access to underlying data array
    const DynamicArray<T>& all() const noexcept;

    // Get view for iterating over single category
    CategoryView category(Category cat) const noexcept;

    // Get view for iterating over multiple categories in specified order
    template<typename... Categories>
    auto categories(Categories... cats) const noexcept -> MultiCategoryView<sizeof...(Categories)>;

    /* --- Size Information --- */

    size_t size() const noexcept;
    size_t size(Category cat) const noexcept;

    bool empty() const noexcept;
    bool empty(Category cat) const noexcept;

    /* --- Direct Element Access --- */

    const T& operator[](size_t idx) const noexcept;
    T& operator[](size_t idx) noexcept;

private:
    DynamicArray<T> mData;                          // Global data storage
    std::array<DynamicArray<size_t>, N> mBuckets;   // Index buckets per category
};

/* === Public Implementation === */

template<typename T, typename Category, size_t N>
void BucketArray<T, Category, N>::clear() noexcept
{
    for (auto& bucket : mBuckets) {
        bucket.clear();
    }
    mData.clear();
}

template<typename T, typename Category, size_t N>
bool BucketArray<T, Category, N>::reserve(size_t cap) noexcept
{
    if (!mData.reserve(cap)) {
        return false;
    }

    const size_t bucketCap = HP_DIV_CEIL(cap, N);
    for (auto& bucket : mBuckets) {
        if (!bucket.reserve(bucketCap)) {
            return false;
        }
    }
    return true;
}

template<typename T, typename Category, size_t N>
size_t BucketArray<T, Category, N>::push(Category cat, const T& value) noexcept
{
    SDL_assert(static_cast<size_t>(cat) < N);

    const size_t idx = mData.size();

    mData.push_back(value);
    mBuckets[static_cast<size_t>(cat)].push_back(idx);

    return idx;
}

template<typename T, typename Category, size_t N>
template<typename... Args>
size_t BucketArray<T, Category, N>::emplace(Category cat, Args&&... args) noexcept
{
    SDL_assert(static_cast<size_t>(cat) < N);

    const size_t idx = mData.size();

    mData.emplace_back(std::forward<Args>(args)...);
    mBuckets[static_cast<size_t>(cat)].push_back(idx);

    return idx;
}

template<typename T, typename Category, size_t N>
template<typename Compare>
void BucketArray<T, Category, N>::sort(Category cat, Compare&& comp) noexcept
{
    auto& bucket = mBuckets[static_cast<size_t>(cat)];
    std::sort(bucket.begin(), bucket.end(), [this, &comp](size_t idx1, size_t idx2) {
        return comp(mData[idx1], mData[idx2]);
    });
}

template<typename T, typename Category, size_t N>
const DynamicArray<T>& BucketArray<T, Category, N>::all() const noexcept
{
    return mData;
}

template<typename T, typename Category, size_t N>
typename BucketArray<T, Category, N>::CategoryView 
BucketArray<T, Category, N>::category(Category cat) const noexcept
{
    return CategoryView(mData, mBuckets[static_cast<size_t>(cat)]);
}

template<typename T, typename Category, size_t N>
template<typename... Categories>
auto BucketArray<T, Category, N>::categories(Categories... cats) const noexcept -> MultiCategoryView<sizeof...(Categories)>
{
    static_assert(sizeof...(Categories) > 0, "At least one category must be provided");
    return MultiCategoryView<sizeof...(Categories)>(*this, std::array<Category, sizeof...(Categories)>{cats...});
}

template<typename T, typename Category, size_t N>
size_t BucketArray<T, Category, N>::size() const noexcept
{
    return mData.size();
}

template<typename T, typename Category, size_t N>
size_t BucketArray<T, Category, N>::size(Category cat) const noexcept
{
    return mBuckets[static_cast<size_t>(cat)].size();
}

template<typename T, typename Category, size_t N>
bool BucketArray<T, Category, N>::empty() const noexcept
{
    return mData.empty();
}

template<typename T, typename Category, size_t N>
bool BucketArray<T, Category, N>::empty(Category cat) const noexcept
{
    return mBuckets[static_cast<size_t>(cat)].empty();
}

template<typename T, typename Category, size_t N>
const T& BucketArray<T, Category, N>::operator[](size_t idx) const noexcept
{
    return mData[idx];
}

template<typename T, typename Category, size_t N>
T& BucketArray<T, Category, N>::operator[](size_t idx) noexcept
{
    return mData[idx];
}

/* === Category Iterator Implementation === */

template<typename T, typename Category, size_t N>
BucketArray<T, Category, N>::CategoryIterator::CategoryIterator(
    const DynamicArray<T>& data, const DynamicArray<size_t>& indices, size_t pos) noexcept
    : mData(data), mIndices(indices), mPos(pos)
{ }

template<typename T, typename Category, size_t N>
const T& BucketArray<T, Category, N>::CategoryIterator::operator*() const noexcept
{
    return mData[mIndices[mPos]];
}

template<typename T, typename Category, size_t N>
const T* BucketArray<T, Category, N>::CategoryIterator::operator->() const noexcept
{
    return &mData[mIndices[mPos]];
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
    // Only search for valid position if we're not creating an end() iterator
    if (mCatIdx < CatCount) {
        findNextValidPosition();
    }
}

template<typename T, typename Category, size_t N>
template<size_t CatCount>
void BucketArray<T, Category, N>::MultiCategoryIterator<CatCount>::findNextValidPosition() noexcept
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
        if (mElemIdx < bucket.size()) {
            return;
        }

        // Otherwise, move to next category
        ++mCatIdx;
        mElemIdx = 0;
    }
}

template<typename T, typename Category, size_t N>
template<size_t CatCount>
const T& BucketArray<T, Category, N>::MultiCategoryIterator<CatCount>::operator*() const noexcept
{
    const size_t bucketIdx = static_cast<size_t>(mCategories[mCatIdx]);
    const auto& bucket = mParent.mBuckets[bucketIdx];
    const size_t dataIdx = bucket[mElemIdx];
    return mParent.mData[dataIdx];
}

template<typename T, typename Category, size_t N>
template<size_t CatCount>
const T* BucketArray<T, Category, N>::MultiCategoryIterator<CatCount>::operator->() const noexcept
{
    return &(operator*());
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
    findNextValidPosition();

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

/* === Category View Implementation === */

template<typename T, typename Category, size_t N>
BucketArray<T, Category, N>::CategoryView::CategoryView(
    const DynamicArray<T>& data, const DynamicArray<size_t>& indices) noexcept
    : mData(data), mIndices(indices)
{ }

template<typename T, typename Category, size_t N>
typename BucketArray<T, Category, N>::CategoryIterator 
BucketArray<T, Category, N>::CategoryView::begin() const noexcept
{
    return CategoryIterator(mData, mIndices, 0);
}

template<typename T, typename Category, size_t N>
typename BucketArray<T, Category, N>::CategoryIterator 
BucketArray<T, Category, N>::CategoryView::end() const noexcept
{
    return CategoryIterator(mData, mIndices, mIndices.size());
}

template<typename T, typename Category, size_t N>
size_t BucketArray<T, Category, N>::CategoryView::size() const noexcept
{
    return mIndices.size();
}

template<typename T, typename Category, size_t N>
bool BucketArray<T, Category, N>::CategoryView::empty() const noexcept
{
    return mIndices.empty();
}

template<typename T, typename Category, size_t N>
const T& BucketArray<T, Category, N>::CategoryView::operator[](size_t idx) const noexcept
{
    return mData[mIndices[idx]];
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
BucketArray<T, Category, N>::MultiCategoryView<CatCount>::begin() const noexcept
{
    return MultiCategoryIterator<CatCount>(mParent, std::array<Category, CatCount>(mCategories), 0, 0);
}

template<typename T, typename Category, size_t N>
template<size_t CatCount>
typename BucketArray<T, Category, N>::template MultiCategoryIterator<CatCount> 
BucketArray<T, Category, N>::MultiCategoryView<CatCount>::end() const noexcept
{
    return MultiCategoryIterator<CatCount>(mParent, std::array<Category, CatCount>(mCategories), CatCount, 0);
}

template<typename T, typename Category, size_t N>
template<size_t CatCount>
size_t BucketArray<T, Category, N>::MultiCategoryView<CatCount>::size() const noexcept
{
    size_t total = 0;
    for (const auto& cat : mCategories) {
        const size_t bucketIdx = static_cast<size_t>(cat);
        if (bucketIdx < N) {
            total += mParent.mBuckets[bucketIdx].size();
        }
    }
    return total;
}

template<typename T, typename Category, size_t N>
template<size_t CatCount>
bool BucketArray<T, Category, N>::MultiCategoryView<CatCount>::empty() const noexcept
{
    for (const auto& cat : mCategories) {
        const size_t bucketIdx = static_cast<size_t>(cat);
        if (bucketIdx < N && !mParent.mBuckets[bucketIdx].empty()) {
            return false;
        }
    }
    return true;
}

} // namespace util

#endif // HP_UTIL_BUCKET_ARRAY_HPP
