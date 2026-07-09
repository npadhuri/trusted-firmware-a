/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * CMSR hardware register-offset descriptors.
 * Follows the same pattern as noc_hw_type.h.
 */

#ifndef QTI_CMSR_HW_TYPE_H
#define QTI_CMSR_HW_TYPE_H

#include <lib/utils_def.h>

#include "cmsr_error.h"

/*
 * CMSR_1_0 — register layout for the CMSR 1.0 IP block.
 *
 * All offsets are relative to the CMSR instance base address.
 */
static struct cmsr_hw CMSR_1_0 __attribute__((unused)) = {
	.hw_master_dwnld_fsm_status                            = 0x0100U,
	.hw_cmsr_dbg_cmsr_core_ctrl_ss_boot_cmplt_status_reg_0 = 0x0104U,
	.hw_dbg_cmsr_core_ctrl_req_timeout                     = 0x0108U,
	.hw_dbg_cmsr_Core_ctrl_req_timeout_core                = 0x010CU,
	.hw_CPU_interrupt_enable                               = 0x0110U,
};

#endif /* QTI_CMSR_HW_TYPE_H */