/*
 * mio.c - Multipliexed IO pins (PS-side)
 *
 * Copyright (C) 2021 Linzhi Ltd.
 *
 * This work is licensed under the terms of the MIT License.
 * A copy of the license can be found in the file COPYING.txt
 */

#include <stddef.h>
#include <assert.h>

#include "regmap.h"
#include "mio.h"


volatile void *mio_base;

static unsigned ref = 0;
static struct regmap mio_map;


void mio_open(void)
{
	if (!ref)
		mio_base = regmap_open(&mio_map, MIO_BASE, MIO_SIZE);
	ref++;
}


void mio_close(void)
{
	assert(ref);
	if (--ref)
		return;
	regmap_close(&mio_map);
}
