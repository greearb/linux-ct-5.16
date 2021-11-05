/* SPDX-License-Identifier: ISC */
/* Copyright (C) 2020 MediaTek Inc. */

#ifndef __MT7915_REGS_H
#define __MT7915_REGS_H

#define INVALID_BASE		0x0
#define INVALID_OFFSET		0x0

struct __map {
	u32 phys;
	u32 maps;
	u32 size;
};

struct __reg {
	u32 base;
	u32 offs;
};

struct __mask {
	u32 end;
	u32 start;
};

/* used to differentiate between generations */
struct mt7915_reg_desc {
	const u32 id;
	const u32 *base_rev;
	const struct __reg *reg_rev;
	const struct __mask *mask_rev;
	const u32 *bit_rev;
	const struct __map *map;
	const u32 map_size;
};

enum base_rev {
	MT_REMAP_L1_CFG_BASE,
	MT_REMAP_L1_BASE,
	MT_REMAP_L2_CFG_BASE,
	MT_REMAP_L2_BASE,
	MT_INFRA_MCU_END_BASE,
	MT_PCIE_MAC_BASE,
	MT_PCIE1_MAC_BASE,
	MT_WFDMA0_BASE,
	MT_WFDMA1_BASE,
	MT_WFDMA0_PCIE1_BASE,
	MT_WFDMA1_PCIE1_BASE,
	MT_WFDMA_EXT_CSR_BASE,
	MT_SWDEF_BASE,
	MT_MCU_WFDMA0_BASE,
	MT_MCU_WFDMA1_BASE,
	__MT_REG_BASE_MAX,
};

enum reg_rev {
	L1_REMAP_CFG_OFFSET,
	L2_REMAP_CFG_OFFSET,
	INT_SOURCE_CSR,
	INT_MASK_CSR,
	INT1_SOURCE_CSR,
	INT1_MASK_CSR,
	INT_MCU_CMD_SOURCE,
	INT_MCU_CMD_EVENT,
	TX_RING_BASE,
	RX_EVENT_RING_BASE,
	RX_DATA_RING_BASE,
	TMAC_CDTR,
	TMAC_ODTR,
	TMAC_ATCR,
	TMAC_TRCR0,
	TMAC_ICR0,
	TMAC_ICR1,
	TMAC_CTCR0,
	TMAC_TFCR0,
	MDP_BNRCFR0,
	MDP_BNRCFR1,
	ARB_DRNGR0,
	ARB_SCR,
	RMAC_MIB_AIRTIME14,
	AGG_AWSCR0,
	AGG_PCR0,
	AGG_ACR0,
	AGG_MRCR,
	AGG_ATCR1,
	AGG_ATCR3,
	LPON_UTTR0,
	LPON_UTTR1,
	MIB_SDR3,
	MIB_SDR4,
	MIB_SDR5,
	MIB_SDR7,
	MIB_SDR8,
	MIB_SDR9,
	MIB_SDR10,
	MIB_SDR11,
	MIB_SDR12,
	MIB_SDR13,
	MIB_SDR14,
	MIB_SDR15,
	MIB_SDR16,
	MIB_SDR17,
	MIB_SDR18,
	MIB_SDR19,
	MIB_SDR20,
	MIB_SDR21,
	MIB_SDR22,
	MIB_SDR23,
	MIB_SDR24,
	MIB_SDR25,
	MIB_SDR27,
	MIB_SDR28,
	MIB_SDR29,
	MIB_SDRVEC,
	MIB_SDR31,
	MIB_SDR32,
	MIB_SDR33,
	MIB_SDR38,
	MIB_SDR39,
	MIB_SDR40,
	MIB_SDR42,
	MIB_SDR43,
	MIB_SDR46,
	MIB_SDRMUBF,
	MIB_DR8,
	MIB_DR9,
	MIB_DR11,
	MIB_MB_SDR0,
	MIB_MB_SDR1,
	TX_AGG_CNT,
	TX_AGG_CNT2,
	MIB_ARNG,
	WTBLON_TOP_WDUCR,
	WTBL_UPDATE,
	PLE_FL_Q_EMPTY,
	PLE_FL_Q_CTRL,
	PLE_AC_QEMPTY,
	PLE_FREEPG_CNT,
	PLE_FREEPG_HEAD_TAIL,
	PLE_PG_HIF_GROUP,
	PLE_HIF_PG_INFO,
	AC_OFFSET,
	__MT_REG_DIFF_MAX,
};

enum mask_rev {
	L2_REMAP_MASK,
	L2_REMAP_OFFSET,
	L2_REMAP_BASE,
	MIB_SDR3_FCS_ERR,
	MIB_MRDY_CNT,
	MIB_MPDU_ATTEMPTS_CNT,
	MIB_MPDU_SUCCESS_CNT,
	MIB_AMPDU_SF_CNT,
	MIB_PF_DROP_CNT,
	MIB_VEC_DROP_CNT,
	MIB_BF_TX_CNT,
	__MT_MASK_DIF_MAX,
};

enum bit_rev {
	RX_DONE_DAND0,
	RX_DONE_DAND1,
	RX_DONE_MCU_WM,
	RX_DONE_MCU_WA,
	RX_DONE_WA_BAND0,
	RX_DONE_WA_BAND1,
	TX_DONE_FWDL,
	TX_DONE_MCU_WM,
	TX_DONE_MCU_WA,
	TX_DONE_BAND0,
	TX_DONE_BAND1,
	RX_MCU_TO_HOST,
	MIB_MB_SDR,
	LPON_TCR,
	__MT_BIT_DIF_MAX,
};

#define __REG_MAP(_dev, id, ofs)	((_dev)->reg->base_rev[(id)] + (ofs))
#define __REG_BASE(_dev, id)		((_dev)->reg->reg_rev[(id)].base)
#define __REG_OFFS(_dev, id)		((_dev)->reg->reg_rev[(id)].offs)

#define __BIT(_dev, id)			BIT((_dev)->reg->bit_rev[(id)])
#define __MASK(_dev, id)		GENMASK((_dev)->reg->mask_rev[(id)].end,	\
						(_dev)->reg->mask_rev[(id)].start)
#define __REG(_dev, id)			__REG_MAP((_dev), __REG_BASE((_dev), (id)),	\
						__REG_OFFS((_dev), (id)))

#define __FIELD_GET(id, _reg)		(((_reg) & __MASK(dev, (id))) >>		\
					 dev->reg->mask_rev[(id)].start)
#define __FIELD_PREP(id, _reg)		(((_reg) << dev->reg->mask_rev[(id)].start) &		\
					 __MASK(dev, (id)))

