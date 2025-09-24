/**
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided "as-is", without any express or implied warranty. In no event
 * will the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including commercial
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not claim that you
 *   wrote the original software. If you use this software in a product, an acknowledgment
 *   in the product documentation would be appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *   as being the original software.
 *
 *   3. This notice may not be removed or altered from any source distribution.
 */

#include "./Detail/Util/DynamicArray.hpp"
#include "./Detail/Util/ObjectPool.hpp"

#include <Hyperion/HP_Rand.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_time.h>
#include <cstdint>

/* === Random Generator State === */

namespace {

class PCG32 {
public:
    /** Lifecycle */
    static HP_RandGen createStacked(uint64_t seed);
    static HP_RandGen* createPooled(uint64_t seed);
    static void destroyPooled(HP_RandGen* generator);

    /** Getter, returns the pointed or default generator */
    static HP_RandGen& get(HP_RandGen* generator);

    /** Generator state update */
    static void setSeed(HP_RandGen* generator, uint32_t seed);
    static void setSeed(HP_RandGen& generator, uint32_t seed);
    static uint32_t next(HP_RandGen* generator);
    static uint32_t next(HP_RandGen& generator);

private:
    PCG32();
    static uint32_t rotr(uint32_t value, uint32_t rot);

private:
    static constexpr uint64_t MULT = 0x5851f42d4c957f2dULL;

private:
    static inline util::ObjectPool<HP_RandGen, 32> mPool;
    HP_RandGen mDefault;
};

/* === Public Implementation === */

HP_RandGen PCG32::createStacked(uint64_t seed)
{
    HP_RandGen generator{};
    setSeed(generator, seed);
    return generator;
}

HP_RandGen* PCG32::createPooled(uint64_t seed)
{
    return mPool.create(createStacked(seed));
}

void PCG32::destroyPooled(HP_RandGen* generator)
{
    mPool.destroy(generator);
}

HP_RandGen& PCG32::get(HP_RandGen* generator)
{
    static PCG32 state;
    return generator ? *generator : state.mDefault;
}

void PCG32::setSeed(HP_RandGen* generator, uint32_t seed)
{
    setSeed(get(generator), seed);
}

void PCG32::setSeed(HP_RandGen& generator, uint32_t seed)
{
    generator.state = 0U;
    generator.inc = (seed << 1u) | 1u;  // Make sure inc is odd
    PCG32::next(generator);
    generator.state += seed;
    PCG32::next(generator);
}

uint32_t PCG32::next(HP_RandGen* generator)
{
    return next(get(generator));
}

uint32_t PCG32::next(HP_RandGen& generator)
{
    uint64_t oldstate = generator.state;
    generator.state = oldstate * MULT + generator.inc;
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return rotr(xorshifted, rot);
}

/* === Private Implementation === */

PCG32::PCG32()
{
    uint64_t seed = 0;
    if (!SDL_GetCurrentTime(reinterpret_cast<SDL_Time*>(&seed))) {
        seed = 0x853c49e6748fea9bULL; // Default on failure
    }
    mDefault = createStacked(seed);
}

uint32_t PCG32::rotr(uint32_t value, uint32_t rot)
{
    return (value >> rot) | (value << ((-rot) & 31));
}

} // namespace

/* === Public API === */

HP_RandGen* HP_CreateRandGen(uint64_t seed)
{
    return PCG32::createPooled(seed);
}

HP_RandGen HP_CreateRandGenTemp(uint64_t seed)
{
    return PCG32::createStacked(seed);
}

void HP_DestroyRandGen(HP_RandGen* generator)
{
    PCG32::destroyPooled(generator);
}

void HP_SetRandGenSeed(HP_RandGen* generator, uint64_t seed)
{
    PCG32::setSeed(generator, seed);
}

bool HP_RandBool(HP_RandGen* generator)
{
    return (PCG32::next(generator) & 0x80000000) != 0;
}

int32_t HP_RandInt(HP_RandGen* generator)
{
    return static_cast<int32_t>(PCG32::next(generator));
}

uint32_t HP_RandUint(HP_RandGen* generator)
{
    return PCG32::next(generator);
}

float HP_RandFloat(HP_RandGen* generator)
{
    // Convert to float [0.0, 1.0] using 24-bit precision
    // Divide by 2^24 for uniform distribution
    return (PCG32::next(generator) >> 8) * 0x1.0p-24f;
}

int HP_RandRangeInt(HP_RandGen* generator, int min, int max)
{
    if (min >= max) return min;

    HP_RandGen& genRef = PCG32::get(generator);

    uint32_t umin = static_cast<uint32_t>(min);
    uint32_t umax = static_cast<uint32_t>(max);
    uint32_t range = umax - umin;

    uint32_t threshold = -range % range;

    uint32_t r;
    do r = PCG32::next(genRef);
    while (r < threshold);

    return min + static_cast<int>(r % range);
}

uint32_t HP_RandRangeUint(HP_RandGen* generator, uint32_t min, uint32_t max)
{
    if (min >= max) return min;

    HP_RandGen& genRef = PCG32::get(generator);

    uint32_t range = max - min;
    uint32_t threshold = -range % range;

    uint32_t r;
    do r = PCG32::next(genRef);
    while (r < threshold);

    return min + (r % range);
}

float HP_RandRangeFloat(HP_RandGen* generator, float min, float max)
{
    return min + (max - min) * HP_RandFloat(generator);
}

void HP_RandShuffle(HP_RandGen* generator, void* array, size_t elemSize, size_t count)
{
    if (!array || count <= 1 || elemSize == 0) {
        return;
    }

    HP_RandGen& genRef = PCG32::get(generator);

    const auto range = [&genRef](uint32_t min, uint32_t max) -> uint32_t
    {
        uint32_t range = max - min;
        uint32_t threshold = -range % range;

        uint32_t r;
        do r = PCG32::next(genRef);
        while (r < threshold);

        return min + (r % range);
    };

    char* arr = static_cast<char*>(array);

    if (elemSize <= 64) {
        char temp[64];
        for (size_t i = count - 1; i > 0; i--) {
            size_t j = range(0, i + 1);
            if (i != j) {
                char* iElem = arr + i * elemSize;
                char* jElem = arr + j * elemSize;
                SDL_memcpy(temp, iElem, elemSize);
                SDL_memcpy(iElem, jElem, elemSize);
                SDL_memcpy(jElem, temp, elemSize);
            }
        }
    }
    else {
        util::DynamicArray<char> temp(elemSize);
        for (size_t i = count - 1; i > 0; i--) {
            size_t j = range(0, i + 1);
            if (i != j) {
                char* iElem = arr + i * elemSize;
                char* jElem = arr + j * elemSize;
                SDL_memcpy(temp.data(), iElem, elemSize);
                SDL_memcpy(iElem, jElem, elemSize);
                SDL_memcpy(jElem, temp.data(), elemSize);
            }
        }
    }
}
