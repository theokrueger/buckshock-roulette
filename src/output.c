//! output.c
//!
//! Handles LED and shock collar outputs
//! Functions for displaying lives and such

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "pico/time.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"

#include "../include/output.h"
#include "../include/input.h"
#include "../include/mcp23008.h"
#include "../include/util.h"
#include "../include/ssd1306.h"

uint p1_shock_slice;
uint p2_shock_slice;

struct MCP23008 p1_live_leds_mcp =  {
	.address = P1_LIVES_ADDRESS,
	.sda_pin = P1_LIVES_SDA,
	.scl_pin = P1_LIVES_SCL,
	.i2c_instance = P1_LIVES_I2C_INSTANCE,
	.state = 0b00000000,
};

struct MCP23008 p2_live_leds_mcp =  {
	.address = P2_LIVES_ADDRESS,
	.sda_pin = P2_LIVES_SDA,
	.scl_pin = P2_LIVES_SCL,
	.i2c_instance = P2_LIVES_I2C_INSTANCE,
	.state = 0b00000000,
};

ssd1306_t info_screen;

void set_input_allowed_led(bool state)
{
	gpio_put(INPUT_ALLOWED_LED, state);
}

void shock(uint slice) {
	pwm_set_enabled(slice, true);
	sleep_ms(SHOCK_FREQ_ON_TIME_MS);
	pwm_set_enabled(slice, false);
	sleep_ms(SHOCK_FREQ_OFF_TIME_MS);
}

uint loops_to_shock = SHOCK_DURATION_MS / (SHOCK_FREQ_ON_TIME_MS + SHOCK_FREQ_OFF_TIME_MS);


/// Shock player 2 for SHOCK_DURATION * mutliplier arg
void p2_shock(uint multiplier)
{
	input_disallow();
	printf("Shocking PLAYER 2 for %u times %u ms\n", multiplier, SHOCK_DURATION_MS);
	gpio_put(P2_SHOCK_LED, 1);
	for (uint i=0; i < loops_to_shock * multiplier; ++i) {
		shock(p2_shock_slice);
	}
	gpio_put(P2_SHOCK_LED, 0);
	input_allow();
}

void p1_shock(uint multiplier)
{
	input_disallow();
	printf("Shocking PLAYER 1 for %u times %u ms\n", multiplier, SHOCK_DURATION_MS);
	gpio_put(P1_SHOCK_LED, 1);
	for (uint i=0; i < loops_to_shock * multiplier; ++i) {
		shock(p1_shock_slice);
	}
	gpio_put(P1_SHOCK_LED, 0);
	input_allow();
}

/// Convert integer into char representing bit states of mcp23008 module
char lives_to_bin(uint lives)
{
	char state = 0;
	for(int i=0; i<lives; ++i) {
		state |= (0b1 << i);
	}
	return state << 0;
}

/// Convert integer into char representing bit states of mcp23008 module
char wins_to_bin(uint wins)
{
	char state = 0;
	for(int i=0; i<wins; ++i) {
		state |= (0b1 << i);
	}
	return state << 6;
}

/// Display this number of lives left for player 1
void p1_display_lives(uint lives_left)
{
	printf("%u Lives left for PLAYER 1\n", lives_left);
	p1_live_leds_mcp.state &= 0b11000000;
	p1_live_leds_mcp.state = lives_to_bin(lives_left);
	update_mcp23008_state(&p1_live_leds_mcp);
}

/// Display this number of lives left for player 2
void p2_display_lives(uint lives_left)
{
	printf("%u Lives left for PLAYER 2\n", lives_left);
	p2_live_leds_mcp.state &= 0b11000000;
	p2_live_leds_mcp.state = lives_to_bin(lives_left);
	update_mcp23008_state(&p2_live_leds_mcp);
}

/// Display this number of lives left for player 1
void p1_set_wins(uint wins)
{
	printf("%u wins for PLAYER 1\n", wins);
	p1_live_leds_mcp.state &= 0b00111111;
	p1_live_leds_mcp.state |= wins_to_bin(wins);
	update_mcp23008_state(&p1_live_leds_mcp);
}

/// Display this number of lives left for player 2
void p2_set_wins(uint wins)
{
	printf("%u wins for PLAYER 2\n", wins);
	p2_live_leds_mcp.state &= 0b00111111;
	p2_live_leds_mcp.state |= wins_to_bin(wins);
	update_mcp23008_state(&p2_live_leds_mcp);
}

/// Display the last round shot on "shot" LEDs
void last_round_shot(bool is_live)
{
	gpio_put(LIVE_ROUND_SHOT_LED, is_live);
	gpio_put(BLANK_ROUND_SHOT_LED, !is_live);
}

/// Clear the last round shot on "shot" LEDs
void clear_last_shot_led()
{
	gpio_put(LIVE_ROUND_SHOT_LED, 0);
	gpio_put(BLANK_ROUND_SHOT_LED, 0);
}

/// Display live/blank LED in chamber
void open_chamber(bool is_live)
{
	gpio_put(CHAMBER_LIVE_LED, is_live);
	gpio_put(CHAMBER_BLANK_LED, !is_live);
}

void close_chamber() {
	gpio_put(CHAMBER_LIVE_LED, 0);
	gpio_put(CHAMBER_BLANK_LED, 0);
}

