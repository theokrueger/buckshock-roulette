//! debugging functions

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "pico/time.h"
#include "hardware/watchdog.h"

#include "../include/input.h"
#include "../include/output.h"
#include "../include/debug.h"
#include "../include/util.h"


void debug_init()
{
	if (!get_trigger_state()) return;

	// run debug stuff only when trigger is help
	sleep_ms(3000);
	if (watchdog_caused_reboot()) {
		printf("Rebooted by Watchdog!\n");
	} else {
		printf("Clean boot\n");
	}
	test_all_io();
}

/// test all currently implemented io functions
void test_all_io()
{
	printf("Begin testing io...\n");
	input_allow();
	// inputs
	printf("Testing inputs...\nPlease press all gun buttons\n");
	bool trigger = 0, rack = 0, invert = 0, lives = 0;
	while(1) {
		if (get_trigger_state()) {
			printf("Trigger pressed!\n");
			trigger = true;
		}
		if (get_rack_state()) {
			printf("Rack pressed!\n");
			rack = true;
		}
		if(get_increase_lives_state()) {
			printf("Increase Lives pressed!\n");
			lives = true;
		}
		if(get_invert_state()) {
			printf("Invert pressed!\n");
			invert = true;
		}
		if (trigger && rack && invert && lives) break;

		sleep_ms(100);
	}

	printf("Please switch OFF player direction (p1 turn)\n");
	while(is_player_2_direction()) sleep_ms(50);
	printf("Success!\n");

	printf("Please switch ON player direction (p2 turn)\n");
	while(!is_player_2_direction()) sleep_ms(50);
	printf("Success!\n");

	printf("Please switch OFF supercharge\n");
	while(get_supercharge_state()) sleep_ms(50);
	printf("Success!\n");

	printf("Please switch ON supercharge\n");
	while(!get_supercharge_state()) sleep_ms(50);
	printf("Success!\n");

	printf("Please point gun at opponent\n");
	while(get_tilt_state()) sleep_ms(50);
	printf("Success!\n");

	printf("Please point gun at yourself\n");
	while(!get_tilt_state()) sleep_ms(50);
	printf("Success!\n");

	printf("Testing input pausing...\n");
	input_disallow();
	printf("Please press many buttons in the next 5 seconds!\n");
	bool fail = false;
	for(int i=0; i< 50; ++i) {
		if(get_trigger_state()) fail = true;
		sleep_ms(100);
	}
	printf("Please do not press any buttons...\n");
	sleep_ms(1000);
	if (fail) {
		printf("Disallow input failed!\n");
	} else {
		printf("Disallow input successful!\n");
	}
	input_allow();
	printf("Input tests successful\n");

        // outputs
	printf("Testing outputs...\nPlease confirm that a given light is on by pressing any button.\n");
	sleep_ms(1000);

	printf("Live last round LED\n");
	last_round_shot(true);
	await_any_input();

	printf("Blank last round LED\n");
	last_round_shot(false);
	await_any_input();
	turn_off_all_leds();

	printf("Chamber Live LED\n");
        open_chamber(true);
	await_any_input();

	printf("Chamber Blank LED\n");
	open_chamber(false);
	await_any_input();
	turn_off_all_leds();

	printf("Input allowed LED\n");
	set_input_allowed_led(true);
	await_any_input();
	turn_off_all_leds();


	printf("Loaded LED\n");
	set_loaded_led(true);
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
	for(int i=1; i<=2; ++i) {
		printf("PLAYER 1 %i wins\n", i);
		p1_set_wins(i);
		await_any_input();

		printf("PLAYER 2 %i wins\n", i);
		p2_set_wins(i);
		await_any_input();
	}
	p1_display_lives(0);
	p2_display_lives(0);
	p1_set_wins(0);
	p2_set_wins(0);

	printf("Lives LEDS off\n");
	await_any_input();

	printf("Testing PWMs\n");
	printf("PLAYER 1 shock\n");
	p1_shock(5);
	await_any_input();
	printf("PLAYER 2 shock\n");
	p2_shock(5);
	await_any_input();
	printf("Output tests successful!\n");
}
