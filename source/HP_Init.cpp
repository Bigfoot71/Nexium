/* HP_Init.cpp -- API definition for Hyperion initialization
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <Hyperion/HP_Init.h>
#include <Hyperion/HP_Core.h>
#include <Hyperion/HP_Rand.h>

#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_time.h>

#include "./Core/HP_InternalLog.hpp"

#include "./Core/HP_CoreState.hpp"
#include "./Audio/HP_AudioState.hpp"
#include "./Render/HP_RenderState.hpp"

#include <memory>

/* === Globals === */

std::unique_ptr<HP_CoreState> gCore;
std::unique_ptr<HP_AudioState> gAudio;
std::unique_ptr<HP_RenderState> gRender;

/* === Public API === */

bool HP_Init(const char* title, int w, int h, HP_Flags flags)
{
    HP_AppDesc desc{
        .flags = flags,
        .name = nullptr,
        .version = nullptr,
        .identifier = nullptr,
        .memory = {}
    };

    return HP_InitEx(title, w, h, &desc);
}

bool HP_InitEx(const char* title, int w, int h, HP_AppDesc* desc)
{
    /* --- Ensures that the application description is valid --- */

    if (desc == nullptr) {
        HP_INTERNAL_LOG(E, "CORE: Failed to initialize Hyperion; App description cannot be null");
        return false;
    }

    /* --- Init each modules --- */

    try {
        gCore = std::make_unique<HP_CoreState>(title, w, h, *desc);
        gAudio = std::make_unique<HP_AudioState>();
        gRender = std::make_unique<HP_RenderState>(*desc);
    }
    catch (const std::exception& e) {
        HP_INTERNAL_LOG(E, e.what());
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

void HP_Quit(void)
{
    gRender.reset();
    gAudio.reset();
    gCore.reset();
}
