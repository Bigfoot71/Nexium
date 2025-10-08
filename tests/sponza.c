#include <NX/Nexium.h>
#include "./common.h"

#define RENDER_WIDTH 1920
#define RENDER_HEIGHT 1080

int main(void)
{
	NX_AppDesc desc = {
		.render3D.resolution.x = RENDER_WIDTH,
		.render3D.resolution.y = RENDER_HEIGHT,
		.render3D.sampleCount = 4,
		.targetFPS = 60
	};

	NX_InitEx("Nexium - Sponza", RENDER_WIDTH, RENDER_HEIGHT, &desc);
	NX_AddSearchPath(RESOURCES_PATH, false);

	NX_Model* model = NX_LoadModel("models/sponza.glb");

	NX_Mesh* sphere = NX_GenMeshSphere(0.1f, 16, 8);
	NX_Material matSphere = NX_GetDefaultMaterial();
	matSphere.shading = NX_SHADING_WIREFRAME;

	NX_Light* lights[4];
	lights[0] = NX_CreateLight(NX_LIGHT_DIR);
	NX_SetLightDirection(lights[0], NX_VEC3(0, -1, 0));
	NX_SetLightColor(lights[0], NX_COLOR(0.8f, 0.9f, 1.0f, 1.0f));
	NX_SetLightEnergy(lights[0], 5.0f);
	NX_SetShadowActive(lights[0], true);
	NX_SetShadowUpdateMode(lights[0], NX_SHADOW_UPDATE_MANUAL);
	NX_UpdateShadowMap(lights[0]);
	NX_SetLightActive(lights[0], true);

	for (int i = 1; i < NX_ARRAY_SIZE(lights); i++)
	{
		lights[i] = NX_CreateLight(NX_LIGHT_OMNI);

		NX_SetLightPosition(lights[i], NX_VEC3(
			NX_RandRangeFloat(NULL, -3, 3),
			NX_RandRangeFloat(NULL, 0, 6),
			NX_RandRangeFloat(NULL, -3, 3)
		));

		NX_SetLightColor(lights[i], NX_ColorFromHSV(
			360 * NX_RandFloat(NULL), 1, 1, 1
		));

		NX_SetShadowActive(lights[i], true);
		NX_SetShadowUpdateMode(lights[i], NX_SHADOW_UPDATE_MANUAL);
		NX_UpdateShadowMap(lights[i]);
		NX_SetLightActive(lights[i], true);
	}

	NX_Cubemap* skybox = NX_CreateCubemap(1024, NX_PIXEL_FORMAT_RGB16F);

	NX_GenerateSkybox(skybox, &(NX_Skybox) {
		.sunDirection = NX_VEC3(-1, -1, -1),
		.skyColorTop = NX_COLOR(0.5f, 0.75f, 1.0f, 1.0f),
		.skyColorHorizon = NX_COLOR(0.6f, 0.75f, 0.9f, 1.0f),
		.sunColor = NX_COLOR(1.0f, 0.95f, 0.8f, 1.0f),
		.groundColor = NX_COLOR(0.2f, 0.17f, 0.13f, 1.0f),
		.sunSize = 0.02f,
		.haze = 0.1f,
		.energy = 1.0f
	});

	NX_ReflectionProbe* skyprobe = NX_CreateReflectionProbe(skybox);

	NX_Camera camera = NX_GetDefaultCamera();
	NX_Environment env = NX_GetDefaultEnvironment();
	NX_BoundingBox bounds = (NX_BoundingBox){
		.min = NX_VEC3(-12, -12, -12),
		.max = NX_VEC3(12, 12, 12)
	};

	env.ambient = NX_COLOR_1(0.1f);
	env.background = NX_BLACK;
	env.bounds = bounds;
	env.bloom.mode = NX_BLOOM_MIX;
	env.bloom.strength = 0.12f;
	env.tonemap.mode = NX_TONEMAP_ACES;
	env.tonemap.exposure = 2.0f;
	env.tonemap.white = 8.0f;
	env.ssao.enabled = true;
	env.sky.intensity = 0.2f;
	env.sky.cubemap = skybox;
	env.sky.probe = skyprobe;

	while (NX_FrameStep())
	{
		if (NX_IsKeyJustPressed(NX_KEY_ESCAPE))
		{
			break;
		}

		CMN_UpdateCamera(&camera, NX_VEC3(0, 0, 0), 8.0f, 4.0f);

		NX_Begin3D(&camera, &env, NULL);
		NX_DrawModel3D(model, NULL);

		NX_Transform transform = NX_TRANSFORM_IDENTITY;

		for (int i = 1; i < NX_ARRAY_SIZE(lights); i++)
		{
			transform.translation = NX_GetLightPosition(lights[i]);
			matSphere.albedo.color = NX_GetLightColor(lights[i]);
			NX_DrawMesh3D(sphere, &matSphere, &transform);
		}

		NX_End3D();

		NX_Begin2D(NULL);
		NX_SetFont2D(NULL);
		NX_SetColor2D(NX_WHITE);
		NX_DrawText2D("Sponza Demo -- Press ESC to exit", NX_VEC2(10, 10), 32, NX_VEC2(2, 2));
		NX_DrawText2D(CMN_FormatText("FPS: %i", NX_GetFPS()), NX_VEC2(10, 40), 32, NX_VEC2(2, 2));

		NX_End2D();
	}

	NX_Quit();
	return 0;
}