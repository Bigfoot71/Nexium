// Just a benchmark test for the overlay

#include <Hyperion/Hyperion.h>
#include "./common.h"

/* === Bunny Structure === */

#define MAX_BUNNIES 500000

typedef struct {
    HP_Vec2 position;
    HP_Vec2 velocity;
    float rotation;
    HP_Color color;
} Bunny;

static Bunny bunnies[MAX_BUNNIES];
static int bunnyCount = 0;

/* === Bunny Functions === */

static void Bunny_Init(Bunny* bunny, HP_Vec2 position);
static void Bunny_Update(Bunny* bunny, float delta);
static void Bunny_Draw(const Bunny* bunny);

/* === Program Text === */

int main(void)
{
    HP_Init("Hyperion - BunnyMark", 800, 450, HP_FLAG_VSYNC_HINT);
    HP_AddSearchPath(RESOURCES_PATH, false);

    HP_Texture* texture = HP_LoadTexture("images/wabbit_alpha.png");
    HP_SetTexture2D(texture);

    while(HP_FrameStep())
    {
        HP_SetWindowTitle(CMN_FormatText(
            "Hyperion - BunnyMark - Bunnies: %i - FPS: %i", bunnyCount, HP_GetFPS()
        ));

        float delta = HP_GetFrameTime();

        if (HP_IsMouseButtonPressed(HP_MOUSE_BUTTON_LEFT)) {
            HP_Vec2 position = HP_GetMousePosition();
            for (int i = 0; i < 100 && bunnyCount < MAX_BUNNIES; i++) {
                Bunny_Init(&bunnies[bunnyCount++], position);
            }
        }

        HP_Begin2D(NULL);
        {
            for (int i = 0; i < bunnyCount; i++) {
                Bunny_Update(&bunnies[i], delta);
                Bunny_Draw(&bunnies[i]);
            }
        }
        HP_End2D();
    }

    return 0;
}

/* === Bunny Functions === */

void Bunny_Init(Bunny* bunny, HP_Vec2 position)
{
    float th = HP_TAU * HP_RandFloat(NULL);
    float speed = HP_RandRangeFloat(NULL, 10.0f, 100.0f);

    bunny->position = position;
    bunny->velocity = HP_VEC2(speed * cosf(th), speed * sinf(th));
    bunny->rotation = th;
    bunny->color = HP_ColorFromHSV(360 * HP_RandFloat(NULL), 1, 1, 1);
}

void Bunny_Update(Bunny* bunny, float delta)
{
    bunny->position = HP_Vec2Add(bunny->position, HP_Vec2Scale(bunny->velocity, delta));

    if (bunny->position.x < 0) {
        bunny->velocity.x *= -1;
        bunny->position.x = 0;
    }
    else if (bunny->position.x > HP_GetWindowWidth()) {
        bunny->velocity.x *= -1;
        bunny->position.x = HP_GetWindowWidth();
    }

    if (bunny->position.y < 0) {
        bunny->velocity.y *= -1;
        bunny->position.y = 0;
    }
    else if (bunny->position.y > HP_GetWindowHeight()) {
        bunny->velocity.y *= -1;
        bunny->position.y = HP_GetWindowHeight();
    }
}

void Bunny_Draw(const Bunny* bunny)
{
    HP_SetColor2D(bunny->color);
    HP_DrawRectEx2D(bunny->position, HP_VEC2_1(16), HP_VEC2_1(0.5f), bunny->rotation);
}
