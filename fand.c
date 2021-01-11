/*
 * fand.c - Daemon that controls fans and monitors their performance
 *
 * Copyright (C) 2021 Linzhi Ltd.
 *
 * This work is licensed under the terms of the MIT License.
 * A copy of the license can be found in the file COPYING.txt
 */

#define _GNU_SOURCE	/* for asprintf */
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

#include <mosquitto.h>

#include "pclk.h"
#include "pwm.h"
#include "rpm.h"


/*
 * Per the "4-Wire Pulse Width Modulation (PWM) Controlled Fans"
 * specification [1], the minimum PWM duty cycle for the fan to start and
 * keep running is vendor-defined, but must not be higher than 30%.
 *
 * We play it safe treat any non-zero PWM value < 30% as 30%. To remove this
 * limit, use pwm_duty_raw instead of pwm_duty.
 *
 * [1] https://www.glkinst.com/cables/cable_pics/4_Wire_PWM_Spec.pdf
 */

#define	FAN_PWM_HZ		25000

/*
 * Per the same specification, the minimum PWM duty cycle for the fan to start
 * and keep running is vendor-defined, but must not be higher than 30%.
 *
 * We play it safe treat any non-zero PWM value < 30% as 30%. To remove this
 * limit, use pwm_duty_raw instead of pwm_duty.
 */

#define	FAN_MIN_DUTY		30

#define	DEFAULT_POLL_INTERVAL_S	1


#define	CONSUMER		"fand"

#define	MQTT_HOST		"localhost"
#define	MQTT_PORT		1883

#define	MQTT_TOPIC_SHUTDOWN	"/sys/shutdown"
#define	MQTT_TOPIC_BASE		"/fan"

/* generations 0 and 1 */

#define	MQTT_TOPIC_L		MQTT_TOPIC_BASE "/left"
#define	MQTT_TOPIC_R		MQTT_TOPIC_BASE "/right"
#define	MQTT_TOPIC_L_PWM_SET	MQTT_TOPIC_L "/pwm-set"
#define	MQTT_TOPIC_R_PWM_SET	MQTT_TOPIC_R "/pwm-set"
#define	MQTT_TOPIC_L_PWM	MQTT_TOPIC_L "/pwm"
#define	MQTT_TOPIC_R_PWM	MQTT_TOPIC_R "/pwm"
#define	MQTT_TOPIC_L_RPM	MQTT_TOPIC_L "/rpm"
#define	MQTT_TOPIC_R_RPM	MQTT_TOPIC_R "/rpm"
#define	MQTT_TOPIC_L_PWM_MIN	MQTT_TOPIC_L "/pwm-min"
#define	MQTT_TOPIC_R_PWM_MIN	MQTT_TOPIC_R "/pwm-min"

/* generation 2 */

#define	MQTT_TOPIC_F_PWM_SET	"/fan/front/pwm-set"
#define	MQTT_TOPIC_RE_PWM_SET	"/fan/rear/pwm-set"
#define	MQTT_TOPIC_F_PWM	"/fan/front/pwm"
#define	MQTT_TOPIC_RE_PWM	"/fan/rear/pwm"
#define	MQTT_TOPIC_F_PWM_MIN	"/fan/front/pwm-min"
#define	MQTT_TOPIC_RE_PWM_MIN	"/fan/rear/pwm-min"
#define	MQTT_TOPIC_F_1_RPM	"/fan/front/1/rpm"
#define	MQTT_TOPIC_F_2_RPM	"/fan/front/2/rpm"
#define	MQTT_TOPIC_R_1_RPM	"/fan/rear/1/rpm"
#define	MQTT_TOPIC_R_2_RPM	"/fan/rear/2/rpm"


enum mqtt_qos {
	qos_be		= 0,
	qos_ack		= 1,
	qos_once	= 2
};


/*
 * Libmosquitto documentation:
 * https://mosquitto.org/api/files/mosquitto-h.htm
 */


struct ctx_rpm {
	struct mosquitto *mosq;
	const char *topic;
};