/* MCU WFDMA0 */
#define MT_MCU_WFDMA0(ofs)		__REG_MAP(dev, MT_MCU_WFDMA0_BASE, (ofs))
#define MT_MCU_WFDMA0_DUMMY_CR		MT_MCU_WFDMA0(0x120)

/* MCU WFDMA1 */
#define MT_MCU_INT_EVENT		__REG(dev, INT_MCU_CMD_EVENT)
#define MT_MCU_INT_EVENT_DMA_STOPPED	BIT(0)
#define MT_MCU_INT_EVENT_DMA_INIT	BIT(1)
#define MT_MCU_INT_EVENT_SER_TRIGGER	BIT(2)
#define MT_MCU_INT_EVENT_RESET_DONE	BIT(3)

/* PLE */
#define MT_PLE_BASE			0x820c0000
#define MT_PLE(ofs)			(MT_PLE_BASE + (ofs))

/* Modify whether txfree struct returns latency or txcount. */
#define MT_PLE_HOST_RPT0               MT_PLE(0x030)
#define MT_PLE_HOST_RPT0_TX_LATENCY    BIT(3)

#define MT_PLE_FL_Q0_CTRL		MT_PLE(0x1b0)
#define MT_PLE_FL_Q1_CTRL		MT_PLE(0x1b4)
#define MT_PLE_FL_Q2_CTRL		MT_PLE(0x1b8)

#define MT_FL_Q_EMPTY			MT_PLE(__REG_OFFS(dev, PLE_FL_Q_EMPTY))
#define MT_FL_Q0_CTRL			MT_PLE(__REG_OFFS(dev, PLE_FL_Q_CTRL))
#define MT_FL_Q2_CTRL			MT_PLE(__REG_OFFS(dev, PLE_FL_Q_CTRL) + 0x8)
#define MT_FL_Q3_CTRL			MT_PLE(__REG_OFFS(dev, PLE_FL_Q_CTRL) + 0xc)

#define MT_PLE_FREEPG_CNT		MT_PLE(__REG_OFFS(dev, PLE_FREEPG_CNT))
#define MT_PLE_FREEPG_HEAD_TAIL		MT_PLE(__REG_OFFS(dev, PLE_FREEPG_HEAD_TAIL))
#define MT_PLE_PG_HIF_GROUP		MT_PLE(__REG_OFFS(dev, PLE_PG_HIF_GROUP))
#define MT_PLE_HIF_PG_INFO		MT_PLE(__REG_OFFS(dev, PLE_HIF_PG_INFO))

#define MT_PLE_AC_QEMPTY(ac, n)		MT_PLE(__REG_OFFS(dev, PLE_AC_QEMPTY) +	\
					       __REG_OFFS(dev, AC_OFFSET) *	\
					       (ac) + ((n) << 2))
#define MT_PLE_AMSDU_PACK_MSDU_CNT(n)	MT_PLE(0x10e0 + ((n) << 2))

#define MT_PSE_BASE			0x820c8000
#define MT_PSE(ofs)			(MT_PSE_BASE + (ofs))

/* WF MDP TOP */
#define MT_MDP_BASE			0x820cd000
#define MT_MDP(ofs)			(MT_MDP_BASE + (ofs))

#define MT_MDP_DCR0			MT_MDP(0x000)
#define MT_MDP_DCR0_DAMSDU_EN		BIT(15)

#define MT_MDP_DCR1			MT_MDP(0x004)
#define MT_MDP_DCR1_MAX_RX_LEN		GENMASK(15, 3)

#define MT_MDP_BNRCFR0(_band)		MT_MDP(__REG_OFFS(dev, MDP_BNRCFR0) + \
					       ((_band) << 8))
#define MT_MDP_RCFR0_MCU_RX_MGMT	GENMASK(5, 4)
#define MT_MDP_RCFR0_MCU_RX_CTL_NON_BAR	GENMASK(7, 6)
#define MT_MDP_RCFR0_MCU_RX_CTL_BAR	GENMASK(9, 8)

#define MT_MDP_BNRCFR1(_band)		MT_MDP(__REG_OFFS(dev, MDP_BNRCFR1) + \
					       ((_band) << 8))
#define MT_MDP_RCFR1_MCU_RX_BYPASS	GENMASK(23, 22)
#define MT_MDP_RCFR1_RX_DROPPED_UCAST	GENMASK(28, 27)
#define MT_MDP_RCFR1_RX_DROPPED_MCAST	GENMASK(30, 29)
#define MT_MDP_TO_HIF			0
#define MT_MDP_TO_WM			1

/* TMAC: band 0(0x820e4000), band 1(0x820f4000) */
#define MT_WF_TMAC_BASE(_band)		((_band) ? 0x820f4000 : 0x820e4000)
#define MT_WF_TMAC(_band, ofs)		(MT_WF_TMAC_BASE(_band) + (ofs))

#define MT_TMAC_TCR0(_band)		MT_WF_TMAC(_band, 0)
#define MT_TMAC_TCR0_TX_BLINK		GENMASK(7, 6)
#define MT_TMAC_TCR0_TBTT_STOP_CTRL	BIT(25)

#define MT_TMAC_CDTR(_band)		MT_WF_TMAC(_band, __REG_OFFS(dev, TMAC_CDTR))
 #define MT_TMAC_ODTR(_band)		MT_WF_TMAC(_band, __REG_OFFS(dev, TMAC_ODTR))
#define MT_TIMEOUT_VAL_PLCP		GENMASK(15, 0)
#define MT_TIMEOUT_VAL_CCA		GENMASK(31, 16)

#define MT_TMAC_ATCR(_band)		MT_WF_TMAC(_band, __REG_OFFS(dev, TMAC_ATCR))
#define MT_TMAC_ATCR_TXV_TOUT		GENMASK(7, 0)

#define MT_TMAC_TRCR0(_band)		MT_WF_TMAC(_band, __REG_OFFS(dev, TMAC_TRCR0))
#define MT_TMAC_TRCR0_TR2T_CHK		GENMASK(8, 0)
#define MT_TMAC_TRCR0_I2T_CHK		GENMASK(24, 16)

#define MT_TMAC_ICR0(_band)		MT_WF_TMAC(_band, __REG_OFFS(dev, TMAC_ICR0))
#define MT_IFS_EIFS_OFDM		GENMASK(8, 0)
#define MT_IFS_RIFS			GENMASK(14, 10)
#define MT_IFS_SIFS			GENMASK(22, 16)
#define MT_IFS_SLOT			GENMASK(30, 24)

