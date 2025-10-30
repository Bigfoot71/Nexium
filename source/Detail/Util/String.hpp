/* String.hpp -- STL-like string with noexcept operations
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_UTIL_STRING_HPP
#define NX_UTIL_STRING_HPP

#include <SDL3/SDL_stdinc.h>
#include <NX/NX_Memory.h>
#include <utility>

namespace util {

/* === Declaration === */

class String {
public:
    /** Constructors */
    String() noexcept;
    String(const char* str) noexcept;
    String(const char* str, size_t len) noexcept;
    String(const String& other) noexcept;
    String(String&& other) noexcept;

    /** Destructor */
    ~String() noexcept;

    /** Assignment operators */
    String& operator=(const String& other) noexcept;
    String& operator=(String&& other) noexcept;
    String& operator=(const char* str) noexcept;

    /** Comparison operators */
    bool operator==(const String& other) const noexcept;
    bool operator==(const char* str) const noexcept;
    bool operator!=(const String& other) const noexcept;
    bool operator!=(const char* str) const noexcept;
    bool operator<(const String& other) const noexcept;

    /** Concatenation operators */
    String operator+(const String& other) const noexcept;
    String operator+(const char* str) const noexcept;
    String& operator+=(const String& other) noexcept;
    String& operator+=(const char* str) noexcept;
    String& operator+=(char c) noexcept;

    /** Element access */
    char operator[](size_t index) const noexcept;
    char at(size_t index) const noexcept;

    /** Getters */
    const char* data() const noexcept;
    size_t size() const noexcept;
    size_t length() const noexcept;
    bool empty() const noexcept;

    /** Capacity */
    size_t capacity() const noexcept;
    void reserve(size_t newCapacity) noexcept;
    void shrink_to_fit() noexcept;

    /** Modifiers */
    void clear() noexcept;
    void append(const char* str) noexcept;
    void append(const char* str, size_t len) noexcept;
    void append(const String& str) noexcept;
    void push_back(char c) noexcept;
    void pop_back() noexcept;

    /** String operations */
    String substr(size_t pos, size_t len = static_cast<size_t>(-1)) const noexcept;
    size_t find(const char* str, size_t pos = 0) const noexcept;
    size_t find(char c, size_t pos = 0) const noexcept;
    size_t rfind(const char* str, size_t pos = static_cast<size_t>(-1)) const noexcept;
    size_t rfind(char c, size_t pos = static_cast<size_t>(-1)) const noexcept;
    bool contains(const char* str) const noexcept;
    bool contains(char c) const noexcept;
    bool starts_with(const char* prefix) const noexcept;
    bool ends_with(const char* suffix) const noexcept;

    /** Replace operations */
    String& replace(size_t pos, size_t len, const char* str) noexcept;
    String& replace_all(const char* from, const char* to) noexcept;
    String& replace_first(const char* from, const char* to) noexcept;

    /** Trim operations */
    String& trim() noexcept;
    String& trim_left() noexcept;
    String& trim_right() noexcept;
    String trimmed() const noexcept;

    /** Case conversion */
    String& to_lower() noexcept;
    String& to_upper() noexcept;
    String lower() const noexcept;
    String upper() const noexcept;

    /** Utility */
    void swap(String& other) noexcept;

    /** Static constants */
    static constexpr size_t npos = static_cast<size_t>(-1);

private:
    char* mData{};
    size_t mSize{};
    size_t mCapacity{};

    /** Helper functions */
    void allocate(size_t size) noexcept;
    void reallocate(size_t newCapacity) noexcept;
    void free_data() noexcept;
    static size_t safe_strlen(const char* str) noexcept;
};

/* === Public Implementation === */

inline String::String() noexcept
    : mData(nullptr), mSize(0), mCapacity(0)
{ }

inline String::String(const char* str) noexcept
    : mData(nullptr), mSize(0), mCapacity(0)
{
    if (str) {
        mSize = safe_strlen(str);
        if (mSize > 0) {
            allocate(mSize);
            if (mData) {
                SDL_memcpy(mData, str, mSize);
                mData[mSize] = '\0';
            }
        }
    }
}

