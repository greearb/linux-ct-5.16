// SPDX-License-Identifier: ISC
/* Copyright (C) 2020 MediaTek Inc. */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/pci.h>

#include "mt7915.h"
#include "mac.h"
#include "../trace.h"

static const u32 mt7915_base[] = {
	[MT_REMAP_L1_CFG_BASE]	= 0xf1000,
	[MT_REMAP_L1_BASE]	= 0xe0000,
	[MT_REMAP_L2_CFG_BASE]	= 0xf1000,
	[MT_REMAP_L2_BASE]	= 0x00000,
	[MT_INFRA_MCU_END_BASE]	= 0x7c3fffff,
	[MT_PCIE_MAC_BASE]	= 0x74030000,
	[MT_PCIE1_MAC_BASE]	= 0x74020000,
	[MT_WFDMA0_BASE]	= 0xd4000,
	[MT_WFDMA1_BASE]	= 0xd5000,
	[MT_WFDMA0_PCIE1_BASE]	= 0xd8000,
	[MT_WFDMA1_PCIE1_BASE]	= 0xd9000,
	[MT_WFDMA_EXT_CSR_BASE]	= 0xd7000,
	[MT_SWDEF_BASE]		= 0x41f200,
	[MT_MCU_WFDMA0_BASE]	= 0x2000,
	[MT_MCU_WFDMA1_BASE]	= 0x3000,
};

static const u32 mt7916_base[] = {
	[MT_REMAP_L1_CFG_BASE]	= 0xfe000,
	[MT_REMAP_L1_BASE]	= 0xe0000,
	[MT_REMAP_L2_CFG_BASE]	= 0x00000,
	[MT_REMAP_L2_BASE]	= 0x40000,
	[MT_INFRA_MCU_END_BASE]	= 0x7c085fff,
	[MT_PCIE_MAC_BASE]	= 0x74030000,
	[MT_PCIE1_MAC_BASE]	= 0x74090000,
	[MT_WFDMA0_BASE]	= 0xd4000,
	[MT_WFDMA1_BASE]	= 0xd5000,
	[MT_WFDMA0_PCIE1_BASE]	= 0xd8000,
	[MT_WFDMA1_PCIE1_BASE]	= 0xd9000,
	[MT_WFDMA_EXT_CSR_BASE]	= 0xd7000,
	[MT_SWDEF_BASE]		= 0x411400,
	[MT_MCU_WFDMA0_BASE]	= 0x2000,
	[MT_MCU_WFDMA1_BASE]	= 0x3000,
};

