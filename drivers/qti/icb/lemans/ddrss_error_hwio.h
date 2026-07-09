/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * DDRSS (DDR Subsystem) physical base addresses for the Lemans (QCS9075)
 * platform.  Update from the Lemans memory map / HPG before enabling.
 */

#ifndef QTI_DDRSS_ERROR_HWIO_H
#define QTI_DDRSS_ERROR_HWIO_H

#include <stdint.h>

/* LLCC (Last-Level Cache Controller) — TCM base addresses */
#define LLCC0_TCM_BASE   0x09200000U
#define LLCC1_TCM_BASE   0x09240000U
#define LLCC2_TCM_BASE   0x09280000U
#define LLCC3_TCM_BASE   0x092C0000U
#define LLCC4_TCM_BASE   0x09300000U
#define LLCC5_TCM_BASE   0x09340000U

/* LLCC — FEAC (Front-End Address Check) base addresses */
#define LLCC0_FEAC_BASE  0x09210000U
#define LLCC1_FEAC_BASE  0x09250000U
#define LLCC2_FEAC_BASE  0x09290000U
#define LLCC3_FEAC_BASE  0x092D0000U
#define LLCC4_FEAC_BASE  0x09310000U
#define LLCC5_FEAC_BASE  0x09350000U

/* MC (Memory Controller) base addresses */
#define MC0_BASE         0x04480000U
#define MC1_BASE         0x04490000U
#define MC2_BASE         0x044A0000U
#define MC3_BASE         0x044B0000U

#endif /* QTI_DDRSS_ERROR_HWIO_H */