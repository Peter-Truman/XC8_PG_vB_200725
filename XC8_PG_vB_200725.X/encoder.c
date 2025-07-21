/*
 * File:   encoder.c
 * Author: PeeWee
 *
 * Created on 21 July 2025, 3:43 PM
 */


// Updated encoder.c with casts to suppress warnings

#include <xc.h>
#include "config.h"  // For pins and constants
#include "encoder.h"  // For ms_counter declaration

volatile unsigned long ms_counter = 0;  // Global timer (volatile because interrupt updates it)

// Detects encoder rotation (simple gray code state machine)
char get_encoder(void) {
    static unsigned char last_state = 0;  // Static: remembers between calls
    unsigned char current = (unsigned char)((ENC_B << 1) | ENC_A);  // Combine pins into 2-bit state, cast to unsigned char
    char dir = 0;
    if (current != last_state) {  // State changed
        switch (last_state) {
            case 0x00: if (current == 0x01) dir = 1; else if (current == 0x02) dir = (char)-1; break;
            case 0x01: if (current == 0x03) dir = 1; else if (current == 0x00) dir = (char)-1; break;
            case 0x02: if (current == 0x00) dir = 1; else if (current == 0x03) dir = (char)-1; break;
            case 0x03: if (current == 0x02) dir = 1; else if (current == 0x01) dir = (char)-1; break;
        }
        last_state = current;
    }
    return dir;
}

// Detects button presses with debounce and long/short distinction
char get_button(void) {
    static unsigned long start_time = 0;
    static char last = 1;  // Assume released (1) initially
    char curr = BUTTON;  // Read current state (0=pressed)
    char res = 0;
    if (curr == 0 && last == 1) {  // Just pressed
        start_time = ms_counter;
    }
    if (curr == 1 && last == 0) {  // Just released
        unsigned long duration = ms_counter - start_time;
        if (duration > DEBOUNCE_MS && duration < LONG_PRESS_MS) res = 1;  // Short
        else if (duration >= LONG_PRESS_MS) res = 2;  // Long
    }
    last = curr;
    return res;
}