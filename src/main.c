#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "pico/time.h"


// pins
#define P1_SHOCK 15
#define P1_SHOCK_LED   14
static uint p1_slice = 0;

#define P2_SHOCK 16
#define P2_SHOCK_LED   17
static uint p2_slice = 0;

#define LIVE_LED  2
#define BLANK_LED 3

#define TRIGGER_BTN 6
#define PUMP_BTN 7
#define SWAP_BTN 8

#define SHOCK_TIME 1000 // shock time in ms

// set up shock collars
// red the red dog collars recieve a 10.65khz AM radio signal, 15 ms off 20 ms on
// sets p1/p2 slices accordingly
void setup_shock() {
	// pwm setup
	gpio_set_function(P1_SHOCK, GPIO_FUNC_PWM);
	p1_slice = pwm_gpio_to_slice_num(P1_SHOCK);
	pwm_set_wrap(p1_slice, 11737); // 125MHz clock / 11737 wrap ~= 10.65Khz wrap
	pwm_set_chan_level(p1_slice, PWM_CHAN_B, 5869); // 50% on time

	gpio_set_function(P2_SHOCK, GPIO_FUNC_PWM);
	p2_slice = pwm_gpio_to_slice_num(P2_SHOCK);
	pwm_set_wrap(p2_slice, 11737); // 125MHz clock / 11737 wrap ~= 10.65Khz wrap
	pwm_set_chan_level(p2_slice, PWM_CHAN_A, 5869); // 50% on time

	// shock led setup
	gpio_init(P1_SHOCK_LED);
	gpio_set_dir(P1_SHOCK_LED, GPIO_OUT);
	gpio_put(P1_SHOCK_LED, 0);

	gpio_init(P2_SHOCK_LED);
	gpio_set_dir(P2_SHOCK_LED, GPIO_OUT);
	gpio_put(P2_SHOCK_LED, 0);
}

// shock player 1
void shock_p1() {
	gpio_put(P1_SHOCK_LED, 1);
	for (int i=0; i<(SHOCK_TIME/35); ++i) {
		pwm_set_enabled(p1_slice, true);
		sleep_ms(20);

		pwm_set_enabled(p1_slice, false);
		sleep_ms(15);
	}
	gpio_put(P1_SHOCK_LED, 0);
}

// shock player 2
void shock_p2() {
	gpio_put(P2_SHOCK_LED, 1);
	for (int i=0; i<(SHOCK_TIME/35); ++i) {
		pwm_set_enabled(p2_slice, true);
		sleep_ms(20);

		pwm_set_enabled(p2_slice, false);
		sleep_ms(15);
	}
	gpio_put(P2_SHOCK_LED, 0);
}

// main
int main() {
	setup_shock();
	while(true) {
		shock_p1();
		sleep_ms(1000);
		shock_p2();
		sleep_ms(1000);
	}
}
