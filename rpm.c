#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

#include "mio.h"
#include "ttc.h"
#include "rpm.h"


/*
 * Fan tacho signal has four edges per revolution, so the tacho signal
 * frequency is twice the rotation speed.
 */

#define	CYCLES_PER_REVOLUTION	2.0


void rpm_init(struct rpm_ctx *ctx, uint8_t ttc, uint8_t timer, uint8_t mio)
{
	switch (ttc) {
	case 0:
		if (!mio || mio == 19 || mio == 31 || mio == 43)
			break; 
		fprintf(stderr,
		    "MIO must be 0 (EMIO), 19, 31, or 43, not %u\n", mio);
		exit(1);
	case 1:
		if (!mio || mio == 17 || mio == 29 || mio == 41)
			break;
		fprintf(stderr,
		    "MIO must be 0 (EMIO), 17, 29, or 41, not %u\n", mio);
		exit(1);
	default:
		fprintf(stderr, "rpm_init: ttc must be 0 or 1, not %u\n", ttc);
		exit(1);
	}

	if (timer && mio) {
		fprintf(stderr, "for MIO, timer must be 0, not %u\n", timer);
		exit(1);
	}

	ttc_open();
	TTC_CLK_CTRL(ttc, timer) = 1 << TTC_CLK_CTRL_EXT_CLK_SHIFT;
	TTC_CNT_CTRL(ttc, timer) = 0;

	if (mio) {
		mio_open();
		MIO_PIN(mio) =
		    (MIO_PIN(mio) & ~(MIO_SEL_MASK << MIO_SEL_SHIFT)) |
		    MIO_SEL_TTC_CLK << MIO_SEL_SHIFT |
		    1 << MIO_TRI_EN_SHIFT;
	}

	ctx->ttc = ttc;
	ctx->timer = timer;
	gettimeofday(&ctx->last_t, NULL);
	ctx->last_n = TTC_COUNTER(ttc, timer);
}


double rpm_poll(struct rpm_ctx *ctx)
{
	struct timeval t;
	double dt, rpm;
	uint16_t n;

	if (gettimeofday(&t, NULL) < 0) {
		perror("gettimeofday");
		exit(1);
	}
	n = TTC_COUNTER(ctx->ttc, ctx->timer);
	dt = t.tv_sec + t.tv_usec * 1e-6 -
	    ctx->last_t.tv_sec - ctx->last_t.tv_usec * 1e-6;
	if (dt <= 0) {
		fprintf(stderr, "time stood still or went backwards ?\n");
		return 0;
	}
	rpm = (uint16_t) (n - ctx->last_n) * 60 / dt / CYCLES_PER_REVOLUTION;
	ctx->last_t = t;
	ctx->last_n = n;
	return rpm;
}
