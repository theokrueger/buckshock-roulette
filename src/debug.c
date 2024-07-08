//! debugging functions

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "pico/time.h"
#include "hardware/watchdog.h"
#include "../include/input.h"
#include "../include/output.h"


void debug_init()
{
#ifdef DEBUG_FUNCS
	sleep_ms(3000);
	if (watchdog_caused_reboot()) {
		printf("Rebooted by Watchdog!\n");
		return 0;
	} else {
		printf("Clean boot\n");
	}

#ifdef RUN_INPUT_TEST
	test_all_io();
#endif

#endif
}

/// await any inputs to proceed to a following thing
void await_any_input()
{
	while(1) {
		if (get_trigger_state_queued() || get_slide_state_queued() || get_rack_state_queued()) return;
		sleep_ms(50);
	}
}

/// test all currently implemented io functions
void test_all_io()
{
	printf("Begin testing io...\n");
	// inputs
	{
		printf("Testing inputs...\nPlease press all buttons\n");
		while(1) {
			bool tr, sl, rk = false;
			if (!tr && get_trigger_state_queued()) {
				printf("Trigger pressed!\n");
				tr = true;
			}
			if (!sl && get_slide_state_queued()) {
				printf("Slide pressed!\n");
				sl = true;
			}
			if (!rk && get_rack_state_queued()) {
				printf("Rack pressed!\n");
				rk = true;
			}
			if (tr && sl && rk) break;

			sleep_ms(50);
		}

		printf("Testing input pausing...\n");
		pause_input();
		// reset button conuts
		get_trigger_state_queued();
		get_slide_state_queued();
		get_rack_state_queued();
		printf("Please press many buttons in the next 3 seconds!\n");
		sleep_ms(3000);
		printf("Please do not press any buttons...\n");
		sleep_ms(500);
		resume_input();
		if (get_trigger_state_queued() || get_slide_state_queued() || get_rack_state_queued()) {
			printf("Pause input failed!\n");

		} else {
			printf("Pause input successful!\n");

		}
	}
	printf("Input tests successful\n");
	// outputs
	{
		printf("Testing outputs...\nPlease confirm that a given light is on by pressing any button.\n");
		sleep_ms(500);
		printf("Live round LED\n");
		last_round_shot(true);
		await_any_input();
		printf("Blank round LED\n");
		last_round_shot(false);
		await_any_input();
		turn_off_all_leds();
		for(int i=1; i<= LIVES_LED_COUNT; ++i) {
			printf("PLAYER 1 %i lives\n", i);
			p1_display_lives(i);
			await_any_input();

			printf("PLAYER 2 %i lives\n", i);
			p2_display_lives(i);
			await_any_input();
		}
		p1_display_lives(0);
		p2_display_lives(0);
		printf("Lives LEDS off\n");
		await_any_input();

		printf("Testing PWMs\n");
		printf("PLAYER 1 shock 5 seconds\n");
		p1_shock(5);
		await_any_input();
		printf("PLAYER 2 shock 5 seconds\n");
		p2_shock(5);
		await_any_input();
	}
	printf("Output tests successful!");
}
