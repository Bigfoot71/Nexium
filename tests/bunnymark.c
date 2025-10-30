// Just a benchmark test for the overlay

#include <NX/Nexium.h>
#include "./common.h"

/* === Bunny Structure === */

#define MAX_BUNNIES 500000

typedef struct {
    NX_Vec2 position;
    NX_Vec2 velocity;
    NX_Color color;
} Bunny;

static Bunny bunnies[MAX_BUNNIES];
static int bunnyCount = 0;

/* === Bunny Functions === */

static void Bunny_Init(Bunny* bunny, NX_Vec2 position);
static void Bunny_Update(Bunny* bunny, float delta);
static void Bunny_Draw(const Bunny* bunny);

/* === Program Text === */

int main(void)
{
    NX_AppDesc desc = {
        .render2D.resolution.x = 800,
        .render2D.resolution.y = 450,
        .targetFPS = 60
    };

    NX_InitEx("Nexium - BunnyMark", 800, 450, &desc);
    NX_AddSearchPath(RESOURCES_PATH, false);

    NX_Texture* texture = NX_LoadTexture("images/wabbit_alpha.png");
    NX_SetTexture2D(texture);

    while(NX_FrameStep())
    {
        NX_SetWindowTitle(CMN_FormatText(
            "Nexium - BunnyMark - Bunnies: %i - FPS: %i", bunnyCount, NX_GetFPS()
        ));

        float delta = NX_GetDeltaTime();

        if (NX_IsMouseButtonPressed(NX_MOUSE_BUTTON_LEFT)) {
            NX_Vec2 position = NX_GetMousePosition();
            for (int i = 0; i < 100 && bunnyCount < MAX_BUNNIES; i++) {
                Bunny_Init(&bunnies[bunnyCount++], position);
            }
        }

        NX_Begin2D(NULL);
        {
            for (int i = 0; i < bunnyCount; i++) {
                Bunny_Update(&bunnies[i], delta);
                Bunny_Draw(&bunnies[i]);
            }
        }
        NX_End2D();
    }

    return 0;
}

/* === Bunny Functions === */

void Bunny_Init(Bunny* bunny, NX_Vec2 position)
{
    float th = NX_TAU * NX_RandFloat(NULL);
    float speed = NX_RandRangeFloat(NULL, 10.0f, 100.0f);

    bunny->position = position;
    bunny->velocity = NX_VEC2(speed * cosf(th), speed * sinf(th));
    bunny->color = NX_ColorFromHSV(360 * NX_RandFloat(NULL), 1, 1, 1);
}

void Bunny_Update(Bunny* bunny, float delta)
{
    bunny->position = NX_Vec2Add(bunny->position, NX_Vec2Scale(bunny->velocity, delta));

    if (bunny->position.x < 0) {
        bunny->velocity.x *= -1;
        bunny->position.x = 0;
    }
    else if (bunny->position.x > NX_GetWindowWidth()) {
        bunny->velocity.x *= -1;
        bunny->position.x = NX_GetWindowWidth();
    }

    if (bunny->position.y < 0) {
        bunny->velocity.y *= -1;
        bunny->position.y = 0;
    }
    else if (bunny->position.y > NX_GetWindowHeight()) {
        bunny->velocity.y *= -1;
        bunny->position.y = NX_GetWindowHeight();
    }
}

void Bunny_Draw(const Bunny* bunny)
{
    NX_SetColor2D(bunny->color);
    NX_DrawRect2D(bunny->position.x - 16, bunny->position.y - 16, 32, 32);
}
