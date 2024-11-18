/* 
 * File:   timers.c
 * Author: Thomas Groom
 *
 * Created on March 1, 2024
 */

#include <xc.h>
#include "timers.h"

/************************************
 * Function to set up timers
************************************/
void Timer_init(void) {
    // Configuration for TMR1 which is used to implement PWM for the red LED
    PMD1bits.TMR1MD = 0;
    TMR1CLKbits.CS = 0b0001;                   
    T1GCONbits.T1GE = 0;
    T1GCONbits.T1GPOL = 0;
    T1CONbits.CKPS = 0b00;      // 0b11 (30.5 Hz), 0b10 (61Hz), 0b01 (122Hz), 0b00 (222Hz) 
    T1CONbits.T1RD16 = 1;       // 16-bit read/write mode
    T1CONbits.NOT_SYNC = 1;  
    TMR1H = 0x00;            
    TMR1L = 0x00;
    T1CONbits.ON = 1;
    
    // Configuration for TMR3 which is used to implement PWM for the green LED
    PMD1bits.TMR3MD = 0;
    TMR3CLKbits.CS = 0b0001;                   
    T3GCONbits.T3GE = 0;
    T3GCONbits.T3GPOL = 0;
    T3CONbits.CKPS = 0b00;
    T3CONbits.T3RD16 = 1;
    T3CONbits.NOT_SYNC = 1;  
    TMR3H = 0x00;            
    TMR3L = 0x00;
    T3CONbits.ON = 1;
    
    // Configuration for TMR5 which is used to implement PWM for the blue LED
    PMD1bits.TMR5MD = 0;
    TMR5CLKbits.CS = 0b0001;
    T5GCONbits.T5GE = 0;
    T5GCONbits.T5GPOL = 0;
    T5CONbits.CKPS = 0b00;
    T5CONbits.T5RD16 = 1;
    T5CONbits.NOT_SYNC = 1;
    TMR5H = 0x00;
    TMR5L = 0x00;
    T5CONbits.ON = 1;
    
    // Timer 7 is used to track the duration of forward/reverse movements
    PMD1bits.TMR7MD = 0;
    TMR7CLKbits.CS = 0b0001;
    T7GCONbits.T7GE = 0;
    T7GCONbits.T7GPOL = 0;
    T7CONbits.CKPS = 0b10;
    T7CONbits.T7RD16 = 1;
    T7CONbits.NOT_SYNC = 1;
    TMR7H = 0xB1;  // Reset to have 20000 counts before overflow
    TMR7L = 0xE0;
    T7CONbits.ON = 1;
}
