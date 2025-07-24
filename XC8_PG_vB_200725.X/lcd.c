/*
 * File:   lcd.c
 * Author: PeeWee
 *
 * Created on 21 July 2025, 3:44 PM
 */


// Updated lcd.c with LCD_Data moved before LCD_String

// lcd.c
// lcd.c
#include "lcd.h"
#include <xc.h>
#include <stdint.h>

void LCD_WriteNibble(uint8_t nibble) {
    LCD_D4 = (nibble >> 0) & 1;
    LCD_D5 = (nibble >> 1) & 1;
    LCD_D6 = (nibble >> 2) & 1;
    LCD_D7 = (nibble >> 3) & 1;
    LCD_EN = 1;
    __delay_us(1); // Pulse enable
    LCD_EN = 0;
    __delay_us(50); // Wait for LCD to process
}

void LCD_Command(uint8_t cmd) {
    LCD_RS = 0;
    LCD_WriteNibble(cmd >> 4);
    LCD_WriteNibble(cmd);
    __delay_us(50);
}

void LCD_WriteChar(uint8_t chr) {
    LCD_RS = 1;
    LCD_WriteNibble(chr >> 4);
    LCD_WriteNibble(chr);
    __delay_us(50);
}

void LCD_SetCursor(uint8_t line, uint8_t pos) {
    uint8_t addr;
    switch (line) {
        case 1: addr = 0x00 + pos; break;
        case 2: addr = 0x40 + pos; break;
        case 3: addr = 0x14 + pos; break;
        case 4: addr = 0x54 + pos; break;
        default: addr = 0x00; break;
    }
    LCD_Command(0x80 | addr);
}

void LCD_Clear(void) {
    LCD_Command(0x01);
    __delay_ms(2);
}

void LCD_Init(void) {
    TRISA = 0x00; // All PORTA as output for LCD
    LCD_RS = 0;
    LCD_RW = 0;
    LCD_EN = 0;
    LCD_D4 = 0;
    LCD_D5 = 0;
    LCD_D6 = 0;
    LCD_D7 = 0;
    __delay_ms(50); // Power-up delay
    LCD_WriteNibble(0x03);
    __delay_ms(5);
    LCD_WriteNibble(0x03);
    __delay_ms(5);
    LCD_WriteNibble(0x03);
    __delay_ms(5);
    LCD_WriteNibble(0x02); // 4-bit mode
    LCD_Command(0x28); // 4-bit, 2 lines, 5x8 font
    LCD_Command(0x0C); // Display on, cursor off
    LCD_Command(0x06); // Entry mode: increment, no shift
    LCD_Clear();
}