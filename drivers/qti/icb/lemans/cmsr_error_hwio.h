/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * CMSR (Cache Memory Subsystem Registers) physical base addresses for the
 * Lemans (QCS9075) platform.  Update from the Lemans memory map / HPG
 * before enabling.
 */

#ifndef QTI_CMSR_ERROR_HWIO_H
#define QTI_CMSR_ERROR_HWIO_H

#include <stdint.h>

/* CMSR instance base address */
#define CMSR0_BASE                  0x09400000U

/* CMSR0 agent register addresses */
#define CMSR0_AGENT0_CORE_STATUS    0x09401000U
#define CMSR0_AGENT0_UAR_STATUS     0x09401004U
#define CMSR0_AGENT0_CLEAR_RESET    0x09401008U

#endif /* QTI_CMSR_ERROR_HWIO_H */