static const struct __reg mt7915_reg[] = {
	[L1_REMAP_CFG_OFFSET]	= { MT_REMAP_L1_CFG_BASE, 0x1ac },
	[L2_REMAP_CFG_OFFSET]	= { MT_REMAP_L2_CFG_BASE, 0x1b0 },
	[INT_SOURCE_CSR]	= { MT_WFDMA_EXT_CSR_BASE, 0x10 },
	[INT_MASK_CSR]		= { MT_WFDMA_EXT_CSR_BASE, 0x14 },
	[INT1_SOURCE_CSR]	= { MT_WFDMA_EXT_CSR_BASE, 0x88 },
	[INT1_MASK_CSR]		= { MT_WFDMA_EXT_CSR_BASE, 0x8c },
	[INT_MCU_CMD_SOURCE]	= { MT_WFDMA1_BASE, 0x1f0 },
	[INT_MCU_CMD_EVENT]	= { MT_MCU_WFDMA1_BASE, 0x108 },
	[TX_RING_CTRL_FWDL]	= { MT_WFDMA1_BASE, 0x640 },
	[TX_RING_CTRL_WM]	= { MT_WFDMA1_BASE, 0x644 },
	[TX_RING_CTRL_BAND0]	= { MT_WFDMA1_BASE, 0x648 },
	[TX_RING_CTRL_BAND1]	= { MT_WFDMA1_BASE, 0x64c },
	[TX_RING_CTRL_WA]	= { MT_WFDMA1_BASE, 0x650 },
	[RX_RING_CTRL_WM]	= { MT_WFDMA1_BASE, 0x680 },
	[RX_RING_CTRL_WA]	= { INVALID_BASE, INVALID_OFFSET },
	[RX_RING_CTRL_STS0]	= { MT_WFDMA1_BASE, 0x684 },
	[RX_RING_CTRL_STS1]	= { MT_WFDMA1_BASE, 0x688 },
	[RX_RING_CTRL_BAND0]	= { MT_WFDMA0_BASE, 0x680 },
	[RX_RING_CTRL_BAND1]	= { MT_WFDMA0_BASE, 0x684 },
	[TX_RING_BASE]		= { MT_WFDMA1_BASE, 0x400 },
	[RX_EVENT_RING_BASE]	= { MT_WFDMA1_BASE, 0x500 },
	[RX_STS_RING_BASE]	= { MT_WFDMA1_BASE, 0x510 },
	[RX_DATA_RING_BASE]	= { MT_WFDMA0_BASE, 0x500 },
	[TMAC_CDTR]		= { INVALID_BASE, 0x090 },
	[TMAC_ODTR]		= { INVALID_BASE, 0x094 },
	[TMAC_ATCR]		= { INVALID_BASE, 0x098 },
	[TMAC_TRCR0]		= { INVALID_BASE, 0x09c },
	[TMAC_ICR0]		= { INVALID_BASE, 0x0a4 },
	[TMAC_ICR1]		= { INVALID_BASE, 0x0b4 },
	[TMAC_CTCR0]		= { INVALID_BASE, 0x0f4 },
	[TMAC_TFCR0]		= { INVALID_BASE, 0x1e0 },
	[MDP_BNRCFR0]		= { INVALID_BASE, 0x070 },
	[MDP_BNRCFR1]		= { INVALID_BASE, 0x074 },
	[ARB_DRNGR0]		= { INVALID_BASE, 0x194 },
	[ARB_SCR]		= { INVALID_BASE, 0x080 },
	[RMAC_MIB_AIRTIME14]	= { INVALID_BASE, 0x3b8 },
	[AGG_AWSCR0]		= { INVALID_BASE, 0x05c },
	[AGG_PCR0]		= { INVALID_BASE, 0x06c },
	[AGG_ACR0]		= { INVALID_BASE, 0x084 },
	[AGG_MRCR]		= { INVALID_BASE, 0x098 },
	[AGG_ATCR1]		= { INVALID_BASE, 0x0f0 },
	[AGG_ATCR3]		= { INVALID_BASE, 0x0f4 },
	[LPON_UTTR0]		= { INVALID_BASE, 0x080 },
	[LPON_UTTR1]		= { INVALID_BASE, 0x084 },
	[MIB_SDR3]		= { INVALID_BASE, 0x014 },
	[MIB_SDR4]		= { INVALID_BASE, 0x018 },
	[MIB_SDR5]		= { INVALID_BASE, 0x01c },
	[MIB_SDR7]		= { INVALID_BASE, 0x024 },
	[MIB_SDR8]		= { INVALID_BASE, 0x028 },
	[MIB_SDR9]		= { INVALID_BASE, 0x02c },
	[MIB_SDR10]		= { INVALID_BASE, 0x030 },
	[MIB_SDR11]		= { INVALID_BASE, 0x034 },
	[MIB_SDR12]		= { INVALID_BASE, 0x038 },
	[MIB_SDR13]		= { INVALID_BASE, 0x03c },
	[MIB_SDR14]		= { INVALID_BASE, 0x040 },
	[MIB_SDR15]		= { INVALID_BASE, 0x044 },
	[MIB_SDR16]		= { INVALID_BASE, 0x048 },
	[MIB_SDR17]		= { INVALID_BASE, 0x04c },
	[MIB_SDR18]		= { INVALID_BASE, 0x050 },
	[MIB_SDR19]		= { INVALID_BASE, 0x054 },
	[MIB_SDR20]		= { INVALID_BASE, 0x058 },
	[MIB_SDR21]		= { INVALID_BASE, 0x05c },
	[MIB_SDR22]		= { INVALID_BASE, 0x060 },
	[MIB_SDR23]		= { INVALID_BASE, 0x064 },
	[MIB_SDR24]		= { INVALID_BASE, 0x068 },
	[MIB_SDR25]		= { INVALID_BASE, 0x06c },
	[MIB_SDR27]		= { INVALID_BASE, 0x074 },
	[MIB_SDR28]		= { INVALID_BASE, 0x078 },
	[MIB_SDR29]		= { INVALID_BASE, 0x07c },
	[MIB_SDRVEC]		= { INVALID_BASE, 0x080 },
	[MIB_SDR31]		= { INVALID_BASE, 0x084 },
	[MIB_SDR32]		= { INVALID_BASE, 0x088 },
	[MIB_SDR33]		= { INVALID_BASE, 0x08C },
	[MIB_SDR38]		= { INVALID_BASE, 0x0D0 },
	[MIB_SDR39]		= { INVALID_BASE, 0x0D4 },
	[MIB_SDR40]		= { INVALID_BASE, 0x0D8 },
	[MIB_SDR42]		= { INVALID_BASE, 0x0e0 },
	[MIB_SDR43]		= { INVALID_BASE, 0x0e4 },
	[MIB_SDR46]		= { INVALID_BASE, 0x0f0 },
	[MIB_SDRMUBF]		= { INVALID_BASE, 0x090 },
	[MIB_DR8]		= { INVALID_BASE, 0x0c0 },
	[MIB_DR9]		= { INVALID_BASE, 0x0c4 },
	[MIB_DR11]		= { INVALID_BASE, 0x0cc },
	[MIB_MB_SDR0]		= { INVALID_BASE, 0x100 },
	[MIB_MB_SDR1]		= { INVALID_BASE, 0x104 },
	[TX_AGG_CNT]		= { INVALID_BASE, 0x0a8 },
	[TX_AGG_CNT2]		= { INVALID_BASE, 0x164 },
	[MIB_ARNG]		= { INVALID_BASE, 0x4b8 },
	[WTBLON_TOP_WDUCR]	= { INVALID_BASE, 0x0},
	[WTBL_UPDATE]		= { INVALID_BASE, 0x030},
	[PLE_FL_Q_EMPTY]	= { INVALID_BASE, 0x0b0},
	[PLE_FL_Q_CTRL]		= { INVALID_BASE, 0x1b0},
	[PLE_AC_QEMPTY]		= { INVALID_BASE, 0x500},
	[PLE_FREEPG_CNT]	= { INVALID_BASE, 0x100},
	[PLE_FREEPG_HEAD_TAIL]	= { INVALID_BASE, 0x104},
	[PLE_PG_HIF_GROUP]	= { INVALID_BASE, 0x110},
	[PLE_HIF_PG_INFO]	= { INVALID_BASE, 0x114},
	[AC_OFFSET]		= { INVALID_BASE, 0x040},
};

