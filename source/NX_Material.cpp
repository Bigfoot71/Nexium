/* NX_Material.c -- API definition for Nexium's material module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/NX_Material.h>

// ============================================================================
// LOCAL MANAGEMENT
// ============================================================================

static NX_Material INX_DefaultMaterial = NX_BASE_MATERIAL;

// ============================================================================
// PUBLIC API
// ============================================================================

NX_Material NX_GetDefaultMaterial(void)
{
    return INX_DefaultMaterial;
}

void NX_SetDefaultMaterial(const NX_Material* material)
{
    INX_DefaultMaterial = *material;
}

void NX_DestroyMaterialResources(NX_Material* material)
{
    NX_DestroyTexture(material->albedo.texture);
    NX_DestroyTexture(material->emission.texture);
    NX_DestroyTexture(material->orm.texture);
    NX_DestroyTexture(material->normal.texture);
    NX_DestroyShader3D(material->shader);
}
