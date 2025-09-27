/**
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided "as-is", without any express or implied warranty. In no event
 * will the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including commercial
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not claim that you
 *   wrote the original software. If you use this software in a product, an acknowledgment
 *   in the product documentation would be appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *   as being the original software.
 *
 *   3. This notice may not be removed or altered from any source distribution.
 */

#include "../PoolModel.hpp"
#include "./AssimpHelper.hpp"

#include <assimp/GltfMaterial.h>

/* === Helpers === */

namespace {

HP_TextureWrap getWrapMode(aiTextureMapMode wrap)
{
    HP_TextureWrap hpWrap = HP_TEXTURE_WRAP_CLAMP;

    switch (wrap) {
    case aiTextureMapMode_Wrap:
        hpWrap = HP_TEXTURE_WRAP_REPEAT;
        break;
    case aiTextureMapMode_Mirror:
        hpWrap = HP_TEXTURE_WRAP_MIRROR;
        break;
    case aiTextureMapMode_Clamp:
    case aiTextureMapMode_Decal:
    case _aiTextureMapMode_Force32Bit:
    default:
        break;
    }

    return hpWrap;
}

HP_Image loadImage(const aiScene* scene, const aiString& path, bool* isAllocated)
{
    HP_Image image{};

    *isAllocated = false;

    /* --- Handle embedded texture (starts with '*') --- */

    if (path.data[0] == '*')
    {
        int textureIndex = atoi(&path.data[1]);

        /* --- Validate embedded texture index --- */

        if (textureIndex < 0 || textureIndex >= (int)scene->mNumTextures) {
            return image;
        }

        const aiTexture* aiTex = scene->mTextures[textureIndex];

        /* --- Handle compressed embedded texture --- */

        if (aiTex->mHeight == 0) {
            image = HP_LoadImageFromMem(aiTex->pcData, aiTex->mWidth);
            *isAllocated = true;
        }

        /* --- Handle uncompressed (raw RGBA) embedded texture --- */

        else {
            image.w = aiTex->mWidth;
            image.h = aiTex->mHeight;
            image.format = HP_PIXEL_FORMAT_RGBA8;
            // NOTE: No need to copy the data here, the image will be immediately
            //       uploaded to the GPU without being retained afterward
            image.pixels = aiTex->pcData;
        }
    }

    /* --- Handle external texture from file --- */

    else {
        image = HP_LoadImage(path.data);
        if (image.pixels != nullptr) {
            *isAllocated = true;
        }
    }

    return image;
}

} // namespace

/* === Implementation === */