static bool shutting_down = 0;
static bool verbose = 0;
static bool force = 0;
static unsigned generation = 1;


static void update_pwm(struct mosquitto *mosq, const char *topic, uint8_t duty)
{
	char *s;
	int res;

	if (asprintf(&s, "%u", duty) < 0) {
		perror("asprintf");
		return;
	}
	res = mosquitto_publish(mosq, NULL, topic, strlen(s), s, qos_ack, 1);
	free(s);
        if (res == MOSQ_ERR_SUCCESS)
                return;
        fprintf(stderr, "mosquitto_publish: %d\n", res);
        exit(1);
}


static void update_rpm(struct mosquitto *mosq, const char *topic, double rpm)
{
	char *s;
	int res;

	if (asprintf(&s, "%u", (unsigned) rpm) < 0) {
		perror("asprintf");
		return;
	}
	res = mosquitto_publish(mosq, NULL, topic, strlen(s), s, qos_ack, 1);
	free(s);
        if (res == MOSQ_ERR_SUCCESS)
                return;
        fprintf(stderr, "mosquitto_publish: %d\n", res);
        exit(1);
}


static void set_pwm(struct mosquitto *mosq, bool right, uint8_t duty)
{
	const char *topic;

	if (duty && duty < FAN_MIN_DUTY && !force)
		duty = FAN_MIN_DUTY;
	pwm_duty(right, 0, duty / 100.0);
	if (!mosq)
		return;
	switch (generation) {
	case 0:
	case 1:
		topic = right ? MQTT_TOPIC_R_PWM : MQTT_TOPIC_L_PWM;
		break;
	case 2:
		topic = right ? MQTT_TOPIC_RE_PWM : MQTT_TOPIC_F_PWM;
		break;
	default:
		abort();
	}
	update_pwm(mosq, topic, duty);
}


static void init_pwm(struct mosquitto *mosq, bool invert, bool right,
    uint8_t duty)
{
	static unsigned long pclk = 0;

	if (!pclk) {
		pclk = pclk_get();
		if (verbose)
			fprintf(stderr, "pclk is %.6f MHz\n", pclk / 1e6);
	}
	pwm_init(right, 0, pwm_cpu_1x, 0, invert, right ? 28 : 30);
	pwm_interval(right, 0, pclk / FAN_PWM_HZ);
	set_pwm(mosq, right, duty);
	pwm_start(right, 0);
}


#define	MAX_MSG	10	/* PWM range is 0-100, this is plenty */


static void parse_pwm(struct mosquitto *mosq, bool right,
    const char *msg, int len)
{
	if (shutting_down)
		return;
	if (len < 0 || len > MAX_MSG) {
		fprintf(stderr, "invalid message length: %d\n", len);
		return;
	}

	char buf[len + 1];
	char *end;
	unsigned long n = 0;

	if (len) {
		memcpy(buf, msg, len);
		buf[len] = 0;

		n = strtoul(buf, &end, 0);
		if (*end || n > 100) {
			fprintf(stderr, "bad PWM duty: \"%s\"\n", buf);
			return;
		}
	}

	set_pwm(mosq, right, n);
}


static void cb(struct mosquitto *mosq, void *obj,
    const struct mosquitto_message *msg)
{
	(void) obj;

	if (!strcmp(msg->topic, MQTT_TOPIC_SHUTDOWN)) {
		if (msg->payloadlen && *(const char *) msg->payload == '0') {
			shutting_down = 0;
		} else {
			shutting_down = 1;
			set_pwm(mosq, 0, 100);
		}
	} else if (!strcmp(msg->topic, MQTT_TOPIC_L_PWM_SET)) {
		/* "R" (rear) in LC001.01, "L" (left) in LC001.02 */
		parse_pwm(mosq, 0, msg->payload, msg->payloadlen);
	} else if (!strcmp(msg->topic, MQTT_TOPIC_R_PWM_SET)) {
		/* "F" (front) in LC001.01, "R" (right) in LC001.02 */
		parse_pwm(mosq, 1, msg->payload, msg->payloadlen);
	} else if (!strcmp(msg->topic, MQTT_TOPIC_F_PWM_SET)) {
		/* front in LC001.05 */
		parse_pwm(mosq, 0, msg->payload, msg->payloadlen);
	} else if (!strcmp(msg->topic, MQTT_TOPIC_RE_PWM_SET)) {
		/* rear in LC001.05 */
		parse_pwm(mosq, 1, msg->payload, msg->payloadlen);
	} else {
		fprintf(stderr, "unrecognized topic \"%s\"\n", msg->topic);
	}
}


