#ifndef NX_IMPORT_DETAIL_TEXTURE_LOADER_HPP
#define NX_IMPORT_DETAIL_TEXTURE_LOADER_HPP

#include "../Detail/Util/DynamicArray.hpp"
#include "./SceneImporter.hpp"

#include <NX/NX_Texture.h>
#include <NX/NX_Image.h>

#include <assimp/GltfMaterial.h>
#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/texture.h>
#include <assimp/types.h>

#include <condition_variable>
#include <thread>
#include <queue>
#include <array>

namespace import {

/* === Declaration === */

class TextureLoader {
public:
    enum Map {
        MAP_ALBEDO      = 0,
        MAP_EMISSION    = 1,
        MAP_ORM         = 2,
        MAP_NORMAL      = 3,
        MAP_COUNT
    };

public:
    TextureLoader(const SceneImporter& importer);
    NX_Texture* Get(int materialIndex, Map map);

private:
    /** Temporary image data */
    struct Image {
        aiTextureMapMode wrap[2];
        NX_Image image;
        bool owned;
    };

    /** Material map array */
    using MaterialTextures = std::array<NX_Texture*, MAP_COUNT>;
    using MaterialImages = std::array<Image, MAP_COUNT>;

private:
    /** Base loading function */
    bool LoadImage(Image* image, const aiMaterial* material, aiTextureType type, uint32_t index, bool asData);
    bool LoadImage(Image* image, const aiMaterial* material, Map map);

    /** Loading functions */
    bool LoadImageAlbedo(Image* image, const aiMaterial* material);
    bool LoadImageEmission(Image* image, const aiMaterial* material);
    bool LoadImageORM(Image* image, const aiMaterial* material);
    bool LoadImageNormal(Image* image, const aiMaterial* material);

