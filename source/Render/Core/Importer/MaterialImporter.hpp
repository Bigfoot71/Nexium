#ifndef NX_RENDER_MATERIAL_IMPORTER_HPP
#define NX_RENDER_MATERIAL_IMPORTER_HPP

#include <NX/NX_Render.h>

#include "./SceneImporter.hpp"
#include "../PoolTexture.hpp"
#include "./AssimpHelper.hpp"

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
    void loadMaterial(NX_Material* material, const aiMaterial* aiMat);

    /** Image/texture loading */
    NX_Image loadImage(const aiString& path, bool asData, bool* isAllocated);
    NX_Texture* loadTexture(const aiMaterial* material, aiTextureType type, uint32_t index, bool asData);
    NX_Texture* loadTextureORM(const aiMaterial* material, bool* hasOcclusion, bool* hasRoughness, bool* hasMetalness);

    /** Helpers */
    static NX_TextureWrap getWrapMode(aiTextureMapMode wrap);

private:
    const SceneImporter& mImporter;
    PoolTexture& mPoolTexture;
};

/* === Public Implementation === */

inline MaterialImporter::MaterialImporter(const SceneImporter& importer, PoolTexture& poolTexture)
    : mImporter(importer), mPoolTexture(poolTexture)
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
        loadMaterial(&model->materials[i], mImporter.material(i));
    }

    return true;
}

/* === Private Material === */

