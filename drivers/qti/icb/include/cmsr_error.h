/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * CMSR (Cache Memory Subsystem Registers) error handler — public types and
 * data structures.
 */

#ifndef QTI_CMSR_ERROR_H
#define QTI_CMSR_ERROR_H

#include <stdbool.h>
#include <stdint.h>

/* Error-code constants */
#define CMSRERR_CORE_FSM_REG        0xC001U
#define CMSRERR_CORE_SS_STATUS_REG  0xC002U
#define CMSRERR_UAR_STATUS          0xC003U
#define CMSRERR_BOOT_STATUS_REG     0xC004U
#define CMSR_ERR_INV_INTR_VECTOR    0xC005U
#define CMSR_INIT_ERR               0xC006U
#define CMSR_ERROR_FATAL            0xC007U

/* Hardware register-offset descriptor */
struct cmsr_hw {
	uint16_t hw_master_dwnld_fsm_status;
	uint16_t hw_cmsr_dbg_cmsr_core_ctrl_ss_boot_cmplt_status_reg_0;
	uint16_t hw_dbg_cmsr_core_ctrl_req_timeout;
	uint16_t hw_dbg_cmsr_Core_ctrl_req_timeout_core;
	uint16_t hw_CPU_interrupt_enable;
};

/* Per-agent information */
struct CMSRERR_agent_info_t {
	char    *name;
	void    *core_status_addr;   /* NULL if not applicable */
	void    *uar_status_addr;    /* NULL if not applicable */
	void    *clear_reset_addr;   /* NULL if not applicable */
};

/* Captured syndrome registers */
struct CMSRERR_syndrome_t {
	uint32_t master_dwnld_fsm_status;
	uint32_t status_reg_1;
	uint32_t status_reg_2;
	uint32_t cmsr_core_ctrl_ss_boot_cmplt_status_reg_0;
};

/* Per-CMSR-instance configuration */
struct CMSRERR_err_info_t {
	char                       *name;
	void                       *base_addr;
	uint32_t                    num_agents;
	struct CMSRERR_agent_info_t *cmsr_agents;
	struct CMSRERR_syndrome_t   cmsr_syndrome;
};

/* Top-level CMSR info structure */
typedef struct {
	struct cmsr_hw          *cmsr_hw;
	uint32_t                 num_cmsrs;
	struct CMSRERR_err_info_t *cmsrs;

	/* Interrupt vectors */
	uintptr_t  cmsr_core_timeout_intr_vector;
	uintptr_t  cmsr_agent_timeout_intr_vector;
	uintptr_t  cmsr_boot_mode_err_intr_vector;

	/* Interrupt enable flags */
	bool  cmsr_core_timeout_intr_enable;
	bool  cmsr_agent_timeout_intr_enable;
	bool  cmsr_boot_mode_err_intr_enable;

	/* Error-fatal flags */
	bool  cmsr_core_timeout_error_fatal;
	bool  cmsr_agent_timeout_error_fatal;
	bool  cmsr_boot_mode_err_error_fatal;

	/* Timeout values written to hardware */
	uint32_t  cmsr_agent_timeout_val;
	uint32_t  cmsr_core_timeout_val;
} CMSRERR_info_t;

/* Property-data wrapper */
typedef struct {
	uint32_t        family;
	bool            match;       /* true = exact version match required */
	uint32_t        version;
	CMSRERR_info_t *CMSRInfo;
} CMSRERR_propdata_t;

/* Config-info table */
typedef struct {
	uint32_t            num_configs;
	CMSRERR_propdata_t *configs;
} CMSRERR_config_info_t;

void  qti_cmsr_error_init(void);

#endif /* QTI_CMSR_ERROR_H */