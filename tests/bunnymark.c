/* bunnymark.c -- Benchmark test for 2D overlay performance
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/Nexium.h>
#include "./common.h"

/* --- Bunny Definition --- */

#define MAX_BUNNIES 500000

typedef struct {
    NX_Vec2 position;
    NX_Vec2 velocity;
    NX_Color color;
} Bunny;

static Bunny bunnies[MAX_BUNNIES];
static int bunnyCount = 0;

/* --- Bunny Functions --- */

static void Bunny_Init(Bunny* bunny, NX_Vec2 position)
{
    float th = NX_TAU * NX_RandFloat(NULL);
    float speed = NX_RandRangeFloat(NULL, 10.0f, 100.0f);

    bunny->position = position;
    bunny->velocity = NX_VEC2(speed * cosf(th), speed * sinf(th));
    bunny->color = NX_ColorFromHSV(360 * NX_RandFloat(NULL), 1, 1, 1);
}

static void Bunny_Update(Bunny* bunny, float delta)
{
    bunny->position = NX_Vec2Add(bunny->position, NX_Vec2Scale(bunny->velocity, delta));

    float w = NX_GetWindowWidth();
    float h = NX_GetWindowHeight();

    if (bunny->position.x < 0 || bunny->position.x > w)
        bunny->velocity.x *= -1, bunny->position.x = NX_CLAMP(bunny->position.x, 0, w);

    if (bunny->position.y < 0 || bunny->position.y > h)
        bunny->velocity.y *= -1, bunny->position.y = NX_CLAMP(bunny->position.y, 0, h);
}

static void Bunny_Draw(const Bunny* bunny)
{
    NX_SetColor2D(bunny->color);
    NX_DrawRect2D(bunny->position.x - 16, bunny->position.y - 16, 32, 32);
}

/* --- Main Program --- */

int main(void)
{
    /* --- Initialize application --- */

    NX_AppDesc desc = {
        .render2D.resolution = { 800, 450 },
        .targetFPS = 60
    };

    NX_InitEx("Nexium - BunnyMark", 800, 450, &desc);
    NX_AddSearchPath(RESOURCES_PATH, false);

    NX_Texture* texture = NX_LoadTexture("images/wabbit_alpha.png");
    NX_SetTexture2D(texture);

    /* --- Main loop --- */

    while (NX_FrameStep())
    {
        /* --- Update window title --- */

        NX_SetWindowTitle(CMN_FormatText(
            "Nexium - BunnyMark - Bunnies: %i - FPS: %i", bunnyCount, NX_GetFPS()
        ));

        float delta = NX_GetDeltaTime();

        /* --- Add new bunnies on mouse press --- */

        if (NX_IsMouseButtonPressed(NX_MOUSE_BUTTON_LEFT)) {
            NX_Vec2 pos = NX_GetMousePosition();
            for (int i = 0; i < 100 && bunnyCount < MAX_BUNNIES; i++)
                Bunny_Init(&bunnies[bunnyCount++], pos);
        }

        /* --- 2D Rendering --- */

        NX_Begin2D(NULL);
        for (int i = 0; i < bunnyCount; i++) {
            Bunny_Update(&bunnies[i], delta);
            Bunny_Draw(&bunnies[i]);
        }
        NX_End2D();
    }

    /* --- Cleanup --- */

    NX_DestroyTexture(texture);
    NX_Quit();

    return 0;
}
