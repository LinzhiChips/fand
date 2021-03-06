/*
 * pclk.c - Read the PCLK frequency
 *
 * Copyright (C) 2021 Linzhi Ltd.
 *
 * This work is licensed under the terms of the MIT License.
 * A copy of the license can be found in the file COPYING.txt
 */

#include <stdlib.h>
#include <stdio.h>
#include "pclk.h"


#define	PCLK_PATH	"/sys/kernel/debug/clk/cpu_1x/clk_rate"


unsigned long pclk_get(void)
{
	FILE *file;
	unsigned long hz;

	file = fopen(PCLK_PATH, "r");
	if (!file) {
		perror(PCLK_PATH);
		exit(1);
	}
	if (fscanf(file, "%lu", &hz) != 1) {
		fprintf(stderr, "cannot read PCLK (clock_1x) frequency\n");
		exit(1);
	}
	(void) fclose(file);
	return hz;
}
