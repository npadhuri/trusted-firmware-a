/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * DDRSS (DDR Subsystem) error handler.
 */

#include <stdbool.h>
#include <stdint.h>

#include <common/debug.h>
#include <lib/mmio.h>
#include <platform_def.h>
#include <qti_interrupt_svc.h>

#include <drivers/qti/chipinfo/chipinfo.h>

#include "ddrss_error.h"

/* I/O helpers */
#define OUT_32(addr, data) \
	(*((volatile uint32_t *)(addr)) = ((uint32_t)(data)))
#define IN_32(addr) \
	((uint32_t)(*((volatile uint32_t *)(addr))))
#define OUTM_32(addr, mask, data) \
	OUT_32(addr, ((IN_32(addr) & ~(mask)) | ((data) & (mask))))
#define REG_ADDR(base, offset) \
	(((uint8_t *)(base)) + (offset))
#define REGISTER_VALID(offs) ((offs) != 0xFFFFU)

/* Error status masks */
#define DECODE_ERR_STATUS_MASK    0x1U
#define UIMAGE_ERR_STATUS_MASK    0x2U
#define BYPASS_ERR_STATUS_MASK    0x1U

/* External config tables — defined in the platform data file */
extern DDRSSERR_config_info_t     ddrss_config_info;
extern DDRSSERR_config_info_oem_t ddrss_config_info_oem;

static DDRSSERR_info_t     *ddrss_info;
static DDRSSERR_info_oem_t *ddrss_info_oem;

bool ddrss_error_target_fatal_filter(DDRSSERR_info_t *info);

/* Platform config lookup */
static DDRSSERR_propdata_t *ddrss_error_platform_get_propdata(void)
{
	enum chipinfo_family family  = chipinfo_get_chip_family();
	uint32_t             version = chipinfo_get_chip_version();
	uint32_t             i;

	for (i = 0U; i < ddrss_config_info.num_configs; i++) {
		DDRSSERR_propdata_t *cfg = &ddrss_config_info.configs[i];

		if ((uint32_t)family != cfg->family)
			continue;
		if (version != cfg->version &&
		    (cfg->match || version < cfg->version))
			continue;
		return cfg;
	}
	return NULL;
}

static DDRSSERR_propdata_oem_t *ddrss_error_platform_get_propdata_oem(void)
{
	enum chipinfo_family family  = chipinfo_get_chip_family();
	uint32_t             version = chipinfo_get_chip_version();
	uint32_t             i;

	for (i = 0U; i < ddrss_config_info_oem.num_configs; i++) {
		DDRSSERR_propdata_oem_t *cfg = &ddrss_config_info_oem.configs[i];

		if ((uint32_t)family != cfg->family)
			continue;
		if (version != cfg->version &&
		    (cfg->match || version < cfg->version))
			continue;
		return cfg;
	}
	return NULL;
}

