/* 
 * File:   interrupts.c
 * Author: Thomas Groom & Luke Alderson
 *
 * Created on March 1, 2024
 */

#include <xc.h>
#include "interrupts.h"

extern volatile unsigned char RED_BRIGHTNESS;
extern volatile unsigned char GREEN_BRIGHTNESS;
extern volatile unsigned char BLUE_BRIGHTNESS;
extern volatile unsigned int deltaTime;

// Function to turn on interrupts
void Interrupts_init(void) {
	// Turn on peripheral interrupts, the interrupt source, global interrupts
    INTCONbits.PEIE = 1;    // Enable peripheral interrupts
    INTCONbits.INT0EDG = 1; // Explicitly set all interrupts to rising edge

    PIE5bits.TMR1IE = 1;    // Enable interrupt source TMR1 (hardware timer 1)
    IPR5bits.TMR1IP = 1;    // Set TMR1 interrupt to high priority
    
    PIE5bits.TMR3IE = 1;    // Enable interrupt source TMR1 (hardware timer 1)
    IPR5bits.TMR3IP = 1;    // Set TMR1 interrupt to high priority
    
    PIE5bits.TMR5IE = 1;    // Enable interrupt source TMR1 (hardware timer 1)
    IPR5bits.TMR5IP = 1;    // Set TMR1 interrupt to high priority
    
    INTCONbits.GIE = 1;     // Turn on interrupts globally
}

// ISR for timers 1, 3, 5 and 7
void __interrupt(high_priority) HighISR() {

    if (PIR5bits.TMR1IF) // ISR for TMR1
    { 	
        LATGbits.LATG0 = !LATGbits.LATG0;
        TMR1H = LATGbits.LATG0 ? ~RED_BRIGHTNESS : RED_BRIGHTNESS;    // A1        
        TMR1L = 0x00;
        PIR5bits.TMR1IF = 0;
    }
    
    if (PIR5bits.TMR3IF) // ISR for TMR3
    { 	
        LATEbits.LATE7 = !LATEbits.LATE7;
        TMR3H = LATEbits.LATE7 ? ~GREEN_BRIGHTNESS : GREEN_BRIGHTNESS;
        TMR3L = 0x00;
        PIR5bits.TMR3IF = 0;
    }
    
    if (PIR5bits.TMR5IF) // ISR for TMR5
    { 	
        LATAbits.LATA3 = !LATAbits.LATA3;
        TMR5H = LATAbits.LATA3 ? ~BLUE_BRIGHTNESS : BLUE_BRIGHTNESS;
        TMR5L = 0x00;
        PIR5bits.TMR5IF = 0;
    }
    
    if (PIR5bits.TMR7IF) // ISR for TMR7
    { 	
        deltaTime++;
        TMR5H = 0xB1;  // Reset to have 20000 counts before overflow
        TMR5L = 0xE0;
        PIR5bits.TMR7IF = 0;
    }
}

