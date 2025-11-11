/* NX_Probe.c -- API definition for Nexium's environment module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/NX_Probe.h>

// ============================================================================
// LOCAL MANAGEMENT
// ============================================================================

static NX_Probe INX_DefaultProbe = NX_BASE_PROBE;

// ============================================================================
// PUBLIC API
// ============================================================================

NX_Probe NX_GetDefaultProbe()
{
    return INX_DefaultProbe;
}

void NX_SetDefaultProbe(const NX_Probe* probe)
{
    INX_DefaultProbe = probe ? *probe : NX_BASE_PROBE;
}