#define MT_TMAC_ICR1(_band)		MT_WF_TMAC(_band, __REG_OFFS(dev, TMAC_ICR1))
#define MT_IFS_EIFS_CCK			GENMASK(8, 0)

#define MT_TMAC_CTCR0(_band)		MT_WF_TMAC(_band, __REG_OFFS(dev, TMAC_CTCR0))
#define MT_TMAC_CTCR0_INS_DDLMT_REFTIME		GENMASK(5, 0)
#define MT_TMAC_CTCR0_INS_DDLMT_EN		BIT(17)
#define MT_TMAC_CTCR0_INS_DDLMT_VHT_SMPDU_EN	BIT(18)

#define MT_TMAC_TFCR0(_band)		MT_WF_TMAC(_band, __REG_OFFS(dev, TMAC_TFCR0))

/* WF DMA TOP: band 0(0x820e7000),band 1(0x820f7000) */
#define MT_WF_DMA_BASE(_band)		((_band) ? 0x820f7000 : 0x820e7000)
#define MT_WF_DMA(_band, ofs)		(MT_WF_DMA_BASE(_band) + (ofs))

#define MT_DMA_DCR0(_band)		MT_WF_DMA(_band, 0x000)
#define MT_DMA_DCR0_MAX_RX_LEN		GENMASK(15, 3)
#define MT_DMA_DCR0_RXD_G5_EN		BIT(23)

/* ETBF: band 0(0x820ea000), band 1(0x820fa000) */
#define MT_WF_ETBF_BASE(_band)		((_band) ? 0x820fa000 : 0x820ea000)
#define MT_WF_ETBF(_band, ofs)		(MT_WF_ETBF_BASE(_band) + (ofs))

#define MT_ETBF_TX_NDP_BFRP(_band)	MT_WF_ETBF(_band, 0x040)
#define MT_ETBF_TX_FB_CPL		GENMASK(31, 16)
#define MT_ETBF_TX_FB_TRI		GENMASK(15, 0)

#define MT_ETBF_RX_FB_CONT(_band)	MT_WF_ETBF(_band, 0x068)
#define MT_ETBF_RX_FB_BW		GENMASK(7, 6)
#define MT_ETBF_RX_FB_NC		GENMASK(5, 3)
#define MT_ETBF_RX_FB_NR		GENMASK(2, 0)

#define MT_ETBF_TX_APP_CNT(_band)	MT_WF_ETBF(_band, 0x0f0)
#define MT_ETBF_TX_IBF_CNT		GENMASK(31, 16)
#define MT_ETBF_TX_EBF_CNT		GENMASK(15, 0)

#define MT_ETBF_RX_FB_CNT(_band)	MT_WF_ETBF(_band, 0x0f8)
#define MT_ETBF_RX_FB_ALL		GENMASK(31, 24)
#define MT_ETBF_RX_FB_HE		GENMASK(23, 16)
#define MT_ETBF_RX_FB_VHT		GENMASK(15, 8)
#define MT_ETBF_RX_FB_HT		GENMASK(7, 0)

/* LPON: band 0(0x820eb000), band 1(0x820fb000) */
#define MT_WF_LPON_BASE(_band)		((_band) ? 0x820fb000 : 0x820eb000)
#define MT_WF_LPON(_band, ofs)		(MT_WF_LPON_BASE(_band) + (ofs))

#define MT_LPON_UTTR0(_band)		MT_WF_LPON(_band, __REG_OFFS(dev, LPON_UTTR0))
#define MT_LPON_UTTR1(_band)		MT_WF_LPON(_band, __REG_OFFS(dev, LPON_UTTR1))

#define MT_LPON_TCR(_band, n)		MT_WF_LPON(_band, 0x0a8 +	\
						   (((n) * 4) << __BIT(dev, LPON_TCR)))
#define MT_LPON_TCR_SW_MODE		GENMASK(1, 0)
#define MT_LPON_TCR_SW_WRITE		BIT(0)
#define MT_LPON_TCR_SW_ADJUST		BIT(1)
#define MT_LPON_TCR_SW_READ		GENMASK(1, 0)

/* MIB: band 0(0x820ed000), band 1(0x820fd000) */
/* These counters are (mostly?) clear-on-read.  So, some should not
 * be read at all in case firmware is already reading them.  These
 * are commented with 'DNR' below.  The DNR stats will be read by querying
 * the firmware API for the appropriate message.  For counters the driver
 * does read, the driver should accumulate the counters.
 */
#define MT_WF_MIB_BASE(_band)		((_band) ? 0x820fd000 : 0x820ed000)
#define MT_WF_MIB(_band, ofs)		(MT_WF_MIB_BASE(_band) + (ofs))

#define MT_MIB_SDR3(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDR3))
#define MT_MIB_SDR3_FCS_ERR_MASK	__MASK(dev, MIB_SDR3_FCS_ERR)

#define MT_MIB_SDR4(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDR4))
#define MT_MIB_SDR4_RX_FIFO_FULL_MASK	GENMASK(15, 0)
#define MT_MIB_SDR4_RX_OOR_MASK		GENMASK(23, 16)

/* rx mpdu counter, full 32 bits */
#define MT_MIB_SDR5(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDR5))

#define MT_MIB_SDR6(_band)		MT_WF_MIB(_band, 0x020)
#define MT_MIB_SDR6_CHANNEL_IDL_CNT_MASK	GENMASK(15, 0)

#define MT_MIB_SDR7(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDR7))
#define MT_MIB_SDR7_RX_VECTOR_MISMATCH_CNT_MASK	GENMASK(15, 0)

#define MT_MIB_SDR8(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDR8))
#define MT_MIB_SDR8_RX_DELIMITER_FAIL_CNT_MASK	GENMASK(15, 0)

/* aka CCA_NAV_TX_TIME */
#define MT_MIB_SDR9_DNR(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDR9))
#define MT_MIB_SDR9_CCA_BUSY_TIME_MASK		GENMASK(23, 0)

#define MT_MIB_SDR10_DNR(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDR10))
#define MT_MIB_SDR10_MRDY_COUNT_MASK		__MASK(dev, MIB_MRDY_CNT)

#define MT_MIB_SDR11(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDR11))
#define MT_MIB_SDR11_RX_LEN_MISMATCH_CNT_MASK	GENMASK(15, 0)

/* tx ampdu cnt, full 32 bits */
#define MT_MIB_SDR12(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDR12))

#define MT_MIB_SDR13(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDR13))
#define MT_MIB_SDR13_TX_STOP_Q_EMPTY_CNT_MASK	GENMASK(15, 0)

