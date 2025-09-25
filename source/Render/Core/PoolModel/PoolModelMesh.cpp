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

namespace render {

template <bool HasBones>
HP_Mesh* PoolModel::processMesh(const aiMesh* mesh, const HP_Mat4& transform)
{
    /* --- Validate input parameters --- */

    if (!mesh) {
        HP_INTERNAL_LOG(E, "RENDER: Invalid parameters during assimp mesh processing");
        return nullptr;
    }

    /* --- Validate mesh data presence --- */

    if (mesh->mNumVertices == 0 || mesh->mNumFaces == 0) {
        HP_INTERNAL_LOG(E, "RENDER: Empty mesh detected during assimp mesh processing");
        return nullptr;
    }

    /* --- Allocate vertex and index buffers --- */

    int vertexCount = mesh->mNumVertices;
    int indexCount = 3 * mesh->mNumFaces;

    HP_Vertex3D* vertices = static_cast<HP_Vertex3D*>(SDL_calloc(vertexCount, sizeof(HP_Vertex3D)));
    if (!vertices) {
        HP_INTERNAL_LOG(E, "RENDER: Unable to allocate memory for vertices");
        return nullptr;
    }

    uint32_t* indices = static_cast<uint32_t*>(SDL_calloc(indexCount, sizeof(uint32_t)));
    if (!indices) {
        HP_INTERNAL_LOG(E, "RENDER: Unable to allocate memory for indices");
        SDL_free(vertices);
        return nullptr;
    }

    /* --- Initialize bounding box --- */

    HP_BoundingBox aabb = {
        .min = {+FLT_MAX, +FLT_MAX, +FLT_MAX},
        .max = {-FLT_MAX, -FLT_MAX, -FLT_MAX}
    };

    /* --- Compute normal matrix --- */

    HP_Mat3 matNormal;
    if constexpr (!HasBones) {
        matNormal = HP_Mat3Normal(&transform);
    }

    /* --- Process vertex attributes --- */

    for (size_t i = 0; i < vertexCount; i++)
    {
        HP_Vertex3D& vertex = vertices[i];

        /* --- Position --- */

        HP_Vec3 lPosition = assimp_cast<HP_Vec3>(mesh->mVertices[i]);
        HP_Vec3 gPosition = lPosition * transform;

        // NOTE: Meshes with bones keep vertices in local space and will rely on bind pose if needed

        if constexpr (HasBones) {
            vertex.position = lPosition;
        }
        else {
            vertex.position = gPosition;
        }

        /* --- Bounds update --- */

        // NOTE: Always use global position for AABB
        aabb.min = HP_Vec3Min(aabb.min, gPosition);
        aabb.max = HP_Vec3Max(aabb.max, gPosition);

        /* --- Texture coordinates --- */

        if (mesh->mTextureCoords[0] && mesh->mNumUVComponents[0] >= 2) {
            vertex.texcoord = assimp_cast<HP_Vec2>(mesh->mTextureCoords[0][i]);
        }
        else {
            vertex.texcoord = HP_VEC2(0.0f, 0.0f);
        }

        /* --- Normals --- */

        if (mesh->mNormals) {
            vertex.normal = assimp_cast<HP_Vec3>(mesh->mNormals[i]);
            if constexpr (!HasBones) vertex.normal *= matNormal;
        }
        else {
            vertex.normal = HP_VEC3(0.0f, 0.0f, 1.0f);
        }

        /* --- Tangent --- */

        if (mesh->mNormals && mesh->mTangents && mesh->mBitangents) {
            const HP_Vec3& normal = vertex.normal;
            HP_Vec3 tangent = assimp_cast<HP_Vec3>(mesh->mTangents[i]);
            HP_Vec3 bitangent = assimp_cast<HP_Vec3>(mesh->mBitangents[i]);
            if constexpr (!HasBones) tangent *= matNormal, bitangent *= matNormal;
            HP_Vec3 reconstructedBitangent = HP_Vec3Cross(normal, tangent);
            float handedness = HP_Vec3Dot(reconstructedBitangent, bitangent);
            vertex.tangent = HP_VEC4(tangent.x, tangent.y, tangent.z, (handedness < 0.0f) ? -1.0f : 1.0f);
        }
        else {
            vertex.tangent = HP_VEC4(1.0f, 0.0f, 0.0f, 1.0f);
        }

        /* --- Vertex color --- */

        if (mesh->mColors[0]) {
            vertex.color.r = mesh->mColors[0][i].r;
            vertex.color.g = mesh->mColors[0][i].g;
            vertex.color.b = mesh->mColors[0][i].b;
            vertex.color.a = mesh->mColors[0][i].a;
        }
        else {
            vertex.color = HP_WHITE;
        }
    }

    /* --- Process bone data --- */

    if (mesh->mNumBones > 0)
    {
        for (size_t boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++)
        {
            const aiBone* bone = mesh->mBones[boneIndex];
            if (!bone) {
                HP_INTERNAL_LOG(W, "RENDER: nullptr bone at index %zu", boneIndex);
                continue;
            }

            /* --- Process all vertex weights for this bone --- */

            for (size_t weightIndex = 0; weightIndex < bone->mNumWeights; weightIndex++)
            {
                const aiVertexWeight* weight = &bone->mWeights[weightIndex];

                uint32_t vertexId = weight->mVertexId;
                float weightValue = weight->mWeight;

                // Validate vertex ID
                if (vertexId >= vertexCount) {
                    HP_INTERNAL_LOG(E, "RENDER: Invalid vertex ID %u in bone weights (max: %zu)", vertexId, vertexCount);
                    continue;
                }

                // Skip weights that are too small to matter
                if (weightValue < 0.001f) {
                    continue;
                }

                // Find an empty slot in the vertex bone data (max 4 bones per vertex)
                HP_Vertex3D& vertex = vertices[vertexId];
                bool slotFound = false;

                for (int slot = 0; slot < 4; slot++) {
                    if (vertex.weights.v[slot] == 0.0f) {
                        vertex.weights.v[slot] = weightValue;
                        vertex.boneIds.v[slot] = static_cast<int>(boneIndex);
                        slotFound = true;
                        break;
                    }
                }

                if (!slotFound) {
                    // If all 4 slots are occupied, replace the smallest weight
                    int minWeightIndex = 0;
                    for (int slot = 1; slot < 4; slot++) {
                        if (vertex.weights.v[slot] < vertex.weights.v[minWeightIndex]) {
                            minWeightIndex = slot;
                        }
                    }

                    if (weightValue > vertex.weights.v[minWeightIndex]) {
                        vertex.weights.v[minWeightIndex] = weightValue;
                        vertex.boneIds.v[minWeightIndex] = static_cast<int>(boneIndex);
                    }
                }
            }
        }

        /* --- Normalize bone weights for each vertex --- */

        for (size_t i = 0; i < vertexCount; i++) {
            HP_Vertex3D& boneVertex = vertices[i];
            float totalWeight = 0.0f;
            // Calculate total weight
            for (int j = 0; j < 4; j++) {
                totalWeight += boneVertex.weights.v[j];
            }
            // Normalize weights if total > 0
            if (totalWeight > 0.0f) {
                for (int j = 0; j < 4; j++) {
                    boneVertex.weights.v[j] /= totalWeight;
                }
            }
            else {
                // If no bone weights, assign to first bone with weight 1.0
                boneVertex.weights.v[0] = 1.0f;
                boneVertex.boneIds.v[0] = 0;
            }
        }
    }
    else {
        // No bones found for this mesh
        for (size_t i = 0; i < vertexCount; i++) {
            vertices[i].weights.v[0] = 1.0f;
            vertices[i].boneIds.v[0] = 0;
        }
    }

    /* --- Process indices and validate faces --- */

    size_t indexOffset = 0;
    for (size_t i = 0; i < mesh->mNumFaces; i++) {
        const aiFace* face = &mesh->mFaces[i];
        if (face->mNumIndices != 3) {
            HP_INTERNAL_LOG(E, "RENDER: Non-triangular face detected (indices: %u)", face->mNumIndices);
            SDL_free(vertices);
            SDL_free(indices);
            return nullptr;
        }
        for (uint32_t j = 0; j < 3; j++) {
            if (face->mIndices[j] >= mesh->mNumVertices) {
                HP_INTERNAL_LOG(E, "RENDER: Invalid vertex index (%u >= %u)", face->mIndices[j], mesh->mNumVertices);
                SDL_free(vertices);
                SDL_free(indices);
                return nullptr;
            }
        }
        indices[indexOffset++] = face->mIndices[0];
        indices[indexOffset++] = face->mIndices[1];
        indices[indexOffset++] = face->mIndices[2];
    }

    /* --- Final validation: index count consistency --- */

    if (indexOffset != indexCount) {
        HP_INTERNAL_LOG(E, "RENDER: Inconsistency in the number of indices (%zu != %zu)", indexOffset, indexCount);
        SDL_free(vertices);
        SDL_free(indices);
        return nullptr;
    }

    /* --- Create the mesh in the pool and return it --- */

    HP_Mesh* modelMesh = mPoolMesh.createMesh(vertices, vertexCount, indices, indexCount, aabb, true);
    if (mesh == nullptr) {
        SDL_free(vertices);
        SDL_free(indices);
        return nullptr;
    }

    return modelMesh;
}

bool PoolModel::processMeshesRecursive(HP_Model* model, const aiScene* scene, const aiNode* node, const HP_Mat4& parentTransform)
{
    HP_Mat4 localTransform = assimp_cast<HP_Mat4>(node->mTransformation);
    HP_Mat4 globalTransform = HP_Mat4Mul(&localTransform, &parentTransform);

    for (uint32_t i = 0; i < node->mNumMeshes; i++)
    {
        uint32_t meshIndex = node->mMeshes[i];
        const aiMesh* mesh = scene->mMeshes[meshIndex];

        model->meshMaterials[meshIndex] = scene->mMeshes[meshIndex]->mMaterialIndex;

        if (mesh->mNumBones) {
            model->meshes[meshIndex] = processMesh<true>(mesh, globalTransform);
        }
        else {
            model->meshes[meshIndex] = processMesh<false>(mesh, globalTransform);
        }

        if (model->meshes[meshIndex] == nullptr) {
            HP_INTERNAL_LOG(E, "RENDER: Unable to load mesh [%d]; The model will be invalid", node->mMeshes[i]);
            return false;
        }
    }

    for (uint32_t i = 0; i < node->mNumChildren; i++) {
        if(!processMeshesRecursive(model, scene, node->mChildren[i], globalTransform)) {
            return false;
        }
    }

    return true;
}

bool PoolModel::processMeshes(HP_Model* model, const aiScene* scene, const aiNode* node)
{
    model->meshCount = scene->mNumMeshes;

    model->meshes = static_cast<HP_Mesh**>(SDL_calloc(model->meshCount, sizeof(HP_Mesh*)));
    if (model->meshes == nullptr) {
        HP_INTERNAL_LOG(E, "RENDER: Unable to allocate memory for meshes; The model will be invalid");
        return false;
    }

    model->meshMaterials = static_cast<int*>(SDL_calloc(model->meshCount, sizeof(int)));
    if (model->meshMaterials == nullptr) {
        HP_INTERNAL_LOG(E, "RENDER: Unable to allocate memory for mesh materials array; The model will be invalid");
        SDL_free(model->meshes);
        return false;
    }

    if (!processMeshesRecursive(model, scene, node, HP_MAT4_IDENTITY)) {
        for (int i = 0; i < model->meshCount; i++) {
            mPoolMesh.destroyMesh(model->meshes[i]);
        }
        SDL_free(model->meshMaterials);
        SDL_free(model->meshes);
        return false;
    }

    return true;
}

} // namespace render
