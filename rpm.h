#ifndef RPM_H
#define	RPM_H

struct rpm_ctx {
	uint8_t ttc;
	uint8_t timer;
	uint16_t last_n;
	struct timeval last_t;
};


void rpm_init(struct rpm_ctx *ctx, uint8_t ttc, uint8_t timer, uint8_t mio);
double rpm_poll(struct rpm_ctx *ctx);

#endif /* !RPM_H */
