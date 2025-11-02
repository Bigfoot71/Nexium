/* NX_Environment.c -- API definition for Nexium's environment module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/NX_Environment.h>

// ============================================================================
// LOCAL MANAGEMENT
// ============================================================================

static NX_Environment INX_DefaultEnvironment = NX_BASE_ENVIRONMENT;

// ============================================================================
// PUBLIC API
// ============================================================================

NX_Environment NX_GetDefaultEnvironment()
{
    return INX_DefaultEnvironment;
}

void NX_SetDefaultEnvironment(const NX_Environment* env)
{
    INX_DefaultEnvironment = *env;
}