/// Turn off every LED
void turn_off_all_leds()
{
	gpio_put(P1_SHOCK_LED, 0);
	gpio_put(P2_SHOCK_LED, 0);
	gpio_put(CHAMBER_LIVE_LED, 0);
	gpio_put(CHAMBER_BLANK_LED, 0);
	gpio_put(LIVE_ROUND_SHOT_LED, 0);
	gpio_put(BLANK_ROUND_SHOT_LED, 0);
	gpio_put(LOADED_LED, 0);
	gpio_put(INPUT_ALLOWED_LED, 0);
}

/// Show n live and n blank rounds loaded on some LEDs
void display_loaded_rounds(uint live, uint blank)
{
	printf("%u live and %u blank rounds in chamber\n", live, blank);
	for(int i=0; i<10; ++i) {
		gpio_put(LIVE_ROUND_SHOT_LED, 1);
		gpio_put(BLANK_ROUND_SHOT_LED, 1);
		sleep_ms(200);
		clear_last_shot_led();
		sleep_ms(200);
	}
        while(live || blank) {
		clear_last_shot_led();
		sleep_ms(300);
		int tot = live + blank;
		if (get_rand_inclusive(1, tot) <= live) {
			gpio_put(LIVE_ROUND_SHOT_LED, 1);
			--live;
		}
		else {
			gpio_put(BLANK_ROUND_SHOT_LED, 1);
			--blank;
		}
		sleep_ms(300);
	}
	clear_last_shot_led();
}

void set_loaded_led(bool state)
{
	gpio_put(LOADED_LED, state);
}

void set_info_text(char *str)
{
	printf("Setting text to %s", str);
	ssd1306_clear(&info_screen);
	ssd1306_draw_string(&info_screen, 8, 24, 2, str);
	ssd1306_show(&info_screen);
}

void clear_text()
{
	ssd1306_clear(&info_screen);
}

/// Setup outputs
void setup_outputs()
{
	// led setup
	gpio_init(P1_SHOCK_LED);
	gpio_set_dir(P1_SHOCK_LED, GPIO_OUT);
	gpio_put(P1_SHOCK_LED, 0);

	gpio_init(P2_SHOCK_LED);
	gpio_set_dir(P2_SHOCK_LED, GPIO_OUT);
	gpio_put(P2_SHOCK_LED, 0);

	gpio_init(CHAMBER_LIVE_LED);
	gpio_set_dir(CHAMBER_LIVE_LED, GPIO_OUT);
	gpio_put(CHAMBER_LIVE_LED, 0);

	gpio_init(CHAMBER_BLANK_LED);
	gpio_set_dir(CHAMBER_BLANK_LED, GPIO_OUT);
	gpio_put(CHAMBER_BLANK_LED, 0);

	gpio_init(LIVE_ROUND_SHOT_LED);
	gpio_set_dir(LIVE_ROUND_SHOT_LED, GPIO_OUT);
	gpio_put(LIVE_ROUND_SHOT_LED, 0);

	gpio_init(BLANK_ROUND_SHOT_LED);
	gpio_set_dir(BLANK_ROUND_SHOT_LED, GPIO_OUT);
	gpio_put(BLANK_ROUND_SHOT_LED, 0);

	gpio_init(INPUT_ALLOWED_LED);
	gpio_set_dir(INPUT_ALLOWED_LED, GPIO_OUT);
	gpio_put(INPUT_ALLOWED_LED, 0);

	gpio_init(LOADED_LED);
	gpio_set_dir(LOADED_LED, GPIO_OUT);
	gpio_put(LOADED_LED, 0);

	// pwm setup
	int wrap = PICO_CLOCK_HZ / SHOCK_FREQ_HZ;
	int level = wrap / 2; // 50% on time

	gpio_set_function(P1_SHOCK_PIN, GPIO_FUNC_PWM);
	p1_shock_slice = pwm_gpio_to_slice_num(P1_SHOCK_PIN);
	pwm_set_wrap(p1_shock_slice, wrap);
	pwm_set_chan_level(p1_shock_slice, PWM_CHAN_A, level);
	pwm_set_enabled(p1_shock_slice, false);

	gpio_set_function(P2_SHOCK_PIN, GPIO_FUNC_PWM);
	p2_shock_slice = pwm_gpio_to_slice_num(P2_SHOCK_PIN);
	pwm_set_wrap(p2_shock_slice, wrap);
	pwm_set_chan_level(p2_shock_slice, PWM_CHAN_B, level);
	pwm_set_enabled(p2_shock_slice, false);


	// mcp23008 setup
	i2c_init(p1_live_leds_mcp.i2c_instance, 100 * 1000); // 100khz baud
	gpio_set_function(p1_live_leds_mcp.sda_pin, GPIO_FUNC_I2C);
	gpio_set_function(p1_live_leds_mcp.scl_pin, GPIO_FUNC_I2C);
	gpio_pull_up(p1_live_leds_mcp.sda_pin);
	gpio_pull_up(p1_live_leds_mcp.scl_pin);

	setup_mcp23008(&p1_live_leds_mcp);
	update_mcp23008_state(&p1_live_leds_mcp);

	setup_mcp23008(&p2_live_leds_mcp);
	update_mcp23008_state(&p2_live_leds_mcp);

	info_screen.external_vcc=false;
	ssd1306_init(&info_screen, INFO_SCREEN_WIDTH, INFO_SCREEN_HEIGHT, INFO_SCREEN_ADDRESS, INFO_SCREEN_I2C_INSTANCE);
	ssd1306_clear(&info_screen);

}
