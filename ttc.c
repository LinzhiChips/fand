/*
 * ttc.c - Zynq Triple Timer Counter
 *
 * Copyright (C) 2021 Linzhi Ltd.
 *
 * This work is licensed under the terms of the MIT License.
 * A copy of the license can be found in the file COPYING.txt
 */

#include <stddef.h>
#include <assert.h>

#include "regmap.h"
#include "ttc.h"


volatile void *ttc_base;

static unsigned ref = 0;
static struct regmap ttc_map;


void ttc_open(void)
{
	if (!ref)
		ttc_base = regmap_open(&ttc_map, TTC_BASE, TTC_SIZE);
	ref++;
}


void ttc_close(void)
{
	assert(ref);
	if (--ref)
		return;
	regmap_close(&ttc_map);
	ttc_base = NULL;
}