/* counts all mpdus in ampdu, regardless of success */
#define MT_MIB_SDR14(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDR14))
#define MT_MIB_SDR14_TX_MPDU_ATTEMPTS_CNT_MASK	__MASK(dev, MIB_MPDU_ATTEMPTS_CNT)

/* counts all successfully tx'd mpdus in ampdu */
#define MT_MIB_SDR15(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDR15))
#define MT_MIB_SDR15_TX_MPDU_SUCCESS_CNT_MASK	__MASK(dev, MIB_MPDU_SUCCESS_CNT)

/* in units of 'us' */
#define MT_MIB_SDR16_DNR(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDR16))
#define MT_MIB_SDR16_PRIMARY_CCA_BUSY_TIME_MASK	GENMASK(23, 0)

#define MT_MIB_SDR17_DNR(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDR17))
#define MT_MIB_SDR17_SECONDARY_CCA_BUSY_TIME_MASK	GENMASK(23, 0)

#define MT_MIB_SDR18(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDR18))
#define MT_MIB_SDR18_PRIMARY_ENERGY_DETECT_TIME_MASK	GENMASK(23, 0)

/* units are us */
#define MT_MIB_SDR19_DNR(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDR19))
#define MT_MIB_SDR19_CCK_MDRDY_TIME_MASK	GENMASK(23, 0)

#define MT_MIB_SDR20_DNR(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDR20))
#define MT_MIB_SDR20_OFDM_VHT_MDRDY_TIME_MASK	GENMASK(23, 0)

#define MT_MIB_SDR21_DNR(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDR21))
#define MT_MIB_SDR20_GREEN_MDRDY_TIME_MASK	GENMASK(23, 0)

/* rx ampdu count, 32-bit */
#define MT_MIB_SDR22(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDR22))

/* rx ampdu bytes count, 32-bit */
#define MT_MIB_SDR23(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDR23))

/* rx ampdu valid subframe count */
#define MT_MIB_SDR24(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDR24))
#define MT_MIB_SDR24_RX_AMPDU_SF_CNT_MASK	__MASK(dev, MIB_AMPDU_SF_CNT)

/* rx ampdu valid subframe bytes count, 32bits */
#define MT_MIB_SDR25(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDR25))

/* remaining windows protected stats */
#define MT_MIB_SDR27(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDR27))
#define MT_MIB_SDR27_TX_RWP_FAIL_CNT_MASK	GENMASK(15, 0)

#define MT_MIB_SDR28(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDR28))
#define MT_MIB_SDR28_TX_RWP_NEED_CNT_MASK	GENMASK(15, 0)

#define MT_MIB_SDR29(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDR29))
#define MT_MIB_SDR29_RX_PFDROP_CNT_MASK		__MASK(dev, MIB_PF_DROP_CNT)

#define MT_MIB_SDRVEC(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDRVEC))
#define MT_MIB_SDR30_RX_VEC_QUEUE_OVERFLOW_DROP_CNT_MASK	__MASK(dev, MIB_VEC_DROP_CNT)

/* rx blockack count, 32 bits */
#define MT_MIB_SDR31(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDR31))

#define MT_MIB_SDR32(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDR32))
#define MT_MIB_SDR32_TX_PKT_EBF_CNT_MASK	GENMASK(15, 0)

#define MT_MIB_SDR33(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDR33))
#define MT_MIB_SDR32_TX_PKT_IBF_CNT_MASK	__MASK(dev, MIB_BF_TX_CNT)

#define MT_MIB_SDRMUBF(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDRMUBF))
#define MT_MIB_MU_BF_TX_CNT		GENMASK(15, 0)

/* 36, 37 both DNR */

#define MT_MIB_SDR38(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDR38)) //0x0d0)
#define MT_MIB_CTRL_TX_CNT		GENMASK(23, 0)

#define MT_MIB_SDR39(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDR39)) //0x0d4)
#define MT_MIB_MGT_RETRY_CNT		GENMASK(23, 0)

#define MT_MIB_SDR40(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDR40)) //0x0d8)
#define MT_MIB_DATA_RETRY_CNT		GENMASK(23, 0)

#define MT_MIB_SDR42(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDR42)) // 0x0e0)
#define MT_MIB_RX_PARTIAL_BEACON_BSSID0	GENMASK(15, 0)
#define MT_MIB_RX_PARTIAL_BEACON_BSSID1	GENMASK(31, 16)

#define MT_MIB_SDR43(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDR43)) // 0x0e4)
#define MT_MIB_RX_PARTIAL_BEACON_BSSID2	GENMASK(15, 0)
#define MT_MIB_RX_PARTIAL_BEACON_BSSID3	GENMASK(31, 16)

/* This counter shall increment when  PPDUs dropped by the oppo_ps_rx_dis
 * mechanism
 */
#define MT_MIB_SDR46(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_SDR46)) // 0x0f0)
#define MT_MIB_OPPO_PS_RX_DIS_DROP_COUNT GENMASK(15, 0)


#define MT_MIB_DR8(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_DR8))
#define MT_MIB_DR9(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_DR9))
#define MT_MIB_DR11(_band)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_DR11))

#define MT_MIB_MB_SDR0(_band, n)	MT_WF_MIB(_band, __REG_OFFS(dev, MIB_MB_SDR0) +	\
						  ((n) << __BIT(dev, MIB_MB_SDR)))
#define MT_MIB_RTS_RETRIES_COUNT_MASK	GENMASK(31, 16)
#define MT_MIB_RTS_COUNT_MASK		GENMASK(15, 0)

#define MT_MIB_MB_SDR1(_band, n)	MT_WF_MIB(_band, __REG_OFFS(dev, MIB_MB_SDR1) + \
						  ((n) << __BIT(dev, MIB_MB_SDR)))
#define MT_MIB_BA_MISS_COUNT_MASK	GENMASK(15, 0)
#define MT_MIB_ACK_FAIL_COUNT_MASK	GENMASK(31, 16)

#define MT_TX_AGG_CNT(_band, n)		MT_WF_MIB(_band, __REG_OFFS(dev, TX_AGG_CNT) +	\
						  ((n) << 2))
#define MT_TX_AGG_CNT2(_band, n)	MT_WF_MIB(_band, __REG_OFFS(dev, TX_AGG_CNT2) +	\
						  ((n) << 2))
#define MT_MIB_ARNG(_band, n)		MT_WF_MIB(_band, __REG_OFFS(dev, MIB_ARNG) +	\
						  ((n) << 2))
#define MT_MIB_ARNCR_RANGE(val, n)	(((val) >> ((n) << 3)) & GENMASK(7, 0))

