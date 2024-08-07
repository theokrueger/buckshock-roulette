//! input.c
//!
//! Handles button input and offloads input checking onto second core
//! Includes helper functions for accessing button states and auto-managing mutex
#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "pico/sync.h"
#include "pico/multicore.h"
#include "pico/time.h"

#include "../include/input.h"

mutex_t trigger_btn_mutex;
bool trigger_btn_pressed;

mutex_t slide_btn_mutex;
bool slide_btn_pressed;

mutex_t rack_btn_mutex;
bool rack_btn_pressed;

mutex_t pause_input_loop;

/// Function passed to core1 by setup_input()
void check_input_loop()
{
	while(true) {
		bool delta = false;
		mutex_enter_blocking(&pause_input_loop);
		if (get_trigger_state()) {
			mutex_enter_blocking(&trigger_btn_mutex);
			trigger_btn_pressed = true;
			mutex_exit(&trigger_btn_mutex);
			delta = true;
		}

		if (get_slide_state()) {
			mutex_enter_blocking(&slide_btn_mutex);
			slide_btn_pressed = true;
			mutex_exit(&slide_btn_mutex);
			delta = true;
		}

		if (get_rack_state()) {
			mutex_enter_blocking(&rack_btn_mutex);
			rack_btn_pressed = true;
			mutex_exit(&rack_btn_mutex);
			delta = true;
		}
		mutex_exit(&pause_input_loop);
		if(delta) sleep_ms(INPUT_DEBOUNCE_MS);
		sleep_ms(15); // input is checked every ~15ms
	}
}

/// GPIO init for buttons and mutex
void setup_input() {
	// buttons
	gpio_init(TRIGGER_BTN);
	gpio_set_dir(TRIGGER_BTN, GPIO_IN);
	mutex_init(&trigger_btn_mutex);
	trigger_btn_pressed = false;

	gpio_init(SLIDE_BTN);
	gpio_set_dir(SLIDE_BTN, GPIO_IN);
	mutex_init(&slide_btn_mutex);
	slide_btn_pressed = false;

	gpio_init(RACK_BTN);
	gpio_set_dir(RACK_BTN, GPIO_IN);
	mutex_init(&rack_btn_mutex);
	rack_btn_pressed = false;

	// other
	mutex_init(&pause_input_loop);

	// send input loop to core1
	multicore_launch_core1(check_input_loop);
}


/// increment pause int
void pause_input() {
	mutex_enter_blocking(&pause_input_loop);
}

void resume_input() {
	mutex_exit(&pause_input_loop);
}

bool get_trigger_state_queued()
{
	mutex_enter_blocking(&trigger_btn_mutex);
	bool state = trigger_btn_pressed;
	trigger_btn_pressed = false;
	mutex_exit(&trigger_btn_mutex);
	return state;
}

bool get_trigger_state()
{
	return gpio_get(TRIGGER_BTN);
}

bool get_slide_state_queued()
{
	mutex_enter_blocking(&slide_btn_mutex);
	bool state = slide_btn_pressed;
	slide_btn_pressed = false;
	mutex_exit(&slide_btn_mutex);
	return state;
}

bool get_slide_state()
{
	return gpio_get(SLIDE_BTN);
}

bool get_rack_state_queued()
{
	mutex_enter_blocking(&rack_btn_mutex);
	bool state = rack_btn_pressed;
	rack_btn_pressed = false;
	mutex_exit(&rack_btn_mutex);
	return state;
}

bool get_rack_state()
{
	return gpio_get(RACK_BTN);
}