inline String::String(const char* str, size_t len) noexcept
    : mData(nullptr), mSize(0), mCapacity(0)
{
    if (str && len > 0) {
        mSize = len;
        allocate(mSize);
        if (mData) {
            SDL_memcpy(mData, str, mSize);
            mData[mSize] = '\0';
        }
    }
}

inline String::String(const String& other) noexcept
    : mData(nullptr), mSize(0), mCapacity(0)
{
    if (other.mSize > 0 && other.mData) {
        mSize = other.mSize;
        allocate(mSize);
        if (mData) {
            SDL_memcpy(mData, other.mData, mSize);
            mData[mSize] = '\0';
        }
    }
}

inline String::String(String&& other) noexcept
    : mData(std::exchange(other.mData, nullptr))
    , mSize(std::exchange(other.mSize, 0))
    , mCapacity(std::exchange(other.mCapacity, 0))
{ }

inline String::~String() noexcept
{
    free_data();
}

inline String& String::operator=(const String& other) noexcept
{
    if (this != &other) {
        free_data();
        if (other.mSize > 0 && other.mData) {
            mSize = other.mSize;
            allocate(mSize);
            if (mData) {
                SDL_memcpy(mData, other.mData, mSize);
                mData[mSize] = '\0';
            }
        }
    }
    return *this;
}

inline String& String::operator=(String&& other) noexcept
{
    if (this != &other) {
        free_data();
        mData = std::exchange(other.mData, nullptr);
        mSize = std::exchange(other.mSize, 0);
        mCapacity = std::exchange(other.mCapacity, 0);
    }
    return *this;
}

inline String& String::operator=(const char* str) noexcept
{
    free_data();
    if (str) {
        mSize = safe_strlen(str);
        if (mSize > 0) {
            allocate(mSize);
            if (mData) {
                SDL_memcpy(mData, str, mSize);
                mData[mSize] = '\0';
            }
        }
    }
    return *this;
}

inline bool String::operator==(const String& other) const noexcept
{
    if (mSize != other.mSize) return false;
    if (mData == other.mData) return true;
    if (!mData || !other.mData) return false;
    return SDL_memcmp(mData, other.mData, mSize) == 0;
}

inline bool String::operator==(const char* str) const noexcept
{
    if (!str) return mSize == 0;
    if (!mData) return *str == '\0';
    return SDL_strcmp(mData, str) == 0;
}

inline bool String::operator!=(const String& other) const noexcept
{
    return !(*this == other);
}

inline bool String::operator!=(const char* str) const noexcept
{
    return !(*this == str);
}

inline bool String::operator<(const String& other) const noexcept
{
    if (!mData && !other.mData) return false;
    if (!mData) return true;
    if (!other.mData) return false;
    return SDL_strcmp(mData, other.mData) < 0;
}

inline String String::operator+(const String& other) const noexcept
{
    String result(*this);
    result += other;
    return result;
}

inline String String::operator+(const char* str) const noexcept
{
    String result(*this);
    result += str;
    return result;
}

inline String& String::operator+=(const String& other) noexcept
{
    append(other.mData, other.mSize);
    return *this;
}

inline String& String::operator+=(const char* str) noexcept
{
    append(str);
    return *this;
}

inline String& String::operator+=(char c) noexcept
{
    push_back(c);
    return *this;
}

inline char String::operator[](size_t index) const noexcept
{
    if (index >= mSize || !mData) return '\0';
    return mData[index];
}

inline char String::at(size_t index) const noexcept
{
    return (*this)[index];
}

inline const char* String::data() const noexcept
{
    return mData ? mData : "";
}

inline size_t String::size() const noexcept
{
    return mSize;
}

inline size_t String::length() const noexcept
{
    return mSize;
}

inline bool String::empty() const noexcept
{
    return mSize == 0;
}

inline size_t String::capacity() const noexcept
{
    return mCapacity;
}

inline void String::reserve(size_t newCapacity) noexcept
{
    if (newCapacity > mCapacity) {
        reallocate(newCapacity);
    }
}

inline void String::shrink_to_fit() noexcept
{
    if (mCapacity > mSize) {
        reallocate(mSize);
    }
}

inline void String::clear() noexcept
{
    if (mData) {
        mData[0] = '\0';
    }
    mSize = 0;
}

inline void String::append(const char* str) noexcept
{
    if (str) {
        append(str, safe_strlen(str));
    }
}