inline void MaterialImporter::loadMaterial(NX_Material* material, const aiMaterial* aiMat)
{
    const aiScene* scene = mImporter.scene();

    /* --- Initialize material defaults --- */

    *material = NX_GetDefaultMaterial();

    /* --- Load the albedo color --- */

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

    /* --- Load albedo texture --- */

    material->albedo.texture = loadTexture(aiMat, aiTextureType_BASE_COLOR, 0, false);
    if (material->albedo.texture == nullptr) {
        material->albedo.texture = loadTexture(aiMat, aiTextureType_DIFFUSE, 0, false);
    }

    /* --- Load normal map --- */

    material->normal.texture = loadTexture(aiMat, aiTextureType_NORMALS, 0, true);
    if (material->normal.texture != nullptr) {
        float normalScale;
        if (aiMat->Get(AI_MATKEY_BUMPSCALING, normalScale) == AI_SUCCESS) {
            material->normal.scale = normalScale;
        }
    }

    /* --- Load emission map --- */

    aiColor4D emissionColor;
    if (aiMat->Get(AI_MATKEY_COLOR_EMISSIVE, emissionColor) == AI_SUCCESS) {
        material->emission.color = assimp_cast<NX_Color>(emissionColor);
        material->emission.energy = 1.0f;
    }

    material->emission.texture = loadTexture(aiMat, aiTextureType_EMISSIVE, 0, false);
    if (material->emission.texture != nullptr) {
        material->emission.energy = 1.0f;
    }

    /* --- Load ORM map --- */

    bool hasOcclusion = false;
    bool hasRoughness = false;
    bool hasMetalness = false;

    material->orm.texture = loadTextureORM(
        aiMat, &hasOcclusion, &hasRoughness, &hasMetalness
    );

    float roughness;
    if (aiMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS) {
        material->orm.roughness = roughness;
    }
    else if (hasRoughness) {
        material->orm.roughness = 1.0f;
    }

    float metalness;
    if (aiMat->Get(AI_MATKEY_METALLIC_FACTOR, metalness) == AI_SUCCESS) {
        material->orm.metalness = metalness;
    }
    else if (hasMetalness) {
        material->orm.metalness = 1.0f;
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

inline NX_Image MaterialImporter::loadImage(const aiString& path, bool asData, bool* isAllocated)
{
    NX_Image image{};

    *isAllocated = false;

    /* --- Handle embedded texture (starts with '*') --- */

    if (path.data[0] == '*')
    {
        int textureIndex = atoi(&path.data[1]);

        /* --- Validate embedded texture index --- */

        if (textureIndex < 0 || textureIndex >= mImporter.textureCount()) {
            return image;
        }

        const aiTexture* aiTex = mImporter.texture(textureIndex);

        /* --- Handle compressed embedded texture --- */

        if (aiTex->mHeight == 0) {
            if (asData) {
                image = NX_LoadImageAsDataFromMem(aiTex->pcData, aiTex->mWidth);
            }
            else {
                image = NX_LoadImageFromMem(aiTex->pcData, aiTex->mWidth);
            }
            *isAllocated = true;
        }

        /* --- Handle uncompressed (raw RGBA) embedded texture --- */

        else {
            image.w = aiTex->mWidth;
            image.h = aiTex->mHeight;
            image.format = NX_PIXEL_FORMAT_RGBA8;
            // NOTE: No need to copy the data here, the image will be immediately
            //       uploaded to the GPU without being retained afterward
            image.pixels = aiTex->pcData;
        }
    }

    /* --- Handle external texture from file --- */

    else {
        if (asData) {
            image = NX_LoadImageAsData(path.data);
        }
        else {
            image = NX_LoadImage(path.data);
        }
        if (image.pixels != nullptr) {
            *isAllocated = true;
        }
    }

    return image;
}

inline NX_Texture* MaterialImporter::loadTexture(const aiMaterial* material, aiTextureType type, uint32_t index, bool asData)
{
    /* --- Get texture info --- */

    // TODO: Currently, only the first wrap mode is considered, which may be incorrect
    //       The wrap system with NX_Texture should be revised to handle wrapping on each axis
    //       See also 'loadTextureORM' if a change is made

    aiTextureMapMode wrapMode[2]{};
    aiString path{};

    if (material->GetTexture(type, index, &path, nullptr, nullptr, nullptr, nullptr, wrapMode) != AI_SUCCESS) {
        return nullptr; // No texture of this type
    }

    /* --- Loads the texture into RAM --- */

    bool isAllocated = false;
    NX_Image image = loadImage(path, asData, &isAllocated);
    if (image.pixels == nullptr) {
        return nullptr;
    }

    /* --- Upload the texture to VRAM --- */

    NX_Texture* texture = mPoolTexture.createTexture(image, getWrapMode(wrapMode[0]));
    if (isAllocated) {
        NX_DestroyImage(&image);
    }

    return texture;
}

inline NX_Texture* MaterialImporter::loadTextureORM(const aiMaterial* material, bool* hasOcclusion, bool* hasRoughness, bool* hasMetalness)
{
    NX_Texture* ormTexture = nullptr;

    *hasOcclusion = false;
    *hasRoughness = false;
    *hasMetalness = false;

    /* --- Check for glTF combined metallic-roughness texture first --- */

    aiTextureMapMode gltfWrapMode[2]{};
    aiString gltfPath{};

    if (material->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &gltfPath, nullptr, nullptr, nullptr, nullptr, gltfWrapMode) == AI_SUCCESS)
    {
        bool gltfAllocated;
        NX_Image gltfImage = loadImage(gltfPath, true, &gltfAllocated);

        if (gltfImage.pixels != nullptr)
        {
            *hasRoughness = true;
            *hasMetalness = true;

            // Load separate occlusion if available
            bool occlusionAllocated = false;
            NX_Image occlusionImage{};
            aiString occlusionPath{};
            if (material->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &occlusionPath, nullptr, nullptr, nullptr, nullptr, nullptr) == AI_SUCCESS) {
                occlusionImage = loadImage(occlusionPath, true, &occlusionAllocated);
                *hasOcclusion = (occlusionImage.pixels != nullptr);
            }
            else if (material->GetTexture(aiTextureType_LIGHTMAP, 0, &occlusionPath, nullptr, nullptr, nullptr, nullptr, nullptr) == AI_SUCCESS) {
                occlusionImage = loadImage(occlusionPath, true, &occlusionAllocated);
                *hasOcclusion = (occlusionImage.pixels != nullptr);
            }

            // Compose ORM: O=occlusion, R=gltf.green, M=gltf.blue
            const NX_Image* sources[3] = {
                occlusionImage.pixels ? &occlusionImage : nullptr, // Red channel
                &gltfImage,                                     // Green channel (roughness from glTF)
                &gltfImage                                      // Blue channel (metalness from glTF)
            };

            NX_Image ormImage = NX_ComposeImagesRGB(sources, NX_WHITE);

            if (ormImage.pixels != nullptr) {
                ormTexture = mPoolTexture.createTexture(ormImage, getWrapMode(gltfWrapMode[0]));
                NX_DestroyImage(&ormImage);
            }

            // Cleanup
            if (gltfAllocated) {
                NX_DestroyImage(&gltfImage);
            }
            if (occlusionAllocated && occlusionImage.pixels) {
                NX_DestroyImage(&occlusionImage);
            }

            return ormTexture;
        }
    }

    /* --- Fallback: Load individual textures --- */

    NX_Image occlusionImage{};
    NX_Image roughnessImage{};
    NX_Image metalnessImage{};

    bool occlusionAllocated = false;
    bool roughnessAllocated = false;
    bool metalnessAllocated = false;

    aiTextureMapMode occlusionWrapMode[2]{};
    aiTextureMapMode roughnessWrapMode[2]{};
    aiTextureMapMode metalnessWrapMode[2]{};

    // Load occlusion
    struct aiString occlusionPath;
    if (material->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &occlusionPath, nullptr, nullptr, nullptr, nullptr, occlusionWrapMode) == AI_SUCCESS) {
        occlusionImage = loadImage(occlusionPath, true, &occlusionAllocated);
        *hasOcclusion = (occlusionImage.pixels != nullptr);
    }
    else if (material->GetTexture(aiTextureType_LIGHTMAP, 0, &occlusionPath, nullptr, nullptr, nullptr, nullptr, occlusionWrapMode) == AI_SUCCESS) {
        occlusionImage = loadImage(occlusionPath, true, &occlusionAllocated);
        *hasOcclusion = (occlusionImage.pixels != nullptr);
    }

    // Load roughness (with shininess fallback)
    struct aiString roughnessPath;
    if (material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &roughnessPath, nullptr, nullptr, nullptr, nullptr, roughnessWrapMode) == AI_SUCCESS) {
        roughnessImage = loadImage(roughnessPath, true, &roughnessAllocated);
        *hasRoughness = (roughnessImage.pixels != nullptr);
    }
    else if (material->GetTexture(aiTextureType_SHININESS, 0, &roughnessPath, nullptr, nullptr, nullptr, nullptr, roughnessWrapMode) == AI_SUCCESS) {
        roughnessImage = loadImage(roughnessPath, true, &roughnessAllocated);
        if (roughnessImage.pixels) {
            *hasRoughness = true;
            NX_InvertImage(&roughnessImage); // Convert shininess to roughness
        }
    }

    // Load metalness
    struct aiString metalnessPath;
    if (material->GetTexture(aiTextureType_METALNESS, 0, &metalnessPath, nullptr, nullptr, nullptr, nullptr, metalnessWrapMode) == AI_SUCCESS) {
        metalnessImage = loadImage(metalnessPath, true, &metalnessAllocated);
        *hasMetalness = (metalnessImage.pixels != nullptr);
    }

    // Compose ORM using the utility function
    const NX_Image* sources[3] = {
        occlusionImage.pixels ? &occlusionImage : nullptr,   // Red channel
        roughnessImage.pixels ? &roughnessImage : nullptr,   // Green channel
        metalnessImage.pixels ? &metalnessImage : nullptr    // Blue channel
    };

    NX_Image ormImage = NX_ComposeImagesRGB(sources, NX_WHITE);

    if (ormImage.pixels)
    {
        aiTextureMapMode wrapMode = aiTextureMapMode_Clamp;

        if (sources[1]) wrapMode = roughnessWrapMode[0];
        else if (sources[2]) wrapMode = metalnessWrapMode[0];
        else if (sources[0]) wrapMode = occlusionWrapMode[0];

        ormTexture = mPoolTexture.createTexture(ormImage, getWrapMode(wrapMode));
        NX_DestroyImage(&ormImage);
    }

    // Cleanup
    if (occlusionAllocated && occlusionImage.pixels) NX_DestroyImage(&occlusionImage);
    if (roughnessAllocated && roughnessImage.pixels) NX_DestroyImage(&roughnessImage);
    if (metalnessAllocated && metalnessImage.pixels) NX_DestroyImage(&metalnessImage);

    return ormTexture;
}

inline NX_TextureWrap MaterialImporter::getWrapMode(aiTextureMapMode wrap)
{
    NX_TextureWrap hpWrap = NX_TEXTURE_WRAP_CLAMP;

    switch (wrap) {
    case aiTextureMapMode_Wrap:
        hpWrap = NX_TEXTURE_WRAP_REPEAT;
        break;
    case aiTextureMapMode_Mirror:
        hpWrap = NX_TEXTURE_WRAP_MIRROR;
        break;
    case aiTextureMapMode_Clamp:
    case aiTextureMapMode_Decal:
    case _aiTextureMapMode_Force32Bit:
    default:
        break;
    }

    return hpWrap;
}

} // namespace render

#endif // NX_RENDER_MATERIAL_IMPORTER_HPP
