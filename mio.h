/*
 * mio.h - Multipliexed IO pins (PS-side)
 *
 * Copyright (C) 2021 Linzhi Ltd.
 *
 * This work is licensed under the terms of the MIT License.
 * A copy of the license can be found in the file COPYING.txt
 */

#ifndef MIO_H
#define	MIO_H

#define	MIO_BASE	0xF8000700
#define	MIO_SIZE	0xD4

enum io_type {
	LVCMOS18	= 1,
	LVCMOS25	= 2,
	LVCMOS33	= 3,
	HSTL		= 4,
};

enum mio_sel {
	MIO_SEL_GPIO	= 0,
	MIO_SEL_WAVE	= 6 << 4,
	MIO_SEL_TTC_CLK	= 6 << 4,
};

#define	MIO_PIN(n)	(*(volatile uint32_t *) (mio_base + (n) * 4))

#define	MIO_DIS_HSTL_RCVR_SHIFT		13
#define	MIO_DIS_HSTLRCVR_MASK		1
#define	MIO_PULLUP_SHIFT		12
#define	MIO_PULLUP_MASK			1
#define	MIO_IO_TYPE_SHIFT		9	/* see enum io_type */
#define	MIO_IO_TYPE_MASK		7
#define	MIO_FAST_SHIFT			8
#define	MIO_FAST_MASK			1
#define	MIO_SEL_SHIFT			1
#define	MIO_SEL_MASK			0x3f
#define	MIO_TRI_EN_SHIFT		0
#define	MIO_TRI_EN_MASK			1


volatile void *mio_base;


void mio_open(void);
void mio_close(void);

#endif /* !MIO_H */