/* Handle LLCC errors (TCM + FEAC) */
static bool ddrss_error_handle_llcc(void)
{
	bool error_detected = false;
	uint32_t idx;

	for (idx = 0U; idx < ddrss_info->num_llccs; idx++) {
		struct DDRSSERR_llcc_info_t *llcc    = &ddrss_info->llccs[idx];
		struct DDRSS_tcm_hw_t       *tcm_hw  = ddrss_info->tcm_hw;
		struct DDRSS_feac_hw_t      *feac_hw = ddrss_info->feac_hw;
		uint32_t val;
		uint32_t feac_intr_status;

		/* --- TCM errors --- */
		if (llcc->tcm_base_addr != NULL && tcm_hw != NULL) {
			val = IN_32(REG_ADDR(llcc->tcm_base_addr,
					     tcm_hw->interrupt_status));
			if (val != 0U) {
				struct DDRSSERR_llcc_syndrome_t *syn =
					&llcc->tcm_syndrome;

				error_detected = true;

				if (REGISTER_VALID(tcm_hw->status1))
					syn->status1 = IN_32(REG_ADDR(llcc->tcm_base_addr, tcm_hw->status1));
				if (REGISTER_VALID(tcm_hw->status2))
					syn->status2 = IN_32(REG_ADDR(llcc->tcm_base_addr, tcm_hw->status2));
				if (REGISTER_VALID(tcm_hw->status3))
					syn->status3 = IN_32(REG_ADDR(llcc->tcm_base_addr, tcm_hw->status3));
				if (REGISTER_VALID(tcm_hw->status4))
					syn->status4 = IN_32(REG_ADDR(llcc->tcm_base_addr, tcm_hw->status4));
				if (REGISTER_VALID(tcm_hw->status5))
					syn->status5 = IN_32(REG_ADDR(llcc->tcm_base_addr, tcm_hw->status5));
				if (REGISTER_VALID(tcm_hw->status6))
					syn->status6 = IN_32(REG_ADDR(llcc->tcm_base_addr, tcm_hw->status6));
				if (REGISTER_VALID(tcm_hw->status7))
					syn->status7 = IN_32(REG_ADDR(llcc->tcm_base_addr, tcm_hw->status7));
				if (REGISTER_VALID(tcm_hw->status8))
					syn->status8 = IN_32(REG_ADDR(llcc->tcm_base_addr, tcm_hw->status8));

				ERROR("(%x %s 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x)\n",
				      DDRSSERR_FEWC_STATUS_REG, llcc->name,
				      syn->status1, syn->status2, syn->status3,
				      syn->status4, syn->status5, syn->status6,
				      syn->status7, syn->status8);

				/* Clear */
				OUT_32(REG_ADDR(llcc->tcm_base_addr,
						tcm_hw->tcm_error_cfg), 0x1U);
				OUT_32(REG_ADDR(llcc->tcm_base_addr,
						tcm_hw->interrupt_clear), 0x1U);
			}
		}

		/* --- FEAC decode errors --- */
		if (llcc->feac_base_addr == NULL || feac_hw == NULL)
			continue;

		feac_intr_status = IN_32(REG_ADDR(llcc->feac_base_addr,
						  feac_hw->interrupt_status));

		if ((feac_intr_status & DECODE_ERR_STATUS_MASK) != 0U) {
			struct DDRSSERR_llcc_syndrome_t *syn = &llcc->feac_syndrome;

			error_detected = true;

			if (REGISTER_VALID(feac_hw->status0))
				syn->status0 = IN_32(REG_ADDR(llcc->feac_base_addr, feac_hw->status0));
			if (REGISTER_VALID(feac_hw->status1))
				syn->status1 = IN_32(REG_ADDR(llcc->feac_base_addr, feac_hw->status1));
			if (REGISTER_VALID(feac_hw->status2))
				syn->status2 = IN_32(REG_ADDR(llcc->feac_base_addr, feac_hw->status2));
			if (REGISTER_VALID(feac_hw->status3))
				syn->status3 = IN_32(REG_ADDR(llcc->feac_base_addr, feac_hw->status3));
			if (REGISTER_VALID(feac_hw->status4))
				syn->status4 = IN_32(REG_ADDR(llcc->feac_base_addr, feac_hw->status4));
			if (REGISTER_VALID(feac_hw->status5))
				syn->status5 = IN_32(REG_ADDR(llcc->feac_base_addr, feac_hw->status5));
			if (REGISTER_VALID(feac_hw->status6))
				syn->status6 = IN_32(REG_ADDR(llcc->feac_base_addr, feac_hw->status6));

			ERROR("(%x %s 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x)\n",
			      DDRSSERR_FEAC_DEC_ERR_STATUS_REG, llcc->name,
			      syn->status1, syn->status2, syn->status3,
			      syn->status4, syn->status5, syn->status0,
			      syn->status6);

			/* Clear */
			OUTM_32(REG_ADDR(llcc->feac_base_addr, feac_hw->dec_intrpt_cfg),
				0x1U, 0x1U);
			OUTM_32(REG_ADDR(llcc->feac_base_addr, feac_hw->interrupt_clr),
				0x1U, 0x1U);
		}

		/* --- FEAC uimage errors --- */
		if (((feac_intr_status & UIMAGE_ERR_STATUS_MASK) != 0U) &&
		    (llcc->feac_extended_syndrome != NULL)) {
			struct DDRSSERR_llcc_feac_ext_syndrome_t *ext =
				llcc->feac_extended_syndrome;

			error_detected = true;

			if (REGISTER_VALID(feac_hw->uimage_region_err_status1))
				ext->uimage_region_err_status1 =
					IN_32(REG_ADDR(llcc->feac_base_addr,
						feac_hw->uimage_region_err_status1));
			if (REGISTER_VALID(feac_hw->uimage_region_err_status2))
				ext->uimage_region_err_status2 =
					IN_32(REG_ADDR(llcc->feac_base_addr,
						feac_hw->uimage_region_err_status2));
			if (REGISTER_VALID(feac_hw->uimage_region_err_status3))
				ext->uimage_region_err_status3 =
					IN_32(REG_ADDR(llcc->feac_base_addr,
						feac_hw->uimage_region_err_status3));
			if (REGISTER_VALID(feac_hw->uimage_region_err_status4))
				ext->uimage_region_err_status4 =
					IN_32(REG_ADDR(llcc->feac_base_addr,
						feac_hw->uimage_region_err_status4));
			if (REGISTER_VALID(feac_hw->uimage_region_err_status5))
				ext->uimage_region_err_status5 =
					IN_32(REG_ADDR(llcc->feac_base_addr,
						feac_hw->uimage_region_err_status5));
			if (REGISTER_VALID(feac_hw->uimage_region_err_status6))
				ext->uimage_region_err_status6 =
					IN_32(REG_ADDR(llcc->feac_base_addr,
						feac_hw->uimage_region_err_status6));

			ERROR("(%x %s 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x)\n",
			      DDRSSERR_FEAC_UIMAGE_ERR_STATUS_REG, llcc->name,
			      ext->uimage_region_err_status1,
			      ext->uimage_region_err_status2,
			      ext->uimage_region_err_status3,
			      ext->uimage_region_err_status4,
			      ext->uimage_region_err_status5,
			      ext->uimage_region_err_status6);

			/* Clear */
			OUTM_32(REG_ADDR(llcc->feac_base_addr, feac_hw->dec_intrpt_cfg),
				0x4U, 0x4U);
			OUTM_32(REG_ADDR(llcc->feac_base_addr, feac_hw->interrupt_clr),
				0x2U, 0x2U);
		}

		/* --- FEAC bypass errors --- */
		if (REGISTER_VALID(feac_hw->interrupt_bypass_status)) {
			uint32_t bypass_status = IN_32(REG_ADDR(llcc->feac_base_addr,
								feac_hw->interrupt_bypass_status));

			if (((bypass_status & BYPASS_ERR_STATUS_MASK) != 0U) &&
			    (llcc->feac_extended_syndrome != NULL)) {
				struct DDRSSERR_llcc_feac_ext_syndrome_t *ext =
					llcc->feac_extended_syndrome;

				error_detected = true;

				if (REGISTER_VALID(feac_hw->bypass_err_status1))
					ext->bypass_err_status1 = IN_32(REG_ADDR(llcc->feac_base_addr, feac_hw->bypass_err_status1));
				if (REGISTER_VALID(feac_hw->bypass_err_status2))
					ext->bypass_err_status2 = IN_32(REG_ADDR(llcc->feac_base_addr, feac_hw->bypass_err_status2));
				if (REGISTER_VALID(feac_hw->bypass_err_status3))
					ext->bypass_err_status3 = IN_32(REG_ADDR(llcc->feac_base_addr, feac_hw->bypass_err_status3));
				if (REGISTER_VALID(feac_hw->bypass_err_status4))
					ext->bypass_err_status4 = IN_32(REG_ADDR(llcc->feac_base_addr, feac_hw->bypass_err_status4));
				if (REGISTER_VALID(feac_hw->bypass_err_status5))
					ext->bypass_err_status5 = IN_32(REG_ADDR(llcc->feac_base_addr, feac_hw->bypass_err_status5));
				if (REGISTER_VALID(feac_hw->bypass_err_status6))
					ext->bypass_err_status6 = IN_32(REG_ADDR(llcc->feac_base_addr, feac_hw->bypass_err_status6));

				ERROR("(%x %s 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x)\n",
				      DDRSSERR_FEAC_BYPASS_ERR_STATUS_REG, llcc->name,
				      ext->bypass_err_status1, ext->bypass_err_status2,
				      ext->bypass_err_status3, ext->bypass_err_status4,
				      ext->bypass_err_status5, ext->bypass_err_status6);

				/* Clear */
				OUTM_32(REG_ADDR(llcc->feac_base_addr, feac_hw->dec_intrpt_cfg),
					0x8U, 0x8U);
				OUTM_32(REG_ADDR(llcc->feac_base_addr, feac_hw->interrupt_bypass_clr),
					0x1U, 0x1U);
			}
		}
	}

	return error_detected;
}

