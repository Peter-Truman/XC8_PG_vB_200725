/*
 * File:   main.c
 * Author: PeeWee
 *
 * Created on 21 July 2025, 3:47 PM
 */


// Updated main.c with fixes

#include <xc.h>  // XC8 core
#include <stdio.h>  // For sprintf (string formatting in display)
#include "config.h"  // Pins and constants
#include "lcd.h"  // LCD functions
#include "encoder.h"  // Input functions

// Configuration bits for PIC18F25K22
#pragma config FOSC = INTIO67   // Internal oscillator block
#pragma config PLLCFG = OFF     // Oscillator used directly (no PLL)
#pragma config PRICLKEN = OFF   // Primary clock can be disabled by software
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor disabled
#pragma config IESO = OFF       // Internal/External Oscillator Switchover disabled
#pragma config PWRTEN = OFF     // Power-up Timer disabled
#pragma config BOREN = SBORDIS  // Brown-out Reset enabled in hardware only
#pragma config BORV = 190       // VBOR set to 1.90 V nominal
#pragma config WDTEN = OFF      // Watchdog Timer disabled
#pragma config WDTPS = 32768    // Watchdog Postscaler 1:32768
#pragma config CCP2MX = PORTC1  // CCP2 input/output is multiplexed with RC1
#pragma config PBADEN = OFF     // PORTB<5:0> pins are configured as digital I/O on Reset
#pragma config CCP3MX = PORTB5  // P3A/CCP3 input/output is multiplexed with RB5
#pragma config HFOFST = ON      // HFINTOSC output and ready status are not delayed by the oscillator stable status
#pragma config T3CMX = PORTC0   // T3CKI is on RC0
#pragma config P2BMX = PORTD2   // P2B is on RD2
#pragma config MCLRE = INTMCLR  // RE3 input pin enabled; MCLR disabled
#pragma config STVREN = ON      // Stack full/underflow will cause Reset
#pragma config LVP = OFF        // Single-Supply ICSP disabled
#pragma config XINST = OFF      // Instruction set extension and Indexed Addressing mode disabled
#pragma config CP0 = OFF        // Block 0 (000800-001FFFh) not code-protected
#pragma config CP1 = OFF        // Block 1 (002000-003FFFh) not code-protected
#pragma config CP2 = OFF        // Block 2 (004000-005FFFh) not code-protected
#pragma config CP3 = OFF        // Block 3 (006000-007FFFh) not code-protected
#pragma config CPB = OFF        // Boot block (000000-0007FFh) not code-protected
#pragma config CPD = OFF        // Data EEPROM not code-protected
#pragma config WRT0 = OFF       // Block 0 (000800-001FFFh) not write-protected
#pragma config WRT1 = OFF       // Block 1 (002000-003FFFh) not write-protected
#pragma config WRT2 = OFF       // Block 2 (004000-005FFFh) not write-protected
#pragma config WRT3 = OFF       // Block 3 (006000-007FFFh) not write-protected
#pragma config WRTC = OFF       // Configuration registers (300000-3000FFh) not write-protected
#pragma config WRTB = OFF       // Boot Block (000000-0007FFh) not write-protected
#pragma config WRTD = OFF       // Data EEPROM not write-protected
#pragma config EBTR0 = OFF      // Block 0 (000800-001FFFh) not protected from table reads executed in other blocks
#pragma config EBTR1 = OFF      // Block 1 (002000-003FFFh) not protected from table reads executed in other blocks
#pragma config EBTR2 = OFF      // Block 2 (004000-005FFFh) not protected from table reads executed in other blocks
#pragma config EBTR3 = OFF      // Block 3 (006000-007FFFh) not protected from table reads executed in other blocks
#pragma config EBTRB = OFF      // Boot Block (000000-0007FFh) not protected from table reads executed in other blocks

// Menu states (enum: like named numbers)
typedef enum { IDLE, MENU, EDIT } State;
State current_state = IDLE;  // Start in idle
int current_index = 0;  // Selected menu item
int temp_value = 0;  // Temporary edit value

// Parameter structure (expandable array)
typedef struct {
    const char *name;  // e.g., "High Pressure"
    int value, min, max;  // Current, range
    const char *unit;  // e.g., "psi"
} Parameter;

