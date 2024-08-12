//! game.c
//!
//! driver for running the entire game, post-setups.

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "pico/time.h"
#include "pico/rand.h"
#include "hardware/watchdog.h"

#include "../include/util.h"
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

/// Load the chamber and light up chamber LEDs according to contents
int load_chamber(uint chamber[], uint chamber_size)
{
	uint live_rounds = imin(
		imax(
			chamber_size / 2 +
			get_rand_inclusive(0 - SHELL_DEVIATION, SHELL_DEVIATION * !(chamber_size % 2)),
			MIN_LIVE_SHELLS
			),
		chamber_size - MIN_BLANK_SHELLS
		);

	display_loaded_rounds(live_rounds, chamber_size - live_rounds);
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
}

bool should_damage_p1()
{
	if (is_player_2_direction()) {
		return !get_tilt_state();
	} else {
		return get_tilt_state();
	}
}

void display_cell_screen(uint chamber[], uint chamber_size, uint rounds_shot)
{
	char str[] = "Round n: x";
	int target = get_rand_inclusive(rounds_shot, chamber_size - 1);
	str[6] = '1' + target - rounds_shot;
	str[9] = chamber[target] ? 'X' : 'O';
	set_info_text(str);
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
		sleep_ms(5000);
		printf("Begin subround %u\n", subround + 1);
		clear_last_shot_led();
		// load the chamber
		uint chamber_size = imax(STARTING_SHELLS +
					       (subround > 0) +
					       (STARTING_SHELLS_INCREMENT * round) +
					       get_rand_inclusive(0 - STARTING_SHELLS_DEVIATION, STARTING_SHELLS_DEVIATION),
					       MIN_LIVE_SHELLS + MIN_BLANK_SHELLS);

		if (chamber_size > CHAMBER_SIZE) {
			chamber_size = CHAMBER_SIZE;
		}
		uint chamber[chamber_size];
		load_chamber(chamber, chamber_size);

		uint shots_fired = 0;
		input_allow();

		open_chamber(chamber[shots_fired]);
		set_loaded_led(true);
		display_cell_screen(chamber, chamber_size, shots_fired);
	        while(shots_fired < chamber_size)
		{
			if (get_trigger_state()) {
				int tot = 0;
				for(int i=0; i<10; ++i) {
					if (get_trigger_state()) ++tot;
					sleep_ms(1);
				}
				if (tot < 8) continue;

				input_disallow();
				clear_last_shot_led();
				clear_text();
				printf("Trigger Pulled!\n");
				set_loaded_led(false);
				if (chamber[shots_fired]) {
					int damage_val = 1 + get_supercharge_state();
					if (should_damage_p1()) {
						printf("Player 1 damaged for %i!\n", damage_val);
						p1_lives -= damage_val;
						p1_display_lives(p1_lives);
						p1_shock(damage_val);
						if (p1_lives <= 0) {
							p1_display_lives(0);
							return 2; // p2 wins
						}
					}
					else {
						printf("Player 2 damaged for %i!\n", damage_val);
						p2_lives -= damage_val;
						p2_display_lives(p2_lives);
						p2_shock(damage_val);
						if (p2_lives <= 0) {
							p2_display_lives(0);
							return 1; // p1 wins
						}
					}
				}

				last_round_shot(chamber[shots_fired]);
				++shots_fired;
                                printf("Awaiting shell rack\n");
                                // await shell re-rack
				input_allow();
                                while(!get_rack_state()) sleep_ms(100);
                                while(get_rack_state()) sleep_ms(100);
                                printf("Shell rack done!\n");
				set_loaded_led(true);

                                sleep_ms(1500);
				display_cell_screen(chamber, chamber_size, shots_fired);
				open_chamber(chamber[shots_fired]);
			}
			else if (get_rack_state()) {
				int tot = 0;
				for(int i=0; i<10; ++i) {
					if (get_rack_state()) ++tot;
					sleep_ms(1);
				}
				if (tot < 8) continue;

				input_disallow();
				clear_text();
				set_loaded_led(false);

				close_chamber();
				printf("Shell racked!\n");
				last_round_shot(chamber[shots_fired]);
				++shots_fired;
				sleep_ms(1000);
				input_allow();
				while(get_rack_state()) sleep_ms(100);
				open_chamber(chamber[shots_fired]);
				display_cell_screen(chamber, chamber_size, shots_fired);
				set_loaded_led(true);
			}
			else if (get_invert_state()) {
				input_disallow();
				clear_text();
				set_loaded_led(false);
				printf("Inverted loaded round!");
				chamber[shots_fired] = !chamber[shots_fired];
				sleep_ms(1000);
				set_loaded_led(true);
				input_allow();
			}
			else if (get_increase_lives_state()) {
				input_disallow();
				printf("Added a life to player %i\n", is_player_2_direction() + 1);
				if (is_player_2_direction() && p2_lives < max_lives ) {
				        ++p2_lives;
					p2_display_lives(p2_lives);
				} else if (p1_lives < max_lives) {
				        ++p1_lives;
					p1_display_lives(p1_lives);
				}
				sleep_ms(1000);
				input_allow();
			}
		}
		input_disallow();
		++subround;
	}
	return 0;
}

/// Run the game
void run_game()
{
	await_any_input();
	printf("Starting game!\n");
	uint p1_wins = 0;
	uint p2_wins = 0;
	for(uint cur_round = 0;
	    cur_round < MAX_ROUNDS &&
		    p1_wins < ROUNDS_TO_WIN &&
		    p2_wins < ROUNDS_TO_WIN;
	    ++cur_round) {
		turn_off_all_leds();
		input_disallow();
		uint res = play_round(cur_round);
		if(res == 1) {
			++p1_wins;
			p1_set_wins(p1_wins);
		}
		else {
			++p2_wins;
			p2_set_wins(p2_wins);
		}
	}

	if (p1_wins == ROUNDS_TO_WIN) {
		printf("Player 1 wins the game!\n");
	} else {
		printf("Player 2 wins the game!\n");
	}
}
