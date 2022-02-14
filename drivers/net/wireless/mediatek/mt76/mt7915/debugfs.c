// SPDX-License-Identifier: ISC
/* Copyright (C) 2020 MediaTek Inc. */

#include "mt7915.h"
#include "eeprom.h"
#include "mcu.h"

/** global debugfs **/

struct hw_queue_map {
	const char *name;
	u8 index;
	u8 pid;
	u8 qid;
};

static int
mt7915_implicit_txbf_set(void *data, u64 val)
{
	struct mt7915_dev *dev = data;

	if (test_bit(MT76_STATE_RUNNING, &dev->mphy.state))
		return -EBUSY;

	dev->ibf = !!val;

	return mt7915_mcu_set_txbf(dev, MT_BF_TYPE_UPDATE);
}

static int
mt7915_implicit_txbf_get(void *data, u64 *val)
{
	struct mt7915_dev *dev = data;

	*val = dev->ibf;

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(fops_implicit_txbf, mt7915_implicit_txbf_get,
			 mt7915_implicit_txbf_set, "%lld\n");

/* test knob of system layer 1/2 error recovery */
static int mt7915_ser_trigger_set(void *data, u64 val)
{
	enum {
		SER_SET_RECOVER_L1 = 1,
		SER_SET_RECOVER_L2,
		SER_ENABLE = 2,
		SER_RECOVER
	};
	struct mt7915_dev *dev = data;
	int ret = 0;

	switch (val) {
	case SER_SET_RECOVER_L1:
	case SER_SET_RECOVER_L2:
		ret = mt7915_mcu_set_ser(dev, SER_ENABLE, BIT(val), 0);
		if (ret)
			return ret;

		return mt7915_mcu_set_ser(dev, SER_RECOVER, val, 0);
	default:
		break;
	}

	return ret;
}

DEFINE_DEBUGFS_ATTRIBUTE(fops_ser_trigger, NULL,
			 mt7915_ser_trigger_set, "%lld\n");

static int
mt7915_radar_trigger(void *data, u64 val)
{
	struct mt7915_dev *dev = data;

	return mt7915_mcu_rdd_cmd(dev, RDD_RADAR_EMULATE, 1, 0, 0);
}

DEFINE_DEBUGFS_ATTRIBUTE(fops_radar_trigger, NULL,
			 mt7915_radar_trigger, "%lld\n");

static int
mt7915_fw_debug_wm_set(void *data, u64 val)
{
	struct mt7915_dev *dev = data;
	enum {
		DEBUG_TXCMD = 62,
		DEBUG_CMD_RPT_TX,
		DEBUG_CMD_RPT_TRIG,
		DEBUG_SPL,
		DEBUG_RPT_RX,
	} debug;
	int ret;

	dev->fw_debug_wm = val ? MCU_FW_LOG_TO_HOST : 0;

	ret = mt7915_mcu_fw_log_2_host(dev, MCU_FW_LOG_WM, dev->fw_debug_wm);
	if (ret)
		return ret;

	for (debug = DEBUG_TXCMD; debug <= DEBUG_RPT_RX; debug++) {
		ret = mt7915_mcu_fw_dbg_ctrl(dev, debug, !!dev->fw_debug_wm);
		if (ret)
			return ret;
	}

	/* WM CPU info record control */
	mt76_clear(dev, MT_CPU_UTIL_CTRL, BIT(0));
	mt76_wr(dev, MT_DIC_CMD_REG_CMD, BIT(2) | BIT(13) | !dev->fw_debug_wm);
	mt76_wr(dev, MT_MCU_WM_CIRQ_IRQ_MASK_CLR_ADDR, BIT(5));
	mt76_wr(dev, MT_MCU_WM_CIRQ_IRQ_SOFT_ADDR, BIT(5));

	return 0;
}

static int
mt7915_fw_debug_wm_get(void *data, u64 *val)
{
	struct mt7915_dev *dev = data;

	*val = dev->fw_debug_wm;

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(fops_fw_debug_wm, mt7915_fw_debug_wm_get,
			 mt7915_fw_debug_wm_set, "%lld\n");

static int
mt7915_fw_debug_wa_set(void *data, u64 val)
{
	struct mt7915_dev *dev = data;
	int ret;

	dev->fw_debug_wa = val ? MCU_FW_LOG_TO_HOST : 0;

	ret = mt7915_mcu_fw_log_2_host(dev, MCU_FW_LOG_WA, dev->fw_debug_wa);
	if (ret)
		return ret;

	return mt7915_mcu_wa_cmd(dev, MCU_WA_PARAM_CMD(SET), MCU_WA_PARAM_PDMA_RX,
				 !!dev->fw_debug_wa, 0);
}

static int
mt7915_fw_debug_wa_get(void *data, u64 *val)
{
	struct mt7915_dev *dev = data;

	*val = dev->fw_debug_wa;

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(fops_fw_debug_wa, mt7915_fw_debug_wa_get,
			 mt7915_fw_debug_wa_set, "%lld\n");

static int
mt7915_fw_util_wm_show(struct seq_file *file, void *data)
{
	struct mt7915_dev *dev = file->private;

	if (dev->fw_debug_wm) {
		seq_printf(file, "Busy: %u%%  Peak busy: %u%%\n",
			   mt76_rr(dev, MT_CPU_UTIL_BUSY_PCT),
			   mt76_rr(dev, MT_CPU_UTIL_PEAK_BUSY_PCT));
		seq_printf(file, "Idle count: %u  Peak idle count: %u\n",
			   mt76_rr(dev, MT_CPU_UTIL_IDLE_CNT),
			   mt76_rr(dev, MT_CPU_UTIL_PEAK_IDLE_CNT));
	}

	return 0;
}

DEFINE_SHOW_ATTRIBUTE(mt7915_fw_util_wm);

static int
mt7915_fw_util_wa_show(struct seq_file *file, void *data)
{
	struct mt7915_dev *dev = file->private;

	if (dev->fw_debug_wa)
		return mt7915_mcu_wa_cmd(dev, MCU_WA_PARAM_CMD(QUERY),
					 MCU_WA_PARAM_CPU_UTIL, 0, 0);

	return 0;
}

DEFINE_SHOW_ATTRIBUTE(mt7915_fw_util_wa);

struct mt7915_txo_worker_info {
	char* buf;
	int sofar;
	int size;
};

static void mt7915_txo_worker(void *wi_data, struct ieee80211_sta *sta)
{
	struct mt7915_txo_worker_info *wi = wi_data;
	struct mt7915_sta *msta = (struct mt7915_sta *)sta->drv_priv;
	struct mt76_testmode_data *td = &msta->test;
	struct ieee80211_vif *vif;
	struct wireless_dev *wdev;

	if (wi->sofar >= wi->size)
		return; /* buffer is full */

	vif = container_of((void *)msta->vif, struct ieee80211_vif, drv_priv);
	wdev = ieee80211_vif_to_wdev(vif);

	wi->sofar += scnprintf(wi->buf + wi->sofar, wi->size - wi->sofar,
			       "vdev (%s) active=%d tpc=%d sgi=%d mcs=%d nss=%d"
			       " pream=%d retries=%d dynbw=%d bw=%d\n",
			       wdev->netdev->name,
			       td->txo_active, td->tx_power[0],
			       td->tx_rate_sgi, td->tx_rate_idx,
			       td->tx_rate_nss, td->tx_rate_mode,
			       td->tx_xmit_count, td->tx_dynbw,
			       td->txbw);
}

static ssize_t mt7915_read_set_rate_override(struct file *file,
					     char __user *user_buf,
					     size_t count, loff_t *ppos)
{
	struct mt7915_dev *dev = file->private_data;
        struct ieee80211_hw *hw = dev->mphy.hw;
	char *buf2;
	int size = 8000;
	int rv, sofar;
	struct mt7915_txo_worker_info wi;
	const char buf[] =
		"This allows specify specif tx rate parameters for all DATA"
		" frames on a vdev\n"
		"To set a value, you specify the dev-name and key-value pairs:\n"
		"tpc=10 sgi=1 mcs=x nss=x pream=x retries=x dynbw=0|1 bw=x enable=0|1\n"
		"pream: 0=cck, 1=ofdm, 2=HT, 3=VHT, 4=HE_SU\n"
		"cck-mcs: 0=1Mbps, 1=2Mbps, 3=5.5Mbps, 3=11Mbps\n"
		"ofdm-mcs: 0=6Mbps, 1=9Mbps, 2=12Mbps, 3=18Mbps, 4=24Mbps, 5=36Mbps,"
		" 6=48Mbps, 7=54Mbps\n"
		"tpc: adjust power from defaults, in 1/2 db units 0 - 31, 16 is default\n"
		"bw is 0-3 for 20-160\n"
		" For example, wlan0:\n"
		"echo \"wlan0 tpc=255 sgi=1 mcs=0 nss=1 pream=3 retries=1 dynbw=0 bw=0"
		" active=1\" > ...mt76/set_rate_override\n";

	buf2 = kzalloc(size, GFP_KERNEL);
	if (!buf2)
		return -ENOMEM;
	strcpy(buf2, buf);
	sofar = strlen(buf2);

	wi.sofar = sofar;
	wi.buf = buf2;
	wi.size = size;

	ieee80211_iterate_stations_atomic(hw, mt7915_txo_worker, &wi);

	rv = simple_read_from_buffer(user_buf, count, ppos, buf2, wi.sofar);
	kfree(buf2);
	return rv;
}

/* Set the rates for specific types of traffic.
 */
static ssize_t mt7915_write_set_rate_override(struct file *file,
					      const char __user *user_buf,
					      size_t count, loff_t *ppos)
{
	struct mt7915_dev *dev = file->private_data;
	struct mt7915_sta *msta;
	struct ieee80211_vif *vif;
	struct mt76_testmode_data *td = NULL;
	struct wireless_dev *wdev;
	struct mt76_wcid *wcid;
	struct mt76_phy *mphy = &dev->mt76.phy;
	char buf[180];
	char tmp[20];
	char *tok;
	int ret, i, j;
	unsigned int vdev_id = 0xFFFF;
	char *bufptr = buf;
	long rc;
	char dev_name_match[IFNAMSIZ + 2];

	memset(buf, 0, sizeof(buf));

	simple_write_to_buffer(buf, sizeof(buf) - 1, ppos, user_buf, count);

	/* make sure that buf is null terminated */
	buf[sizeof(buf) - 1] = 0;

	/* drop the possible '\n' from the end */
	if (buf[count - 1] == '\n')
		buf[count - 1] = 0;

	mutex_lock(&mphy->dev->mutex);

	/* Ignore empty lines, 'echo' appends them sometimes at least. */
	if (buf[0] == 0) {
		ret = count;
		goto exit;
	}

	/* String starts with vdev name, ie 'wlan0'  Find the proper vif that
	 * matches the name.
	 */
	for (i = 0; i < ARRAY_SIZE(dev->mt76.wcid_mask); i++) {
		u32 mask = dev->mt76.wcid_mask[i];
		u32 phy_mask = dev->mt76.wcid_phy_mask[i];

		if (!mask)
			continue;

		for (j = i * 32; mask; j++, mask >>= 1, phy_mask >>= 1) {
			if (!(mask & 1))
				continue;

			wcid = rcu_dereference(dev->mt76.wcid[j]);
			if (!wcid)
				continue;

			msta = container_of(wcid, struct mt7915_sta, wcid);

			vif = container_of((void *)msta->vif, struct ieee80211_vif, drv_priv);

			wdev = ieee80211_vif_to_wdev(vif);

			if (!wdev)
				continue;

			snprintf(dev_name_match, sizeof(dev_name_match) - 1, "%s ",
				 wdev->netdev->name);

			if (strncmp(dev_name_match, buf, strlen(dev_name_match)) == 0) {
				vdev_id = j;
				td = &msta->test;
				bufptr = buf + strlen(dev_name_match) - 1;
				break;
			}
		}
	}

	if (vdev_id == 0xFFFF) {
		if (strstr(buf, "active=0")) {
			/* Ignore, we are disabling it anyway */
			ret = count;
			goto exit;
		} else {
			dev_info(dev->mt76.dev,
				 "mt7915: set-rate-override, unknown netdev name: %s\n", buf);
		}
		ret = -EINVAL;
		goto exit;
	}

#define MT7915_PARSE_LTOK(a, b)					\
	do {								\
		tok = strstr(bufptr, " " #a "=");			\
		if (tok) {						\
			char *tspace;					\
			tok += 1; /* move past initial space */		\
			strncpy(tmp, tok + strlen(#a "="), sizeof(tmp) - 1); \
			tmp[sizeof(tmp) - 1] = 0;			\
			tspace = strstr(tmp, " ");			\
			if (tspace)					\
				*tspace = 0;				\
			if (kstrtol(tmp, 0, &rc) != 0)			\
				dev_info(dev->mt76.dev,			\
					 "mt7915: set-rate-override: " #a \
					 "= could not be parsed, tmp: %s\n", \
					 tmp);				\
			else						\
				td->b = rc;				\
		}							\
	} while (0)

	/* TODO:  Allow configuring LTF? */
	td->tx_ltf = 1; /* 0: HTLTF 3.2us, 1: HELTF, 6.4us, 2 HELTF 12,8us */

	MT7915_PARSE_LTOK(tpc, tx_power[0]);
	MT7915_PARSE_LTOK(sgi, tx_rate_sgi);
	MT7915_PARSE_LTOK(mcs, tx_rate_idx);
	MT7915_PARSE_LTOK(nss, tx_rate_nss);
	MT7915_PARSE_LTOK(pream, tx_rate_mode);
	MT7915_PARSE_LTOK(retries, tx_xmit_count);
	MT7915_PARSE_LTOK(dynbw, tx_dynbw);
	MT7915_PARSE_LTOK(bw, txbw);
	MT7915_PARSE_LTOK(active, txo_active);

	dev_info(dev->mt76.dev,
		 "mt7915: set-rate-overrides, vdev %i(%s) active=%d tpc=%d sgi=%d mcs=%d"
		 " nss=%d pream=%d retries=%d dynbw=%d bw=%d\n",
		 vdev_id, dev_name_match,
		 td->txo_active, td->tx_power[0], td->tx_rate_sgi, td->tx_rate_idx,
		 td->tx_rate_nss, td->tx_rate_mode, td->tx_xmit_count, td->tx_dynbw,
		 td->txbw);

	ret = count;

exit:
	mutex_unlock(&mphy->dev->mutex);
	return ret;
}

static const struct file_operations fops_set_rate_override = {
	.read = mt7915_read_set_rate_override,
	.write = mt7915_write_set_rate_override,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

static int
mt7915_txs_for_no_skb_set(void *data, u64 val)
{
	struct mt7915_dev *dev = data;

	dev->txs_for_no_skb_enabled = !!val;

	return 0;
}

static int
mt7915_txs_for_no_skb_get(void *data, u64 *val)
{
	struct mt7915_dev *dev = data;

	*val = dev->txs_for_no_skb_enabled;

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(fops_txs_for_no_skb, mt7915_txs_for_no_skb_get,
			 mt7915_txs_for_no_skb_set, "%lld\n");

static int
mt7915_rx_group_5_enable_set(void *data, u64 val)
{
	struct mt7915_dev *dev = data;

	mutex_lock(&dev->mt76.mutex);

	dev->rx_group_5_enable = !!val;

	/* Enabled if we requested enabled OR if monitor mode is enabled. */
	mt76_rmw_field(dev, MT_DMA_DCR0(0), MT_DMA_DCR0_RXD_G5_EN,
		       dev->phy.is_monitor_mode || dev->rx_group_5_enable);
	mt76_testmode_reset(dev->phy.mt76, true);

	mutex_unlock(&dev->mt76.mutex);

	return 0;
}

static int
mt7915_rx_group_5_enable_get(void *data, u64 *val)
{
	struct mt7915_dev *dev = data;

	*val = dev->rx_group_5_enable;

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(fops_rx_group_5_enable, mt7915_rx_group_5_enable_get,
			 mt7915_rx_group_5_enable_set, "%lld\n");

static void
mt7915_ampdu_stat_read_phy(struct mt7915_phy *phy,
			   struct seq_file *file)
{
	struct mt7915_dev *dev = phy->dev;
	bool ext_phy = phy != &dev->phy;
	int bound[15], range[4], i, n;

	/* Tx ampdu stat */
	for (i = 0; i < ARRAY_SIZE(range); i++)
		range[i] = mt76_rr(dev, MT_MIB_ARNG(ext_phy, i));

	for (i = 0; i < ARRAY_SIZE(bound); i++)
		bound[i] = MT_MIB_ARNCR_RANGE(range[i / 4], i % 4) + 1;

	seq_printf(file, "\nPhy %d\n", ext_phy);

	seq_printf(file, "Length: %8d | ", bound[0]);
	for (i = 0; i < ARRAY_SIZE(bound) - 1; i++)
		seq_printf(file, "%3d -%3d | ",
			   bound[i] + 1, bound[i + 1]);

	seq_puts(file, "\nCount:  ");
	n = ext_phy ? ARRAY_SIZE(dev->mt76.aggr_stats) / 2 : 0;
	for (i = 0; i < ARRAY_SIZE(bound); i++)
		seq_printf(file, "%8d | ", dev->mt76.aggr_stats[i + n]);
	seq_puts(file, "\n");

	seq_printf(file, "BA miss count: %d\n", phy->mib.ba_miss_cnt);
}

static void
mt7915_txbf_stat_read_phy(struct mt7915_phy *phy, struct seq_file *s)
{
	static const char * const bw[] = {
		"BW20", "BW40", "BW80", "BW160"
	};
	struct mib_stats *mib = &phy->mib;

	/* Tx Beamformer monitor */
	seq_puts(s, "\nTx Beamformer applied PPDU counts: ");

	seq_printf(s, "iBF: %d, eBF: %d\n",
		   mib->tx_bf_ibf_ppdu_cnt,
		   mib->tx_bf_ebf_ppdu_cnt);

	/* Tx Beamformer Rx feedback monitor */
	seq_puts(s, "Tx Beamformer Rx feedback statistics: ");

	seq_printf(s, "All: %d, HE: %d, VHT: %d, HT: %d, ",
		   mib->tx_bf_rx_fb_all_cnt,
		   mib->tx_bf_rx_fb_he_cnt,
		   mib->tx_bf_rx_fb_vht_cnt,
		   mib->tx_bf_rx_fb_ht_cnt);

	seq_printf(s, "%s, NC: %d, NR: %d\n",
		   bw[mib->tx_bf_rx_fb_bw],
		   mib->tx_bf_rx_fb_nc_cnt,
		   mib->tx_bf_rx_fb_nr_cnt);

	/* Tx Beamformee Rx NDPA & Tx feedback report */
	seq_printf(s, "Tx Beamformee successful feedback frames: %d\n",
		   mib->tx_bf_fb_cpl_cnt);
	seq_printf(s, "Tx Beamformee feedback triggered counts: %d\n",
		   mib->tx_bf_fb_trig_cnt);

	/* Tx SU & MU counters */
	seq_printf(s, "Tx multi-user Beamforming counts: %d\n",
		   mib->tx_bf_cnt);
	seq_printf(s, "Tx multi-user MPDU counts: %d\n", mib->tx_mu_mpdu_cnt);
	seq_printf(s, "Tx multi-user successful MPDU counts: %d\n",
		   mib->tx_mu_acked_mpdu_cnt);
	seq_printf(s, "Tx single-user successful MPDU counts: %d\n",
		   mib->tx_su_acked_mpdu_cnt);

	seq_puts(s, "\n");
}

static int
mt7915_tx_stats_show(struct seq_file *file, void *data)
{
	struct mt7915_phy *phy = file->private;
	struct mt7915_dev *dev = phy->dev;
	struct mib_stats *mib = &phy->mib;
	int i;

	mutex_lock(&dev->mt76.mutex);

	mt7915_ampdu_stat_read_phy(phy, file);
	mt7915_mac_update_stats(phy);
	mt7915_txbf_stat_read_phy(phy, file);

	/* Tx amsdu info */
	seq_puts(file, "Tx MSDU statistics:\n");
	for (i = 0; i < ARRAY_SIZE(mib->tx_amsdu); i++) {
		seq_printf(file, "AMSDU pack count of %d MSDU in TXD: %8d ",
			   i + 1, mib->tx_amsdu[i]);
		if (mib->tx_amsdu_cnt)
			seq_printf(file, "(%3d%%)\n",
				   mib->tx_amsdu[i] * 100 / mib->tx_amsdu_cnt);
		else
			seq_puts(file, "\n");
	}

	mutex_unlock(&dev->mt76.mutex);

	return 0;
}

DEFINE_SHOW_ATTRIBUTE(mt7915_tx_stats);

static void
mt7915_hw_queue_read(struct seq_file *s, u32 base, u32 size,
		     const struct hw_queue_map *map)
{
	struct mt7915_phy *phy = s->private;
	struct mt7915_dev *dev = phy->dev;
	u32 i, val;

	val = mt76_rr(dev, base + MT_FL_Q_EMPTY);
	for (i = 0; i < size; i++) {
		u32 ctrl, head, tail, queued;

		if (val & BIT(map[i].index))
			continue;

		ctrl = BIT(31) | (map[i].pid << 10) | (map[i].qid << 24);
		mt76_wr(dev, base + MT_FL_Q0_CTRL, ctrl);

		head = mt76_get_field(dev, base + MT_FL_Q2_CTRL,
				      GENMASK(11, 0));
		tail = mt76_get_field(dev, base + MT_FL_Q2_CTRL,
				      GENMASK(27, 16));
		queued = mt76_get_field(dev, base + MT_FL_Q3_CTRL,
					GENMASK(11, 0));

		seq_printf(s, "\t%s: ", map[i].name);
		seq_printf(s, "queued:0x%03x head:0x%03x tail:0x%03x\n",
			   queued, head, tail);
	}
}

static void
mt7915_sta_hw_queue_read(void *data, struct ieee80211_sta *sta)
{
	struct mt7915_sta *msta = (struct mt7915_sta *)sta->drv_priv;
	struct mt7915_dev *dev = msta->vif->phy->dev;
	struct seq_file *s = data;
	u8 ac;

	for (ac = 0; ac < 4; ac++) {
		u32 qlen, ctrl, val;
		u32 idx = msta->wcid.idx >> 5;
		u8 offs = msta->wcid.idx & GENMASK(4, 0);

		ctrl = BIT(31) | BIT(11) | (ac << 24);
		val = mt76_rr(dev, MT_PLE_AC_QEMPTY(ac, idx));

		if (val & BIT(offs))
			continue;

		mt76_wr(dev, MT_PLE_BASE + MT_FL_Q0_CTRL, ctrl | msta->wcid.idx);
		qlen = mt76_get_field(dev, MT_PLE_BASE + MT_FL_Q3_CTRL,
				      GENMASK(11, 0));
		seq_printf(s, "\tSTA %pM wcid %d: AC%d%d queued:%d\n",
			   sta->addr, msta->wcid.idx, msta->vif->wmm_idx,
			   ac, qlen);
	}
}

static int
mt7915_hw_queues_show(struct seq_file *file, void *data)
{
	struct mt7915_phy *phy = file->private;
	struct mt7915_dev *dev = phy->dev;
	static const struct hw_queue_map ple_queue_map[] = {
		{ "CPU_Q0",  0,  1, MT_CTX0	      },
		{ "CPU_Q1",  1,  1, MT_CTX0 + 1	      },
		{ "CPU_Q2",  2,  1, MT_CTX0 + 2	      },
		{ "CPU_Q3",  3,  1, MT_CTX0 + 3	      },
		{ "ALTX_Q0", 8,  2, MT_LMAC_ALTX0     },
		{ "BMC_Q0",  9,  2, MT_LMAC_BMC0      },
		{ "BCN_Q0",  10, 2, MT_LMAC_BCN0      },
		{ "PSMP_Q0", 11, 2, MT_LMAC_PSMP0     },
		{ "ALTX_Q1", 12, 2, MT_LMAC_ALTX0 + 4 },
		{ "BMC_Q1",  13, 2, MT_LMAC_BMC0  + 4 },
		{ "BCN_Q1",  14, 2, MT_LMAC_BCN0  + 4 },
		{ "PSMP_Q1", 15, 2, MT_LMAC_PSMP0 + 4 },
	};
	static const struct hw_queue_map pse_queue_map[] = {
		{ "CPU Q0",  0,  1, MT_CTX0	      },
		{ "CPU Q1",  1,  1, MT_CTX0 + 1	      },
		{ "CPU Q2",  2,  1, MT_CTX0 + 2	      },
		{ "CPU Q3",  3,  1, MT_CTX0 + 3	      },
		{ "HIF_Q0",  8,  0, MT_HIF0	      },
		{ "HIF_Q1",  9,  0, MT_HIF0 + 1	      },
		{ "HIF_Q2",  10, 0, MT_HIF0 + 2	      },
		{ "HIF_Q3",  11, 0, MT_HIF0 + 3	      },
		{ "HIF_Q4",  12, 0, MT_HIF0 + 4	      },
		{ "HIF_Q5",  13, 0, MT_HIF0 + 5	      },
		{ "LMAC_Q",  16, 2, 0		      },
		{ "MDP_TXQ", 17, 2, 1		      },
		{ "MDP_RXQ", 18, 2, 2		      },
		{ "SEC_TXQ", 19, 2, 3		      },
		{ "SEC_RXQ", 20, 2, 4		      },
	};
	u32 val, head, tail;

	/* ple queue */
	val = mt76_rr(dev, MT_PLE_FREEPG_CNT);
	head = mt76_get_field(dev, MT_PLE_FREEPG_HEAD_TAIL, GENMASK(11, 0));
	tail = mt76_get_field(dev, MT_PLE_FREEPG_HEAD_TAIL, GENMASK(27, 16));
	seq_puts(file, "PLE page info:\n");
	seq_printf(file,
		   "\tTotal free page: 0x%08x head: 0x%03x tail: 0x%03x\n",
		   val, head, tail);

	val = mt76_rr(dev, MT_PLE_PG_HIF_GROUP);
	head = mt76_get_field(dev, MT_PLE_HIF_PG_INFO, GENMASK(11, 0));
	tail = mt76_get_field(dev, MT_PLE_HIF_PG_INFO, GENMASK(27, 16));
	seq_printf(file, "\tHIF free page: 0x%03x res: 0x%03x used: 0x%03x\n",
		   val, head, tail);

	seq_puts(file, "PLE non-empty queue info:\n");
	mt7915_hw_queue_read(file, MT_PLE_BASE, ARRAY_SIZE(ple_queue_map),
			     &ple_queue_map[0]);

	/* iterate per-sta ple queue */
	ieee80211_iterate_stations_atomic(phy->mt76->hw,
					  mt7915_sta_hw_queue_read, file);
	/* pse queue */
	seq_puts(file, "PSE non-empty queue info:\n");
	mt7915_hw_queue_read(file, MT_PSE_BASE, ARRAY_SIZE(pse_queue_map),
			     &pse_queue_map[0]);

	return 0;
}

DEFINE_SHOW_ATTRIBUTE(mt7915_hw_queues);

static int
mt7915_xmit_queues_show(struct seq_file *file, void *data)
{
	struct mt7915_phy *phy = file->private;
	struct mt7915_dev *dev = phy->dev;
	struct {
		struct mt76_queue *q;
		char *queue;
	} queue_map[] = {
		{ phy->mt76->q_tx[MT_TXQ_BE],	 "   MAIN"  },
		{ dev->mt76.q_mcu[MT_MCUQ_WM],	 "  MCUWM"  },
		{ dev->mt76.q_mcu[MT_MCUQ_WA],	 "  MCUWA"  },
		{ dev->mt76.q_mcu[MT_MCUQ_FWDL], "MCUFWDL" },
	};
	int i;

	seq_puts(file, "     queue | hw-queued |      head |      tail |\n");
	for (i = 0; i < ARRAY_SIZE(queue_map); i++) {
		struct mt76_queue *q = queue_map[i].q;

		if (!q)
			continue;

		seq_printf(file, "   %s | %9d | %9d | %9d |\n",
			   queue_map[i].queue, q->queued, q->head,
			   q->tail);
	}

	return 0;
}

DEFINE_SHOW_ATTRIBUTE(mt7915_xmit_queues);

static int
mt7915_rate_txpower_show(struct seq_file *file, void *data)
{
	static const char * const sku_group_name[] = {
		"CCK", "OFDM", "HT20", "HT40",
		"VHT20", "VHT40", "VHT80", "VHT160",
		"RU26", "RU52", "RU106", "RU242/SU20",
		"RU484/SU40", "RU996/SU80", "RU2x996/SU160"
	};
	struct mt7915_phy *phy = file->private;
	s8 txpower[MT7915_SKU_RATE_NUM], *buf;
	int i;

	seq_puts(file, "Per-chain txpower in 1/2 db units.\n");
	seq_printf(file, "\nPhy %d\n", phy != &phy->dev->phy);
	mt7915_mcu_get_txpower_sku(phy, txpower, sizeof(txpower));
	for (i = 0, buf = txpower; i < ARRAY_SIZE(mt7915_sku_group_len); i++) {
		u8 mcs_num = mt7915_sku_group_len[i];

		if (i >= SKU_VHT_BW20 && i <= SKU_VHT_BW160)
			mcs_num = 10;

		mt76_seq_puts_array(file, sku_group_name[i], buf, mcs_num);
		buf += mt7915_sku_group_len[i];
	}

	return 0;
}

DEFINE_SHOW_ATTRIBUTE(mt7915_rate_txpower);

static int
mt7915_twt_stats(struct seq_file *s, void *data)
{
	struct mt7915_dev *dev = dev_get_drvdata(s->private);
	struct mt7915_twt_flow *iter;

	rcu_read_lock();

	seq_puts(s, "     wcid |       id |    flags |      exp | mantissa");
	seq_puts(s, " | duration |            tsf |\n");
	list_for_each_entry_rcu(iter, &dev->twt_list, list)
		seq_printf(s,
			"%9d | %8d | %5c%c%c%c | %8d | %8d | %8d | %14lld |\n",
			iter->wcid, iter->id,
			iter->sched ? 's' : 'u',
			iter->protection ? 'p' : '-',
			iter->trigger ? 't' : '-',
			iter->flowtype ? '-' : 'a',
			iter->exp, iter->mantissa,
			iter->duration, iter->tsf);

	rcu_read_unlock();

	return 0;
}

static int
mt7915_last_mcu_read(struct seq_file *file, void *data)
{
	struct mt7915_dev *dev = dev_get_drvdata(file->private);

	seq_puts(file, "MCU Settings:\n");

#define O_MCU_LE32(o, a)					\
	seq_printf(file,					\
		   "\t%-42s %d\n", #a,				\
		   le32_to_cpu((o)->last_mcu.a))		\

#define O_MCU_LE32X(o, a)					\
	seq_printf(file,					\
		   "\t%-42s 0x%0x\n", #a,			\
		   le32_to_cpu((o)->last_mcu.a))		\

#define O_MCU_LE16(o, a)					\
	seq_printf(file,					\
		   "\t%-42s %d\n", #a,				\
		   le16_to_cpu((o)->last_mcu.a))		\

#define O_MCU_LE16X(o, a)					\
	seq_printf(file,					\
		   "\t%-42s 0x%0x\n", #a,			\
		   le16_to_cpu((o)->last_mcu.a))		\

#define O_MCU_MAC(o, a)						\
	seq_printf(file,					\
		   "\t%-42s %pM\n", #a,				\
		   (o)->last_mcu.a)				\

#define O_MCU_U8(o, a)						\
	seq_printf(file,					\
		   "\t%-42s %d\n", #a,				\
		   (unsigned int)((o)->last_mcu.a))		\

#define O_MCU_U8X(o, a)						\
	seq_printf(file,					\
		   "\t%-42s 0x%x\n", #a,			\
		   (unsigned int)((o)->last_mcu.a))		\

#define O_MCU_U16(o, a)						\
	seq_printf(file,					\
		   "\t%-42s %d\n", #a,				\
		   (unsigned int)((o)->last_mcu.a))		\

#define O_MCU_S16(o, a)						\
	seq_printf(file,					\
		   "\t%-42s %d\n", #a,				\
		   (unsigned int)((o)->last_mcu.a))		\

/* dev variants */
#define D_MCU_LE32X(a)	O_MCU_LE32X(dev, a)
#define D_MCU_LE32(a)	O_MCU_LE32(dev, a)
#define D_MCU_LE16X(a)	O_MCU_LE16X(dev, a)
#define D_MCU_LE16(a)	O_MCU_LE16(dev, a)
#define D_MCU_MAC(a)	O_MCU_MAC(dev, a)
#define D_MCU_U8(a)	O_MCU_U8(dev, a)
#define D_MCU_U8X(a)	O_MCU_U8X(dev, a)
#define D_MCU_U16(a)	O_MCU_U16(dev, a)
#define D_MCU_S16(a)	O_MCU_S16(dev, a)

	D_MCU_U8(mcu_chan_info.control_ch);
	D_MCU_U8(mcu_chan_info.center_ch);
	D_MCU_U8(mcu_chan_info.bw);
	D_MCU_U8(mcu_chan_info.tx_streams_num);
	D_MCU_U8(mcu_chan_info.rx_streams);
	D_MCU_U8(mcu_chan_info.switch_reason);
	D_MCU_U8(mcu_chan_info.band_idx);
	D_MCU_U8(mcu_chan_info.center_ch2);
	D_MCU_LE16(mcu_chan_info.cac_case);
	D_MCU_U8(mcu_chan_info.channel_band);
	D_MCU_LE32(mcu_chan_info.outband_freq);
	D_MCU_U8(mcu_chan_info.txpower_drop);
	D_MCU_U8(mcu_chan_info.ap_bw);
	D_MCU_U8(mcu_chan_info.ap_center_ch);

	return 0;
}

int mt7915_init_debugfs(struct mt7915_phy *phy)
{
	struct mt7915_dev *dev = phy->dev;
	bool ext_phy = phy != &dev->phy;
	struct dentry *dir;

	dir = mt76_register_debugfs_fops(phy->mt76, NULL);
	if (!dir)
		return -ENOMEM;

	debugfs_create_file("hw-queues", 0400, dir, phy,
			    &mt7915_hw_queues_fops);
	debugfs_create_file("xmit-queues", 0400, dir, phy,
			    &mt7915_xmit_queues_fops);
	debugfs_create_file("tx_stats", 0400, dir, phy, &mt7915_tx_stats_fops);
	debugfs_create_file("fw_debug_wm", 0600, dir, dev, &fops_fw_debug_wm);
	debugfs_create_file("fw_debug_wa", 0600, dir, dev, &fops_fw_debug_wa);
	debugfs_create_file("fw_util_wm", 0400, dir, dev,
			    &mt7915_fw_util_wm_fops);
	debugfs_create_file("fw_util_wa", 0400, dir, dev,
			    &mt7915_fw_util_wa_fops);
	debugfs_create_file("force_txs", 0600, dir, dev, &fops_txs_for_no_skb);
	debugfs_create_file("rx_group_5_enable", 0600, dir, dev, &fops_rx_group_5_enable);
	debugfs_create_file("implicit_txbf", 0600, dir, dev,
			    &fops_implicit_txbf);
	debugfs_create_file("txpower_sku", 0400, dir, phy,
			    &mt7915_rate_txpower_fops);
	debugfs_create_devm_seqfile(dev->mt76.dev, "twt_stats", dir,
				    mt7915_twt_stats);
	debugfs_create_devm_seqfile(dev->mt76.dev, "last_mcu", dir,
				    mt7915_last_mcu_read);
	debugfs_create_file("ser_trigger", 0200, dir, dev, &fops_ser_trigger);
	if (!dev->dbdc_support || ext_phy) {
		debugfs_create_u32("dfs_hw_pattern", 0400, dir,
				   &dev->hw_pattern);
		debugfs_create_file("radar_trigger", 0200, dir, dev,
				    &fops_radar_trigger);
	}
	debugfs_create_file("set_rate_override", 0600, dir,
			    dev, &fops_set_rate_override);

	return 0;
}

#ifdef CONFIG_MAC80211_DEBUGFS
/** per-station debugfs **/

static ssize_t mt7915_sta_fixed_rate_set(struct file *file,
					 const char __user *user_buf,
					 size_t count, loff_t *ppos)
{
	struct ieee80211_sta *sta = file->private_data;
	struct mt7915_sta *msta = (struct mt7915_sta *)sta->drv_priv;
	struct mt7915_dev *dev = msta->vif->phy->dev;
	struct ieee80211_vif *vif;
	struct sta_phy phy = {};
	char buf[100];
	int ret;
	u32 field;
	u8 i, gi, he_ltf;

	if (count >= sizeof(buf))
		return -EINVAL;

	if (copy_from_user(buf, user_buf, count))
		return -EFAULT;

	if (count && buf[count - 1] == '\n')
		buf[count - 1] = '\0';
	else
		buf[count] = '\0';

	/* mode - cck: 0, ofdm: 1, ht: 2, gf: 3, vht: 4, he_su: 8, he_er: 9
	 * bw - bw20: 0, bw40: 1, bw80: 2, bw160: 3
	 * nss - vht: 1~4, he: 1~4, others: ignore
	 * mcs - cck: 0~4, ofdm: 0~7, ht: 0~32, vht: 0~9, he_su: 0~11, he_er: 0~2
	 * gi - (ht/vht) lgi: 0, sgi: 1; (he) 0.8us: 0, 1.6us: 1, 3.2us: 2
	 * ldpc - off: 0, on: 1
	 * stbc - off: 0, on: 1
	 * he_ltf - 1xltf: 0, 2xltf: 1, 4xltf: 2
	 */
	if (sscanf(buf, "%hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu",
		   &phy.type, &phy.bw, &phy.nss, &phy.mcs, &gi,
		   &phy.ldpc, &phy.stbc, &he_ltf) != 8) {
		dev_warn(dev->mt76.dev,
			 "format: Mode BW NSS MCS (HE)GI LDPC STBC HE_LTF\n");
		field = RATE_PARAM_AUTO;
		goto out;
	}

	phy.ldpc = (phy.bw || phy.ldpc) * GENMASK(2, 0);
	for (i = 0; i <= phy.bw; i++) {
		phy.sgi |= gi << (i << sta->he_cap.has_he);
		phy.he_ltf |= he_ltf << (i << sta->he_cap.has_he);
	}
	field = RATE_PARAM_FIXED;

out:
	vif = container_of((void *)msta->vif, struct ieee80211_vif, drv_priv);
	ret = mt7915_mcu_set_fixed_rate_ctrl(dev, vif, sta, &phy, field);
	if (ret)
		return -EFAULT;

	return count;
}

static const struct file_operations fops_fixed_rate = {
	.write = mt7915_sta_fixed_rate_set,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

static int
mt7915_queues_show(struct seq_file *s, void *data)
{
	struct ieee80211_sta *sta = s->private;

	mt7915_sta_hw_queue_read(s, sta);

	return 0;
}

DEFINE_SHOW_ATTRIBUTE(mt7915_queues);

static int
mt7915_mcu_settings_show(struct seq_file *file, void *data)
{
	struct ieee80211_sta *sta = file->private;
	struct mt7915_sta *msta = (struct mt7915_sta *)sta->drv_priv;
	struct mt7915_vif *mvif = msta->vif;
	int i;

	/* Last-applied settings to MCU */
	seq_puts(file, "MCU Settings:\n");
	seq_puts(file, " VIF Settings:\n");

#define MCU_LE32X(a)	O_MCU_LE32X(mvif, a)
#define MCU_LE32(a)	O_MCU_LE32(mvif, a)
#define MCU_LE16X(a)	O_MCU_LE16X(mvif, a)
#define MCU_LE16(a)	O_MCU_LE16(mvif, a)
#define MCU_MAC(a)	O_MCU_MAC(mvif, a)
#define MCU_U8(a)	O_MCU_U8(mvif, a)
#define MCU_U8X(a)	O_MCU_U8X(mvif, a)
#define MCU_U16(a)	O_MCU_U16(mvif, a)
#define MCU_S16(a)	O_MCU_S16(mvif, a)

/* mSTA variants */
#define S_MCU_LE32X(a)	O_MCU_LE32X(msta, a)
#define S_MCU_LE32(a)	O_MCU_LE32(msta, a)
#define S_MCU_LE16X(a)	O_MCU_LE16X(msta, a)
#define S_MCU_LE16(a)	O_MCU_LE16(msta, a)
#define S_MCU_MAC(a)	O_MCU_MAC(msta, a)
#define S_MCU_U8(a)	O_MCU_U8(msta, a)
#define S_MCU_U8X(a)	O_MCU_U8X(msta, a)
#define S_MCU_U16(a)	O_MCU_U16(msta, a)
#define S_MCU_S16(a)	O_MCU_S16(msta, a)

	MCU_LE32(bss_info_basic.network_type);
	MCU_U8(bss_info_basic.active);
	MCU_U8(bss_info_basic.rsv0);
	MCU_LE16(bss_info_basic.bcn_interval);
	MCU_MAC(bss_info_basic.bssid);
	MCU_U8(bss_info_basic.wmm_idx);
	MCU_U8(bss_info_basic.dtim_period);
	MCU_U8(bss_info_basic.bmc_wcid_lo);
	MCU_U8(bss_info_basic.cipher);
	MCU_U8(bss_info_basic.phy_mode);
	MCU_U8(bss_info_basic.max_bssid);
	MCU_U8(bss_info_basic.non_tx_bssid);
	MCU_U8(bss_info_basic.bmc_wcid_hi);

	seq_puts(file, "\n");
	MCU_U8(bss_info_omac.hw_bss_idx);
	MCU_U8(bss_info_omac.omac_idx);
	MCU_U8(bss_info_omac.band_idx);

	seq_puts(file, "\n");
	MCU_U8(bss_info_rf_ch.pri_ch);
	MCU_U8(bss_info_rf_ch.center_ch0);
	MCU_U8(bss_info_rf_ch.center_ch1);
	MCU_U8(bss_info_rf_ch.bw);
	MCU_U8(bss_info_rf_ch.he_ru26_block);
	MCU_U8(bss_info_rf_ch.he_all_disable);

	seq_puts(file, "\n");
	MCU_U8(bss_info_ra.op_mode);
	MCU_U8(bss_info_ra.adhoc_en);
	MCU_U8(bss_info_ra.short_preamble);
	MCU_U8(bss_info_ra.tx_streams);
	MCU_U8(bss_info_ra.rx_streams);
	MCU_U8(bss_info_ra.algo);
	MCU_U8(bss_info_ra.force_sgi);
	MCU_U8(bss_info_ra.force_gf);
	MCU_U8(bss_info_ra.ht_mode);
	MCU_U8(bss_info_ra.has_20_sta);
	MCU_U8(bss_info_ra.bss_width_trigger_events);
	MCU_U8(bss_info_ra.vht_nss_cap);
	/* these are not used, evidently */
	/* MCU_U8(bss_info_ra.vht_bw_signal); */
	/* MCU_U8(bss_info_ra.vht_force_sgi); */
	MCU_U8(bss_info_ra.se_off);
	MCU_U8(bss_info_ra.antenna_idx);
	MCU_U8(bss_info_ra.train_up_rule);
	MCU_U16(bss_info_ra.train_up_high_thres);
	MCU_S16(bss_info_ra.train_up_rule_rssi);
	MCU_U16(bss_info_ra.low_traffic_thres);
	MCU_LE16(bss_info_ra.max_phyrate);
	MCU_LE32(bss_info_ra.phy_cap);
	MCU_LE32(bss_info_ra.interval);
	MCU_LE32(bss_info_ra.fast_interval);

	seq_puts(file, "\n");
	MCU_U8(bss_info_he.he_pe_duration);
	MCU_U8(bss_info_he.vht_op_info_present);
	MCU_LE16(bss_info_he.he_rts_thres);
	MCU_LE16(bss_info_he.max_nss_mcs[CMD_HE_MCS_BW80]);
	MCU_LE16(bss_info_he.max_nss_mcs[CMD_HE_MCS_BW160]);
	MCU_LE16(bss_info_he.max_nss_mcs[CMD_HE_MCS_BW8080]);

	seq_puts(file, "\n");
	MCU_LE32(bss_info_hw_amsdu.cmp_bitmap_0);
	MCU_LE32(bss_info_hw_amsdu.cmp_bitmap_1);
	MCU_LE16(bss_info_hw_amsdu.trig_thres);
	MCU_U8(bss_info_hw_amsdu.enable);

	seq_puts(file, "\n");
	MCU_LE32(bss_info_ext_bss.mbss_tsf_offset);

	seq_puts(file, "\n");
	MCU_LE16(bss_info_bmc_rate.bc_trans);
	MCU_LE16(bss_info_bmc_rate.mc_trans);
	MCU_U8(bss_info_bmc_rate.short_preamble);

	seq_puts(file, "\n STA Settings:\n");

	S_MCU_U8(sta_rec_ba.tid);
	S_MCU_U8(sta_rec_ba.ba_type);
	S_MCU_U8(sta_rec_ba.amsdu);
	S_MCU_U8(sta_rec_ba.ba_en);
	S_MCU_LE16(sta_rec_ba.ssn);
	S_MCU_LE16(sta_rec_ba.winsize);

	seq_puts(file, "\n");
	S_MCU_U8(wtbl_ba.tid);
	S_MCU_U8(wtbl_ba.ba_type);
	S_MCU_LE16(wtbl_ba.sn);
	S_MCU_U8(wtbl_ba.ba_en);
	S_MCU_U8(wtbl_ba.ba_winsize_idx);
	S_MCU_LE16(wtbl_ba.ba_winsize);
	S_MCU_MAC(wtbl_ba.peer_addr);
	S_MCU_U8(wtbl_ba.rst_ba_tid);
	S_MCU_U8(wtbl_ba.rst_ba_sel);
	S_MCU_U8(wtbl_ba.rst_ba_sb);
	S_MCU_U8(wtbl_ba.band_idx);

	seq_puts(file, "\n");
	S_MCU_U8(wtbl_rx.rcid);
	S_MCU_U8(wtbl_rx.rca1);
	S_MCU_U8(wtbl_rx.rca2);
	S_MCU_U8(wtbl_rx.rv);

	seq_puts(file, "\n");
	S_MCU_LE32(sta_rec_basic.conn_type);
	S_MCU_U8(sta_rec_basic.conn_state);
	S_MCU_U8(sta_rec_basic.qos);
	S_MCU_LE16(sta_rec_basic.aid);
	S_MCU_MAC(sta_rec_basic.peer_addr);
	S_MCU_LE16(sta_rec_basic.extra_info);

	seq_puts(file, "\n");
	S_MCU_LE32X(sta_rec_he.he_cap);
	S_MCU_U8(sta_rec_he.t_frame_dur);
	S_MCU_U8(sta_rec_he.max_ampdu_exp);
	S_MCU_U8(sta_rec_he.bw_set);
	S_MCU_U8(sta_rec_he.device_class);
	S_MCU_U8(sta_rec_he.dcm_tx_mode);
	S_MCU_U8(sta_rec_he.dcm_tx_max_nss);
	S_MCU_U8(sta_rec_he.dcm_rx_mode);
	S_MCU_U8(sta_rec_he.dcm_rx_max_nss);
	S_MCU_U8(sta_rec_he.dcm_max_ru);
	S_MCU_U8(sta_rec_he.punc_pream_rx);
	S_MCU_U8(sta_rec_he.pkt_ext);
	S_MCU_LE16(sta_rec_he.max_nss_mcs[CMD_HE_MCS_BW80]);
	S_MCU_LE16(sta_rec_he.max_nss_mcs[CMD_HE_MCS_BW160]);
	S_MCU_LE16(sta_rec_he.max_nss_mcs[CMD_HE_MCS_BW8080]);

	seq_puts(file, "\n");
	S_MCU_U8X(sta_rec_uapsd.dac_map);
	S_MCU_U8X(sta_rec_uapsd.tac_map);
	S_MCU_U8(sta_rec_uapsd.max_sp);
	S_MCU_LE16(sta_rec_uapsd.listen_interval);

	seq_puts(file, "\n");
	S_MCU_U8(sta_rec_muru.cfg.ofdma_dl_en);
	S_MCU_U8(sta_rec_muru.cfg.ofdma_ul_en);
	S_MCU_U8(sta_rec_muru.cfg.mimo_dl_en);
	S_MCU_U8(sta_rec_muru.cfg.mimo_ul_en);
	S_MCU_U8(sta_rec_muru.ofdma_dl.punc_pream_rx);
	S_MCU_U8(sta_rec_muru.ofdma_dl.he_20m_in_40m_2g);
	S_MCU_U8(sta_rec_muru.ofdma_dl.he_20m_in_160m);
	S_MCU_U8(sta_rec_muru.ofdma_dl.he_80m_in_160m);
	S_MCU_U8(sta_rec_muru.ofdma_dl.lt16_sigb);
	S_MCU_U8(sta_rec_muru.ofdma_dl.rx_su_comp_sigb);
	S_MCU_U8(sta_rec_muru.ofdma_dl.rx_su_non_comp_sigb);
	S_MCU_U8(sta_rec_muru.ofdma_ul.t_frame_dur);
	S_MCU_U8(sta_rec_muru.ofdma_ul.mu_cascading);
	S_MCU_U8(sta_rec_muru.ofdma_ul.uo_ra);
	S_MCU_U8(sta_rec_muru.ofdma_ul.he_2x996_tone);
	S_MCU_U8(sta_rec_muru.ofdma_ul.rx_t_frame_11ac);
	S_MCU_U8(sta_rec_muru.mimo_dl.vht_mu_bfee);
	S_MCU_U8(sta_rec_muru.mimo_dl.partial_bw_dl_mimo);
	S_MCU_U8(sta_rec_muru.mimo_ul.full_ul_mimo);
	S_MCU_U8(sta_rec_muru.mimo_ul.partial_ul_mimo);

	seq_puts(file, "\n");
	S_MCU_LE32X(sta_rec_vht.vht_cap);
	S_MCU_LE16X(sta_rec_vht.vht_rx_mcs_map);
	S_MCU_LE16X(sta_rec_vht.vht_tx_mcs_map);

	seq_puts(file, "\n");
	S_MCU_U8(sta_rec_amsdu.max_amsdu_num);
	S_MCU_U8(sta_rec_amsdu.max_mpdu_size);
	S_MCU_U8(sta_rec_amsdu.amsdu_en);

	seq_puts(file, "\n");
	S_MCU_LE16X(sta_rec_ht.ht_cap);

	seq_puts(file, "\n");
	S_MCU_U8(wtbl_smps.smps);

	seq_puts(file, "\n");
	S_MCU_U8(wtbl_ht.ht);
	S_MCU_U8(wtbl_ht.ldpc);
	S_MCU_U8(wtbl_ht.af);
	S_MCU_U8(wtbl_ht.mm);

	seq_puts(file, "\n");
	S_MCU_U8(wtbl_vht.ldpc);
	S_MCU_U8(wtbl_vht.dyn_bw);
	S_MCU_U8(wtbl_vht.vht);
	S_MCU_U8(wtbl_vht.txop_ps);

	seq_puts(file, "\n");
	S_MCU_U8(wtbl_hdr_trans.to_ds);
	S_MCU_U8(wtbl_hdr_trans.from_ds);
	S_MCU_U8(wtbl_hdr_trans.no_rx_trans);

	seq_puts(file, "\n");
	S_MCU_LE16X(sta_rec_bf.pfmu);
	S_MCU_U8(sta_rec_bf.su_mu);
	S_MCU_U8(sta_rec_bf.bf_cap);
	S_MCU_U8(sta_rec_bf.sounding_phy);
	S_MCU_U8(sta_rec_bf.ndpa_rate);
	S_MCU_U8(sta_rec_bf.ndp_rate);
	S_MCU_U8(sta_rec_bf.rept_poll_rate);
	S_MCU_U8(sta_rec_bf.tx_mode);
	S_MCU_U8(sta_rec_bf.bf_cap);
	S_MCU_U8(sta_rec_bf.ncol);
	S_MCU_U8(sta_rec_bf.nrow);
	S_MCU_U8(sta_rec_bf.bw);
	S_MCU_U8(sta_rec_bf.mem_total);
	S_MCU_U8(sta_rec_bf.mem_20m);
	for (i = 0; i < 4; i++) {
		S_MCU_U8(sta_rec_bf.mem[i].row);
		S_MCU_U8(sta_rec_bf.mem[i].col);
		S_MCU_U8(sta_rec_bf.mem[i].row_msb);
	}
	S_MCU_LE16X(sta_rec_bf.smart_ant);
	S_MCU_U8(sta_rec_bf.se_idx);
	S_MCU_U8X(sta_rec_bf.auto_sounding);
	S_MCU_U8(sta_rec_bf.ibf_timeout);
	S_MCU_U8(sta_rec_bf.ibf_dbw);
	S_MCU_U8(sta_rec_bf.ibf_ncol);
	S_MCU_U8(sta_rec_bf.ibf_nrow);
	S_MCU_U8(sta_rec_bf.nrow_bw160);
	S_MCU_U8(sta_rec_bf.ncol_bw160);
	S_MCU_U8(sta_rec_bf.ru_start_idx);
	S_MCU_U8(sta_rec_bf.ru_end_idx);
	S_MCU_U8(sta_rec_bf.trigger_su);
	S_MCU_U8(sta_rec_bf.trigger_mu);
	S_MCU_U8(sta_rec_bf.ng16_su);
	S_MCU_U8(sta_rec_bf.ng16_mu);
	S_MCU_U8(sta_rec_bf.codebook42_su);
	S_MCU_U8(sta_rec_bf.codebook75_mu);
	S_MCU_U8(sta_rec_bf.he_ltf);

	seq_puts(file, "\n");
	S_MCU_U8(sta_rec_bfee.fb_identity_matrix);
	S_MCU_U8(sta_rec_bfee.ignore_feedback);

	seq_puts(file, "\n");
	S_MCU_U8(sta_rec_ra.valid);
	S_MCU_U8(sta_rec_ra.auto_rate);
	S_MCU_U8(sta_rec_ra.phy_mode);
	S_MCU_U8(sta_rec_ra.channel);
	S_MCU_U8(sta_rec_ra.bw);
	S_MCU_U8(sta_rec_ra.disable_cck);
	S_MCU_U8(sta_rec_ra.ht_mcs32);
	S_MCU_U8(sta_rec_ra.ht_gf);
	S_MCU_U8(sta_rec_ra.ht_mcs[0]);
	S_MCU_U8(sta_rec_ra.ht_mcs[1]);
	S_MCU_U8(sta_rec_ra.ht_mcs[2]);
	S_MCU_U8(sta_rec_ra.ht_mcs[3]);
	S_MCU_U8(sta_rec_ra.mmps_mode);
	S_MCU_U8(sta_rec_ra.gband_256);
	S_MCU_U8(sta_rec_ra.af);
	S_MCU_U8(sta_rec_ra.auth_wapi_mode);
	S_MCU_U8(sta_rec_ra.rate_len);
	S_MCU_U8(sta_rec_ra.supp_mode);
	S_MCU_U8(sta_rec_ra.supp_cck_rate);
	S_MCU_U8(sta_rec_ra.supp_ofdm_rate);
	S_MCU_LE32X(sta_rec_ra.supp_ht_mcs);
	S_MCU_LE16X(sta_rec_ra.supp_vht_mcs[0]);
	S_MCU_LE16X(sta_rec_ra.supp_vht_mcs[1]);
	S_MCU_LE16X(sta_rec_ra.supp_vht_mcs[2]);
	S_MCU_LE16X(sta_rec_ra.supp_vht_mcs[3]);
	S_MCU_U8(sta_rec_ra.op_mode);
	S_MCU_U8(sta_rec_ra.op_vht_chan_width);
	S_MCU_U8(sta_rec_ra.op_vht_rx_nss);
	S_MCU_U8(sta_rec_ra.op_vht_rx_nss_type);
	S_MCU_LE32X(sta_rec_ra.sta_cap);
	S_MCU_U8(sta_rec_ra.phy.type);
	S_MCU_U8X(sta_rec_ra.phy.flag);
	S_MCU_U8(sta_rec_ra.phy.stbc);
	S_MCU_U8(sta_rec_ra.phy.sgi);
	S_MCU_U8(sta_rec_ra.phy.bw);
	S_MCU_U8(sta_rec_ra.phy.ldpc);
	S_MCU_U8(sta_rec_ra.phy.mcs);
	S_MCU_U8(sta_rec_ra.phy.nss);
	S_MCU_U8(sta_rec_ra.phy.he_ltf);

	seq_puts(file, "\n");
	S_MCU_LE32X(sta_rec_ra_fixed.field);
	S_MCU_U8(sta_rec_ra_fixed.op_mode);
	S_MCU_U8(sta_rec_ra_fixed.op_vht_chan_width);
	S_MCU_U8(sta_rec_ra_fixed.op_vht_rx_nss);
	S_MCU_U8(sta_rec_ra_fixed.op_vht_rx_nss_type);
	S_MCU_U8(sta_rec_ra_fixed.phy.type);
	S_MCU_U8X(sta_rec_ra_fixed.phy.flag);
	S_MCU_U8(sta_rec_ra_fixed.phy.stbc);
	S_MCU_U8(sta_rec_ra_fixed.phy.sgi);
	S_MCU_U8(sta_rec_ra_fixed.phy.bw);
	S_MCU_U8(sta_rec_ra_fixed.phy.ldpc);
	S_MCU_U8(sta_rec_ra_fixed.phy.mcs);
	S_MCU_U8(sta_rec_ra_fixed.phy.nss);
	S_MCU_U8(sta_rec_ra_fixed.phy.he_ltf);

	S_MCU_U8(sta_rec_ra_fixed.spe_en);
	S_MCU_U8(sta_rec_ra_fixed.short_preamble);
	S_MCU_U8(sta_rec_ra_fixed.is_5g);
	S_MCU_U8(sta_rec_ra_fixed.mmps_mode);

	return 0;
}

DEFINE_SHOW_ATTRIBUTE(mt7915_mcu_settings);

void mt7915_sta_add_debugfs(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
			    struct ieee80211_sta *sta, struct dentry *dir)
{
	debugfs_create_file("fixed_rate", 0600, dir, sta, &fops_fixed_rate);
	debugfs_create_file("hw-queues", 0400, dir, sta, &mt7915_queues_fops);
	debugfs_create_file("mcu_settings", 0400, dir, sta, &mt7915_mcu_settings_fops);
}

#endif