/* drop due to retries being exhausted */
#define MT_MIB_M0DROPSR00(_band)	MT_WF_MIB(_band, 0x190)
#define MT_MIB_RTS_RETRY_FAIL_DROP_MASK	GENMASK(15, 0)
#define MT_MIB_MPDU_RETRY_FAIL_DROP_MASK GENMASK(31, 16)

/* life time out limit */
#define MT_MIB_M0DROPSR01(_band)	MT_WF_MIB(_band, 0x194)
#define MT_MIB_LTO_FAIL_DROP_MASK	GENMASK(15, 0)

/* increment when using double number of spatial streams */
#define MT_MIB_SDR50(_band)		MT_WF_MIB(_band, 0x1dc)
#define MT_MIB_DBNSS_CNT_DROP_MASK	GENMASK(15, 0)

/* NOTE:  Would need to poll this quickly since it is 16-bit */
#define MT_MIB_SDR51(_band)		MT_WF_MIB(_band, 0x1e0)
#define MT_MIB_RX_FCS_OK_MASK		GENMASK(15, 0)

#define MT_WTBLON_TOP_BASE		0x34000
#define MT_WTBLON_TOP(ofs)		(MT_WTBLON_TOP_BASE + (ofs))
#define MT_WTBLON_TOP_WDUCR		MT_WTBLON_TOP(0x0)
#define MT_WTBLON_TOP_WDUCR_GROUP	GENMASK(2, 0)

#define MT_WTBL_UPDATE			MT_WTBLON_TOP(0x030)
#define MT_WTBL_UPDATE_WLAN_IDX		GENMASK(9, 0)
#define MT_WTBL_UPDATE_ADM_COUNT_CLEAR	BIT(12)
#define MT_WTBL_UPDATE_BUSY		BIT(31)

#define MT_WTBL_BASE			0x38000
#define MT_WTBL_LMAC_ID			GENMASK(14, 8)
#define MT_WTBL_LMAC_DW			GENMASK(7, 2)
#define MT_WTBL_LMAC_OFFS(_id, _dw)	(MT_WTBL_BASE | \
					FIELD_PREP(MT_WTBL_LMAC_ID, _id) | \
					FIELD_PREP(MT_WTBL_LMAC_DW, _dw))

/* AGG: band 0(0x20800), band 1(0xa0800) */
#define MT_WF_AGG_BASE(_band)		((_band) ? 0xa0800 : 0x20800)
#define MT_WF_AGG(_band, ofs)		(MT_WF_AGG_BASE(_band) + (ofs))

#define MT_AGG_AWSCR0(_band, _n)	MT_WF_AGG(_band, 0x05c + (_n) * 4)
#define MT_AGG_PCR0(_band, _n)		MT_WF_AGG(_band, 0x06c + (_n) * 4)
#define MT_AGG_PCR0_MM_PROT		BIT(0)
#define MT_AGG_PCR0_GF_PROT		BIT(1)
#define MT_AGG_PCR0_BW20_PROT		BIT(2)
#define MT_AGG_PCR0_BW40_PROT		BIT(4)
#define MT_AGG_PCR0_BW80_PROT		BIT(6)
#define MT_AGG_PCR0_ERP_PROT		GENMASK(12, 8)
#define MT_AGG_PCR0_VHT_PROT		BIT(13)
#define MT_AGG_PCR0_PTA_WIN_DIS		BIT(15)

#define MT_AGG_PCR1_RTS0_NUM_THRES	GENMASK(31, 23)
#define MT_AGG_PCR1_RTS0_LEN_THRES	GENMASK(19, 0)

#define MT_AGG_ACR0(_band)		MT_WF_AGG(_band, __REG_OFFS(dev, AGG_ACR0))
#define MT_AGG_ACR_CFEND_RATE		GENMASK(13, 0)
#define MT_AGG_ACR_BAR_RATE		GENMASK(29, 16)

#define MT_AGG_MRCR(_band)		MT_WF_AGG(_band, __REG_OFFS(dev, AGG_MRCR))
#define MT_AGG_MRCR_BAR_CNT_LIMIT		GENMASK(15, 12)
#define MT_AGG_MRCR_LAST_RTS_CTS_RN		BIT(6)
#define MT_AGG_MRCR_RTS_FAIL_LIMIT		GENMASK(11, 7)
#define MT_AGG_MRCR_TXCMD_RTS_FAIL_LIMIT	GENMASK(28, 24)

#define MT_AGG_ATCR1(_band)		MT_WF_AGG(_band, __REG_OFFS(dev, AGG_ATCR1))
#define MT_AGG_ATCR3(_band)		MT_WF_AGG(_band, __REG_OFFS(dev, AGG_ATCR3))

/* ARB: band 0(0x820e3000), band 1(0x820f3000) */
#define MT_WF_ARB_BASE(_band)		((_band) ? 0x820f3000 : 0x820e3000)
#define MT_WF_ARB(_band, ofs)		(MT_WF_ARB_BASE(_band) + (ofs))

#define MT_ARB_SCR(_band)		MT_WF_ARB(_band, __REG_OFFS(dev, ARB_SCR))
#define MT_ARB_SCR_TX_DISABLE		BIT(8)
#define MT_ARB_SCR_RX_DISABLE		BIT(9)

#define MT_ARB_DRNGR0(_band, _n)	MT_WF_ARB(_band, (__REG_OFFS(dev, ARB_DRNGR0) +	\
							  (_n) * 4))

/* RMAC: band 0(0x820e5000), band 1(0x820f5000) */
#define MT_WF_RMAC_BASE(_band)		((_band) ? 0x820f5000 : 0x820e5000)
#define MT_WF_RMAC(_band, ofs)		(MT_WF_RMAC_BASE(_band) + (ofs))

#define MT_WF_RFCR(_band)		MT_WF_RMAC(_band, 0x000)
#define MT_WF_RFCR_DROP_STBC_MULTI	BIT(0)
#define MT_WF_RFCR_DROP_FCSFAIL		BIT(1)
#define MT_WF_RFCR_DROP_VERSION		BIT(3)
#define MT_WF_RFCR_DROP_PROBEREQ	BIT(4)
#define MT_WF_RFCR_DROP_MCAST		BIT(5)
#define MT_WF_RFCR_DROP_BCAST		BIT(6)
#define MT_WF_RFCR_DROP_MCAST_FILTERED	BIT(7)
#define MT_WF_RFCR_DROP_A3_MAC		BIT(8)
#define MT_WF_RFCR_DROP_A3_BSSID	BIT(9)
#define MT_WF_RFCR_DROP_A2_BSSID	BIT(10)
#define MT_WF_RFCR_DROP_OTHER_BEACON	BIT(11)
#define MT_WF_RFCR_DROP_FRAME_REPORT	BIT(12)
#define MT_WF_RFCR_DROP_CTL_RSV		BIT(13)
#define MT_WF_RFCR_DROP_CTS		BIT(14)
#define MT_WF_RFCR_DROP_RTS		BIT(15)
#define MT_WF_RFCR_DROP_DUPLICATE	BIT(16)
#define MT_WF_RFCR_DROP_OTHER_BSS	BIT(17)
#define MT_WF_RFCR_DROP_OTHER_UC	BIT(18)
#define MT_WF_RFCR_DROP_OTHER_TIM	BIT(19)
#define MT_WF_RFCR_DROP_NDPA		BIT(20)
#define MT_WF_RFCR_DROP_UNWANTED_CTL	BIT(21)