/* Handle MC errors */
static bool ddrss_error_handle_mc(void)
{
	bool error_detected = false;
	uint32_t idx;

	for (idx = 0U; idx < ddrss_info->num_mcs; idx++) {
		struct DDRSSERR_mc_info_t *mc    = &ddrss_info->mcs[idx];
		struct DDRSS_mc_hw_t      *mc_hw = ddrss_info->mc_hw;
		uint32_t val;

		if (mc->base_addr == NULL || mc_hw == NULL)
			continue;

		val = IN_32(REG_ADDR(mc->base_addr,
				     mc_hw->interrupt_status)) & (0x1U << 16);
		if (val != 0U) {
			mc->mc_syndrome.addr_decerr_esyn_0 =
				IN_32(REG_ADDR(mc->base_addr,
					       mc_hw->addr_decerr_esyn_0));
			mc->mc_syndrome.addr_decerr_esyn_1 =
				IN_32(REG_ADDR(mc->base_addr,
					       mc_hw->addr_decerr_esyn_1));

			ERROR("(%x %s 0x%x 0x%x)\n",
			      DDRSSERR_MC_ADDR_DECERR_ESYN, mc->name,
			      mc->mc_syndrome.addr_decerr_esyn_0,
			      mc->mc_syndrome.addr_decerr_esyn_1);
			error_detected = true;

			/* Clear */
			OUT_32(REG_ADDR(mc->base_addr, mc_hw->interrupt_clear),
			       0x1U << 16);
		}

		/* SHKE thermal errors (optional) */
		if (idx < ddrss_info->num_mcs_shke &&
		    ddrss_info->mcs_shke != NULL &&
		    ddrss_info->shke_hw != NULL) {
			uint32_t thermal_val = IN_32(REG_ADDR(mc->base_addr,
							      mc_hw->interrupt_status)) &
					       (0x1U << 4);

			if (thermal_val != 0U) {
				struct DDRSSERR_shke_info_t *shke =
					&ddrss_info->mcs_shke[idx];

				shke->shke_syndrome.refresh_err_esyn =
					IN_32(REG_ADDR(mc->base_addr,
						       mc_hw->refresh_err_esyn));

				ERROR("(%x %s 0x%x)\n",
				      DDRSS_MC_REFRESH_ERR_STATUS, mc->name,
				      shke->shke_syndrome.refresh_err_esyn);

				/* Clear */
				OUT_32(REG_ADDR(mc->base_addr,
						mc_hw->interrupt_clear),
				       0x1U << 4);
			}
		}
	}

	return error_detected;
}