static const struct __reg mt7916_reg[] = {
	[L1_REMAP_CFG_OFFSET]	= { MT_REMAP_L1_CFG_BASE, 0x260 },
	[L2_REMAP_CFG_OFFSET]	= { MT_REMAP_L2_CFG_BASE, 0x1b8 },
	[INT_SOURCE_CSR]	= { MT_WFDMA0_BASE, 0x200 },
	[INT_MASK_CSR]		= { MT_WFDMA0_BASE, 0x204 },
	[INT1_SOURCE_CSR]	= { MT_WFDMA0_PCIE1_BASE, 0x200 },
	[INT1_MASK_CSR]		= { MT_WFDMA0_PCIE1_BASE, 0x204 },
	[INT_MCU_CMD_SOURCE]	= { MT_WFDMA0_BASE, 0x1f0 },
	[INT_MCU_CMD_EVENT]	= { MT_MCU_WFDMA0_BASE, 0x108 },
	[TX_RING_CTRL_FWDL]	= { MT_WFDMA0_BASE, 0x640 },
	[TX_RING_CTRL_WM]	= { MT_WFDMA0_BASE, 0x644 },
	[TX_RING_CTRL_BAND0]	= { MT_WFDMA0_BASE, 0x648 },
	[TX_RING_CTRL_BAND1]	= { MT_WFDMA0_BASE, 0x64c },
	[TX_RING_CTRL_WA]	= { MT_WFDMA0_BASE, 0x650 },
	[RX_RING_CTRL_WM]	= { MT_WFDMA0_BASE, 0x680 },
	[RX_RING_CTRL_WA]	= { MT_WFDMA0_BASE, 0x684 },
	[RX_RING_CTRL_STS0]	= { MT_WFDMA0_BASE, 0x688 },
	[RX_RING_CTRL_STS1]	= { MT_WFDMA0_BASE, 0x68c },
	[RX_RING_CTRL_BAND0]	= { MT_WFDMA0_BASE, 0x690 },
	[RX_RING_CTRL_BAND1]	= { MT_WFDMA0_BASE, 0x694 },
	[TX_RING_BASE]		= { MT_WFDMA0_BASE, 0x400 },
	[RX_EVENT_RING_BASE]	= { MT_WFDMA0_BASE, 0x500 },
	[RX_STS_RING_BASE]	= { MT_WFDMA0_BASE, 0x520 },
	[RX_DATA_RING_BASE]	= { MT_WFDMA0_BASE, 0x540 },
	[TMAC_CDTR]		= { INVALID_BASE, 0x0c8 },
	[TMAC_ODTR]		= { INVALID_BASE, 0x0cc },
	[TMAC_ATCR]		= { INVALID_BASE, 0x00c },
	[TMAC_TRCR0]		= { INVALID_BASE, 0x010 },
	[TMAC_ICR0]		= { INVALID_BASE, 0x014 },
	[TMAC_ICR1]		= { INVALID_BASE, 0x018 },
	[TMAC_CTCR0]		= { INVALID_BASE, 0x114 },
	[TMAC_TFCR0]		= { INVALID_BASE, 0x0e4 },
	[MDP_BNRCFR0]		= { INVALID_BASE, 0x090 },
	[MDP_BNRCFR1]		= { INVALID_BASE, 0x094 },
	[ARB_DRNGR0]		= { INVALID_BASE, 0x1e0 },
	[ARB_SCR]		= { INVALID_BASE, 0x000 },
	[RMAC_MIB_AIRTIME14]	= { INVALID_BASE, 0x0398 },
	[AGG_AWSCR0]		= { INVALID_BASE, 0x030 },
	[AGG_PCR0]		= { INVALID_BASE, 0x040 },
	[AGG_ACR0]		= { INVALID_BASE, 0x054 },
	[AGG_MRCR]		= { INVALID_BASE, 0x068 },
	[AGG_ATCR1]		= { INVALID_BASE, 0x1a8 },
	[AGG_ATCR3]		= { INVALID_BASE, 0x080 },
	[LPON_UTTR0]		= { INVALID_BASE, 0x360 },
	[LPON_UTTR1]		= { INVALID_BASE, 0x364 },
	[MIB_SDR3]		= { INVALID_BASE, 0x698 },
	[MIB_SDR4]		= { INVALID_BASE, 0x788 },
	[MIB_SDR5]		= { INVALID_BASE, 0x780 },
	[MIB_SDR7]		= { INVALID_BASE, 0x5a8 },
	[MIB_SDR8]		= { INVALID_BASE, 0x78c },
	[MIB_SDR9]		= { INVALID_BASE, 0x024 },
	[MIB_SDR10]		= { INVALID_BASE, 0x76c },
	[MIB_SDR11]		= { INVALID_BASE, 0x790 },
	[MIB_SDR12]		= { INVALID_BASE, 0x558 },
	[MIB_SDR13]		= { INVALID_BASE, 0x560 },
	[MIB_SDR14]		= { INVALID_BASE, 0x564 },
	[MIB_SDR15]		= { INVALID_BASE, 0x568 },
	[MIB_SDR16]		= { INVALID_BASE, 0x7fc },
	[MIB_SDR17]		= { INVALID_BASE, 0x800 },
	[MIB_SDR18]		= { INVALID_BASE, 0x030 },
	[MIB_SDR19]		= { INVALID_BASE, 0x5ac },
	[MIB_SDR20]		= { INVALID_BASE, 0x5b0 },
	[MIB_SDR21]		= { INVALID_BASE, 0x5b4 },
	[MIB_SDR22]		= { INVALID_BASE, 0x770 },
	[MIB_SDR23]		= { INVALID_BASE, 0x774 },
	[MIB_SDR24]		= { INVALID_BASE, 0x778 },
	[MIB_SDR25]		= { INVALID_BASE, 0x77c },
	[MIB_SDR27]		= { INVALID_BASE, 0x080 },
	[MIB_SDR28]		= { INVALID_BASE, 0x084 },
	[MIB_SDR29]		= { INVALID_BASE, 0x650 },
	[MIB_SDRVEC]		= { INVALID_BASE, 0x5a8 },
	[MIB_SDR31]		= { INVALID_BASE, 0x55c },
	[MIB_SDR32]		= { INVALID_BASE, 0x7a8 },
	[MIB_SDRMUBF]		= { INVALID_BASE, 0x7ac },
	[MIB_DR8]		= { INVALID_BASE, 0x56c },
	[MIB_DR9]		= { INVALID_BASE, 0x570 },
	[MIB_DR11]		= { INVALID_BASE, 0x574 },
	[MIB_MB_SDR0]		= { INVALID_BASE, 0x688 },
	[MIB_MB_SDR1]		= { INVALID_BASE, 0x690 },
	[TX_AGG_CNT]		= { INVALID_BASE, 0x7dc },
	[TX_AGG_CNT2]		= { INVALID_BASE, 0x7ec },
	[MIB_ARNG]		= { INVALID_BASE, 0x0b0 },
	[WTBLON_TOP_WDUCR]	= { INVALID_BASE, 0x200},
	[WTBL_UPDATE]		= { INVALID_BASE, 0x230},
	[PLE_FL_Q_EMPTY]	= { INVALID_BASE, 0x360},
	[PLE_FL_Q_CTRL]		= { INVALID_BASE, 0x3e0},
	[PLE_AC_QEMPTY]		= { INVALID_BASE, 0x600},
	[PLE_FREEPG_CNT]	= { INVALID_BASE, 0x380},
	[PLE_FREEPG_HEAD_TAIL]	= { INVALID_BASE, 0x384},
	[PLE_PG_HIF_GROUP]	= { INVALID_BASE, 0x00c},
	[PLE_HIF_PG_INFO]	= { INVALID_BASE, 0x388},
	[AC_OFFSET]		= { INVALID_BASE, 0x080},
};