static struct mosquitto *setup_mqtt(void)
{
	struct mosquitto *mosq;
	int res;

	mosquitto_lib_init();
	mosq = mosquitto_new(NULL, 1, NULL);
	if (!mosq) {
		fprintf(stderr, "mosquitto_new failed\n");
		exit(1);
	}
	if (mosquitto_connect(mosq, MQTT_HOST, MQTT_PORT, 3600)) {
		fprintf(stderr, "unable to connect\n");
		exit(1);
	}
	res = mosquitto_subscribe(mosq, NULL, MQTT_TOPIC_SHUTDOWN, qos_ack);
	if (res) {
		fprintf(stderr, "mosquitto_subscribe: %d\n", res);
		exit(1);
	}
	res = mosquitto_subscribe(mosq, NULL, MQTT_TOPIC_L_PWM_SET, qos_ack);
	if (res) {
		fprintf(stderr, "mosquitto_subscribe: %d\n", res);
		exit(1);
	}
	res = mosquitto_subscribe(mosq, NULL, MQTT_TOPIC_R_PWM_SET, qos_ack);
	if (res) {
		fprintf(stderr, "mosquitto_subscribe: %d\n", res);
		exit(1);
	}
	res = mosquitto_subscribe(mosq, NULL, MQTT_TOPIC_F_PWM_SET, qos_ack);
	if (res) {
		fprintf(stderr, "mosquitto_subscribe: %d\n", res);
		exit(1);
	}
	res = mosquitto_subscribe(mosq, NULL, MQTT_TOPIC_RE_PWM_SET, qos_ack);
	if (res) {
		fprintf(stderr, "mosquitto_subscribe: %d\n", res);
		exit(1);
	}
	mosquitto_message_callback_set(mosq, cb);
	return mosq;
}


static void mqtt_start(struct mosquitto *mosq)
{
	int res;

	/*
	 * @@@ quick and dirty. We could also use mosquitto_socket to get the
	 * fd, then select/poll, and call mosquitto_loop_{read,write,misc}
	 * accordingly. However, this would also burden us with reconnect
	 * handling and such.
	 */
	res = mosquitto_loop_start(mosq);
	if (res < 0) {
		fprintf(stderr, "mosquitto_loop_start: %d\n", res);
		exit(1);
	}
}


static void manual(const char *arg, bool invert)
{
	char *end;
	unsigned long n;

	n = strtoul(arg, &end, 0);
	if (*end || n > 100) {
		fprintf(stderr,
		    "PWM duty cycle must be 0 <= n <= 1, not \"%s\"\n", arg);
		exit(1);
	}
	init_pwm(NULL, invert, 0, n);
	pwm_start(0, 0);
}


static void daemonize(void)
{
	pid_t pid;

	pid = fork();
	if (pid < 0) {
		perror("fork");
		exit(1);
	}
	if (pid)
		exit(0);
	if (setsid() < 0) {
		perror("setsid");
		exit(1);
	}
	(void) close(0);
	(void) close(1);
	if (!verbose)
		(void) close(2);
}


static void set_generation(void)
{
	const char *gen = getenv("BOARD_GENERATION");

	if (!gen) {
		fprintf(stderr, "BOARD_GENERATION is not set\n");
		exit(1);
	}
	if (!strcmp(gen, "2"))
		generation = 2;
}


