/*
 * ttc.h - Zynq Triple Timer Counter
 *
 * Copyright (C) 2021 Linzhi Ltd.
 *
 * This work is licensed under the terms of the MIT License.
 * A copy of the license can be found in the file COPYING.txt
 */

#ifndef TTC_H
#define	TTC_H

#define	TTC_BASE	0xF8001000
#define	TTC_SIZE	(0x1000 + 0x84)

#define	TTC_CLK_CTRL(ttc, t) \
	(*(volatile uint32_t *) (ttc_base + (ttc) * 0x1000 + (t) * 4))

#define	TTC_CLK_CTRL_EXT_NEG_SHIFT	6
#define	TTC_CLK_CTRL_EXT_NEG_MASK	1
#define	TTC_CLK_CTRL_EXT_CLK_SHIFT	5
#define	TTC_CLK_CTRL_EXT_CLK_MASK	1
#define	TTC_CLK_CTRL_PRE_SHR_SHIFT	1
#define	TTC_CLK_CTRL_PRE_SHR_MASK	15
#define	TTC_CLK_CTRL_PRE_EN_SHIFT	0
#define	TTC_CLK_CTRL_PRE_EN_MASK	1

#define TTC_CNT_CTRL(ttc, t) \
	(*(volatile uint32_t *) (ttc_base + 0xc + (ttc) * 0x1000 + (t) * 4))

#define	TTC_CNT_CTRL_WAVE_HL_SHIFT	6
#define	TTC_CNT_CTRL_WAVE_HL_MASK	1
#define	TTC_CNT_CTRL_WAVE_nEN_SHIFT	5
#define	TTC_CNT_CTRL_WAVE_nEN_MASK	1
#define	TTC_CNT_CTRL_RST_SHIFT		4
#define	TTC_CNT_CTRL_RST_MASK		1
#define	TTC_CNT_CTRL_MATCH_EN_SHIFT	3
#define	TTC_CNT_CTRL_MATCH_EN_MASK	1
#define	TTC_CNT_CTRL_DEC_SHIFT		2
#define	TTC_CNT_CTRL_DEC_MASK		1
#define	TTC_CNT_CTRL_INTERVAL_SHIFT	1
#define	TTC_CNT_CTRL_INTERVAL_MASK	1
#define	TTC_CNT_CTRL_nEN_SHIFT		0
#define	TTC_CNT_CTRL_nEN_MASK		1

#define TTC_COUNTER(ttc, t) \
	(*(volatile uint32_t *) (ttc_base + 0x18 + (ttc) * 0x1000 + (t) * 4))
#define TTC_INTERVAL(ttc, t) \
	(*(volatile uint32_t *) (ttc_base + 0x24 + (ttc) * 0x1000 + (t) * 4))
#define TTC_MATCH_1(ttc, t) \
	(*(volatile uint32_t *) (ttc_base + 0x30 + (ttc) * 0x1000 + (t) * 4))
#define TTC_MATCH_2(ttc, t) \
	(*(volatile uint32_t *) (ttc_base + 0x3c + (ttc) * 0x1000 + (t) * 4))
#define TTC_MATCH_3(ttc, t) \
	(*(volatile uint32_t *) (ttc_base + 0x48 + (ttc) * 0x1000 + (t) * 4))

#define	TTC_ISR(ttc, t) \
	(*(volatile uint32_t *) (ttc_base + 0x54 + (ttc) * 0x1000 + (t) * 4))
#define	TTC_IER(ttc, t) \
	(*(volatile uint32_t *) (ttc_base + 0x60 + (ttc) * 0x1000 + (t) * 4))


/* For TTC_ISR and TTC_IER */

#define	TTC_INT_EV_SHIFT		5
#define	TTC_INT_EV_MASK			1
#define	TTC_INT_OVR_SHIFT		4
#define	TTC_INT_OVR_MASK		1
#define	TTC_INT_MATCH_3_SHIFT		3
#define	TTC_INT_MATCH_3_MASK		1
#define	TTC_INT_MATCH_2_SHIFT		2
#define	TTC_INT_MATCH_2_MASK		1
#define	TTC_INT_MATCH_1_SHIFT		1
#define	TTC_INT_MATCH_1_MASK		1
#define	TTC_INT_INTERVAL_SHIFT		0
#define	TTC_INT_INTERVAL_MASK		1

#define	TTC_EV_CTRL(ttc, t) \
	(*(volatile uint32_t *) (ttc_base + 0x6c + (ttc) * 0x1000 + (t) * 4))

#define	TTC_EV_CTRL_OV_SHIFT		2
#define	TTC_EV_CTRL_OV_MASK		1
#define	TTC_EV_CTRL_LOW_SHIFT		1
#define	TTC_EV_CTRL_LOW_MASK		1
#define	TTC_EV_CTRL_EN_SHIFT		0
#define	TTC_EV_CTRL_EN_MASK		1

#define	TTC_EV_REG(ttc, t) \
	(*(volatile uint32_t *) (ttc_base + 0x78 + (ttc) * 0x1000 + (t) * 4))

volatile void *ttc_base;


void ttc_open(void);
void ttc_close(void);

#endif /* !TTC_H */