static const struct __mask mt7915_mask[] = {
	[L2_REMAP_MASK]		= {19, 0},
	[L2_REMAP_OFFSET]	= {11, 0},
	[L2_REMAP_BASE]		= {31, 12},
	[MIB_SDR3_FCS_ERR]	= {15, 0},
	[MIB_MRDY_CNT]		= {25, 0},
	[MIB_MPDU_ATTEMPTS_CNT]	= {23, 0},
	[MIB_MPDU_SUCCESS_CNT]	= {23, 0},
	[MIB_AMPDU_SF_CNT]	= {23, 0},
	[MIB_PF_DROP_CNT]	= {7, 0},
	[MIB_VEC_DROP_CNT]	= {15, 0},
	[MIB_BF_TX_CNT]		= {15, 0},
};

static const struct __mask mt7916_mask[] = {
	[L2_REMAP_MASK]		= {31, 16},
	[L2_REMAP_OFFSET]	= {15, 0},
	[L2_REMAP_BASE]		= {31, 16},
	[MIB_SDR3_FCS_ERR]	= {31, 16},
	[MIB_MRDY_CNT]		= {31, 0},
	[MIB_MPDU_ATTEMPTS_CNT]	= {31, 0},
	[MIB_MPDU_SUCCESS_CNT]	= {31, 0},
	[MIB_AMPDU_SF_CNT]	= {31, 0},
	[MIB_PF_DROP_CNT]	= {15, 0},
	[MIB_VEC_DROP_CNT]	= {31, 16},
	[MIB_BF_TX_CNT]		= {31, 16},
};

static const u32 mt7915_bit[] = {
	[RX_DONE_DAND0]		= 16,
	[RX_DONE_DAND1]		= 17,
	[RX_DONE_MCU_WM]	= 0,
	[RX_DONE_MCU_WA]	= 1,
	[RX_DONE_WA_BAND0]	= 1,
	[RX_DONE_WA_BAND1]	= 2,
	[TX_DONE_FWDL]		= 26,
	[TX_DONE_MCU_WM]	= 27,
	[TX_DONE_MCU_WA]	= 15,
	[TX_DONE_BAND0]		= 30,
	[TX_DONE_BAND1]		= 31,
	[RX_MCU_TO_HOST]	= 29,
	[MIB_MB_SDR]		= 2,
	[LPON_TCR]		= 0,
};

static const u32 mt7916_bit[] = {
	[RX_DONE_DAND0]		= 22,
	[RX_DONE_DAND1]		= 23,
	[RX_DONE_MCU_WM]	= 0,
	[RX_DONE_MCU_WA]	= 1,
	[RX_DONE_WA_BAND0]	= 2,
	[RX_DONE_WA_BAND1]	= 3,
	[TX_DONE_FWDL]		= 26,
	[TX_DONE_MCU_WM]	= 27,
	[TX_DONE_MCU_WA]	= 25,
	[TX_DONE_BAND0]		= 30,
	[TX_DONE_BAND1]		= 31,
	[RX_MCU_TO_HOST]	= 29,
	[MIB_MB_SDR]		= 1,
	[LPON_TCR]		= 2,
};