#define MT_WF_RFCR1(_band)		MT_WF_RMAC(_band, 0x004)
#define MT_WF_RFCR1_DROP_ACK		BIT(4)
#define MT_WF_RFCR1_DROP_BF_POLL	BIT(5)
#define MT_WF_RFCR1_DROP_BA		BIT(6)
#define MT_WF_RFCR1_DROP_CFEND		BIT(7)
#define MT_WF_RFCR1_DROP_CFACK		BIT(8)

#define MT_WF_RMAC_MIB_AIRTIME0(_band)	MT_WF_RMAC(_band, 0x0380)
#define MT_WF_RMAC_MIB_RXTIME_CLR	BIT(31)

/* WFDMA0 */
#define MT_WFDMA0(ofs)			__REG_MAP(dev, MT_WFDMA0_BASE, ofs)

#define MT_WFDMA0_RST			MT_WFDMA0(0x100)
#define MT_WFDMA0_RST_LOGIC_RST		BIT(4)
#define MT_WFDMA0_RST_DMASHDL_ALL_RST	BIT(5)

#define MT_WFDMA0_BUSY_ENA		MT_WFDMA0(0x13c)
#define MT_WFDMA0_BUSY_ENA_TX_FIFO0	BIT(0)
#define MT_WFDMA0_BUSY_ENA_TX_FIFO1	BIT(1)
#define MT_WFDMA0_BUSY_ENA_RX_FIFO	BIT(2)

#define MT_WFDMA0_GLO_CFG		MT_WFDMA0(0x208)
#define MT_WFDMA0_GLO_CFG_TX_DMA_EN	BIT(0)
#define MT_WFDMA0_GLO_CFG_RX_DMA_EN	BIT(2)

#define MT_WFDMA0_RST_DTX_PTR		MT_WFDMA0(0x20c)
#define MT_WFDMA0_PRI_DLY_INT_CFG0	MT_WFDMA0(0x2f0)

#define MT_WFDMA0_RX_RING0_EXT_CTRL	MT_WFDMA0(0x680)
#define MT_WFDMA0_RX_RING1_EXT_CTRL	MT_WFDMA0(0x684)
#define MT_WFDMA0_RX_RING2_EXT_CTRL	MT_WFDMA0(0x688)

/* WFDMA1 */
#define MT_WFDMA1(ofs)			__REG_MAP(dev, MT_WFDMA1_BASE, ofs)

#define MT_WFDMA1_RST			MT_WFDMA1(0x100)
#define MT_WFDMA1_RST_LOGIC_RST		BIT(4)
#define MT_WFDMA1_RST_DMASHDL_ALL_RST	BIT(5)

#define MT_WFDMA1_BUSY_ENA		MT_WFDMA1(0x13c)
#define MT_WFDMA1_BUSY_ENA_TX_FIFO0	BIT(0)
#define MT_WFDMA1_BUSY_ENA_TX_FIFO1	BIT(1)
#define MT_WFDMA1_BUSY_ENA_RX_FIFO	BIT(2)

#define MT_WFDMA1_GLO_CFG		MT_WFDMA1(0x208)
#define MT_WFDMA1_GLO_CFG_TX_DMA_EN	BIT(0)
#define MT_WFDMA1_GLO_CFG_RX_DMA_EN	BIT(2)
#define MT_WFDMA1_GLO_CFG_OMIT_TX_INFO	BIT(28)
#define MT_WFDMA1_GLO_CFG_OMIT_RX_INFO	BIT(27)

#define MT_WFDMA1_RST_DTX_PTR		MT_WFDMA1(0x20c)
#define MT_WFDMA1_PRI_DLY_INT_CFG0	MT_WFDMA1(0x2f0)

#define MT_WFDMA1_TX_RING0_EXT_CTRL	MT_WFDMA1(0x600)
#define MT_WFDMA1_TX_RING1_EXT_CTRL	MT_WFDMA1(0x604)
#define MT_WFDMA1_TX_RING2_EXT_CTRL	MT_WFDMA1(0x608)
#define MT_WFDMA1_TX_RING3_EXT_CTRL	MT_WFDMA1(0x60c)
#define MT_WFDMA1_TX_RING4_EXT_CTRL	MT_WFDMA1(0x610)
#define MT_WFDMA1_TX_RING5_EXT_CTRL	MT_WFDMA1(0x614)
#define MT_WFDMA1_TX_RING6_EXT_CTRL	MT_WFDMA1(0x618)
#define MT_WFDMA1_TX_RING7_EXT_CTRL	MT_WFDMA1(0x61c)

#define MT_WFDMA1_TX_RING16_EXT_CTRL	MT_WFDMA1(0x640)
#define MT_WFDMA1_TX_RING17_EXT_CTRL	MT_WFDMA1(0x644)
#define MT_WFDMA1_TX_RING18_EXT_CTRL	MT_WFDMA1(0x648)
#define MT_WFDMA1_TX_RING19_EXT_CTRL	MT_WFDMA1(0x64c)
#define MT_WFDMA1_TX_RING20_EXT_CTRL	MT_WFDMA1(0x650)
#define MT_WFDMA1_TX_RING21_EXT_CTRL	MT_WFDMA1(0x654)
#define MT_WFDMA1_TX_RING22_EXT_CTRL	MT_WFDMA1(0x658)
#define MT_WFDMA1_TX_RING23_EXT_CTRL	MT_WFDMA1(0x65c)

#define MT_WFDMA1_RX_RING0_EXT_CTRL	MT_WFDMA1(0x680)
#define MT_WFDMA1_RX_RING1_EXT_CTRL	MT_WFDMA1(0x684)
#define MT_WFDMA1_RX_RING2_EXT_CTRL	MT_WFDMA1(0x688)
#define MT_WFDMA1_RX_RING3_EXT_CTRL	MT_WFDMA1(0x68c)

