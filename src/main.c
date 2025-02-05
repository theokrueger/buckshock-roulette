//! buckshock-roulette
//!
//! IRL version of "Buckshot Roulette" videogame
//! Featuring shock collars instead of actual weaponry

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "hardware/watchdog.h"

#include "../include/input.h"
#include "../include/output.h"
#include "../include/debug.h"
#include "../include/game.h"
#include "../include/util.h"

int main() {
	// init
	stdio_init_all();
	printf("Starting...\n");
	setup_input();
	setup_outputs();
	debug_init();

	// game start
	run_game();

	sleep_ms(5000);

	// reset the pico after game end
	watchdog_enable(1, 1);
	while(1);
}
