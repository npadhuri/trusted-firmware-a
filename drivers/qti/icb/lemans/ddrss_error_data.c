/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * DDRSS error configuration data for the Lemans (QCS9075) platform.
 */

#include <drivers/qti/chipinfo/chipinfo.h>

#include "ddrss_error.h"
#include "ddrss_error_hwio.h"
#include "ddrss_hw_type.h"
#include <stddef.h>

/* Extended FEAC syndrome storage for each LLCC */
static struct DDRSSERR_llcc_feac_ext_syndrome_t lemans_feac_ext_syndrome[6];

static struct DDRSSERR_llcc_info_t lemans_llcc_instances[] = {
	{
		.name                  = "LLCC0",
		.tcm_base_addr         = (void *)LLCC0_TCM_BASE,
		.feac_base_addr        = (void *)LLCC0_FEAC_BASE,
		.feac_extended_syndrome = &lemans_feac_ext_syndrome[0],
	},
	{
		.name                  = "LLCC1",
		.tcm_base_addr         = (void *)LLCC1_TCM_BASE,
		.feac_base_addr        = (void *)LLCC1_FEAC_BASE,
		.feac_extended_syndrome = &lemans_feac_ext_syndrome[1],
	},
	{
		.name                  = "LLCC2",
		.tcm_base_addr         = (void *)LLCC2_TCM_BASE,
		.feac_base_addr        = (void *)LLCC2_FEAC_BASE,
		.feac_extended_syndrome = &lemans_feac_ext_syndrome[2],
	},
	{
		.name                  = "LLCC3",
		.tcm_base_addr         = (void *)LLCC3_TCM_BASE,
		.feac_base_addr        = (void *)LLCC3_FEAC_BASE,
		.feac_extended_syndrome = &lemans_feac_ext_syndrome[3],
	},
	{
		.name                  = "LLCC4",
		.tcm_base_addr         = (void *)LLCC4_TCM_BASE,
		.feac_base_addr        = (void *)LLCC4_FEAC_BASE,
		.feac_extended_syndrome = &lemans_feac_ext_syndrome[4],
	},
	{
		.name                  = "LLCC5",
		.tcm_base_addr         = (void *)LLCC5_TCM_BASE,
		.feac_base_addr        = (void *)LLCC5_FEAC_BASE,
		.feac_extended_syndrome = &lemans_feac_ext_syndrome[5],
	},
};

static struct DDRSSERR_mc_info_t lemans_mc_instances[] = {
	{
		.name                = "MC0",
		.base_addr           = (void *)MC0_BASE,
		.refresh_rate_mask   = 0xFFU,
		.high_temp_threshold = 0U,
		.low_temp_threshold  = 0U,
		.refresh_rate_shift  = 0U,
	},
	{
		.name                = "MC1",
		.base_addr           = (void *)MC1_BASE,
		.refresh_rate_mask   = 0xFFU,
		.high_temp_threshold = 0U,
		.low_temp_threshold  = 0U,
		.refresh_rate_shift  = 0U,
	},
	{
		.name                = "MC2",
		.base_addr           = (void *)MC2_BASE,
		.refresh_rate_mask   = 0xFFU,
		.high_temp_threshold = 0U,
		.low_temp_threshold  = 0U,
		.refresh_rate_shift  = 0U,
	},
	{
		.name                = "MC3",
		.base_addr           = (void *)MC3_BASE,
		.refresh_rate_mask   = 0xFFU,
		.high_temp_threshold = 0U,
		.low_temp_threshold  = 0U,
		.refresh_rate_shift  = 0U,
	},
};

static DDRSSERR_info_t lemans_ddrss_info = {
	.num_llccs = ARRAY_SIZE(lemans_llcc_instances),
	.llccs     = lemans_llcc_instances,
	.tcm_hw    = &DDRSS_TCM_1_0,
	.feac_hw   = &DDRSS_FEAC_1_0,

	.num_mcs   = ARRAY_SIZE(lemans_mc_instances),
	.mcs       = lemans_mc_instances,
	.mc_hw     = &DDRSS_MC_1_0,

	/* SHKE thermal monitoring not used on Lemans */
	.num_mcs_shke    = 0U,
	.mcs_shke        = NULL,
	.shke_hw         = NULL,
	.shke_global_hw  = NULL,
	.danger_en_addr  = NULL,

	.llcc_intr_vector = UINTPTR_MAX,
	.mc_intr_vector   = UINTPTR_MAX,
};

static DDRSSERR_info_oem_t lemans_ddrss_info_oem = {
	.llcc_intr_enable = false,
	.mc_intr_enable   = false,
	.llcc_error_fatal = true,
	.mc_error_fatal   = true,
};

DDRSSERR_config_info_t ddrss_config_info = {
	.num_configs = 1U,
	.configs = (DDRSSERR_propdata_t []){
		{
			.family    = (uint32_t)CHIPINFO_FAMILY_LEMANSAU,
			.match     = false,
			.version   = CHIPINFO_VERSION(1, 0),
			.DDRSSInfo = &lemans_ddrss_info,
		},
	},
};

DDRSSERR_config_info_oem_t ddrss_config_info_oem = {
	.num_configs = 1U,
	.configs = (DDRSSERR_propdata_oem_t []){
		{
			.family       = (uint32_t)CHIPINFO_FAMILY_LEMANSAU,
			.match        = false,
			.version      = CHIPINFO_VERSION(1, 0),
			.DDRSSInfoOEM = &lemans_ddrss_info_oem,
		},
	},
};