#include <Hyperion/Hyperion.h>
#include "./common.h"

enum AudioFormat {
    WAV, FLAC, MP3, OGG, COUNT
};

static const char* gFormats[] = {
    ".wav", ".flac",
    ".mp3", ".ogg"
};

int main(void)
{
    HP_Init("Hyperion - Streams", 800, 450, HP_FLAG_VSYNC_HINT);
    HP_AddSearchPath(RESOURCES_PATH, false);

    HP_AudioStream* streams[COUNT] = { 0 };
    for (int i = 0; i < COUNT; i++) {
        streams[i] = HP_LoadAudioStream(CMN_FormatText("audio/sine%s", gFormats[i]));
    }

    // TODO: Adding HP_GetAudioStreamCurrentTime() ?
    float currentTime[COUNT] = { 0 };

    while (HP_FrameStep())
    {
        HP_Begin2D(NULL);

        HP_SetColor2D(HP_BLACK);
        HP_DrawRect2D(0, 0, HP_GetWindowWidth(), HP_GetWindowHeight());

        for (int i = 0; i < COUNT; i++)
        {
            bool isPlaying = HP_IsAudioStreamPlaying(streams[i]);

            if (HP_IsKeyJustPressed(HP_KEY_1 + i)) {
                if (isPlaying) {
                    HP_StopAudioStream(streams[i]);
                    currentTime[i] = 0.0f;
                }
                else {
                    HP_PlayAudioStream(streams[i]);
                }
            }

            float y = (i + 1) * 10 + i * 24;
            HP_SetColor2D(isPlaying ? HP_GREEN : HP_YELLOW);
            HP_DrawText2D(CMN_FormatText("KEY%i = %s", i + 1, gFormats[i]), HP_VEC2(10, y), 24, HP_VEC2_1(1));

            HP_DrawRectBorder2D(200, y + 3, 300, 24, 2);

            if (isPlaying) {
                currentTime[i] += HP_GetFrameTime();
                float t = currentTime[i] / HP_GetAudioStreamDuration(streams[i]);
                HP_DrawRect2D(200, y + 3, t * 300, 24);
            }

            if (currentTime[i] >= HP_GetAudioStreamDuration(streams[i])) {
                currentTime[i] = 0.0f;
            }
        }

        HP_End2D();
    }

    HP_Quit();

    return 0;
}