/* WFDMA CSR */
#define MT_WFDMA_EXT_CSR(ofs)		__REG_MAP(dev, MT_WFDMA_EXT_CSR_BASE, (ofs))

#define MT_WFDMA_HOST_CONFIG		MT_WFDMA_EXT_CSR(0x30)
#define MT_WFDMA_HOST_CONFIG_PDMA_BAND	BIT(0)

#define MT_WFDMA_EXT_CSR_HIF_MISC	MT_WFDMA_EXT_CSR(0x44)
#define MT_WFDMA_EXT_CSR_HIF_MISC_BUSY	BIT(0)

#define MT_PCIE_RECOG_ID		0xd7090
#define MT_PCIE_RECOG_ID_MASK		GENMASK(30, 0)
#define MT_PCIE_RECOG_ID_SEM		BIT(31)

/* WFDMA0 PCIE1 */
#define MT_WFDMA0_PCIE1(ofs)		__REG_MAP(dev, MT_WFDMA0_PCIE1_BASE, (ofs))

#define MT_WFDMA0_PCIE1_BUSY_ENA	MT_WFDMA0_PCIE1(0x13c)
#define MT_WFDMA0_PCIE1_BUSY_ENA_TX_FIFO0	BIT(0)
#define MT_WFDMA0_PCIE1_BUSY_ENA_TX_FIFO1	BIT(1)
#define MT_WFDMA0_PCIE1_BUSY_ENA_RX_FIFO	BIT(2)

/* WFDMA1 PCIE1 */
#define MT_WFDMA1_PCIE1(ofs)		__REG_MAP(dev, MT_WFDMA1_PCIE1_BASE, (ofs))

#define MT_WFDMA1_PCIE1_BUSY_ENA	MT_WFDMA1_PCIE1(0x13c)
#define MT_WFDMA1_PCIE1_BUSY_ENA_TX_FIFO0	BIT(0)
#define MT_WFDMA1_PCIE1_BUSY_ENA_TX_FIFO1	BIT(1)
#define MT_WFDMA1_PCIE1_BUSY_ENA_RX_FIFO	BIT(2)

/* WFDMA COMMON */
#define MT_INT_SOURCE_CSR		__REG(dev, INT_SOURCE_CSR)
#define MT_INT_MASK_CSR			__REG(dev, INT_MASK_CSR)

#define MT_INT1_SOURCE_CSR		__REG(dev, INT1_SOURCE_CSR)
#define MT_INT1_MASK_CSR		__REG(dev, INT1_MASK_CSR)

#define MT_TX_RING_BASE			__REG(dev, TX_RING_BASE)
#define MT_RX_EVENT_RING_BASE		__REG(dev, RX_EVENT_RING_BASE)
#define MT_RX_DATA_RING_BASE		__REG(dev, RX_DATA_RING_BASE)

#define MT_INT_RX_DONE_DATA0		__BIT(dev, RX_DONE_DAND0)
#define MT_INT_RX_DONE_DATA1		__BIT(dev, RX_DONE_DAND1)
#define MT_INT_RX_DONE_WM		__BIT(dev, RX_DONE_MCU_WM)
#define MT_INT_RX_DONE_WA		__BIT(dev, RX_DONE_MCU_WA)
#define MT_INT_RX_DONE_WA_MAIN		__BIT(dev, RX_DONE_WA_BAND0)
#define MT_INT_RX_DONE_WA_EXT		__BIT(dev, RX_DONE_WA_BAND1)
#define MT_INT_MCU_CMD			__BIT(dev, RX_MCU_TO_HOST)

#define MT_INT_RX_DONE_MCU		(MT_INT_RX_DONE_WM |		\
					 MT_INT_RX_DONE_WA)
#define MT_INT_BAND0_RX_DONE		(MT_INT_RX_DONE_WA_MAIN |	\
					 MT_INT_RX_DONE_DATA0)
#define MT_INT_BAND1_RX_DONE		(MT_INT_RX_DONE_WA_MAIN |	\
					 MT_INT_RX_DONE_WA_EXT |	\
					 MT_INT_RX_DONE_DATA1)
#define MT_INT_RX_DONE_ALL		(MT_INT_RX_DONE_MCU |		\
					 MT_INT_BAND0_RX_DONE |		\
					 MT_INT_BAND1_RX_DONE)

#define MT_INT_TX_DONE_FWDL		__BIT(dev, TX_DONE_FWDL)
#define MT_INT_TX_DONE_MCU_WM		__BIT(dev, TX_DONE_MCU_WM)
#define MT_INT_TX_DONE_MCU_WA		__BIT(dev, TX_DONE_MCU_WA)
#define MT_INT_TX_DONE_BAND0		__BIT(dev, TX_DONE_BAND0)
#define MT_INT_TX_DONE_BAND1		__BIT(dev, TX_DONE_BAND1)

#define MT_INT_TX_DONE_MCU		(MT_INT_TX_DONE_MCU_WA |	\
					 MT_INT_TX_DONE_MCU_WM |	\
					 MT_INT_TX_DONE_FWDL)
#define MT_MCU_CMD			__REG(dev, INT_MCU_CMD_SOURCE)
#define MT_MCU_CMD_STOP_DMA_FW_RELOAD	BIT(1)
#define MT_MCU_CMD_STOP_DMA		BIT(2)
#define MT_MCU_CMD_RESET_DONE		BIT(3)
#define MT_MCU_CMD_RECOVERY_DONE	BIT(4)
#define MT_MCU_CMD_NORMAL_STATE		BIT(5)
#define MT_MCU_CMD_ERROR_MASK		GENMASK(5, 1)

/* TOP RGU */
#define MT_TOP_RGU_BASE			0x18000000
#define MT_TOP_PWR_CTRL			(MT_TOP_RGU_BASE + (0x0))
#define MT_TOP_PWR_KEY			(0x5746 << 16)
#define MT_TOP_PWR_SW_RST		BIT(0)
#define MT_TOP_PWR_SW_PWR_ON		GENMASK(3, 2)
#define MT_TOP_PWR_HW_CTRL		BIT(4)
#define MT_TOP_PWR_PWR_ON		BIT(7)

/* l1/l2 remap */
#define MT_HIF_REMAP_L1			__REG(dev, L1_REMAP_CFG_OFFSET)
#define MT_HIF_REMAP_L1_MASK		GENMASK(15, 0)
#define MT_HIF_REMAP_L1_OFFSET		GENMASK(15, 0)
#define MT_HIF_REMAP_L1_BASE		GENMASK(31, 16)
#define MT_HIF_REMAP_BASE_L1		__REG_MAP(dev, MT_REMAP_L1_BASE, 0)