static void usage(const char *name)
{
	fprintf(stderr,
"usage: %s [-b] [-f] [-g 0|1|2] [-i] [-t seconds] [-v] [duty]\n\n"
"  -b  fork and run in the background after initializing\n"
"  -f  (force) allow also duty cycles < 30%%\n"
"  -g  LC001 generation: 0 = .01, 1 = .02 to .04, 2 = .05 (default: 1)\n"
"  -i  invert waveform polarity\n"
"  -t seconds\n"
"      tacho poll interval (default: %g s)\n"
"  -v  verbose operation\n\n"
"  duty  set fan 0 PWM (fan(s) affected depends on the board revision) to\n"
"        the specified duty cycle (an integer, 0 <= duty <= 100).\n"
    , name, (double) DEFAULT_POLL_INTERVAL_S);
	exit(1);
}


int main(int argc, char *argv[])
{
	struct mosquitto *mosq;
	struct rpm_ctx rpm_right, rpm_left;
	struct rpm_ctx rpm_front_1, rpm_front_2;
	struct rpm_ctx rpm_rear_1, rpm_rear_2;
	char *end;
	bool bg = 0;
	bool invert = 0;
	double s;
	useconds_t us = DEFAULT_POLL_INTERVAL_S * 1e6;
	int c;

	set_generation();
	while ((c = getopt(argc, argv, "bfg:it:v")) != EOF)
		switch (c) {
		case 'b':
			bg = 1;
			break;
		case 'f':
			force = 1;
			break;
		case 'i':
			invert = 1;
			break;
		case 'g':
			generation = strtoul(optarg, &end, 0);
			if (*end || generation > 2)
				usage(*argv);
			break;
		case 't':
			s = strtof(optarg, &end);
			us = s * 1e6;
			if (*end || s <= 0 || !us) {
				fprintf(stderr, "invalid duration: \"%s\"\n",
				    optarg);
				exit(1);
			}
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			usage(*argv);
		}
	switch (argc - optind) {
	case 0:
		break;
	case 1:
		manual(argv[optind], invert);
		return 0;
	default:
		usage(*argv);
	}

	mosq = setup_mqtt();
	init_pwm(mosq, invert, 0, 100);
	init_pwm(mosq, invert, 1, 100);

	switch (generation) {
	case 0:
	case 1:
		rpm_init(&rpm_right, 0, 1, 0);
		rpm_init(&rpm_left, 1, 1, 0);
		break;
	case 2:
		rpm_init(&rpm_front_1, 0, 1, 0);
		rpm_init(&rpm_front_2, 1, 1, 0);
		rpm_init(&rpm_rear_1, 0, 2, 0);
		rpm_init(&rpm_rear_2, 1, 2, 0);
		break;
	default:
		abort();
	}

	if (bg)
		daemonize();

	mqtt_start(mosq);

	switch (generation) {
	case 0:
	case 1:
		update_pwm(mosq, MQTT_TOPIC_L_PWM_MIN, FAN_MIN_DUTY);
		update_pwm(mosq, MQTT_TOPIC_R_PWM_MIN, FAN_MIN_DUTY);

		while (1) {
			usleep(us);
			update_rpm(mosq, MQTT_TOPIC_R_RPM, rpm_poll(&rpm_right));
			update_rpm(mosq, MQTT_TOPIC_L_RPM, rpm_poll(&rpm_left));
		}
		break;
	case 2:
		update_pwm(mosq, MQTT_TOPIC_F_PWM_MIN, FAN_MIN_DUTY);
		update_pwm(mosq, MQTT_TOPIC_RE_PWM_MIN, FAN_MIN_DUTY);

		while (1) {
			usleep(us);
			update_rpm(mosq, MQTT_TOPIC_F_1_RPM,
			    rpm_poll(&rpm_front_1));
			update_rpm(mosq, MQTT_TOPIC_F_2_RPM,
			    rpm_poll(&rpm_front_2));
			update_rpm(mosq, MQTT_TOPIC_R_1_RPM,
			    rpm_poll(&rpm_rear_1));
			update_rpm(mosq, MQTT_TOPIC_R_2_RPM,
			    rpm_poll(&rpm_rear_2));
		}
		break;
	default:
		abort();
	}

	return 0;
}
