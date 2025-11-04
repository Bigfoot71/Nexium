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
    char GetAt(size_t index) const noexcept;

    /** Getters */
    const char* GetCString() const noexcept;
    size_t GetSize() const noexcept;
    size_t GetLength() const noexcept;
    bool IsEmpty() const noexcept;

    /** Capacity */
    size_t GetCapacity() const noexcept;
    void Reserve(size_t newCapacity) noexcept;
    void ShrinkToFit() noexcept;

    /** Modifiers */
    void Clear() noexcept;
    void Append(const char* str) noexcept;
    void Append(const char* str, size_t len) noexcept;
    void Append(const String& str) noexcept;
    void PushBack(char c) noexcept;
    void PopBack() noexcept;

    /** String operations */
    String Substring(size_t pos, size_t len = static_cast<size_t>(-1)) const noexcept;
    size_t Find(const char* str, size_t pos = 0) const noexcept;
    size_t Find(char c, size_t pos = 0) const noexcept;
    size_t FindLast(const char* str, size_t pos = static_cast<size_t>(-1)) const noexcept;
    size_t FindLast(char c, size_t pos = static_cast<size_t>(-1)) const noexcept;
    bool Contains(const char* str) const noexcept;
    bool Contains(char c) const noexcept;
    bool StartsWith(const char* prefix) const noexcept;
    bool EndsWith(const char* suffix) const noexcept;

    /** Replace operations */
    String& Replace(size_t pos, size_t len, const char* str) noexcept;
    String& ReplaceAll(const char* from, const char* to) noexcept;
    String& ReplaceFirst(const char* from, const char* to) noexcept;

    /** Trim operations */
    String& Trim() noexcept;
    String& TrimLeft() noexcept;
    String& TrimRight() noexcept;
    String TrimmedCopy() const noexcept;

    /** Case conversion */
    String& ToLower() noexcept;
    String& ToUpper() noexcept;
    String LowerCopy() const noexcept;
    String UpperCopy() const noexcept;

    /** Utility */
    void Swap(String& other) noexcept;

    /** Static constants */
    static constexpr size_t npos = static_cast<size_t>(-1);

private:
    char* mData{};
    size_t mSize{};
    size_t mCapacity{};

    /** Helper functions */
    void Allocate(size_t size) noexcept;
    void Reallocate(size_t newCapacity) noexcept;
    void FreeData() noexcept;
    static size_t MeasureCString(const char* str) noexcept;
};

/* === Public Implementation === */

inline String::String() noexcept
    : mData(nullptr), mSize(0), mCapacity(0)
{ }