inline void String::append(const char* str, size_t len) noexcept
{
    if (!str || len == 0) return;

    size_t newSize = mSize + len;
    if (newSize >= mCapacity) {
        reallocate(newSize + 1);
    }

    if (mData && mCapacity > newSize) {
        SDL_memcpy(mData + mSize, str, len);
        mSize = newSize;
        mData[mSize] = '\0';
    }
}

inline void String::append(const String& str) noexcept
{
    append(str.mData, str.mSize);
}

inline void String::push_back(char c) noexcept
{
    char temp[2] = {c, '\0'};
    append(temp, 1);
}

inline void String::pop_back() noexcept
{
    if (mSize > 0) {
        --mSize;
        if (mData) {
            mData[mSize] = '\0';
        }
    }
}

inline String String::substr(size_t pos, size_t len) const noexcept
{
    if (pos >= mSize || !mData) return String();

    size_t actualLen = len;
    if (len == npos || pos + len > mSize) {
        actualLen = mSize - pos;
    }

    return String(mData + pos, actualLen);
}

inline size_t String::find(const char* str, size_t pos) const noexcept
{
    if (!str || !mData || pos >= mSize) return npos;

    const char* result = SDL_strstr(mData + pos, str);
    if (!result) return npos;

    return static_cast<size_t>(result - mData);
}

inline size_t String::find(char c, size_t pos) const noexcept
{
    if (!mData || pos >= mSize) return npos;

    const char* result = SDL_strchr(mData + pos, c);
    if (!result) return npos;

    return static_cast<size_t>(result - mData);
}

inline size_t String::rfind(const char* str, size_t pos) const noexcept
{
    if (!str || !mData || mSize == 0) return npos;

    size_t strLen = safe_strlen(str);
    if (strLen == 0 || strLen > mSize) return npos;

    size_t searchPos = (pos == npos || pos >= mSize) ? mSize - strLen : pos;

    for (size_t i = searchPos + 1; i > 0; --i) {
        size_t checkPos = i - 1;
        if (checkPos + strLen > mSize) continue;
        if (SDL_memcmp(mData + checkPos, str, strLen) == 0) {
            return checkPos;
        }
    }

    return npos;
}

inline size_t String::rfind(char c, size_t pos) const noexcept
{
    if (!mData || mSize == 0) return npos;

    size_t searchPos = (pos == npos || pos >= mSize) ? mSize - 1 : pos;

    for (size_t i = searchPos + 1; i > 0; --i) {
        if (mData[i - 1] == c) {
            return i - 1;
        }
    }

    return npos;
}

inline bool String::contains(const char* str) const noexcept
{
    return find(str) != npos;
}

inline bool String::contains(char c) const noexcept
{
    return find(c) != npos;
}

inline bool String::starts_with(const char* prefix) const noexcept
{
    if (!prefix || !mData) return false;

    size_t prefixLen = safe_strlen(prefix);
    if (prefixLen > mSize) return false;

    return SDL_memcmp(mData, prefix, prefixLen) == 0;
}

inline bool String::ends_with(const char* suffix) const noexcept
{
    if (!suffix || !mData) return false;

    size_t suffixLen = safe_strlen(suffix);
    if (suffixLen > mSize) return false;

    return SDL_memcmp(mData + mSize - suffixLen, suffix, suffixLen) == 0;
}

inline String& String::replace(size_t pos, size_t len, const char* str) noexcept
{
    if (!str || pos >= mSize || !mData) return *this;

    size_t replaceLen = safe_strlen(str);
    size_t actualLen = (pos + len > mSize) ? mSize - pos : len;

    if (replaceLen == actualLen) {
        SDL_memcpy(mData + pos, str, replaceLen);
    }
    else {
        size_t newSize = mSize - actualLen + replaceLen;

        if (newSize >= mCapacity) {
            char* newData = NX_Malloc<char>(newSize + 1);
            if (!newData) return *this;

            SDL_memcpy(newData, mData, pos);
            SDL_memcpy(newData + pos, str, replaceLen);
            SDL_memcpy(newData + pos + replaceLen, mData + pos + actualLen, mSize - pos - actualLen);
            newData[newSize] = '\0';

            NX_Free(mData);
            mData = newData;
            mSize = newSize;
            mCapacity = newSize + 1;
        }
        else {
            SDL_memmove(mData + pos + replaceLen, mData + pos + actualLen, mSize - pos - actualLen);
            SDL_memcpy(mData + pos, str, replaceLen);
            mSize = newSize;
            mData[mSize] = '\0';
        }
    }

    return *this;
}

