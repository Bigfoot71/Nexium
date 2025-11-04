/* INX_VariantMesh.cpp -- Abstraction allowing to represent several types of mesh
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef INX_VARIANT_MESH_HPP
#define INX_VARIANT_MESH_HPP

#include "./NX_DynamicMesh.hpp"
#include <NX/NX_Mesh.h>
#include <variant>

/* === Declaration === */

class INX_VariantMesh {
public:
    /** Constructors */
    template<typename T_Mesh>
    INX_VariantMesh(const T_Mesh* mesh);

    /** Assignment operators for each variant type */
    INX_VariantMesh& operator=(const NX_Mesh* mesh);
    INX_VariantMesh& operator=(const NX_DynamicMesh* mesh);

    /** Variant related */
    size_t index() const;
    template<typename T> T get() const;
    template<size_t I> auto get() const;

    /** Getters */
    NX_ShadowCastMode shadowCastMode() const;
    NX_ShadowFaceMode shadowFaceMode() const;
    const NX_BoundingBox3D& aabb() const;
    NX_Layer layerMask() const;

private:
    std::variant<const NX_Mesh*, const NX_DynamicMesh*> mMesh;
};

/* === Public Implementation === */

template<typename T_Mesh>
INX_VariantMesh::INX_VariantMesh(const T_Mesh* mesh)
    : mMesh(mesh)
{ }

inline INX_VariantMesh& INX_VariantMesh::operator=(const NX_Mesh* mesh)
{
    mMesh = mesh;
    return *this;
}

inline INX_VariantMesh& INX_VariantMesh::operator=(const NX_DynamicMesh* mesh)
{
    mMesh = mesh;
    return *this;
}

inline size_t INX_VariantMesh::index() const
{
    return mMesh.index();
}

template<typename T>
inline T INX_VariantMesh::get() const
{
    return std::get<T>(mMesh);
}

template<size_t I>
inline auto INX_VariantMesh::get() const
{
    return std::get<I>(mMesh);
}

inline NX_ShadowCastMode INX_VariantMesh::shadowCastMode() const
{
    switch (mMesh.index()) {
    case 0: [[likely]] return std::get<0>(mMesh)->shadowCastMode;
    case 1: [[unlikely]] return std::get<1>(mMesh)->shadowCastMode;
    default: NX_UNREACHABLE(); break;
    }
}

inline NX_ShadowFaceMode INX_VariantMesh::shadowFaceMode() const
{
    switch (mMesh.index()) {
    case 0: [[likely]] return std::get<0>(mMesh)->shadowFaceMode;
    case 1: [[unlikely]] return std::get<1>(mMesh)->shadowFaceMode;
    default: NX_UNREACHABLE(); break;
    }
}

inline const NX_BoundingBox3D& INX_VariantMesh::aabb() const
{
    switch (mMesh.index()) {
    case 0: [[likely]] return std::get<0>(mMesh)->aabb;
    case 1: [[unlikely]] return std::get<1>(mMesh)->aabb;
    default: NX_UNREACHABLE(); break;
    }
}

inline NX_Layer INX_VariantMesh::layerMask() const
{
    switch (mMesh.index()) {
    case 0: [[likely]] return std::get<0>(mMesh)->layerMask;
    case 1: [[unlikely]] return std::get<1>(mMesh)->layerMask;
    default: NX_UNREACHABLE(); break;
    }
}

#endif // INX_VARIANT_MESH_HPP
