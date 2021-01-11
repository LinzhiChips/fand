/*
 * regmap.h - Memory-map registers
 *
 * Copyright (C) 2021 Linzhi Ltd.
 *
 * This work is licensed under the terms of the MIT License.
 * A copy of the license can be found in the file COPYING.txt
 */

#ifndef REGMAP_H
#define	REGMAP_H

#include <sys/types.h>


struct regmap {
	int fd;
	volatile void *base;
	size_t size;
};


volatile void *regmap_open(struct regmap *regmap, off_t addr, size_t size);
void regmap_close(const struct regmap *regmap);

#endif /* !REGMAP_H */