    /** Helpers */
    static NX_TextureWrap GetWrapMode(aiTextureMapMode wrap);

private:
    util::DynamicArray<MaterialTextures> mTextures;
    const SceneImporter& mImporter;
};

/* === Public Implementation === */

inline TextureLoader::TextureLoader(const SceneImporter& importer)
    : mImporter(importer)
{
    // REVIEW: This is a quick implementation, but it's at least faster than before.
    //         Note that one of the current big issues is that if two materials use
    //         the same texture, that texture will be loaded twice. I haven't
    //         encountered a model where this happens yet, but it is possible!

    const int matCount = importer.GetMaterialCount();
    mTextures.Resize(matCount);

    /* --- Temporary storage of images --- */

    util::DynamicArray<MaterialImages> images;
    images.Resize(matCount);

    /* --- Thread pool setup --- */

    const int totalJobs = matCount * MAP_COUNT;
    std::atomic<int> nextJob(0);

    const int numThreads = std::min((unsigned int)std::thread::hardware_concurrency(), (unsigned int)totalJobs);
    std::vector<std::thread> pool;
    pool.reserve(numThreads);

    /* --- Simple queue to store ready image indices --- */

    std::mutex readyMutex;
    std::condition_variable readyCV;
    std::queue<std::pair<int,int>> readyQueue;

    /* --- Launch loading threads --- */

    for (int t = 0; t < numThreads; ++t)
    {
        pool.emplace_back([&] {
            while (true)
            {
                int jobIndex = nextJob.fetch_add(1);
                if (jobIndex >= totalJobs) break;

                int i = jobIndex / MAP_COUNT;
                int j = jobIndex % MAP_COUNT;

                LoadImage(&images[i][j], importer.GetMaterial(i), Map(j));

                // Indicate that the image is ready for upload
                {
                    std::lock_guard<std::mutex> lock(readyMutex);
                    readyQueue.push({i,j});
                }
                readyCV.notify_one();
            }
        });
    }

    /* --- Progressive upload loop --- */

    int uploadedCount = 0;
    while (uploadedCount < totalJobs)
    {
        std::pair<int,int> job;
        {
            std::unique_lock<std::mutex> lock(readyMutex);
            readyCV.wait(lock, [&]{ return !readyQueue.empty(); });
            job = readyQueue.front();
            readyQueue.pop();
        }

        int i = job.first;
        int j = job.second;
        auto& img = images[i][j];

        if (img.image.pixels) {
            mTextures[i][j] = NX_CreateTextureFromImageEx(
                &img.image, GetWrapMode(img.wrap[0]),
                NX_GetDefaultTextureFilter()
            );
            if (img.owned) {
                NX_DestroyImage(&img.image);
                img.image.pixels = nullptr;
                img.owned = false;
            }
        }
        else {
            mTextures[i][j] = nullptr;
        }

        uploadedCount++;
    }

    /* --- Join threads --- */

    for (std::thread& t : pool) {
        t.join();
    }
}

inline NX_Texture* TextureLoader::Get(int materialIndex, Map map)
{
    return mTextures[materialIndex][map];
}

/* === Private Implementation === */

inline bool TextureLoader::LoadImage(Image* image, const aiMaterial* material, aiTextureType type, uint32_t index, bool asData)
{
    aiString path{};
    if (material->GetTexture(type, index, &path, nullptr, nullptr, nullptr, nullptr, image->wrap) != AI_SUCCESS) {
        return false; // No texture of this type
    }

    if (path.data[0] == '*')
    {
        int textureIndex = atoi(&path.data[1]);
        const aiTexture* aiTex = mImporter.GetTexture(textureIndex);

        if (aiTex->mHeight == 0) {
            if (asData) {
                image->image = NX_LoadImageRawFromData(aiTex->pcData, aiTex->mWidth);
            }
            else {
                image->image = NX_LoadImageFromData(aiTex->pcData, aiTex->mWidth);
            }
            image->owned = true;
        }
        else {
            image->image.w = aiTex->mWidth;
            image->image.h = aiTex->mHeight;
            image->image.format = NX_PIXEL_FORMAT_RGBA8;
            // NOTE: No need to copy the data here, the image will be immediately
            //       uploaded to the GPU without being retained afterward
            image->image.pixels = aiTex->pcData;
            image->owned = false;
        }
    }
    else {
        if (asData) {
            image->image = NX_LoadImageRaw(path.data);
        }
        else {
            image->image = NX_LoadImage(path.data);
        }
        if (image->image.pixels != nullptr) {
            image->owned = true;
        }
    }

    return true;
}

inline bool TextureLoader::LoadImage(Image* image, const aiMaterial* material, Map map)
{
    switch (map) {
    case MAP_ALBEDO:
        return LoadImageAlbedo(image, material);
    case MAP_EMISSION:
        return LoadImageEmission(image, material);
    case MAP_ORM:
        return LoadImageORM(image, material);
    case MAP_NORMAL:
        return LoadImageNormal(image, material);
    case MAP_COUNT:
        NX_UNREACHABLE();
        break;
    }
    return false;
}

inline bool TextureLoader::LoadImageAlbedo(Image* image, const aiMaterial* material)
{
    bool valid = LoadImage(image, material, aiTextureType_BASE_COLOR, 0, false);
    if (!valid) {
        valid = LoadImage(image, material, aiTextureType_DIFFUSE, 0, false);
    }
    return valid;
}

inline bool TextureLoader::LoadImageEmission(Image* image, const aiMaterial* material)
{
    return LoadImage(image, material, aiTextureType_EMISSIVE, 0, false);
}

inline bool TextureLoader::LoadImageORM(Image* image, const aiMaterial* material)
{
    Image imOcclusion{};
    Image imRoughness{};
    Image imMetalness{};

    /* --- Load occlusion map --- */

    bool retOcclusion = LoadImage(&imOcclusion, material, aiTextureType_AMBIENT_OCCLUSION, 0, true);
    if (!retOcclusion) {
        retOcclusion = LoadImage(&imOcclusion, material, aiTextureType_LIGHTMAP, 0, true);
    }

    /* --- Load roughness map --- */

    bool retRoughness = LoadImage(&imRoughness, material, aiTextureType_DIFFUSE_ROUGHNESS, 0, true);
    if (!retRoughness) {
        retRoughness = LoadImage(&imOcclusion, material, aiTextureType_SHININESS, 0, true);
        if (retRoughness) {
            NX_InvertImage(&imRoughness.image);
        }
    }

    /* --- Load metalness map --- */

    bool retMetalness = LoadImage(&imMetalness, material, aiTextureType_METALNESS, 0, true);
    if (!retMetalness && !retRoughness) {
        retRoughness = LoadImage(
            &imRoughness, material,
            AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, true
        );
        if (retRoughness) {
            retMetalness = retRoughness;
            imMetalness = imRoughness;
        }
    }

    /* --- If no image could be loaded we return --- */

    if (!retOcclusion && !retRoughness && !retMetalness) {
        return false;
    }

    /* --- Compose ORM map and fill out data --- */

    const NX_Image* sources[3] = {
        retOcclusion ? &imOcclusion.image : nullptr,
        retRoughness ? &imRoughness.image : nullptr,
        retMetalness ? &imMetalness.image : nullptr
    };

    image->image = NX_ComposeImagesRGB(sources, NX_WHITE);
    image->owned = true;

    if (retMetalness) {
        image->wrap[0] = imMetalness.wrap[0];
        image->wrap[1] = imMetalness.wrap[1];
    }
    else if (retRoughness) {
        image->wrap[0] = imRoughness.wrap[0];
        image->wrap[1] = imRoughness.wrap[1];
    }
    else if (retOcclusion) {
        image->wrap[0] = imOcclusion.wrap[0];
        image->wrap[1] = imOcclusion.wrap[1];
    }

    /* --- Free allocated data --- */

    if (imOcclusion.owned) {
        NX_DestroyImage(&imOcclusion.image);
    }

    if (imRoughness.owned) {
        NX_DestroyImage(&imRoughness.image);
    }

    if (imMetalness.owned && imMetalness.image.pixels != imRoughness.image.pixels) {
        NX_DestroyImage(&imMetalness.image);
    }

    return true;
}

inline bool TextureLoader::LoadImageNormal(Image* image, const aiMaterial* material)
{
    return LoadImage(image, material, aiTextureType_NORMALS, 0, true);
}

inline NX_TextureWrap TextureLoader::GetWrapMode(aiTextureMapMode wrap)
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

} // import

#endif // NX_IMPORT_DETAIL_TEXTURE_LOADER_HPP
