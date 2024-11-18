/* 
 * File:   LCD.h
 * Author: Thomas Groom & Luke Alderson
 *
 * Created on March 1, 2024
 */

#ifndef _LCD_H
#define _LCD_H
#define _XTAL_FREQ 64000000

// Define Pins Used with the LCD Screen
#define DB7 LATAbits.LATA0
#define DB6 LATGbits.LATG3
#define DB5 LATDbits.LATD0
#define DB4 LATCbits.LATC3
#define E   LATCbits.LATC4
#define RS  LATCbits.LATC5

#include <xc.h>

void LCD_E_TOG(void);
void LCD_sendnibble(unsigned char number);
void LCD_sendbyte(unsigned char Byte, char type);
void LCD_Init(void);
void LCD_setCursor (char row, char col);	
void LCD_sendstring(char *string, char row, char col);

#endif