inline String& String::replace_all(const char* from, const char* to) noexcept
{
    if (!from || !to || !mData) return *this;

    size_t fromLen = safe_strlen(from);
    if (fromLen == 0) return *this;

    size_t pos = 0;
    while ((pos = find(from, pos)) != npos) {
        replace(pos, fromLen, to);
        pos += safe_strlen(to);
    }

    return *this;
}

inline String& String::replace_first(const char* from, const char* to) noexcept
{
    if (!from || !to || !mData) return *this;

    size_t pos = find(from);
    if (pos != npos) {
        replace(pos, safe_strlen(from), to);
    }

    return *this;
}

inline String& String::trim() noexcept
{
    trim_left();
    trim_right();
    return *this;
}

inline String& String::trim_left() noexcept
{
    if (!mData || mSize == 0) return *this;

    size_t start = 0;
    while (start < mSize && (mData[start] == ' ' || mData[start] == '\t' || 
                             mData[start] == '\n' || mData[start] == '\r')) {
        ++start;
    }

    if (start > 0) {
        mSize -= start;
        SDL_memmove(mData, mData + start, mSize);
        mData[mSize] = '\0';
    }

    return *this;
}

inline String& String::trim_right() noexcept
{
    if (!mData || mSize == 0) return *this;

    while (mSize > 0 && (mData[mSize - 1] == ' ' || mData[mSize - 1] == '\t' || 
                         mData[mSize - 1] == '\n' || mData[mSize - 1] == '\r')) {
        --mSize;
    }

    mData[mSize] = '\0';
    return *this;
}

inline String String::trimmed() const noexcept
{
    String result(*this);
    result.trim();
    return result;
}

inline String& String::to_lower() noexcept
{
    if (!mData) return *this;

    for (size_t i = 0; i < mSize; ++i) {
        if (mData[i] >= 'A' && mData[i] <= 'Z') {
            mData[i] += 32;
        }
    }

    return *this;
}

inline String& String::to_upper() noexcept
{
    if (!mData) return *this;

    for (size_t i = 0; i < mSize; ++i) {
        if (mData[i] >= 'a' && mData[i] <= 'z') {
            mData[i] -= 32;
        }
    }

    return *this;
}

inline String String::lower() const noexcept
{
    String result(*this);
    result.to_lower();
    return result;
}

inline String String::upper() const noexcept
{
    String result(*this);
    result.to_upper();
    return result;
}

inline void String::swap(String& other) noexcept
{
    char* tempData = mData;
    size_t tempSize = mSize;
    size_t tempCapacity = mCapacity;

    mData = other.mData;
    mSize = other.mSize;
    mCapacity = other.mCapacity;

    other.mData = tempData;
    other.mSize = tempSize;
    other.mCapacity = tempCapacity;
}

/* === Private Implementation === */

inline void String::allocate(size_t size) noexcept
{
    mCapacity = size + 1;
    mData = NX_Malloc<char>(mCapacity);
    if (mData) {
        mData[0] = '\0';
    }
    else {
        mCapacity = 0;
        mSize = 0;
    }
}

inline void String::reallocate(size_t newCapacity) noexcept
{
    if (newCapacity == 0) {
        free_data();
        return;
    }

    char* newData = NX_Realloc<char>(mData, newCapacity + 1);
    if (newData) {
        mData = newData;
        mCapacity = newCapacity + 1;
        if (mSize >= mCapacity) {
            mSize = mCapacity - 1;
        }
        mData[mSize] = '\0';
    }
}

inline void String::free_data() noexcept
{
    if (mData) {
        NX_Free(mData);
        mData = nullptr;
    }
    mSize = 0;
    mCapacity = 0;
}

inline size_t String::safe_strlen(const char* str) noexcept
{
    if (!str) return 0;
    return SDL_strlen(str);
}

} // namespace util

#endif // NX_UTIL_STRING_HPP
