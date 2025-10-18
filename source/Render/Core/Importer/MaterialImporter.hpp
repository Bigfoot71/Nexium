#ifndef NX_RENDER_MATERIAL_IMPORTER_HPP
#define NX_RENDER_MATERIAL_IMPORTER_HPP

#include <NX/NX_Render.h>

#include "./Detail/TextureLoader.hpp"
#include "./SceneImporter.hpp"
#include "./AssimpHelper.hpp"
#include "../PoolTexture.hpp"

#include <assimp/GltfMaterial.h>
#include <assimp/material.h>
#include <SDL3/SDL_assert.h>

namespace render {

/* === Declaration === */

class MaterialImporter {
public:
    /** Constructors */
    MaterialImporter(const SceneImporter& importer, PoolTexture& poolTexture);

    /** Loads the materials and stores them in the specified model */
    bool loadMaterials(NX_Model* model);

private:
    /** Loads a material into memory */
    void loadMaterial(NX_Material* material, int index);

private:
    const SceneImporter& mImporter;
    TextureLoader mTextureLoader;
};

/* === Public Implementation === */

inline MaterialImporter::MaterialImporter(const SceneImporter& importer, PoolTexture& poolTexture)
    : mImporter(importer), mTextureLoader(importer, poolTexture)
{
    SDL_assert(importer.isValid());
}

inline bool MaterialImporter::loadMaterials(NX_Model* model)
{
    model->materialCount = mImporter.materialCount();
    model->materials = static_cast<NX_Material*>(SDL_malloc(model->materialCount * sizeof(NX_Material)));
    if (model->materials == nullptr) {
        NX_INTERNAL_LOG(E, "RENDER: Unable to allocate memory for materials; The model will be invalid");
        return false;
    }

    for (size_t i = 0; i < model->materialCount; i++) {
        loadMaterial(&model->materials[i], i);
    }

    return true;
}

/* === Private Material === */

inline void MaterialImporter::loadMaterial(NX_Material* material, int index)
{
    const aiMaterial* aiMat = mImporter.material(index);

    /* --- Initialize material defaults --- */

    *material = NX_GetDefaultMaterial();

    /* --- Load albedo map --- */

    material->albedo.texture = mTextureLoader.get(index, TextureLoader::MAP_ALBEDO);

    aiColor4D color;
    if (aiMat->Get(AI_MATKEY_BASE_COLOR, color) == AI_SUCCESS) {
        material->albedo.color = assimp_cast<NX_Color>(color);
    }
    else if (aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
        material->albedo.color = assimp_cast<NX_Color>(color);
    }

    /* --- Load the opacity factor --- */

    if (material->albedo.color.a >= 1.0f) {
        float opacity;
        if (aiMat->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS) {
            material->albedo.color.a = opacity;
        }
        else if (aiMat->Get(AI_MATKEY_TRANSPARENCYFACTOR, opacity) == AI_SUCCESS) {
            material->albedo.color.a = 1.0f - opacity;
        }
        // Indicates light passes through aiMat (glass, transparent plastics)
        else if (aiMat->Get(AI_MATKEY_TRANSMISSION_FACTOR, opacity) == AI_SUCCESS) {
            material->albedo.color.a = 1.0f - opacity;
        }
    }

    /* --- Load emission map --- */

    material->emission.texture = mTextureLoader.get(index, TextureLoader::MAP_EMISSION);
    if (material->emission.texture != nullptr) {
        material->emission.energy = 1.0f;
    }

    aiColor4D emissionColor;
    if (aiMat->Get(AI_MATKEY_COLOR_EMISSIVE, emissionColor) == AI_SUCCESS) {
        material->emission.color = assimp_cast<NX_Color>(emissionColor);
        material->emission.energy = 1.0f;
    }

    /* --- Load ORM map --- */

    material->orm.texture = mTextureLoader.get(index, TextureLoader::MAP_ORM);

    float roughness;
    if (aiMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS) {
        material->orm.roughness = roughness;
    }

    float metalness;
    if (aiMat->Get(AI_MATKEY_METALLIC_FACTOR, metalness) == AI_SUCCESS) {
        material->orm.metalness = metalness;
    }

    /* --- Load normal map --- */

    material->normal.texture = mTextureLoader.get(index, TextureLoader::MAP_NORMAL);
    if (material->normal.texture != nullptr) {
        float normalScale;
        if (aiMat->Get(AI_MATKEY_BUMPSCALING, normalScale) == AI_SUCCESS) {
            material->normal.scale = normalScale;
        }
    }

    /* --- Handle glTF alpha cutoff --- */

    float alphaCutOff;
    if (aiMat->Get(AI_MATKEY_GLTF_ALPHACUTOFF, alphaCutOff) == AI_SUCCESS) {
        material->alphaCutOff = alphaCutOff;
        material->depth.prePass = true;
    }

    /* --- Handle shading mode --- */

    aiShadingMode shadingMode{};
    if (aiMat->Get(AI_MATKEY_SHADING_MODEL, shadingMode) == AI_SUCCESS) {
        if (shadingMode == aiShadingMode_Unlit) {
            material->shading = NX_SHADING_UNLIT;
        }
    }

    /* --- Handle glTF alpha mode --- */

    aiString alphaMode;
    if (aiMat->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode) == AI_SUCCESS) {
        if (SDL_strcmp(alphaMode.data, "MASK") == 0) {
            // This means alphaCutOff should be used
            material->depth.prePass = true;
        }
        else if (strcmp(alphaMode.C_Str(), "BLEND") == 0) {
            material->blend = NX_BLEND_ALPHA;
        }
    }

    /* --- Handle blend function override --- */

    aiBlendMode blendFunc;
    if (aiMat->Get(AI_MATKEY_BLEND_FUNC, blendFunc) == AI_SUCCESS) {
        switch (blendFunc) {
        case aiBlendMode_Default:
            material->blend = NX_BLEND_ALPHA;
            break;
        case aiBlendMode_Additive:
            material->blend = NX_BLEND_ADD;
            break;
        default:
            break;
        }
    }

    /* --- Handle cull mode from two-sided property --- */

    bool twoSided;
    if (aiMat->Get(AI_MATKEY_TWOSIDED, twoSided) == AI_SUCCESS) {
        if (twoSided) material->cull = NX_CULL_NONE;
    }
}

} // namespace render

#endif // NX_RENDER_MATERIAL_IMPORTER_HPP
