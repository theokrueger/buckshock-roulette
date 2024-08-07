//! game.c
//!
//! driver for running the entire game, post-setups.

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "pico/time.h"
#include "pico/rand.h"
#include "hardware/watchdog.h"
#include "../include/input.h"
#include "../include/output.h"
#include "../include/debug.h"
#include "../include/game.h"

#define MAX_ROUNDS 3
#define ROUNDS_TO_WIN 2

#define MIN_LIVE_SHELLS 1
#define MIN_BLANK_SHELLS 1
#define MAX_LIVE_SHELLS 5
#define MAX_BLANK_SHELLS 5
#define SHELL_DEVIATION 1

#define CHAMBER_SIZE 8
#define STARTING_SHELLS 3
#define STARTING_SHELLS_DEVIATION 1
#define STARTING_SHELLS_INCREMENT 2

#define CHAMBER_LOADED_DISPLAY_TIME_MS 5000

/// Maximum of two integers
int imax(int a, int b)
{
	return a > b ? a : b;
}

/// Minimum of two integers
int imin(int a, int b)
{
	return a < b ? a : b;
}

/// Return a random number on range [low, high]
int get_rand_inclusive(int low, int high)
{
	return (get_rand_32() % (high - low + 1)) + low;
}

/// Load the chamber and light up chamber LEDs according to contents
int load_chamber(uint chamber[], uint chamber_size)
{
	pause_input();
	uint live_rounds = imin(
		imax(
			chamber_size / 2 +
			get_rand_inclusive(0 - SHELL_DEVIATION, SHELL_DEVIATION * !(chamber_size % 2)),
			MIN_LIVE_SHELLS
			),
		chamber_size - MIN_BLANK_SHELLS
		);

	printf("loading chamber with %u lives %u blanks!\n[", live_rounds, chamber_size - live_rounds);
	for (uint i=0; i<chamber_size; ++i) {
		if (get_rand_inclusive(1, chamber_size - i) <= live_rounds) {
			--live_rounds;
			chamber[i] = true;
			printf("X");
		} else {
			chamber[i] = false;
			printf("O");
		}
	}
	printf("]\n");
	// TODO light up LEDs
	sleep_ms(CHAMBER_LOADED_DISPLAY_TIME_MS);
	resume_input();
}

/// Play a single round of buckshot roulette, return 1 if p1 win, 2 if p2 win, 0 if tie (impossible)
uint play_round(uint round)
{
	printf("Begin round %u\n", round + 1);
	// Setup player lives
	const uint max_lives = round + 2 + get_rand_inclusive(0, 1);

	uint p1_lives = max_lives;
	uint p2_lives = max_lives;
	p1_display_lives(p1_lives);
	p2_display_lives(p2_lives);

	uint subround = 0;
	while(1)
	{
		printf("Begin subround %u\n", subround + 1);
		clear_last_shot_led();
		// load the chamber
		const uint chamber_size = imax(STARTING_SHELLS +
					       (STARTING_SHELLS_INCREMENT * round) +
					       get_rand_inclusive(0 - STARTING_SHELLS_DEVIATION, STARTING_SHELLS_DEVIATION),
					       MIN_LIVE_SHELLS + MIN_BLANK_SHELLS);
		uint chamber[chamber_size];
		load_chamber(chamber, chamber_size);

		uint shots_fired = 0;
	        while(shots_fired < chamber_size)
		{
			if (get_trigger_state_queued()) {
				printf("Trigger Pulled!\n");
				pause_input();
				// TODO shock more than just p1
				if (chamber[shots_fired]) {
					// TODO supercharge
					printf("Player 1 damaged!\n");
					--p1_lives;
					p1_display_lives(p1_lives);
					p1_shock(1);

					if (p1_lives == 0) {
						resume_input();
						return 2; // p2 wins
					}
				}

				last_round_shot(chamber[shots_fired]);
				++shots_fired;
                                printf("Awaiting shell rack\n");
                                // await shell re-rack
                                while(!get_rack_state()) sleep_ms(10);

                                sleep_ms(250);
				resume_input();
			}
			else if (get_slide_state_queued()) {
				printf("Slide peeked!\n");
				open_chamber(chamber[shots_fired]);
				sleep_ms(100);
				close_chamber();
			}
			else if (get_rack_state_queued()) {
				pause_input();
				printf("Shell racked!\n");
				last_round_shot(chamber[shots_fired]);
				++shots_fired;
				sleep_ms(200);
				resume_input();
			}
		}
		++subround;
	}
	return 0;
}

/// Run the game
void run_game()
{
	printf("Starting game!\n");
	uint p1_wins = 0;
	uint p2_wins = 0;
	for(uint cur_round = 0;
	    cur_round < MAX_ROUNDS &&
		    p1_wins < ROUNDS_TO_WIN &&
		    p2_wins < ROUNDS_TO_WIN;
	    ++cur_round) {
		turn_off_all_leds();
		uint res = play_round(cur_round);
		if(res == 1) ++p1_wins;
		else ++p2_wins;
	}

	if (p1_wins == ROUNDS_TO_WIN) {
		printf("Player 1 wins the game!\n");
	} else {
		printf("Player 2 wins the game!\n");
	}
}
