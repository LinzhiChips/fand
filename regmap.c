/*
 * regmap.c - Memory-map registers
 *
 * Copyright (C) 2021 Linzhi Ltd.
 *
 * This work is licensed under the terms of the MIT License.
 * A copy of the license can be found in the file COPYING.txt
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

#include "regmap.h"


static size_t round_down_to_page(size_t bytes)
{
	int page = getpagesize();

	return bytes - (bytes % page);
}


static size_t round_up_to_page(size_t bytes)
{
	int page = getpagesize();

	return round_down_to_page(bytes + page - 1);
}


volatile void *regmap_open(struct regmap *regmap, off_t addr, size_t size)
{
	off_t start;

	regmap->fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (regmap->fd < 0) {
		perror("/dev/mem");
		exit(1);
	}
	start = round_down_to_page(addr);
	regmap->size = round_up_to_page(size + addr - start);
	regmap->base = mmap(NULL, regmap->size, PROT_READ | PROT_WRITE,
	    MAP_SHARED, regmap->fd, start);
	if (regmap->base == MAP_FAILED) {
		perror("mmap");
		exit(1);
	}
#if 0
fprintf(stderr, "0x%lx+0x%lx: 0x%lx+0x%lx, %p->%p\n",
    (unsigned long) addr, (unsigned long) size,
    (unsigned long) start, (unsigned long) regmap->size,
    regmap->base, regmap->base + addr - start);
#endif
	return regmap->base + addr - start;
}


void regmap_close(const struct regmap *regmap)
{
	if (munmap((void *) regmap->base, regmap->size) < 0) {
		perror("munmap");
		exit(1);
	}
	if (close(regmap->fd) < 0) {
		perror("close");
		exit(1);
	}
}