Parameter params[MAX_MENU_ITEMS] = {
    {"High Pressure", 50, 0, 335, "psi"},
    {"HP Bypass", 0, 0, 45, "sec"},
    {"Low Pressure", 50, 0, 335, "psi"},
    {"LP Bypass", 0, 0, 45, "sec"}
};

// Draw idle screen
void draw_idle(void) {
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_String("Irrisys Menu Demo");
    LCD_SetCursor(1, 0);
    LCD_String("Click to open menu");
}

// Draw main menu (simple list for now; scrolling added later)
void draw_menu(void) {
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_String("Main Menu");
    for (int i = 0; i < MAX_MENU_ITEMS; i++) {
        LCD_SetCursor((unsigned char)(i + 1), 0);  // Lines 1 to 4, cast to unsigned char
        if (i == current_index) LCD_Data('>');  // Highlight
        else LCD_Data(' ');
        LCD_String(params[i].name);
    }
}

// Draw edit screen
void draw_edit(void) {
    char buf[20];  // Buffer for formatted string
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_String("< ");  // Breadcrumb
    LCD_String(params[current_index].name);
    LCD_SetCursor(1, 0);
    LCD_String("  Value: ");
    sprintf(buf, "%d %s", temp_value, params[current_index].unit);  // Format value
    LCD_String(buf);
    LCD_SetCursor(3, 0);
    LCD_String("Short: Save | Long: Exit");
}

// Interrupt service routine (for ms timer) - use __interrupt() for XC8 v2+
void __interrupt() isr(void) {
    if (PIR1bits.TMR1IF) {  // Timer1 overflow
        PIR1bits.TMR1IF = 0;
        // Reload for ~1ms interval (at 16MHz, prescaler 1:8: Fosc/4 = 4MHz, /8 = 500kHz, 500 ticks/ms, use 500 for 1ms)
        TMR1H = (65536 - 500) >> 8;
        TMR1L = (65536 - 500) & 0xFF;
        ms_counter++;
    }
}

// Main function: PIC entry point
void main(void) {
    OSCCON = 0x70;  // Set internal osc to 16MHz (HFINTOSC=16MHz)
    OSCTUNEbits.PLLEN = 0;  // Disable PLL
    ANSELA = 0;  // PORTA digital
    ANSELB = 0;  // PORTB digital
    ANSELC = 0;  // PORTC digital (if used)
    TRISA = 0x00;   // PORTA outputs (for LCD)
    TRISB = 0b01000110;  // RB1, RB2, RB6 inputs
    PORTA = 0;  // Clear ports
    PORTB = 0;
    INTCON2bits.RBPU = 0;  // Enable weak pull-ups on PORTB

    // Setup Timer1 for ms interrupts
    T1CON = 0b00110001;  // 1:8 prescaler, internal clock, enabled
    PIE1bits.TMR1IE = 1;  // Enable interrupt
    INTCONbits.PEIE = 1;  // Peripheral interrupts
    INTCONbits.GIE = 1;   // Global interrupts

    LCD_Init();  // Start LCD
    draw_idle();  // Show initial screen

    while (1) {  // Infinite loop
        char enc = get_encoder();  // Check rotation
        char btn = get_button();   // Check button

        switch (current_state) {
            case IDLE:
                if (btn == 1) {  // Short press: open menu
                    current_state = MENU;
                    current_index = 0;
                    draw_menu();
                }
                break;
            case MENU:
                if (enc) {  // Rotate: change selection with wrap
                    current_index += enc;
                    if (current_index < 0) current_index = MAX_MENU_ITEMS - 1;
                    if (current_index >= MAX_MENU_ITEMS) current_index = 0;
                    draw_menu();
                }
                if (btn == 1) {  // Short: enter edit
                    temp_value = params[current_index].value;
                    current_state = EDIT;
                    draw_edit();
                }
                if (btn == 2) {  // Long: back to idle
                    current_state = IDLE;
                    draw_idle();
                }
                break;
            case EDIT:
                if (enc) {  // Rotate: adjust value with clamps
                    temp_value += enc;
                    if (temp_value < params[current_index].min) temp_value = params[current_index].min;
                    if (temp_value > params[current_index].max) temp_value = params[current_index].max;
                    draw_edit();
                }
                if (btn == 1) {  // Short: save and back to menu
                    params[current_index].value = temp_value;
                    current_state = MENU;
                    draw_menu();
                }
                if (btn == 2) {  // Long: discard and to idle
                    current_state = IDLE;
                    draw_idle();
                }
                break;
        }
    }
}