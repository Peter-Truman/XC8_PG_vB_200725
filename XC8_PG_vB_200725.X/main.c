/*
 * File:   main.c
 * Author: PeeWee
 *
 * Created on 21 July 2025, 3:47 PM
 */


// Updated main.c with fixes

// main.c
// main.c
#include <xc.h>
#include "config.h"
#include "lcd.h"
#include "rtc.h"
#include <stdint.h>

void main(void) {
    OSCCON = 0x72; // Set internal oscillator to 16 MHz

    LCD_Init();
    RTC_Init();

    // Test LCD display
    LCD_Clear();
    LCD_SetCursor(1, 0);
    LCD_WriteChar('H');
    LCD_WriteChar('e');
    LCD_WriteChar('l');
    LCD_WriteChar('l');
    LCD_WriteChar('o');

    while (1) {
        // Main loop - add your code here
        __delay_ms(1000); // Delay to test
    }
}