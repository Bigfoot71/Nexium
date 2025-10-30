/* NX_Init.cpp -- API definition for Nexium initialization
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/NX_Init.h>
#include <NX/NX_Core.h>
#include <NX/NX_Rand.h>
#include <NX/NX_Log.h>

#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_time.h>

#include "./Core/NX_CoreState.hpp"
#include "./Audio/NX_AudioState.hpp"
#include "./Render/NX_RenderState.hpp"

#include <memory>

/* === Globals === */

std::unique_ptr<NX_CoreState> gCore;
std::unique_ptr<NX_AudioState> gAudio;
std::unique_ptr<NX_RenderState> gRender;

/* === Public API === */

bool NX_Init(const char* title, int w, int h, NX_Flags flags)
{
    NX_AppDesc desc{
        .flags = flags,
        .name = nullptr,
        .version = nullptr,
        .identifier = nullptr,
        .memory = {}
    };

    return NX_InitEx(title, w, h, &desc);
}

bool NX_InitEx(const char* title, int w, int h, NX_AppDesc* desc)
{
    /* --- Ensures that the application description is valid --- */

    if (desc == nullptr) {
        NX_LOG(E, "CORE: Failed to initialize Nexium; App description cannot be null");
        return false;
    }

    /* --- Init each modules --- */

    try {
        gCore = std::make_unique<NX_CoreState>(title, w, h, *desc);
        gAudio = std::make_unique<NX_AudioState>();
        gRender = std::make_unique<NX_RenderState>(*desc);
    }
    catch (const std::exception& e) {
        NX_LOG(E, e.what());
        gRender.reset();
        gAudio.reset();
        gCore.reset();
        return false;
    }

    /* --- Get initial ticks to avoid huge startup delta --- */

    gCore->mTicksLast = SDL_GetPerformanceCounter();

    /* --- Oh yeaaaah :3 --- */

    return true;
}

void NX_Quit(void)
{
    gRender.reset();
    gAudio.reset();
    gCore.reset();
}
