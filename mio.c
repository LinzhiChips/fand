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
