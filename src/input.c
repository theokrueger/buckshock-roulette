//! input.c
//!
//! Handles button input and offloads input checking onto second core
//! Includes helper functions for accessing button states and auto-managing mutex
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "pico/time.h"

#include "../include/input.h"
#include "../include/output.h"

bool allow_input = true;

void setup_input() {
	printf("Setting up input\n");
	// buttons
	gpio_init(TRIGGER_BTN);
	gpio_set_dir(TRIGGER_BTN, GPIO_IN);

	gpio_init(RACK_BTN);
	gpio_set_dir(RACK_BTN, GPIO_IN);

	gpio_init(PLAYER_DIRECTION_SWITCH);
	gpio_set_dir(PLAYER_DIRECTION_SWITCH, GPIO_IN);

	gpio_init(SUPERCHARGE_SWITCH);
	gpio_set_dir(SUPERCHARGE_SWITCH, GPIO_IN);

	gpio_init(TILT_SWITCH);
	gpio_set_dir(TILT_SWITCH, GPIO_IN);

	gpio_init(INCREASE_LIVES_BTN);
	gpio_set_dir(INCREASE_LIVES_BTN, GPIO_IN);

	gpio_init(INVERT_BTN);
	gpio_set_dir(INVERT_BTN, GPIO_IN);

}

bool get_trigger_state()
{
	return allow_input && gpio_get(TRIGGER_BTN);
}

bool get_rack_state()
{
	return allow_input && gpio_get(RACK_BTN);
}

bool get_tilt_state()
{
	int tot = 0;
	for(int i=0; i<10; ++i) {
		if (gpio_get(TILT_SWITCH)) ++tot;
		sleep_ms(1);
	}
	printf("tilt cnt: %i\n", tot);
	return tot > 4;
}


bool get_increase_lives_state()
{
	return allow_input && gpio_get(INCREASE_LIVES_BTN);
}

bool get_invert_state()
{
	return allow_input && gpio_get(INVERT_BTN);
}

bool is_player_2_direction()
{
	return gpio_get(PLAYER_DIRECTION_SWITCH);
}

bool get_supercharge_state()
{
	return gpio_get(SUPERCHARGE_SWITCH);
}

void input_disallow()
{
	allow_input = false;
	set_input_allowed_led(false);
	printf("Input disallowed!\n");
}

void input_allow()
{
	allow_input = true;
	set_input_allowed_led(true);
	printf("Input allowed!\n");
}

