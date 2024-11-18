/* 
 * File:   LCD.c
 * Author: Thomas Groom & Luke Alderson
 *
 * Created on March 1, 2024
 */

#include <xc.h>
#include "LCD.h"
#include <stdio.h>
#include <stdlib.h>


/************************************
 * Function to toggle LCD enable bit on then off
 * when this function is called the LCD screen reads the data lines
************************************/
void LCD_E_TOG(void)
{
	//turn the LCD enable bit on
    E = 1;
	__delay_us(2); //wait a short delay
	//turn the LCD enable bit off again
    E = 0;
}

/************************************
 * Function to set the 4-bit data line levels for the LCD
************************************/
void LCD_sendnibble(unsigned char number)
{

	//set the data lines here (think back to LED array output)
    DB4 = 1 & number;
    DB5 = 1 & (number >> 1);
    DB6 = 1 & (number >> 2);
    DB7 = 1 & (number >> 3);
    LCD_E_TOG();			//toggle the enable bit to instruct the LCD to read the data lines
    __delay_us(5);      // Delay 5 uS
}


/************************************
 * Function to send full 8-bit commands/data over the 4-bit interface
 * high nibble (4 most significant bits) are sent first, then low nibble sent
************************************/
void LCD_sendbyte(unsigned char Byte, char type)
{
    // set RS pin whether it is a Command (0) or Data/Char (1) using type argument
    RS = type & 1;
    // send high bits of Byte using LCDout function
    LCD_sendnibble(Byte >> 4);
    // send low bits of Byte using LCDout function
    LCD_sendnibble(Byte);
	
    __delay_us(50);               //delay 50uS (minimum for command to execute)
}

/************************************
 * Function to initialise the LCD after power on
************************************/
void LCD_Init(void)
{
    //Define LCD Pins as Outputs and set all pins low initially.

    TRISCbits.TRISC5 = 0;  // Set B1 as output
    RS = 0;  // Set RS to LOW
    TRISCbits.TRISC4 = 0;  // Set GC2 as output
    E = 0;  // Set E to LOW
    TRISCbits.TRISC3 = 0;  // Set B3 as output
    DB4 = 0;  // Set DB4 to LOW
    TRISDbits.TRISD0 = 0;  // Set B2 as output
    DB5 = 0;  // Set DB5 to LOW
    TRISGbits.TRISG3 = 0;  // Set E3 as output
    DB6 = 0;  // Set DB6 to LOW
    TRISAbits.TRISA0 = 0;  // Set E1 as output
    DB7 = 0;  // Set DB7 to LOW

    // Initialisation sequence code
    __delay_ms(50);  // Wait a little to ensure Vdd rises beyond 4.5V
    LCD_sendnibble(0b0011);  // Put LCD into 4-bit data bus mode
    __delay_us(50);  // Make sure to wait long enough before next instruction
    LCD_sendbyte(0b00101000, 0); // Function Set
    LCD_sendbyte(0b00101000, 0); // Function Set
    LCD_sendbyte(0b00001000, 0); // Turn Display OFF
    LCD_sendbyte(0b00000001, 0); // Clear Display
    __delay_ms(2);
    LCD_sendbyte(0b00000110, 0); // Entry Mode Set (Direction set to 1)
    LCD_sendbyte(0b00001100, 0); // Turn Display ON
	//remember to turn the LCD display back on at the end of the initialisation (not in the data sheet)
}

/************************************
 * Function to set the cursor to a specific position
************************************/
void LCD_setCursor (char row, char col)
{
    if(row == 0){
        //Send 0x80 to set line to 1 (0x00 ddram address)
        LCD_sendbyte(0x80 + col, 0);
    }else{
        //Send 0xC0 to set line to 2 (0x40 ddram address)
        LCD_sendbyte(0xC0 + col, 0);
    }
}

/************************************
 * Function to send string to LCD screen
************************************/
void LCD_sendstring(char *string, char row, char col)
{
    LCD_setCursor(row, col);
	// Code here to send a string to LCD using pointers and LCD_sendbyte function
    while(*string != 0){  // While the data pointed to isn?t a 0x00 do below (strings in C must end with a NULL byte) 
		LCD_sendbyte(*string++,1); 	// Send out the current byte pointed to and increment the pointer
	}
}
