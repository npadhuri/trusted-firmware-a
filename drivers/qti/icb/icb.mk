#
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# QTI ICB (Interconnect Bus) NOC error logger driver
#

$(eval $(call add_define,QTI_ICB_ENABLED))

ICB_BASE	:=	drivers/qti/icb

PLAT_INCLUDES	+=	-I$(ICB_BASE)
PLAT_INCLUDES   +=      -I$(ICB_BASE)/include
BL31_SOURCES    +=      $(ICB_BASE)/icbuarb.c

#
# Platform back-end. Each chipset directory under drivers/qti/icb/<CHIPSET>/
# must provide:
#   - icbuarb_target.c      (defines icbuarb_target_get_info() and
#                            icbuarb_target_init())
#
# When CHIPSET is unset a target stub with default (no-op) behaviour is
# used so the common driver can still be built and linked.
#
ifneq ($(CHIPSET),)
PLAT_INCLUDES	+=	-I$(ICB_BASE)/$(CHIPSET)
BL31_SOURCES	+=	$(ICB_BASE)/$(CHIPSET)/icbuarb_target.c		\
			$(ICB_BASE)/$(CHIPSET)/u_data.c	
endif
