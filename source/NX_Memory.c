#include <NX/NX_Memory.h>
#include <SDL3/SDL_stdinc.h>

// ============================================================================
// PUBLIC API
// ============================================================================

void* NX_Malloc(size_t size)
{
    return SDL_malloc(size);
}

void* NX_Calloc(size_t nmemb, size_t size)
{
    return SDL_calloc(nmemb, size);
}

void* NX_Realloc(void* ptr, size_t size)
{
    return SDL_realloc(ptr, size);
}

void NX_Free(void* ptr)
{
    SDL_free(ptr);
}
