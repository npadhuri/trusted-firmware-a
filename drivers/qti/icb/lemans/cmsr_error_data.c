/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * CMSR error configuration data for the Lemans (QCS9075) platform.
 */

#include <drivers/qti/chipinfo/chipinfo.h>

#include "cmsr_error.h"
#include "cmsr_error_hwio.h"   /* CMSR_BASE, agent addresses */
#include "cmsr_hw_type.h"

static struct CMSRERR_agent_info_t lemans_cmsr0_agents[] = {
	{
		.name             = "CMSR0_AGENT0",
		.core_status_addr = (void *)CMSR0_AGENT0_CORE_STATUS,
		.uar_status_addr  = (void *)CMSR0_AGENT0_UAR_STATUS,
		.clear_reset_addr = (void *)CMSR0_AGENT0_CLEAR_RESET,
	},
};

static struct CMSRERR_err_info_t lemans_cmsr_instances[] = {
	/* [0] CMSR0 */
	{
		.name       = "CMSR0",
		.base_addr  = (void *)CMSR0_BASE,
		.num_agents = ARRAY_SIZE(lemans_cmsr0_agents),
		.cmsr_agents = lemans_cmsr0_agents,
	},
};

static CMSRERR_info_t lemans_cmsr_info = {
	.cmsr_hw   = &CMSR_1_0,
	.num_cmsrs = ARRAY_SIZE(lemans_cmsr_instances),
	.cmsrs     = lemans_cmsr_instances,

	.cmsr_core_timeout_intr_vector  = UINTPTR_MAX,
	.cmsr_agent_timeout_intr_vector = UINTPTR_MAX,
	.cmsr_boot_mode_err_intr_vector = UINTPTR_MAX,

	.cmsr_core_timeout_intr_enable  = false,
	.cmsr_agent_timeout_intr_enable = false,
	.cmsr_boot_mode_err_intr_enable = false,

	.cmsr_core_timeout_error_fatal  = true,
	.cmsr_agent_timeout_error_fatal = true,
	.cmsr_boot_mode_err_error_fatal = true,

	.cmsr_agent_timeout_val = 0x0U,
	.cmsr_core_timeout_val  = 0x0U,
};

CMSRERR_config_info_t cmsr_config_info = {
	.num_configs = 1U,
	.configs = (CMSRERR_propdata_t []){
		{
			.family   = (uint32_t)CHIPINFO_FAMILY_LEMANSAU,
			.match    = false,
			.version  = CHIPINFO_VERSION(1, 0),
			.CMSRInfo = &lemans_cmsr_info,
		},
	},
};