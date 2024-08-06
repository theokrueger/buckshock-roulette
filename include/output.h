#pragma once
//! output.h
//!
//! Handles LED and shock collar outputs
//! Functions for displaying lives and such

#include "pico/stdlib.h"

#define PICO_CLOCK_HZ 125000000 // frequence of pico, 125MHz (default)
#define SHOCK_FREQ_HZ 10650 // shock freqeuency in hz
#define SHOCK_FREQ_ON_TIME_MS 20 // on time for shock radio wave
#define SHOCK_FREQ_OFF_TIME_MS 15 // off time for shock radio wave

#define P1_SHOCK_PIN 15
#define P1_SHOCK_LED 14

#define P2_SHOCK_PIN 16
#define P2_SHOCK_LED 16

#define SHOCK_DURATION_MS 1000 // shock duration in ms

/// Shock player 1 for SHOCK_DURATION * mutliplier arg
void p1_shock(uint multiplier);

/// Shock player 2 for SHOCK_DURATION * mutliplier arg
void p2_shock(uint multiplier);

#define LIVES_LED_COUNT 5

/// Display this number of lives left for player 1
void p1_display_lives(uint lives_left);

/// Display this number of lives left for player 2
void p2_display_lives(uint lives_left);

#define LIVE_ROUND_SHOT_LED 2
#define BLANK_ROUND_SHOT_LED 3

/// Display the last round shot on "shot" LEDs
void last_round_shot(bool is_live);

/// Turn off all LEDs
void turn_off_all_leds();

/// Setup outputs
void setup_outputs();
