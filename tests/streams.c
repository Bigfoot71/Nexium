#include <NX/Nexium.h>
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
    NX_Init("Nexium - Streams", 800, 450, NX_FLAG_VSYNC_HINT);
    NX_AddSearchPath(RESOURCES_PATH, false);

    NX_AudioStream* streams[COUNT] = { 0 };
    for (int i = 0; i < COUNT; i++) {
        streams[i] = NX_LoadAudioStream(CMN_FormatText("audio/sine%s", gFormats[i]));
    }

    // TODO: Adding NX_GetAudioStreamCurrentTime() ?
    float currentTime[COUNT] = { 0 };

    while (NX_FrameStep())
    {
        NX_Begin2D(NULL);

        NX_SetColor2D(NX_BLACK);
        NX_DrawRect2D(0, 0, NX_GetWindowWidth(), NX_GetWindowHeight());

        for (int i = 0; i < COUNT; i++)
        {
            bool isPlaying = NX_IsAudioStreamPlaying(streams[i]);

            if (NX_IsKeyJustPressed(NX_KEY_1 + i)) {
                if (isPlaying) {
                    NX_StopAudioStream(streams[i]);
                    currentTime[i] = 0.0f;
                }
                else {
                    NX_PlayAudioStream(streams[i]);
                }
            }

            float y = (i + 1) * 10 + i * 24;
            NX_SetColor2D(isPlaying ? NX_GREEN : NX_YELLOW);
            NX_DrawText2D(CMN_FormatText("KEY%i = %s", i + 1, gFormats[i]), NX_VEC2(10, y), 24, NX_VEC2_1(1));

            NX_DrawRectBorder2D(200, y + 3, 300, 24, 2);

            if (isPlaying) {
                currentTime[i] += NX_GetDeltaTime();
                float t = currentTime[i] / NX_GetAudioStreamDuration(streams[i]);
                NX_DrawRect2D(200, y + 3, t * 300, 24);
            }

            if (currentTime[i] >= NX_GetAudioStreamDuration(streams[i])) {
                currentTime[i] = 0.0f;
            }
        }

        NX_End2D();
    }

    for (int i = 0; i < COUNT; i++) {
        NX_DestroyAudioStream(streams[i]);
    }

    NX_Quit();

    return 0;
}