#define MT_HIF_REMAP_L2			__REG(dev, L2_REMAP_CFG_OFFSET)
#define MT_HIF_REMAP_L2_MASK		__MASK(dev, L2_REMAP_MASK)
#define MT_HIF_REMAP_L2_OFFSET		__MASK(dev, L2_REMAP_OFFSET)
#define MT_HIF_REMAP_L2_BASE		__MASK(dev, L2_REMAP_BASE)
#define MT_HIF_REMAP_BASE_L2		__REG_MAP(dev, MT_REMAP_L2_BASE, 0)

#define MT_INFRA_BASE			0x18000000
#define MT_WFSYS0_PHY_START		0x18400000
#define MT_WFSYS1_PHY_START		0x18800000
#define MT_WFSYS1_PHY_END		0x18bfffff
#define MT_CBTOP1_PHY_START		0x70000000
#define MT_CBTOP1_PHY_END		0x7fffffff
#define MT_CBTOP2_PHY_START		0xf0000000
#define MT_CBTOP2_PHY_END		0xffffffff
#define MT_CONN_INFRA_MCU_START		0x7c000000
#define MT_CONN_INFRA_MCU_END		__REG_MAP(dev, MT_INFRA_MCU_END_BASE, 0)

/* FW MODE SYNC */
#define MT_SWDEF(ofs)			__REG_MAP(dev, MT_SWDEF_BASE, (ofs))

#define MT_SWDEF_MODE			MT_SWDEF(0x3c)
#define MT_SWDEF_NORMAL_MODE		0
#define MT_SWDEF_ICAP_MODE		1
#define MT_SWDEF_SPECTRUM_MODE		2

#define MT_DIC_CMD_REG_BASE		0x41f000
#define MT_DIC_CMD_REG(ofs)		(MT_DIC_CMD_REG_BASE + (ofs))
#define MT_DIC_CMD_REG_CMD		MT_DIC_CMD_REG(0x10)

#define MT_CPU_UTIL_BASE		0x41f030
#define MT_CPU_UTIL(ofs)		(MT_CPU_UTIL_BASE + (ofs))
#define MT_CPU_UTIL_BUSY_PCT		MT_CPU_UTIL(0x00)
#define MT_CPU_UTIL_PEAK_BUSY_PCT	MT_CPU_UTIL(0x04)
#define MT_CPU_UTIL_IDLE_CNT		MT_CPU_UTIL(0x08)
#define MT_CPU_UTIL_PEAK_IDLE_CNT	MT_CPU_UTIL(0x0c)
#define MT_CPU_UTIL_CTRL		MT_CPU_UTIL(0x1c)

/* LED */
#define MT_LED_TOP_BASE			0x18013000
#define MT_LED_PHYS(_n)			(MT_LED_TOP_BASE + (_n))

#define MT_LED_CTRL(_n)			MT_LED_PHYS(0x00 + ((_n) * 4))
#define MT_LED_CTRL_KICK		BIT(7)
#define MT_LED_CTRL_BLINK_MODE		BIT(2)
#define MT_LED_CTRL_POLARITY		BIT(1)

#define MT_LED_TX_BLINK(_n)		MT_LED_PHYS(0x10 + ((_n) * 4))
#define MT_LED_TX_BLINK_ON_MASK		GENMASK(7, 0)
#define MT_LED_TX_BLINK_OFF_MASK        GENMASK(15, 8)

#define MT_LED_EN(_n)			MT_LED_PHYS(0x40 + ((_n) * 4))

#define MT_LED_GPIO_MUX2                0x70005058 /* GPIO 18 */
#define MT_LED_GPIO_MUX3                0x7000505C /* GPIO 26 */
#define MT_LED_GPIO_SEL_MASK            GENMASK(11, 8)

/* MT TOP */
#define MT_TOP_BASE			0x18060000
#define MT_TOP(ofs)			(MT_TOP_BASE + (ofs))

#define MT_TOP_LPCR_HOST_BAND0		MT_TOP(0x10)
#define MT_TOP_LPCR_HOST_FW_OWN		BIT(0)
#define MT_TOP_LPCR_HOST_DRV_OWN	BIT(1)

#define MT_TOP_MISC			MT_TOP(0xf0)
#define MT_TOP_MISC_FW_STATE		GENMASK(2, 0)

#define MT_HW_BOUND			0x70010020
#define MT_HW_CHIPID			0x70010200
#define MT_HW_REV			0x70010204

/* PCIE MAC */
#define MT_PCIE_MAC(ofs)		__REG_MAP(dev, MT_PCIE_MAC_BASE, (ofs))
#define MT_PCIE_MAC_INT_ENABLE		MT_PCIE_MAC(0x188)

#define MT_PCIE1_MAC(ofs)		__REG_MAP(dev, MT_PCIE1_MAC_BASE, (ofs))
#define MT_PCIE1_MAC_INT_ENABLE		MT_PCIE1_MAC(0x188)

#define MT_WF_IRPI_BASE			0x83006000
#define MT_WF_IRPI(ofs)			(MT_WF_IRPI_BASE + ((ofs) << 16))

/* PHY: band 0(0x83080000), band 1(0x83090000) */
#define MT_WF_PHY_BASE			0x83080000
#define MT_WF_PHY(ofs)			(MT_WF_PHY_BASE + (ofs))

#define MT_WF_PHY_RX_CTRL1(_phy)	MT_WF_PHY(0x2004 + ((_phy) << 16))
#define MT_WF_PHY_RX_CTRL1_IPI_EN	GENMASK(2, 0)
#define MT_WF_PHY_RX_CTRL1_STSCNT_EN	GENMASK(11, 9)

#define MT_WF_PHY_RXTD12(_phy)		MT_WF_PHY(0x8230 + ((_phy) << 16))
#define MT_WF_PHY_RXTD12_IRPI_SW_CLR_ONLY	BIT(18)
#define MT_WF_PHY_RXTD12_IRPI_SW_CLR		BIT(29)

#define MT_MCU_WM_CIRQ_BASE			0x89010000
#define MT_MCU_WM_CIRQ(ofs)			(MT_MCU_WM_CIRQ_BASE + (ofs))
#define MT_MCU_WM_CIRQ_IRQ_MASK_CLR_ADDR	MT_MCU_WM_CIRQ(0x80)
#define MT_MCU_WM_CIRQ_IRQ_SOFT_ADDR		MT_MCU_WM_CIRQ(0xc0)

#endif