inline String::String(const char* str) noexcept
    : mData(nullptr), mSize(0), mCapacity(0)
{
    if (str) {
        mSize = MeasureCString(str);
        if (mSize > 0) {
            Allocate(mSize);
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
        Allocate(mSize);
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
        Allocate(mSize);
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
    FreeData();
}

inline String& String::operator=(const String& other) noexcept
{
    if (this != &other) {
        FreeData();
        if (other.mSize > 0 && other.mData) {
            mSize = other.mSize;
            Allocate(mSize);
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
        FreeData();
        mData = std::exchange(other.mData, nullptr);
        mSize = std::exchange(other.mSize, 0);
        mCapacity = std::exchange(other.mCapacity, 0);
    }
    return *this;
}

inline String& String::operator=(const char* str) noexcept
{
    FreeData();
    if (str) {
        mSize = MeasureCString(str);
        if (mSize > 0) {
            Allocate(mSize);
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
    Append(other.mData, other.mSize);
    return *this;
}

inline String& String::operator+=(const char* str) noexcept
{
    Append(str);
    return *this;
}

inline String& String::operator+=(char c) noexcept
{
    PushBack(c);
    return *this;
}

inline char String::operator[](size_t index) const noexcept
{
    if (index >= mSize || !mData) return '\0';
    return mData[index];
}

inline char String::GetAt(size_t index) const noexcept
{
    return (*this)[index];
}

inline const char* String::GetCString() const noexcept
{
    return mData ? mData : "";
}

inline size_t String::GetSize() const noexcept
{
    return mSize;
}

inline size_t String::GetLength() const noexcept
{
    return mSize;
}

inline bool String::IsEmpty() const noexcept
{
    return mSize == 0;
}

inline size_t String::GetCapacity() const noexcept
{
    return mCapacity;
}

inline void String::Reserve(size_t newCapacity) noexcept
{
    if (newCapacity > mCapacity) {
        Reallocate(newCapacity);
    }
}

inline void String::ShrinkToFit() noexcept
{
    if (mCapacity > mSize) {
        Reallocate(mSize);
    }
}

inline void String::Clear() noexcept
{
    if (mData) {
        mData[0] = '\0';
    }
    mSize = 0;
}

inline void String::Append(const char* str) noexcept
{
    if (str) {
        Append(str, MeasureCString(str));
    }
}

inline void String::Append(const char* str, size_t len) noexcept
{
    if (!str || len == 0) return;

    size_t newSize = mSize + len;
    if (newSize >= mCapacity) {
        Reallocate(newSize + 1);
    }

    if (mData && mCapacity > newSize) {
        SDL_memcpy(mData + mSize, str, len);
        mSize = newSize;
        mData[mSize] = '\0';
    }
}

inline void String::Append(const String& str) noexcept
{
    Append(str.mData, str.mSize);
}

inline void String::PushBack(char c) noexcept
{
    char temp[2] = {c, '\0'};
    Append(temp, 1);
}

inline void String::PopBack() noexcept
{
    if (mSize > 0) {
        --mSize;
        if (mData) {
            mData[mSize] = '\0';
        }
    }
}

inline String String::Substring(size_t pos, size_t len) const noexcept
{
    if (pos >= mSize || !mData) return String();

    size_t actualLen = len;
    if (len == npos || pos + len > mSize) {
        actualLen = mSize - pos;
    }

    return String(mData + pos, actualLen);
}

inline size_t String::Find(const char* str, size_t pos) const noexcept
{
    if (!str || !mData || pos >= mSize) return npos;

    const char* result = SDL_strstr(mData + pos, str);
    if (!result) return npos;

    return static_cast<size_t>(result - mData);
}

inline size_t String::Find(char c, size_t pos) const noexcept
{
    if (!mData || pos >= mSize) return npos;

    const char* result = SDL_strchr(mData + pos, c);
    if (!result) return npos;

    return static_cast<size_t>(result - mData);
}

inline size_t String::FindLast(const char* str, size_t pos) const noexcept
{
    if (!str || !mData || mSize == 0) return npos;

    size_t strLen = MeasureCString(str);
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

inline size_t String::FindLast(char c, size_t pos) const noexcept
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

inline bool String::Contains(const char* str) const noexcept
{
    return Find(str) != npos;
}

inline bool String::Contains(char c) const noexcept
{
    return Find(c) != npos;
}

inline bool String::StartsWith(const char* prefix) const noexcept
{
    if (!prefix || !mData) return false;

    size_t prefixLen = MeasureCString(prefix);
    if (prefixLen > mSize) return false;

    return SDL_memcmp(mData, prefix, prefixLen) == 0;
}

inline bool String::EndsWith(const char* suffix) const noexcept
{
    if (!suffix || !mData) return false;

    size_t suffixLen = MeasureCString(suffix);
    if (suffixLen > mSize) return false;

    return SDL_memcmp(mData + mSize - suffixLen, suffix, suffixLen) == 0;
}

inline String& String::Replace(size_t pos, size_t len, const char* str) noexcept
{
    if (!str || pos >= mSize || !mData) return *this;

    size_t replaceLen = MeasureCString(str);
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

inline String& String::ReplaceAll(const char* from, const char* to) noexcept
{
    if (!from || !to || !mData) return *this;

    size_t fromLen = MeasureCString(from);
    if (fromLen == 0) return *this;

    size_t pos = 0;
    while ((pos = Find(from, pos)) != npos) {
        Replace(pos, fromLen, to);
        pos += MeasureCString(to);
    }

    return *this;
}

inline String& String::ReplaceFirst(const char* from, const char* to) noexcept
{
    if (!from || !to || !mData) return *this;

    size_t pos = Find(from);
    if (pos != npos) {
        Replace(pos, MeasureCString(from), to);
    }

    return *this;
}

inline String& String::Trim() noexcept
{
    TrimLeft();
    TrimRight();
    return *this;
}

inline String& String::TrimLeft() noexcept
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

inline String& String::TrimRight() noexcept
{
    if (!mData || mSize == 0) return *this;

    while (mSize > 0 && (mData[mSize - 1] == ' ' || mData[mSize - 1] == '\t' || 
                         mData[mSize - 1] == '\n' || mData[mSize - 1] == '\r')) {
        --mSize;
    }

    mData[mSize] = '\0';
    return *this;
}

inline String String::TrimmedCopy() const noexcept
{
    String result(*this);
    result.Trim();
    return result;
}

inline String& String::ToLower() noexcept
{
    if (!mData) return *this;

    for (size_t i = 0; i < mSize; ++i) {
        if (mData[i] >= 'A' && mData[i] <= 'Z') {
            mData[i] += 32;
        }
    }

    return *this;
}

inline String& String::ToUpper() noexcept
{
    if (!mData) return *this;

    for (size_t i = 0; i < mSize; ++i) {
        if (mData[i] >= 'a' && mData[i] <= 'z') {
            mData[i] -= 32;
        }
    }

    return *this;
}

inline String String::LowerCopy() const noexcept
{
    String result(*this);
    result.ToLower();
    return result;
}

inline String String::UpperCopy() const noexcept
{
    String result(*this);
    result.ToUpper();
    return result;
}

inline void String::Swap(String& other) noexcept
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

inline void String::Allocate(size_t size) noexcept
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

inline void String::Reallocate(size_t newCapacity) noexcept
{
    if (newCapacity == 0) {
        FreeData();
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

inline void String::FreeData() noexcept
{
    if (mData) {
        NX_Free(mData);
        mData = nullptr;
    }
    mSize = 0;
    mCapacity = 0;
}

inline size_t String::MeasureCString(const char* str) noexcept
{
    if (str != nullptr) {
        return SDL_strlen(str);
    }
    return 0;
}

} // namespace util

#endif // NX_UTIL_STRING_HPP
