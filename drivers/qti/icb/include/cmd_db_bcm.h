#ifndef CMD_DB_BCM_H
#define CMD_DB_BCM_H
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include <stdint.h>

struct bcm_db {
  uint32_t bw_unit;
  uint16_t bcm_port;
  uint8_t  clk_id;
};

#endif /* CMD_DB_BCM_H */
