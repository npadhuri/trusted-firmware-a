/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * DDRSS hardware register-offset descriptors.
 * Follows the same pattern as noc_hw_type.h.
 */

#ifndef QTI_DDRSS_HW_TYPE_H
#define QTI_DDRSS_HW_TYPE_H

#include <lib/utils_def.h>

#include "ddrss_error.h"

/*
 * DDRSS_TCM_1_0 — Tag Cache Memory register layout.
 * Offsets relative to the LLCC TCM base address.
 */
static struct DDRSS_tcm_hw_t DDRSS_TCM_1_0 __attribute__((unused)) = {
	.interrupt_status = 0x0000U,
	.status1          = 0x0020U,
	.status2          = 0x0024U,
	.status3          = 0x0028U,
	.status4          = 0x002CU,
	.status5          = 0x0030U,
	.status6          = 0x0034U,
	.status7          = 0x0038U,
	.status8          = 0x003CU,
	.tcm_error_cfg    = 0x0040U,
	.interrupt_clear  = 0x0044U,
};

/*
 * DDRSS_FEAC_1_0 — Front-End Address Check register layout.
 * Offsets relative to the LLCC FEAC base address.
 */
static struct DDRSS_feac_hw_t DDRSS_FEAC_1_0 __attribute__((unused)) = {
	.interrupt_status          = 0x0000U,
	.status0                   = 0x0004U,
	.status1                   = 0x0008U,
	.status2                   = 0x000CU,
	.status3                   = 0x0010U,
	.status4                   = 0x0014U,
	.status5                   = 0x0018U,
	.status6                   = 0x001CU,
	.dec_intrpt_cfg            = 0x0020U,
	.interrupt_clr             = 0x0024U,
	.uimage_region_err_status1 = 0x0028U,
	.uimage_region_err_status2 = 0x002CU,
	.uimage_region_err_status3 = 0x0030U,
	.uimage_region_err_status4 = 0x0034U,
	.uimage_region_err_status5 = 0x0038U,
	.uimage_region_err_status6 = 0x003CU,
	.interrupt_bypass_status   = 0x0040U,
	.bypass_err_status1        = 0x0044U,
	.bypass_err_status2        = 0x0048U,
	.bypass_err_status3        = 0x004CU,
	.bypass_err_status4        = 0x0050U,
	.bypass_err_status5        = 0x0054U,
	.bypass_err_status6        = 0x0058U,
	.interrupt_bypass_clr      = 0x005CU,
};

/*
 * DDRSS_MC_1_0 — Memory Controller register layout.
 * Offsets relative to the MC base address.
 */
static struct DDRSS_mc_hw_t DDRSS_MC_1_0 __attribute__((unused)) = {
	.interrupt_status   = 0x0000U,
	.addr_decerr_esyn_0 = 0x0004U,
	.addr_decerr_esyn_1 = 0x0008U,
	.interrupt_clear    = 0x000CU,
	.refresh_err_esyn   = 0x0010U,
};

/*
 * DDRSS_SHKE_1_0 — SHKE per-channel register layout.
 */
static struct DDRSS_shke_hw_t DDRSS_SHKE_1_0 __attribute__((unused)) = {
	.temp_error_cfg = 0x0000U,
	.load_confg     = 0x0004U,
};

/*
 * DDRSS_SHKE_GLOBAL_1_0 — SHKE global register layout.
 */
static struct DDRSS_shke_global_hw_t DDRSS_SHKE_GLOBAL_1_0 __attribute__((unused)) = {
	.shke_status = 0x0000U,
};

#endif /* QTI_DDRSS_HW_TYPE_H */