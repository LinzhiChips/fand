#ifndef PWM_H
#define	PWM_H

#include <stdbool.h>
#include <stdint.h>


enum pwm_clk {
	pwm_ext,	/* external clock */
	pwm_cpu_1x,	/* cpu_1x clock (pclk) */
};


/*
 * @@@ We assume the PWM in question will output the wave on MIO.
 */

void pwm_init(uint8_t ttc, uint8_t timer, enum pwm_clk clk, uint8_t clk_shr,
    bool invert, uint8_t mio);
void pwm_interval(uint8_t ttc, uint8_t timer, uint16_t intv);
void pwm_duty(uint8_t ttc, uint8_t timer, float duty);
void pwm_start(uint8_t ttc, uint8_t timer);

#endif /* !PWM_H */