namespace render {

HP_Texture* PoolModel::loadTexture(const aiScene* scene, const aiMaterial* material, aiTextureType type, uint32_t index)
{
    /* --- Get texture info --- */

    aiTextureMapMode wrapMode{};
    aiString path{};

    if (material->GetTexture(type, index, &path, nullptr, nullptr, nullptr, nullptr, &wrapMode) != AI_SUCCESS) {
        return nullptr; // No texture of this type
    }

    /* --- Loads the texture into RAM --- */

    bool isAllocated = false;
    HP_Image image = loadImage(scene, path, &isAllocated);
    if (image.pixels == nullptr) {
        return nullptr;
    }

    /* --- Upload the texture to VRAM --- */

    HP_Texture* texture = mPoolTexture.createTexture(image, getWrapMode(wrapMode));
    if (isAllocated) {
        HP_DestroyImage(&image);
    }

    return texture;
}

HP_Texture* PoolModel::loadTextureORM(const aiScene* scene, const aiMaterial* material, bool* hasOcclusion, bool* hasRoughness, bool* hasMetalness)
{
    HP_Texture* ormTexture = nullptr;

    *hasOcclusion = false;
    *hasRoughness = false;
    *hasMetalness = false;

    /* --- Check for glTF combined metallic-roughness texture first --- */

    aiTextureMapMode gltfWrapMode;
    struct aiString gltfPath;

    if (material->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &gltfPath, nullptr, nullptr, nullptr, nullptr, &gltfWrapMode) == AI_SUCCESS)
    {
        bool gltfAllocated;
        HP_Image gltfImage = loadImage(scene, gltfPath, &gltfAllocated);

        if (gltfImage.pixels != nullptr)
        {
            *hasRoughness = true;
            *hasMetalness = true;

            // Load separate occlusion if available
            bool occlusionAllocated = false;
            HP_Image occlusionImage{};
            aiString occlusionPath{};
            if (material->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &occlusionPath, nullptr, nullptr, nullptr, nullptr, nullptr) == AI_SUCCESS) {
                occlusionImage = loadImage(scene, occlusionPath, &occlusionAllocated);
                *hasOcclusion = (occlusionImage.pixels != nullptr);
            }
            else if (material->GetTexture(aiTextureType_LIGHTMAP, 0, &occlusionPath, nullptr, nullptr, nullptr, nullptr, nullptr) == AI_SUCCESS) {
                occlusionImage = loadImage(scene, occlusionPath, &occlusionAllocated);
                *hasOcclusion = (occlusionImage.pixels != nullptr);
            }

            // Compose ORM: O=occlusion, R=gltf.green, M=gltf.blue
            const HP_Image* sources[3] = {
                occlusionImage.pixels ? &occlusionImage : nullptr, // Red channel
                &gltfImage,                                     // Green channel (roughness from glTF)
                &gltfImage                                      // Blue channel (metalness from glTF)
            };

            HP_Image ormImage = HP_ComposeImagesRGB(sources, HP_WHITE);

            if (ormImage.pixels != nullptr) {
                ormTexture = mPoolTexture.createTexture(ormImage, getWrapMode(gltfWrapMode));
                HP_DestroyImage(&ormImage);
            }

            // Cleanup
            if (gltfAllocated) {
                HP_DestroyImage(&gltfImage);
            }
            if (occlusionAllocated && occlusionImage.pixels) {
                HP_DestroyImage(&occlusionImage);
            }

            return ormTexture;
        }
    }

    /* --- Fallback: Load individual textures --- */

    HP_Image occlusionImage{};
    HP_Image roughnessImage{};
    HP_Image metalnessImage{};

    bool occlusionAllocated = false;
    bool roughnessAllocated = false;
    bool metalnessAllocated = false;

    aiTextureMapMode occlusionWrapMode{};
    aiTextureMapMode roughnessWrapMode{};
    aiTextureMapMode metalnessWrapMode{};

    // Load occlusion
    struct aiString occlusionPath;
    if (material->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &occlusionPath, nullptr, nullptr, nullptr, nullptr, &occlusionWrapMode) == AI_SUCCESS) {
        occlusionImage = loadImage(scene, occlusionPath, &occlusionAllocated);
        *hasOcclusion = (occlusionImage.pixels != nullptr);
    }
    else if (material->GetTexture(aiTextureType_LIGHTMAP, 0, &occlusionPath, nullptr, nullptr, nullptr, nullptr, &occlusionWrapMode) == AI_SUCCESS) {
        occlusionImage = loadImage(scene, occlusionPath, &occlusionAllocated);
        *hasOcclusion = (occlusionImage.pixels != nullptr);
    }

    // Load roughness (with shininess fallback)
    struct aiString roughnessPath;
    if (material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &roughnessPath, nullptr, nullptr, nullptr, nullptr, &roughnessWrapMode) == AI_SUCCESS) {
        roughnessImage = loadImage(scene, roughnessPath, &roughnessAllocated);
        *hasRoughness = (roughnessImage.pixels != nullptr);
    }
    else if (material->GetTexture(aiTextureType_SHININESS, 0, &roughnessPath, nullptr, nullptr, nullptr, nullptr, &roughnessWrapMode) == AI_SUCCESS) {
        roughnessImage = loadImage(scene, roughnessPath, &roughnessAllocated);
        if (roughnessImage.pixels) {
            *hasRoughness = true;
            HP_InvertImage(&roughnessImage); // Convert shininess to roughness
        }
    }

    // Load metalness
    struct aiString metalnessPath;
    if (material->GetTexture(aiTextureType_METALNESS, 0, &metalnessPath, nullptr, nullptr, nullptr, nullptr, &metalnessWrapMode) == AI_SUCCESS) {
        metalnessImage = loadImage(scene, metalnessPath, &metalnessAllocated);
        *hasMetalness = (metalnessImage.pixels != nullptr);
    }

    // Compose ORM using the utility function
    const HP_Image* sources[3] = {
        occlusionImage.pixels ? &occlusionImage : nullptr,   // Red channel
        roughnessImage.pixels ? &roughnessImage : nullptr,   // Green channel
        metalnessImage.pixels ? &metalnessImage : nullptr    // Blue channel
    };

    HP_Image ormImage = HP_ComposeImagesRGB(sources, HP_WHITE);

    if (ormImage.pixels)
    {
        aiTextureMapMode wrapMode = aiTextureMapMode_Clamp;

        if (sources[1]) wrapMode = roughnessWrapMode;
        else if (sources[2]) wrapMode = metalnessWrapMode;
        else if (sources[0]) wrapMode = occlusionWrapMode;

        ormTexture = mPoolTexture.createTexture(ormImage, getWrapMode(wrapMode));
        HP_DestroyImage(&ormImage);
    }

    // Cleanup
    if (occlusionAllocated && occlusionImage.pixels) HP_DestroyImage(&occlusionImage);
    if (roughnessAllocated && roughnessImage.pixels) HP_DestroyImage(&roughnessImage);
    if (metalnessAllocated && metalnessImage.pixels) HP_DestroyImage(&metalnessImage);

    return ormTexture;
}

