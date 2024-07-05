#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "pico/time.h"
#include "pico/sync.h"
#include "pico/multicore.h"
#include "pico/rand.h"
#include "hardware/pwm.h"

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
static mutex_t trigger_btn_mutex;
static bool trigger_btn_pressed = 0;
#define CHECK_BTN 7
static mutex_t check_btn_mutex;
static bool check_btn_pressed = 0;
#define EJECT_BTN 8
static mutex_t eject_btn_mutex;
static bool eject_btn_pressed= 0;

// consts
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

// set up game buttons/leds
void setup_game_io() {
	// led setup
	gpio_init(LIVE_LED);
	gpio_set_dir(LIVE_LED, GPIO_OUT);
	gpio_put(LIVE_LED, 0);

	gpio_init(BLANK_LED);
	gpio_set_dir(BLANK_LED, GPIO_OUT);
	gpio_put(BLANK_LED, 0);

	// button setup
	gpio_init(TRIGGER_BTN);
	gpio_set_dir(TRIGGER_BTN, GPIO_IN);
	mutex_init(&trigger_btn_mutex);

	gpio_init(CHECK_BTN);
	gpio_set_dir(CHECK_BTN, GPIO_IN);
	mutex_init(&check_btn_mutex);

	gpio_init(EJECT_BTN);
	gpio_set_dir(EJECT_BTN, GPIO_IN);
	mutex_init(&eject_btn_mutex);
}

// function called  by second core to allow queued inputs
void input_loop() {
	while(true) {
		if (gpio_get(TRIGGER_BTN)) {
			mutex_enter_blocking(&trigger_btn_mutex);
			trigger_btn_pressed = true;
			mutex_exit(&trigger_btn_mutex);
		}
	        if (gpio_get(CHECK_BTN)) {
			mutex_enter_blocking(&check_btn_mutex);
			check_btn_pressed = true;
			mutex_exit(&check_btn_mutex);
		}
		if (gpio_get(EJECT_BTN)) {
			mutex_enter_blocking(&eject_btn_mutex);
			eject_btn_pressed= true;
			mutex_exit(&eject_btn_mutex);
		}
		sleep_ms(10);
	}
}
// main loop
int main() {
	stdio_init_all();
	setup_shock();
	setup_game_io();
	multicore_launch_core1(input_loop);
	sleep_ms(3000); // test
	// full game loop
	while(true) {
		printf("starting a new game\n");
		// reset button states
		mutex_enter_blocking(&trigger_btn_mutex);
		trigger_btn_pressed = false;
		mutex_exit(&trigger_btn_mutex);

		mutex_enter_blocking(&check_btn_mutex);
		check_btn_pressed = false;
		mutex_exit(&check_btn_mutex);

		mutex_enter_blocking(&eject_btn_mutex);
		eject_btn_pressed= false;
		mutex_exit(&eject_btn_mutex);

		// game vars
		uint player_turn = 1; // 1 for p1, 2 for p2

		uint chamber_size = (get_rand_32() % 7) + 2; // random chamber size 2-8
		uint lives = chamber_size / 2;
		uint blanks = chamber_size - lives;
		// 50% chance to randomly plus or minus lives or blanks
		if (get_rand_32() % 2 == 0) {
			if (get_rand_32() % 2 == 0 && blanks > 1) {
				++lives;
				--blanks;
			} else if (lives > 1) {
				--lives;
				++blanks;
			}
		}
		bool chamber[chamber_size]; // true for live, false for blank
		uint rounds_ejected = 0;
		// load the chamber
		{
			uint lives_left = lives;
			uint blanks_left = blanks;
			for (int i=0; i < chamber_size; ++i) {
				if (get_rand_32() % (lives_left + blanks_left) + 1 > blanks_left) {
					--lives_left;
					chamber[i] = true;
				} else {
					--blanks_left;
					chamber[i] = false;
				}
			}
			printf("chamber is loaded!\n[");
			for (int i=0; i<chamber_size; ++i) {
				printf("%c", chamber[i] ? 'X' : 'O');
			}
			printf("]\n");
		}

		// await input
		while(rounds_ejected < chamber_size) {
			// trigger pulled
			mutex_enter_blocking(&trigger_btn_mutex);
			if (trigger_btn_pressed) {
				printf("trigger pulled\n");
				trigger_btn_pressed = false;
				mutex_exit(&trigger_btn_mutex);
				if (chamber[rounds_ejected]) {
					printf("round was live!\n");
					shock_p1();
				} else {
					printf("round was blank!\n");
				}
				++rounds_ejected;
			} else {
				mutex_exit(&trigger_btn_mutex);
			}
			sleep_ms(100);
		}

		// proceed to next round
	}
}
