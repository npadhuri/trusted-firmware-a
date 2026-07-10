/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * CMSR (Cache Memory Subsystem Registers) error handler.
 */

#include <stdbool.h>
#include <stdint.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <platform_def.h>
#include <qti_interrupt_svc.h>

#include <drivers/qti/chipinfo/chipinfo.h>

#include "cmsr_error.h"

/* I/O helpers */
#define OUT_32(addr, data) \
	(*((volatile uint32_t *)(addr)) = ((uint32_t)(data)))
#define IN_32(addr) \
	((uint32_t)(*((volatile uint32_t *)(addr))))
#define REG_ADDR(base, offset) \
	(((uint8_t *)(base)) + (offset))

/* External config table — defined in the platform data file */
extern CMSRERR_config_info_t cmsr_config_info;

static CMSRERR_info_t *cmsr_info;

void cmsr_error_init_target(CMSRERR_info_t *info);
bool cmsr_error_target_fatal_filter(CMSRERR_info_t *info);

/* Platform config lookup */
static CMSRERR_propdata_t *cmsr_error_platform_get_propdata(void)
{
	enum chipinfo_family family  = chipinfo_get_chip_family();
	uint32_t             version = chipinfo_get_chip_version();
	uint32_t             i;

	for (i = 0U; i < cmsr_config_info.num_configs; i++) {
		CMSRERR_propdata_t *cfg = &cmsr_config_info.configs[i];

		if ((uint32_t)family != cfg->family)
			continue;

		if (version != cfg->version &&
		    (cfg->match || version < cfg->version))
			continue;

		return cfg;
	}

	return NULL;
}

/* Handle core-timeout interrupt */
static bool cmsr_error_handle_core_timeout(void)
{
	bool error_detected = false;
	uint32_t i, j;

	for (i = 0U; i < cmsr_info->num_cmsrs; i++) {
		struct CMSRERR_err_info_t *cmsr = &cmsr_info->cmsrs[i];
		struct CMSRERR_syndrome_t *syn  = &cmsr->cmsr_syndrome;

		syn->master_dwnld_fsm_status =
			IN_32(REG_ADDR(cmsr->base_addr,
				       cmsr_info->cmsr_hw->hw_master_dwnld_fsm_status));

		ERROR("(%x %s %u 0x%x)\n",
		      CMSRERR_CORE_FSM_REG, cmsr->name, i,
		      syn->master_dwnld_fsm_status);

		for (j = 0U; j < cmsr->num_agents; j++) {
			struct CMSRERR_agent_info_t *agent =
				&cmsr->cmsr_agents[j];

			if (agent->core_status_addr != NULL) {
				syn->status_reg_1 =
					IN_32(agent->core_status_addr);
				ERROR("(%x %s %u 0x%x)\n",
				      CMSRERR_CORE_SS_STATUS_REG,
				      agent->name, i, syn->status_reg_1);
				error_detected = true;
			}
		}
	}

	return error_detected;
}

/* Handle agent-timeout interrupt */
static bool cmsr_error_handle_agent_timeout(void)
{
	bool error_detected = false;
	uint32_t i, j;

	for (i = 0U; i < cmsr_info->num_cmsrs; i++) {
		struct CMSRERR_err_info_t *cmsr = &cmsr_info->cmsrs[i];
		struct CMSRERR_syndrome_t *syn  = &cmsr->cmsr_syndrome;

		for (j = 0U; j < cmsr->num_agents; j++) {
			struct CMSRERR_agent_info_t *agent =
				&cmsr->cmsr_agents[j];

			if (agent->uar_status_addr == NULL)
				continue;

			syn->status_reg_2 = IN_32(agent->uar_status_addr);
			ERROR("(%x %s %u 0x%x)\n",
			      CMSRERR_UAR_STATUS, agent->name, i,
			      syn->status_reg_2);

			if ((syn->status_reg_2 & 0x2U) != 0U)
				error_detected = true;

			/*
			 * Assert then de-assert clear-reset after 1 µs
			 * (HPG §2.5.1).
			 */
			if (agent->clear_reset_addr != NULL) {
				OUT_32(agent->clear_reset_addr,
				       IN_32(agent->clear_reset_addr) | 0x4U);
				udelay(1);
				OUT_32(agent->clear_reset_addr,
				       IN_32(agent->clear_reset_addr) & ~0x4U);
			}
		}
	}

	return error_detected;
}

/* Handle boot-mode error interrupt */
static bool cmsr_error_handle_boot_mode(void)
{
	bool error_detected = false;
	uint32_t i;

	for (i = 0U; i < cmsr_info->num_cmsrs; i++) {
		struct CMSRERR_err_info_t *cmsr = &cmsr_info->cmsrs[i];
		struct CMSRERR_syndrome_t *syn  = &cmsr->cmsr_syndrome;

		syn->cmsr_core_ctrl_ss_boot_cmplt_status_reg_0 =
			IN_32(REG_ADDR(cmsr->base_addr,
				       cmsr_info->cmsr_hw->hw_cmsr_dbg_cmsr_core_ctrl_ss_boot_cmplt_status_reg_0));

		ERROR("(%x %s %u 0x%x)\n",
		      CMSRERR_BOOT_STATUS_REG, cmsr->name, i,
		      syn->cmsr_core_ctrl_ss_boot_cmplt_status_reg_0);

		error_detected = true;
	}

	return error_detected;
}

