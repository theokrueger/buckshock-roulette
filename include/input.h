#pragma once
//! input.h
//!
//! Handles button input and offloads input checking onto second core
//! Includes helper functions for accessing button states and auto-managing mutex

/* BUTTONS */
#define TRIGGER_BTN 27

/// Return the current, unqueued state of the trigger button
bool get_trigger_state();

#define RACK_BTN 21

/// Return the current, unqueued state of the rack button
bool get_rack_state();

#define PLAYER_DIRECTION_SWITCH 19

/// Return the current state of the player direction switch. True for player 2
bool is_player_2_direction();

#define SUPERCHARGE_SWITCH 26

/// Return the current state of the supercharge switch
bool get_supercharge_state();

#define TILT_SWITCH 20

/// Return the current state of the tilt switch
bool get_tilt_state();

#define INCREASE_LIVES_BTN 2

bool get_increase_lives_state();

#define INVERT_BTN 3

bool get_invert_state();

/* INPUT CHECKING */
/// Setup GPIO buttons, and send check input loop to other core
void setup_input();

/// Disallow input
void input_disallow();

/// Allow input once again
void input_allow();
