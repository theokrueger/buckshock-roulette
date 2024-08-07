#pragma once
//! input.h
//!
//! Handles button input and offloads input checking onto second core
//! Includes helper functions for accessing button states and auto-managing mutex
#include "pico/stdlib.h"
#include "pico/sync.h"

#define INPUT_DEBOUNCE_MS 300

/* BUTTONS */
#define TRIGGER_BTN 2

/// Return the queued state of the trigger button
bool get_trigger_state_queued();

/// Return the current, unqueued state of the trigger button
bool get_trigger_state();


#define SLIDE_BTN 3

/// Return the queued state of the slide button
bool get_slide_state_queued();

/// Return the current, unqueued state of the slide button
bool get_slide_state();


#define RACK_BTN 4

/// Return the queued state of the rack button
bool get_rack_state_queued();

/// Return the current, unqueued state of the rack button
bool get_rack_state();


/* INPUT CHECKING */
/// Setup GPIO buttons, and send check input loop to other core
void setup_input();

/// Pause input checking core
void pause_input();

/// Resume input checking core
void resume_input();
