/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * DDRSS (DDR Subsystem) error handler — public types and data structures.
 */

#ifndef QTI_DDRSS_ERROR_H
#define QTI_DDRSS_ERROR_H

#include <stdbool.h>
#include <stdint.h>

/* Error-code constants */
#define DDRSSERR_FEWC_STATUS_REG            0xD001U
#define DDRSSERR_FEAC_DEC_ERR_STATUS_REG    0xD002U
#define DDRSSERR_FEAC_UIMAGE_ERR_STATUS_REG 0xD003U
#define DDRSSERR_FEAC_BYPASS_ERR_STATUS_REG 0xD004U
#define DDRSSERR_MC_ADDR_DECERR_ESYN        0xD005U
#define DDRSS_MC_REFRESH_ERR_STATUS         0xD006U
#define DDRSS_ERR_INV_INTR_VECTOR           0xD007U
#define DDRSS_INIT_ERR                      0xD008U
#define DDRSS_ERROR_FATAL                   0xD009U

/* Hardware register-offset descriptors */

/* TCM (Tag Cache Memory) */
struct DDRSS_tcm_hw_t {
	uint16_t interrupt_status;
	uint16_t status1;
	uint16_t status2;
	uint16_t status3;
	uint16_t status4;
	uint16_t status5;
	uint16_t status6;
	uint16_t status7;
	uint16_t status8;
	uint16_t tcm_error_cfg;
	uint16_t interrupt_clear;
};

/* FEAC (Front-End Address Check) */
struct DDRSS_feac_hw_t {
	uint16_t interrupt_status;
	uint16_t status0;
	uint16_t status1;
	uint16_t status2;
	uint16_t status3;
	uint16_t status4;
	uint16_t status5;
	uint16_t status6;
	uint16_t dec_intrpt_cfg;
	uint16_t interrupt_clr;
	uint16_t uimage_region_err_status1;
	uint16_t uimage_region_err_status2;
	uint16_t uimage_region_err_status3;
	uint16_t uimage_region_err_status4;
	uint16_t uimage_region_err_status5;
	uint16_t uimage_region_err_status6;
	uint16_t interrupt_bypass_status;
	uint16_t bypass_err_status1;
	uint16_t bypass_err_status2;
	uint16_t bypass_err_status3;
	uint16_t bypass_err_status4;
	uint16_t bypass_err_status5;
	uint16_t bypass_err_status6;
	uint16_t interrupt_bypass_clr;
};

/* MC (Memory Controller) */
struct DDRSS_mc_hw_t {
	uint16_t interrupt_status;
	uint16_t addr_decerr_esyn_0;
	uint16_t addr_decerr_esyn_1;
	uint16_t interrupt_clear;
	uint16_t refresh_err_esyn;
};

/* SHKE (Self-Refresh / Keep-Alive Engine) */
struct DDRSS_shke_hw_t {
	uint16_t temp_error_cfg;
	uint16_t load_confg;
};

struct DDRSS_shke_global_hw_t {
	uint16_t shke_status;
};

/* Captured syndrome registers */

struct DDRSSERR_llcc_syndrome_t {
	uint32_t status0;
	uint32_t status1;
	uint32_t status2;
	uint32_t status3;
	uint32_t status4;
	uint32_t status5;
	uint32_t status6;
	uint32_t status7;
	uint32_t status8;
};

struct DDRSSERR_llcc_feac_ext_syndrome_t {
	uint32_t uimage_region_err_status1;
	uint32_t uimage_region_err_status2;
	uint32_t uimage_region_err_status3;
	uint32_t uimage_region_err_status4;
	uint32_t uimage_region_err_status5;
	uint32_t uimage_region_err_status6;
	uint32_t bypass_err_status1;
	uint32_t bypass_err_status2;
	uint32_t bypass_err_status3;
	uint32_t bypass_err_status4;
	uint32_t bypass_err_status5;
	uint32_t bypass_err_status6;
};

struct DDRSSERR_mc_syndrome_t {
	uint32_t addr_decerr_esyn_0;
	uint32_t addr_decerr_esyn_1;
};

struct DDRSSERR_shke_syndrome_t {
	uint32_t temp_error_cfg;
	uint32_t shke_status;
	uint32_t refresh_err_esyn;
};

/* Per-LLCC instance */
struct DDRSSERR_llcc_info_t {
	char                                     *name;
	void                                     *tcm_base_addr;
	void                                     *feac_base_addr;
	struct DDRSSERR_llcc_syndrome_t           tcm_syndrome;
	struct DDRSSERR_llcc_syndrome_t           feac_syndrome;
	struct DDRSSERR_llcc_feac_ext_syndrome_t *feac_extended_syndrome;
};

/* Per-MC instance */
struct DDRSSERR_mc_info_t {
	char                          *name;
	void                          *base_addr;
	struct DDRSSERR_mc_syndrome_t  mc_syndrome;
	uint32_t                       refresh_rate_mask;
	uint32_t                       high_temp_threshold;
	uint32_t                       low_temp_threshold;
	uint32_t                       refresh_rate_shift;
};

/* Per-SHKE instance */
struct DDRSSERR_shke_info_t {
	char                            *name;
	void                            *base_addr;
	void                            *global_base_addr;
	struct DDRSSERR_shke_syndrome_t  shke_syndrome;
	uint32_t                         high_temp_threshold;
	uint32_t                         low_temp_threshold;
	uint32_t                         refresh_rate_shift;
};

/* Top-level DDRSS info structure */
typedef struct {
	uint32_t                    num_llccs;
	struct DDRSSERR_llcc_info_t *llccs;
	struct DDRSS_tcm_hw_t       *tcm_hw;
	struct DDRSS_feac_hw_t      *feac_hw;

	uint32_t                    num_mcs;
	struct DDRSSERR_mc_info_t   *mcs;
	struct DDRSS_mc_hw_t        *mc_hw;

	uint32_t                    num_mcs_shke;
	struct DDRSSERR_shke_info_t *mcs_shke;
	struct DDRSS_shke_hw_t      *shke_hw;
	struct DDRSS_shke_global_hw_t *shke_global_hw;

	void                        *danger_en_addr;

	/* Interrupt vectors */
	uintptr_t  llcc_intr_vector;
	uintptr_t  mc_intr_vector;
} DDRSSERR_info_t;

/* OEM info */
typedef struct {
	bool  llcc_intr_enable;
	bool  mc_intr_enable;
	bool  llcc_error_fatal;
	bool  mc_error_fatal;
} DDRSSERR_info_oem_t;

/* Property-data wrappers */
typedef struct {
	uint32_t           family;
	bool               match;
	uint32_t           version;
	DDRSSERR_info_t   *DDRSSInfo;
} DDRSSERR_propdata_t;

typedef struct {
	uint32_t             family;
	bool                 match;
	uint32_t             version;
	DDRSSERR_info_oem_t *DDRSSInfoOEM;
} DDRSSERR_propdata_oem_t;

typedef struct {
	uint32_t             num_configs;
	DDRSSERR_propdata_t *configs;
} DDRSSERR_config_info_t;

typedef struct {
	uint32_t                 num_configs;
	DDRSSERR_propdata_oem_t *configs;
} DDRSSERR_config_info_oem_t;

void qti_ddrss_error_init(void);

#endif /* QTI_DDRSS_ERROR_H */