/* ISR */
static void *ddrss_error_handle_interrupt(uint32_t int_num, void *ctx)
{
	uintptr_t intr_vector = (uintptr_t)int_num;
	bool fatal_fault_detected = false;

	if (ddrss_info == NULL || ddrss_info_oem == NULL) {
		ERROR("DDRSS: (%x) invalid interrupt vector\n",
		      DDRSS_ERR_INV_INTR_VECTOR);
		return ctx;
	}

	if (ddrss_info->llcc_intr_vector == intr_vector) {
		if (ddrss_error_handle_llcc())
			fatal_fault_detected = ddrss_info_oem->llcc_error_fatal;
	} else if (ddrss_info->mc_intr_vector == intr_vector) {
		if (ddrss_error_handle_mc())
			fatal_fault_detected = ddrss_info_oem->mc_error_fatal;
	} else {
		ERROR("DDRSS: (%x) unknown interrupt vector %u\n",
		      DDRSS_ERR_INV_INTR_VECTOR, int_num);
	}

	if (fatal_fault_detected &&
	    ddrss_error_target_fatal_filter(ddrss_info)) {
		ERROR("Fatal DDRSS error detected!\n");
		panic();
	}

	return ctx;
}

/* DDRSS-specific init (called from qti_icb_error_init in noc_error.c) */
static void ddrss_error_init(void)
{
	DDRSSERR_propdata_t     *propdata;
	DDRSSERR_propdata_oem_t *propdata_oem;
	uint32_t i;

	propdata = ddrss_error_platform_get_propdata();
	if (propdata == NULL)
		return;

	propdata_oem = ddrss_error_platform_get_propdata_oem();
	if (propdata_oem == NULL)
		return;

	ddrss_info     = propdata->DDRSSInfo;
	ddrss_info_oem = propdata_oem->DDRSSInfoOEM;

	if (ddrss_info == NULL || ddrss_info_oem == NULL)
		return;

	/* Register LLCC error ISR. */
	if (ddrss_info_oem->llcc_intr_enable) {
		if (qti_interrupt_svc_register(
			    ddrss_info->llcc_intr_vector,
			    ddrss_error_handle_interrupt,
			    (void *)(uintptr_t)
			    ddrss_info->llcc_intr_vector) != 0) {
			ERROR("DDRSS: (%x) LLCC ISR registration failed\n",
			      DDRSS_INIT_ERR);
			panic();
			return;
		}

		/* Enable SB and DB errors for each LLCC. */
		for (i = 0U; i < ddrss_info->num_llccs; i++) {
			struct DDRSSERR_llcc_info_t *llcc = &ddrss_info->llccs[i];

			if (ddrss_info->tcm_hw != NULL &&
			    llcc->tcm_base_addr != NULL)
				OUTM_32(REG_ADDR(llcc->tcm_base_addr,
						 ddrss_info->tcm_hw->tcm_error_cfg),
					0x3U, 0x3U);

			if (ddrss_info->feac_hw != NULL &&
			    llcc->feac_base_addr != NULL)
				OUTM_32(REG_ADDR(llcc->feac_base_addr,
						 ddrss_info->feac_hw->dec_intrpt_cfg),
					0x3U, 0x3U);
		}
	}

	/* Register MC error ISR. */
	if (ddrss_info_oem->mc_intr_enable) {
		if (qti_interrupt_svc_register(
			    ddrss_info->mc_intr_vector,
			    ddrss_error_handle_interrupt,
			    (void *)(uintptr_t)
			    ddrss_info->mc_intr_vector) != 0) {
			ERROR("DDRSS: (%x) MC ISR registration failed\n",
			      DDRSS_INIT_ERR);
			panic();
			return;
		}
	}
}

/* Weak target-layer hook */
#pragma weak ddrss_error_target_fatal_filter
bool ddrss_error_target_fatal_filter(DDRSSERR_info_t *info __unused)
{
	return true;
}

void qti_ddrss_error_init(void)
{
	ddrss_error_init();
}