static const struct __map mt7915_reg_map[] = {
	{ 0x00400000, 0x80000, 0x10000 }, /* WF_MCU_SYSRAM */
	{ 0x00410000, 0x90000, 0x10000 }, /* WF_MCU_SYSRAM (configure regs) */
	{ 0x40000000, 0x70000, 0x10000 }, /* WF_UMAC_SYSRAM */
	{ 0x54000000, 0x02000, 0x1000 }, /* WFDMA PCIE0 MCU DMA0 */
	{ 0x55000000, 0x03000, 0x1000 }, /* WFDMA PCIE0 MCU DMA1 */
	{ 0x58000000, 0x06000, 0x1000 }, /* WFDMA PCIE1 MCU DMA0 (MEM_DMA) */
	{ 0x59000000, 0x07000, 0x1000 }, /* WFDMA PCIE1 MCU DMA1 */
	{ 0x7c000000, 0xf0000, 0x10000 }, /* CONN_INFRA */
	{ 0x7c020000, 0xd0000, 0x10000 }, /* CONN_INFRA, WFDMA */
	{ 0x80020000, 0xb0000, 0x10000 }, /* WF_TOP_MISC_OFF */
	{ 0x81020000, 0xc0000, 0x10000 }, /* WF_TOP_MISC_ON */
	{ 0x820c0000, 0x08000, 0x4000 }, /* WF_UMAC_TOP (PLE) */
	{ 0x820c8000, 0x0c000, 0x2000 }, /* WF_UMAC_TOP (PSE) */
	{ 0x820cc000, 0x0e000, 0x2000 }, /* WF_UMAC_TOP (PP) */
	{ 0x820ce000, 0x21c00, 0x0200 }, /* WF_LMAC_TOP (WF_SEC) */
	{ 0x820cf000, 0x22000, 0x1000 }, /* WF_LMAC_TOP (WF_PF) */
	{ 0x820d0000, 0x30000, 0x10000 }, /* WF_LMAC_TOP (WF_WTBLON) */
	{ 0x820e0000, 0x20000, 0x0400 }, /* WF_LMAC_TOP BN0 (WF_CFG) */
	{ 0x820e1000, 0x20400, 0x0200 }, /* WF_LMAC_TOP BN0 (WF_TRB) */
	{ 0x820e2000, 0x20800, 0x0400 }, /* WF_LMAC_TOP BN0 (WF_AGG) */
	{ 0x820e3000, 0x20c00, 0x0400 }, /* WF_LMAC_TOP BN0 (WF_ARB) */
	{ 0x820e4000, 0x21000, 0x0400 }, /* WF_LMAC_TOP BN0 (WF_TMAC) */
	{ 0x820e5000, 0x21400, 0x0800 }, /* WF_LMAC_TOP BN0 (WF_RMAC) */
	{ 0x820e7000, 0x21e00, 0x0200 }, /* WF_LMAC_TOP BN0 (WF_DMA) */
	{ 0x820e9000, 0x23400, 0x0200 }, /* WF_LMAC_TOP BN0 (WF_WTBLOFF) */
	{ 0x820ea000, 0x24000, 0x0200 }, /* WF_LMAC_TOP BN0 (WF_ETBF) */
	{ 0x820eb000, 0x24200, 0x0400 }, /* WF_LMAC_TOP BN0 (WF_LPON) */
	{ 0x820ec000, 0x24600, 0x0200 }, /* WF_LMAC_TOP BN0 (WF_INT) */
	{ 0x820ed000, 0x24800, 0x0800 }, /* WF_LMAC_TOP BN0 (WF_MIB) */
	{ 0x820f0000, 0xa0000, 0x0400 }, /* WF_LMAC_TOP BN1 (WF_CFG) */
	{ 0x820f1000, 0xa0600, 0x0200 }, /* WF_LMAC_TOP BN1 (WF_TRB) */
	{ 0x820f2000, 0xa0800, 0x0400 }, /* WF_LMAC_TOP BN1 (WF_AGG) */
	{ 0x820f3000, 0xa0c00, 0x0400 }, /* WF_LMAC_TOP BN1 (WF_ARB) */
	{ 0x820f4000, 0xa1000, 0x0400 }, /* WF_LMAC_TOP BN1 (WF_TMAC) */
	{ 0x820f5000, 0xa1400, 0x0800 }, /* WF_LMAC_TOP BN1 (WF_RMAC) */
	{ 0x820f7000, 0xa1e00, 0x0200 }, /* WF_LMAC_TOP BN1 (WF_DMA) */
	{ 0x820f9000, 0xa3400, 0x0200 }, /* WF_LMAC_TOP BN1 (WF_WTBLOFF) */
	{ 0x820fa000, 0xa4000, 0x0200 }, /* WF_LMAC_TOP BN1 (WF_ETBF) */
	{ 0x820fb000, 0xa4200, 0x0400 }, /* WF_LMAC_TOP BN1 (WF_LPON) */
	{ 0x820fc000, 0xa4600, 0x0200 }, /* WF_LMAC_TOP BN1 (WF_INT) */
	{ 0x820fd000, 0xa4800, 0x0800 }, /* WF_LMAC_TOP BN1 (WF_MIB) */
	{ 0x0, 0x0, 0x0 }, /* imply end of search */
};

