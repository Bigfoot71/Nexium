#ifndef HP_UTIL_OBJECT_RING_HPP
#define HP_UTIL_OBJECT_RING_HPP

#include <SDL3/SDL_stdinc.h>
#include <array>

namespace util {

/* === Declaration === */

template<typename T, size_t N>
class ObjectRing {
    static_assert(N > 0, "N must be greater than 0");

public:
    /** Prend les parametres de construction et le construit N vois */
    template<typename... Args>
    explicit ObjectRing(Args&&... args);

    /** Operateur d'accés à l'objet actif */
    const T& operator*() const noexcept;
    T& operator*() noexcept;

    /** Operateur d'accés aux membres de l'objet actif */
    const T* operator->() const noexcept;
    T* operator->() noexcept;

    /** Faire tourner l'objet actif vers l'objet suivant */
    void rotate();

    /** Taille du ring */
    constexpr size_t size() const;

private:
    template<typename... Args, size_t... Is>
    inline ObjectRing(std::index_sequence<Is...>, Args&&... args);

private:
    std::array<T, N> mObjects;
    size_t mActiveIndex;
};

/* === Public Implementation === */

template<typename T, size_t N>
template<typename... Args>
ObjectRing<T, N>::ObjectRing(Args&&... args)
    : ObjectRing(std::make_index_sequence<N>{}, std::forward<Args>(args)...)
{ }

template<typename T, size_t N>
const T& ObjectRing<T, N>::operator*() const noexcept
{
    return mObjects[mActiveIndex];
}

template<typename T, size_t N>
T& ObjectRing<T, N>::operator*() noexcept
{
    return mObjects[mActiveIndex];
}

template<typename T, size_t N>
T* ObjectRing<T, N>::operator->() noexcept
{
    return &mObjects[mActiveIndex];
}

template<typename T, size_t N>
const T* ObjectRing<T, N>::operator->() const noexcept
{
    return &mObjects[mActiveIndex];
}

template<typename T, size_t N>
void ObjectRing<T, N>::rotate()
{
    mActiveIndex = (mActiveIndex + 1) % N;
}

template<typename T, size_t N>
constexpr size_t ObjectRing<T, N>::size() const
{
    return N;
}

/* === Private Implementation === */

template<typename T, size_t N>
template<typename... Args, size_t... Is>
ObjectRing<T, N>::ObjectRing(std::index_sequence<Is...>, Args&&... args)
    : mObjects{ ((void)Is, T(std::forward<Args>(args)...))... }
    , mActiveIndex(0)
{ }

} // namespace util

#endif // HP_UTIL_OBJECT_RING_HPP
