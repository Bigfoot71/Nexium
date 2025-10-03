/* scene.frag -- Scene fragment shader override template
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

vec4 ALBEDO             = vec4(1.0);
vec3 EMISSION           = vec3(0.0);
float AO_LIGHT_AFFECT   = 0.0;
float OCCLUSION         = 1.0;
float ROUGHNESS         = 1.0;
float METALNESS         = 0.0;
vec3 NORMAL_MAP         = vec3(0.0, 0.0, 1.0);
float NORMAL_SCALE      = 1.0;

#define fragment() //< The macro can be replaced by user code

void FragmentOverride()
{
    // TODO: We shouldn't be sampling on behalf of the user
    //       but for now this is 100% consistent with vertex()

    ALBEDO = vColor * uMaterial.albedoColor * texture(uTexAlbedo, vTexCoord);

    EMISSION = uMaterial.emissionColor * texture(uTexEmission, vTexCoord).rgb;
    EMISSION *= uMaterial.emissionEnergy;

    AO_LIGHT_AFFECT = uMaterial.aoLightAffect;

    vec3 orm = texture(uTexORM, vTexCoord).rgb;
    OCCLUSION = uMaterial.occlusion * orm.x;
    ROUGHNESS = uMaterial.roughness * orm.y;
    METALNESS = uMaterial.metalness * orm.z;

    NORMAL_MAP = texture(uTexNormal, vTexCoord).rgb;
    NORMAL_SCALE = uMaterial.normalScale;

    fragment();
}