/* ISR — registered for all three CMSR interrupt vectors */
static void *cmsr_error_handle_interrupt(uint32_t int_num, void *ctx)
{
	uintptr_t intr_vector = (uintptr_t)int_num;
	bool fatal_fault_detected    = false;
	bool spurious_fault_detected = false;

	if (cmsr_info == NULL) {
		ERROR("CMSR: (%x) invalid interrupt vector\n",
		      CMSR_ERR_INV_INTR_VECTOR);
		return ctx;
	}

	if (cmsr_info->cmsr_core_timeout_intr_vector == intr_vector) {
		if (cmsr_error_handle_core_timeout()) {
			fatal_fault_detected =
				cmsr_info->cmsr_core_timeout_error_fatal;
		} else {
			spurious_fault_detected = true;
			(void)qti_interrupt_svc_unregister(
				cmsr_info->cmsr_core_timeout_intr_vector);
		}
	} else if (cmsr_info->cmsr_agent_timeout_intr_vector == intr_vector) {
		if (cmsr_error_handle_agent_timeout()) {
			fatal_fault_detected =
				cmsr_info->cmsr_agent_timeout_error_fatal;
		} else {
			spurious_fault_detected = true;
			(void)qti_interrupt_svc_unregister(
				cmsr_info->cmsr_agent_timeout_intr_vector);
		}
	} else if (cmsr_info->cmsr_boot_mode_err_intr_vector == intr_vector) {
		if (cmsr_error_handle_boot_mode()) {
			fatal_fault_detected =
				cmsr_info->cmsr_boot_mode_err_error_fatal;
		} else {
			spurious_fault_detected = true;
			(void)qti_interrupt_svc_unregister(
				cmsr_info->cmsr_boot_mode_err_intr_vector);
		}
	} else {
		ERROR("CMSR: (%x) unknown interrupt vector %u\n",
		      CMSR_ERR_INV_INTR_VECTOR, int_num);
	}

	if (spurious_fault_detected ||
	    (fatal_fault_detected &&
	     cmsr_error_target_fatal_filter(cmsr_info))) {
		ERROR("Fatal CMSR error detected!\n");
		panic();
	}

	return ctx;
}

/* Public init entry point */
static void cmsr_error_init(void)
{
	CMSRERR_propdata_t *propdata;
	uint32_t i;

	propdata = cmsr_error_platform_get_propdata();
	if (propdata == NULL)
		return;

	cmsr_info = propdata->CMSRInfo;
	if (cmsr_info == NULL)
		return;

	/* Target-specific hook (stub or platform override). */
	cmsr_error_init_target(cmsr_info);

	/* Register core-timeout ISR. */
	if (cmsr_info->cmsr_core_timeout_intr_enable) {
		if (qti_interrupt_svc_register(
			    cmsr_info->cmsr_core_timeout_intr_vector,
			    cmsr_error_handle_interrupt,
			    (void *)(uintptr_t)
			    cmsr_info->cmsr_core_timeout_intr_vector) != 0) {
			ERROR("CMSR: (%x) core-timeout ISR registration failed\n",
			      CMSR_INIT_ERR);
			panic();
			return;
		}
	}

	/* Register agent-timeout ISR. */
	if (cmsr_info->cmsr_agent_timeout_intr_enable) {
		if (qti_interrupt_svc_register(
			    cmsr_info->cmsr_agent_timeout_intr_vector,
			    cmsr_error_handle_interrupt,
			    (void *)(uintptr_t)
			    cmsr_info->cmsr_agent_timeout_intr_vector) != 0) {
			ERROR("CMSR: (%x) agent-timeout ISR registration failed\n",
			      CMSR_INIT_ERR);
			panic();
			return;
		}
	}

	/* Register boot-mode-error ISR. */
	if (cmsr_info->cmsr_boot_mode_err_intr_enable) {
		if (qti_interrupt_svc_register(
			    cmsr_info->cmsr_boot_mode_err_intr_vector,
			    cmsr_error_handle_interrupt,
			    (void *)(uintptr_t)
			    cmsr_info->cmsr_boot_mode_err_intr_vector) != 0) {
			ERROR("CMSR: (%x) boot-mode ISR registration failed\n",
			      CMSR_INIT_ERR);
			panic();
			return;
		}
	}

	/* Program timeout values and enable CPU interrupts per CMSR. */
	for (i = 0U; i < cmsr_info->num_cmsrs; i++) {
		struct CMSRERR_err_info_t *cmsr = &cmsr_info->cmsrs[i];

		if (cmsr->base_addr == NULL)
			continue;

		OUT_32(REG_ADDR(cmsr->base_addr,
				cmsr_info->cmsr_hw->hw_dbg_cmsr_core_ctrl_req_timeout),
		       cmsr_info->cmsr_agent_timeout_val);

		OUT_32(REG_ADDR(cmsr->base_addr,
				cmsr_info->cmsr_hw->hw_dbg_cmsr_Core_ctrl_req_timeout_core),
		       cmsr_info->cmsr_core_timeout_val);

		OUT_32(REG_ADDR(cmsr->base_addr,
				cmsr_info->cmsr_hw->hw_CPU_interrupt_enable),
		       0x1U);
	}
}

/* Weak target-layer hooks — platforms override these in their own file */
#pragma weak cmsr_error_init_target
void cmsr_error_init_target(CMSRERR_info_t *info __unused)
{
}

#pragma weak cmsr_error_target_fatal_filter
bool cmsr_error_target_fatal_filter(CMSRERR_info_t *info __unused)
{
	return true;
}

void qti_cmsr_error_init(void)
{
	cmsr_error_init();
}