static const struct __map mt7916_reg_map[] = {
	{ 0x54000000, 0x02000, 0x1000 }, /* WFDMA_0 (PCIE0 MCU DMA0) */
	{ 0x55000000, 0x03000, 0x1000 }, /* WFDMA_1 (PCIE0 MCU DMA1) */
	{ 0x56000000, 0x04000, 0x1000 }, /* WFDMA_2 (Reserved) */
	{ 0x57000000, 0x05000, 0x1000 }, /* WFDMA_3 (MCU wrap CR) */
	{ 0x58000000, 0x06000, 0x1000 }, /* WFDMA_4 (PCIE1 MCU DMA0) */
	{ 0x59000000, 0x07000, 0x1000 }, /* WFDMA_5 (PCIE1 MCU DMA1) */
	{ 0x820c0000, 0x08000, 0x4000 }, /* WF_UMAC_TOP (PLE) */
	{ 0x820c8000, 0x0c000, 0x2000 }, /* WF_UMAC_TOP (PSE) */
	{ 0x820cc000, 0x0e000, 0x2000 }, /* WF_UMAC_TOP (PP) */
	{ 0x820e0000, 0x20000, 0x0400 }, /* WF_LMAC_TOP BN0 (WF_CFG) */
	{ 0x820e1000, 0x20400, 0x0200 }, /* WF_LMAC_TOP BN0 (WF_TRB) */
	{ 0x820e2000, 0x20800, 0x0400 }, /* WF_LMAC_TOP BN0 (WF_AGG) */
	{ 0x820e3000, 0x20c00, 0x0400 }, /* WF_LMAC_TOP BN0 (WF_ARB) */
	{ 0x820e4000, 0x21000, 0x0400 }, /* WF_LMAC_TOP BN0 (WF_TMAC) */
	{ 0x820e5000, 0x21400, 0x0800 }, /* WF_LMAC_TOP BN0 (WF_RMAC) */
	{ 0x820ce000, 0x21c00, 0x0200 }, /* WF_LMAC_TOP (WF_SEC) */
	{ 0x820e7000, 0x21e00, 0x0200 }, /* WF_LMAC_TOP BN0 (WF_DMA) */
	{ 0x820cf000, 0x22000, 0x1000 }, /* WF_LMAC_TOP (WF_PF) */
	{ 0x820e9000, 0x23400, 0x0200 }, /* WF_LMAC_TOP BN0 (WF_WTBLOFF) */
	{ 0x820ea000, 0x24000, 0x0200 }, /* WF_LMAC_TOP BN0 (WF_ETBF) */
	{ 0x820eb000, 0x24200, 0x0400 }, /* WF_LMAC_TOP BN0 (WF_LPON) */
	{ 0x820ec000, 0x24600, 0x0200 }, /* WF_LMAC_TOP BN0 (WF_INT) */
	{ 0x820ed000, 0x24800, 0x0800 }, /* WF_LMAC_TOP BN0 (WF_MIB) */
	{ 0x820ca000, 0x26000, 0x2000 }, /* WF_LMAC_TOP BN0 (WF_MUCOP) */
	{ 0x820d0000, 0x30000, 0x10000}, /* WF_LMAC_TOP (WF_WTBLON) */
	{ 0x00400000, 0x80000, 0x10000}, /* WF_MCU_SYSRAM */
	{ 0x00410000, 0x90000, 0x10000}, /* WF_MCU_SYSRAM (configure cr) */
	{ 0x820f0000, 0xa0000, 0x0400 }, /* WF_LMAC_TOP BN1 (WF_CFG) */
	{ 0x820f1000, 0xa0600, 0x0200 }, /* WF_LMAC_TOP BN1 (WF_TRB) */
	{ 0x820f2000, 0xa0800, 0x0400 }, /* WF_LMAC_TOP BN1 (WF_AGG) */
	{ 0x820f3000, 0xa0c00, 0x0400 }, /* WF_LMAC_TOP BN1 (WF_ARB) */
	{ 0x820f4000, 0xa1000, 0x0400 }, /* WF_LMAC_TOP BN1 (WF_TMAC) */
	{ 0x820f5000, 0xa1400, 0x0800 }, /* WF_LMAC_TOP BN1 (WF_RMAC) */
	{ 0x820f7000, 0xa1e00, 0x0200 }, /* WF_LMAC_TOP BN1 (WF_DMA) */
	{ 0x820f9000, 0xa3400, 0x0200 }, /* WF_LMAC_TOP BN1 (WF_WTBLOFF) */
	{ 0x820fa000, 0xa4000, 0x0200 }, /* WF_LMAC_TOP BN1 (WF_ETBF) */
	{ 0x820fb000, 0xa4200, 0x0400 }, /* WF_LMAC_TOP BN1 (WF_LPON) */
	{ 0x820fc000, 0xa4600, 0x0200 }, /* WF_LMAC_TOP BN1 (WF_INT) */
	{ 0x820fd000, 0xa4800, 0x0800 }, /* WF_LMAC_TOP BN1 (WF_MIB) */
	{ 0x820c4000, 0xa8000, 0x1000 }, /* WF_LMAC_TOP (WF_UWTBL ) */
	{ 0x820b0000, 0xae000, 0x1000 }, /* [APB2] WFSYS_ON */
	{ 0x80020000, 0xb0000, 0x10000}, /* WF_TOP_MISC_OFF */
	{ 0x81020000, 0xc0000, 0x10000}, /* WF_TOP_MISC_ON */
	{ 0x0, 0x0, 0x100000 }, /* fixed remap range */
	{ 0x0, 0x0, 0x0 }, /* imply end of search */
};

static const struct mt7915_reg_desc reg_desc[] = {
	{ 0x7915,
	  mt7915_base,
	  mt7915_reg,
	  mt7915_mask,
	  mt7915_bit,
	  mt7915_reg_map,
	  ARRAY_SIZE(mt7915_reg_map)
	},
	{ 0x7906,
	  mt7916_base,
	  mt7916_reg,
	  mt7916_mask,
	  mt7916_bit,
	  mt7916_reg_map,
	  ARRAY_SIZE(mt7916_reg_map)
	},
};

static u32 mt7915_reg_map_l1(struct mt7915_dev *dev, u32 addr)
{
	u32 offset = FIELD_GET(MT_HIF_REMAP_L1_OFFSET, addr);
	u32 base = FIELD_GET(MT_HIF_REMAP_L1_BASE, addr);

	dev->bus_ops->rmw(&dev->mt76, MT_HIF_REMAP_L1,
			  MT_HIF_REMAP_L1_MASK,
		FIELD_PREP(MT_HIF_REMAP_L1_MASK, base));
	/* use read to push write */
	dev->bus_ops->rr(&dev->mt76, MT_HIF_REMAP_L1);

	return MT_HIF_REMAP_BASE_L1 + offset;
}

static u32 mt7915_reg_map_l2(struct mt7915_dev *dev, u32 addr)
{
	u32 offset =  __FIELD_GET(L2_REMAP_OFFSET, addr);
	u32 base = __FIELD_GET(L2_REMAP_BASE, addr);

	dev->bus_ops->rmw(&dev->mt76, MT_HIF_REMAP_L2,
			  MT_HIF_REMAP_L2_MASK,
			  __FIELD_PREP(L2_REMAP_MASK, base));

	/* use read to push write */
	dev->bus_ops->rr(&dev->mt76, MT_HIF_REMAP_L2);

	return MT_HIF_REMAP_BASE_L2 + offset;
}

static u32 __mt7915_reg_addr(struct mt7915_dev *dev, u32 addr)
{
	int i;

	if (addr < 0x100000)
		return addr;

	if (!dev->reg->map) {
		dev_err(dev->mt76.dev, "err: reg_map is null\n");
		return addr;
	}

	for (i = 0; i < dev->reg->map_size; i++) {
		u32 ofs;

		if (addr < dev->reg->map[i].phys)
			continue;

		ofs = addr - dev->reg->map[i].phys;
		if (ofs > dev->reg->map[i].size)
			continue;

		return dev->reg->map[i].maps + ofs;
	}

	if ((addr >= MT_INFRA_BASE && addr < MT_WFSYS0_PHY_START) ||
	    (addr >= MT_WFSYS0_PHY_START && addr < MT_WFSYS1_PHY_START) ||
	    (addr >= MT_WFSYS1_PHY_START && addr <= MT_WFSYS1_PHY_END))
		return mt7915_reg_map_l1(dev, addr);

	if (dev_is_pci(dev->mt76.dev) &&
	    ((addr >= MT_CBTOP1_PHY_START && addr <= MT_CBTOP1_PHY_END) ||
	     (addr >= MT_CBTOP2_PHY_START && addr <= MT_CBTOP2_PHY_END))) {
		/* CONN_INFRA: covert to phyiscal addr and use layer 1 remap */
		if (addr >= MT_CONN_INFRA_MCU_START &&
		    addr <= MT_CONN_INFRA_MCU_END)
			addr = addr - MT_CONN_INFRA_MCU_START + MT_INFRA_BASE;

		return mt7915_reg_map_l1(dev, addr);
	}

	return mt7915_reg_map_l2(dev, addr);
}

