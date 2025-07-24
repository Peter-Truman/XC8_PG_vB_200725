/*
 * File:   lcd.c
 * Author: PeeWee
 *
 * Created on 21 July 2025, 3:44 PM
 */


// Updated lcd.c with LCD_Data moved before LCD_String

#include <xc.h>  // XC8 core include
#include "config.h"  // For pins and _XTAL_FREQ delays

// Private helper: Sends 4-bit data to LCD
static void LCD_SendNibble(unsigned char nibble, unsigned char rs) {
    PORTA = (PORTA & 0xF0) | (nibble & 0x0F);  // Set data bits RA0-RA3
    LCD_RS = rs;  // 0 for command, 1 for data
    LCD_RW = 0;   // Write mode
    LCD_EN = 1;   // Enable pulse
    __delay_us(1);
    LCD_EN = 0;
    __delay_us(50);  // Short delay for LCD to process
}

// Private helper: Sends full 8-bit command (in two nibbles)
static void LCD_Command(unsigned char cmd) {
    LCD_SendNibble(cmd >> 4, 0);  // High nibble
    LCD_SendNibble(cmd & 0x0F, 0);  // Low nibble
}

// Sends a single 8-bit data byte (in two nibbles) - moved before LCD_String
void LCD_Data(unsigned char data) {
    LCD_SendNibble(data >> 4, 1);
    LCD_SendNibble(data & 0x0F, 1);
}

// Initializes LCD in 4-bit mode for 4x20 display
void LCD_Init(void) {
    __delay_ms(15);  // Power-on delay
    LCD_SendNibble(0x03, 0);
    __delay_ms(5);
    LCD_SendNibble(0x03, 0);
    __delay_us(100);
    LCD_SendNibble(0x03, 0);
    __delay_us(100);
    LCD_SendNibble(0x02, 0);  // Set 4-bit mode
    __delay_us(100);
    LCD_Command(0x28);  // 4-bit, 2-line mode (extends to 4 lines on your LCD)
    LCD_Command(0x0C);  // Display on, cursor off, no blink
    LCD_Command(0x06);  // Entry mode: increment cursor
    LCD_Command(0x01);  // Clear display
    __delay_ms(2);
}

// Clears the entire screen
void LCD_Clear(void) {
    LCD_Command(0x01);
    __delay_ms(2);
}

// Sets cursor position (row 0-3, col 0-19 for 4x20)
void LCD_SetCursor(unsigned char row, unsigned char col) {
    unsigned char addr;
    switch (row) {
        case 0: addr = 0x00; break;
        case 1: addr = 0x40; break;
        case 2: addr = 0x14; break;  // Specific to 4x20 mapping
        case 3: addr = 0x54; break;
        default: addr = 0x00;
    }
    addr += col;
    LCD_Command(0x80 | addr);  // Set DDRAM address
}

// Prints a null-terminated string
void LCD_String(const char *str) {
    while (*str) {  // Loop until end of string
        LCD_Data(*str++);
    }
}