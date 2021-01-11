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