static u32 mt7915_rr(struct mt76_dev *mdev, u32 offset)
{
	struct mt7915_dev *dev = container_of(mdev, struct mt7915_dev, mt76);
	u32 addr = __mt7915_reg_addr(dev, offset);

	return dev->bus_ops->rr(mdev, addr);
}

static void mt7915_wr(struct mt76_dev *mdev, u32 offset, u32 val)
{
	struct mt7915_dev *dev = container_of(mdev, struct mt7915_dev, mt76);
	u32 addr = __mt7915_reg_addr(dev, offset);

	dev->bus_ops->wr(mdev, addr, val);
}

static u32 mt7915_rmw(struct mt76_dev *mdev, u32 offset, u32 mask, u32 val)
{
	struct mt7915_dev *dev = container_of(mdev, struct mt7915_dev, mt76);
	u32 addr = __mt7915_reg_addr(dev, offset);

	return dev->bus_ops->rmw(mdev, addr, mask, val);
}

static int mt7915_mmio_init(struct mt76_dev *mdev,
			    void __iomem *mem_base,
			    u32 device_id)
{
	struct mt76_bus_ops *bus_ops;
	struct mt7915_dev *dev;
	int i;

	dev = container_of(mdev, struct mt7915_dev, mt76);
	mt76_mmio_init(&dev->mt76, mem_base);

	for (i = 0; i < ARRAY_SIZE(reg_desc); i++) {
		if (device_id == reg_desc[i].id) {
			dev->reg = &reg_desc[i];
			break;
		}
	}

	dev->bus_ops = dev->mt76.bus;
	bus_ops = devm_kmemdup(dev->mt76.dev, dev->bus_ops, sizeof(*bus_ops),
			       GFP_KERNEL);
	if (!bus_ops)
		return -ENOMEM;

	bus_ops->rr = mt7915_rr;
	bus_ops->wr = mt7915_wr;
	bus_ops->rmw = mt7915_rmw;
	dev->mt76.bus = bus_ops;

	mdev->rev = (device_id << 16) |
		    (mt76_rr(dev, MT_HW_REV) & 0xff);
	dev_info(mdev->dev, "ASIC revision: %04x\n", mdev->rev);

	return 0;
}

void mt7915_dual_hif_set_irq_mask(struct mt7915_dev *dev,
				  bool write_reg,
				  u32 clear, u32 set)
{
	struct mt76_dev *mdev = &dev->mt76;
	unsigned long flags;

	spin_lock_irqsave(&mdev->mmio.irq_lock, flags);

	mdev->mmio.irqmask &= ~clear;
	mdev->mmio.irqmask |= set;

	if (write_reg) {
		mt76_wr(dev, MT_INT_MASK_CSR, mdev->mmio.irqmask);
		mt76_wr(dev, MT_INT1_MASK_CSR, mdev->mmio.irqmask);
	}

	spin_unlock_irqrestore(&mdev->mmio.irq_lock, flags);
}

static void mt7915_rx_poll_complete(struct mt76_dev *mdev,
				    enum mt76_rxq_id q)
{
	struct mt7915_dev *dev = container_of(mdev, struct mt7915_dev, mt76);
	u32 rx_irq_mask = 0;

	switch (q) {
	case MT_RXQ_MAIN:
		rx_irq_mask = MT_INT_RX_DONE_DATA0;
		break;
	case MT_RXQ_MCU:
		rx_irq_mask = MT_INT_RX_DONE_WM;
		break;
	case MT_RXQ_MCU_WA:
		rx_irq_mask = MT_INT_RX_DONE_WA;
		break;
	case MT_RXQ_EXT:
		rx_irq_mask = MT_INT_RX_DONE_DATA1;
		break;
	case MT_RXQ_EXT_WA:
		rx_irq_mask = MT_INT_RX_DONE_WA_EXT;
		break;
	case MT_RXQ_MAIN_WA:
		rx_irq_mask = MT_INT_RX_DONE_WA_MAIN;
		break;
	default:
		break;
	}

	mt7915_irq_enable(dev, rx_irq_mask);
}

