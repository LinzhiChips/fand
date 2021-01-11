/*
 * pwm.c - Set up PWM (Pulse-Width Modulation) outputs
 *
 * Copyright (C) 2021 Linzhi Ltd.
 *
 * This work is licensed under the terms of the MIT License.
 * A copy of the license can be found in the file COPYING.txt
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "mio.h"
#include "ttc.h"
#include "pwm.h"


void pwm_init(uint8_t ttc, uint8_t timer, enum pwm_clk clk, uint8_t clk_shr,
    bool invert, uint8_t mio)
{
	switch (ttc) {
	case 0:
		if (mio == 18 || mio == 30 || mio == 42)
			break;
		fprintf(stderr, "MIO must be 18, 30, or 42, not %u\n", mio);
		exit(1);
	case 1:
		if (mio == 16 || mio == 28 || mio == 40)
			break;
		fprintf(stderr, "MIO must be 16, 28, or 40, not %u\n", mio);
		exit(1);
	default:
		fprintf(stderr, "pwm_init: ttc must be 0 or 1, not %d\n", ttc);
		exit(1);
	}

	ttc_open();
	TTC_CLK_CTRL(ttc, timer) =
	    (clk == pwm_ext ? 1 << TTC_CLK_CTRL_EXT_CLK_SHIFT : 0) |
	    (clk_shr ? (clk_shr - 1) << TTC_CLK_CTRL_PRE_SHR_SHIFT : 0) |
	    (clk_shr ? 1 << TTC_CLK_CTRL_PRE_EN_SHIFT : 0);
	TTC_CNT_CTRL(ttc, timer) =
	    invert << TTC_CNT_CTRL_WAVE_HL_SHIFT |
	    1 << TTC_CNT_CTRL_MATCH_EN_SHIFT |
	    1 << TTC_CNT_CTRL_INTERVAL_SHIFT |
	    1 << TTC_CNT_CTRL_nEN_SHIFT;

	mio_open();
	MIO_PIN(mio) =
	    (MIO_PIN(mio) & ~(MIO_SEL_MASK << MIO_SEL_SHIFT)) |
	    MIO_SEL_WAVE << MIO_SEL_SHIFT;
}


void pwm_interval(uint8_t ttc, uint8_t timer, uint16_t intv)
{
	TTC_INTERVAL(ttc, timer) = intv;
}


void pwm_duty(uint8_t ttc, uint8_t timer, float duty)
{
	uint16_t interval = TTC_INTERVAL(ttc, timer);

	TTC_MATCH_1(ttc, timer) = duty * interval;
}


void pwm_start(uint8_t ttc, uint8_t timer)
{
	TTC_CNT_CTRL(ttc, timer) &= ~(1 << TTC_CNT_CTRL_nEN_SHIFT);
}