bool PoolModel::processMaterials(HP_Model* model, const aiScene* scene)
{
    /* --- Allocate materials array --- */

    model->materialCount = scene->mNumMaterials;
    model->materials = static_cast<HP_Material*>(SDL_malloc(model->materialCount * sizeof(HP_Material)));
    if (model->materials == nullptr) {
        HP_INTERNAL_LOG(E, "RENDER: Unable to allocate memory for materials; The model will be invalid");
        return false;
    }

    /* --- Process each material --- */

    for (size_t i = 0; i < model->materialCount; i++)
    {
        const aiMaterial* material = scene->mMaterials[i];
        HP_Material& modelMaterial = model->materials[i];

        /* --- Initialize material defaults --- */

        modelMaterial = HP_GetDefaultMaterial();

        /* --- Load the albedo color --- */

        aiColor4D color;
        if (material->Get(AI_MATKEY_BASE_COLOR, color) == AI_SUCCESS) {
            modelMaterial.albedo.color = assimp_cast<HP_Color>(color);
        }
        else if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
            modelMaterial.albedo.color = assimp_cast<HP_Color>(color);
        }

        /* --- Load the opacity factor --- */

        if (modelMaterial.albedo.color.a >= 1.0f) {
            float opacity;
            if (material->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS) {
                modelMaterial.albedo.color.a = opacity;
            }
            else if (material->Get(AI_MATKEY_TRANSPARENCYFACTOR, opacity) == AI_SUCCESS) {
                modelMaterial.albedo.color.a = 1.0f - opacity;
            }
            // Indicates light passes through material (glass, transparent plastics)
            else if (material->Get(AI_MATKEY_TRANSMISSION_FACTOR, opacity) == AI_SUCCESS) {
                modelMaterial.albedo.color.a = 1.0f - opacity;
            }
        }

        /* --- Load albedo texture --- */

        modelMaterial.albedo.texture = loadTexture(scene, material, aiTextureType_BASE_COLOR, 0);
        if (modelMaterial.albedo.texture == nullptr) {
            modelMaterial.albedo.texture = loadTexture(scene, material, aiTextureType_DIFFUSE, 0);
        }

        /* --- Load normal map --- */

        modelMaterial.normal.texture = loadTexture(scene, material, aiTextureType_NORMALS, 0);

        if (modelMaterial.normal.texture != nullptr) {
            float normalScale;
            if (material->Get(AI_MATKEY_BUMPSCALING, normalScale) == AI_SUCCESS) {
                modelMaterial.normal.scale = normalScale;
            }
        }

        /* --- Load emission map --- */

        aiColor4D emissionColor;
        if (material->Get(AI_MATKEY_COLOR_EMISSIVE, emissionColor) == AI_SUCCESS) {
            modelMaterial.emission.color = assimp_cast<HP_Color>(emissionColor);
            modelMaterial.emission.energy = 1.0f;
        }

        modelMaterial.emission.texture = loadTexture(scene, material, aiTextureType_EMISSIVE, 0);
        if (modelMaterial.emission.texture != nullptr) {
            modelMaterial.emission.energy = 1.0f;
        }

        /* --- Load ORM map --- */

        bool hasOcclusion = false;
        bool hasRoughness = false;
        bool hasMetalness = false;

        modelMaterial.orm.texture = loadTextureORM(
            scene, material,
            &hasOcclusion,
            &hasRoughness,
            &hasMetalness
        );

        float roughness;
        if (material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS) {
            modelMaterial.orm.roughness = roughness;
        }
        else if (hasRoughness) {
            modelMaterial.orm.roughness = 1.0f;
        }

        float metalness;
        if (material->Get(AI_MATKEY_METALLIC_FACTOR, metalness) == AI_SUCCESS) {
            modelMaterial.orm.metalness = metalness;
        }
        else if (hasMetalness) {
            modelMaterial.orm.metalness = 1.0f;
        }

        /* --- Handle cull mode from two-sided property --- */

        bool twoSided;
        if (material->Get(AI_MATKEY_TWOSIDED, twoSided) == AI_SUCCESS) {
            if (twoSided) modelMaterial.cull = HP_CULL_NONE;
        }

        /* --- Handle the alpha cutoff for glTF models --- */

        float alphaCutOff;
        if (material->Get(AI_MATKEY_GLTF_ALPHACUTOFF, alphaCutOff) == AI_SUCCESS) {
            modelMaterial.alphaCutOff = alphaCutOff;
        }

        /* --- Handle blend function override --- */

        aiBlendMode blendFunc;
        if (material->Get(AI_MATKEY_BLEND_FUNC, blendFunc) == AI_SUCCESS) {
            switch (blendFunc) {
            case aiBlendMode_Default:
                modelMaterial.blend = HP_BLEND_ALPHA;
                break;
            case aiBlendMode_Additive:
                modelMaterial.blend = HP_BLEND_ADD;
                break;
            default:
                break;
            }
        }
    }

    return true;
}

} // namespace render