/* TODO: support 2/4/6/8 MSI-X vectors */
static void mt7915_irq_tasklet(struct tasklet_struct *t)
{
	struct mt7915_dev *dev = from_tasklet(dev, t, irq_tasklet);
	u32 intr, intr1, mask;

	mt76_wr(dev, MT_INT_MASK_CSR, 0);
	if (dev->hif2)
		mt76_wr(dev, MT_INT1_MASK_CSR, 0);

	intr = mt76_rr(dev, MT_INT_SOURCE_CSR);
	intr &= dev->mt76.mmio.irqmask;
	mt76_wr(dev, MT_INT_SOURCE_CSR, intr);

	if (dev->hif2) {
		intr1 = mt76_rr(dev, MT_INT1_SOURCE_CSR);
		intr1 &= dev->mt76.mmio.irqmask;
		mt76_wr(dev, MT_INT1_SOURCE_CSR, intr1);

		intr |= intr1;
	}

	trace_dev_irq(&dev->mt76, intr, dev->mt76.mmio.irqmask);

	mask = intr & MT_INT_RX_DONE_ALL;
	if (intr & MT_INT_TX_DONE_MCU)
		mask |= MT_INT_TX_DONE_MCU;

	mt7915_irq_disable(dev, mask);

	if (intr & MT_INT_TX_DONE_MCU)
		napi_schedule(&dev->mt76.tx_napi);

	if (intr & MT_INT_RX_DONE_DATA0)
		napi_schedule(&dev->mt76.napi[MT_RXQ_MAIN]);

	if (intr & MT_INT_RX_DONE_DATA1)
		napi_schedule(&dev->mt76.napi[MT_RXQ_EXT]);

	if (intr & MT_INT_RX_DONE_WM)
		napi_schedule(&dev->mt76.napi[MT_RXQ_MCU]);

	if (intr & MT_INT_RX_DONE_WA)
		napi_schedule(&dev->mt76.napi[MT_RXQ_MCU_WA]);

	if (!is_mt7915(&dev->mt76)) {
		if (intr & MT_INT_RX_DONE_WA_MAIN)
			napi_schedule(&dev->mt76.napi[MT_RXQ_MAIN_WA]);
	}

	if (intr & MT_INT_RX_DONE_WA_EXT)
		napi_schedule(&dev->mt76.napi[MT_RXQ_EXT_WA]);

	if (intr & MT_INT_MCU_CMD) {
		u32 val = mt76_rr(dev, MT_MCU_CMD);

		mt76_wr(dev, MT_MCU_CMD, val);
		if (val & MT_MCU_CMD_ERROR_MASK) {
			dev->reset_state = val;
			ieee80211_queue_work(mt76_hw(dev), &dev->reset_work);
			wake_up(&dev->reset_wait);
		}
	}
}

static irqreturn_t mt7915_irq_handler(int irq, void *dev_instance)
{
	struct mt7915_dev *dev = dev_instance;

	mt76_wr(dev, MT_INT_MASK_CSR, 0);
	if (dev->hif2)
		mt76_wr(dev, MT_INT1_MASK_CSR, 0);

	if (!test_bit(MT76_STATE_INITIALIZED, &dev->mphy.state))
		return IRQ_NONE;

	tasklet_schedule(&dev->irq_tasklet);

	return IRQ_HANDLED;
}

int mt7915_mmio_probe(struct device *pdev,
		      void __iomem *mem_base,
		      u32 device_id,
		      int irq, struct mt7915_hif *hif2)
{
	static const struct mt76_driver_ops drv_ops = {
		/* txwi_size = txd size + txp size */
		.txwi_size = MT_TXD_SIZE + sizeof(struct mt7915_txp),
		.drv_flags = MT_DRV_TXWI_NO_FREE | MT_DRV_HW_MGMT_TXQ,
		.survey_flags = SURVEY_INFO_TIME_TX |
				SURVEY_INFO_TIME_RX |
				SURVEY_INFO_TIME_BSS_RX,
		.token_size = MT7915_TOKEN_SIZE,
		.tx_prepare_skb = mt7915_tx_prepare_skb,
		.tx_complete_skb = mt7915_tx_complete_skb,
		.rx_skb = mt7915_queue_rx_skb,
		.rx_poll_complete = mt7915_rx_poll_complete,
		.sta_ps = mt7915_sta_ps,
		.sta_add = mt7915_mac_sta_add,
		.sta_remove = mt7915_mac_sta_remove,
		.update_survey = mt7915_update_channel,
	};
	struct ieee80211_ops *ops;
	struct mt7915_dev *dev;
	struct mt76_dev *mdev;
	int ret;

	ops = devm_kmemdup(pdev, &mt7915_ops, sizeof(mt7915_ops), GFP_KERNEL);
	if (!ops)
		return -ENOMEM;

	mdev = mt76_alloc_device(pdev, sizeof(*dev), ops, &drv_ops);
	if (!mdev)
		return -ENOMEM;

	dev = container_of(mdev, struct mt7915_dev, mt76);

	ret = mt7915_mmio_init(mdev, mem_base, device_id);
	if (ret)
		goto error;

	tasklet_setup(&dev->irq_tasklet, mt7915_irq_tasklet);

	mt76_wr(dev, MT_INT_MASK_CSR, 0);
	/* master switch of PCIe tnterrupt enable */
	if (dev_is_pci(pdev))
		mt76_wr(dev, MT_PCIE_MAC_INT_ENABLE, 0xff);

	ret = devm_request_irq(mdev->dev, irq, mt7915_irq_handler,
			       IRQF_SHARED, KBUILD_MODNAME, dev);
	if (ret)
		goto error;

	if (hif2) {
		dev->hif2 = hif2;

		mt76_wr(dev, MT_INT1_MASK_CSR, 0);
		/* master switch of PCIe tnterrupt enable */
		if (dev_is_pci(pdev))
			mt76_wr(dev, MT_PCIE1_MAC_INT_ENABLE, 0xff);

		ret = devm_request_irq(mdev->dev, dev->hif2->irq,
				       mt7915_irq_handler, IRQF_SHARED,
				       KBUILD_MODNAME "-hif", dev);
		if (ret) {
			put_device(dev->hif2->dev);
			goto free_irq;
		}
	}

	ret = mt7915_register_device(dev);
	if (ret)
		goto free_hif2_irq;

	return 0;

free_hif2_irq:
	if (dev->hif2)
		devm_free_irq(mdev->dev, dev->hif2->irq, dev);
free_irq:
	devm_free_irq(mdev->dev, irq, dev);
error:
	mt76_free_device(&dev->mt76);

	return ret;
}

static int __init mt7915_init(void)
{
	int ret;

	ret = pci_register_driver(&mt7915_hif_driver);
	if (ret)
		return ret;

	ret = pci_register_driver(&mt7915_pci_driver);
	if (ret)
		pci_unregister_driver(&mt7915_hif_driver);

	return ret;
}

static void __exit mt7915_exit(void)
{
	pci_unregister_driver(&mt7915_pci_driver);
	pci_unregister_driver(&mt7915_hif_driver);
}

module_init(mt7915_init);
module_exit(mt7915_exit);
MODULE_LICENSE("Dual BSD/GPL");
