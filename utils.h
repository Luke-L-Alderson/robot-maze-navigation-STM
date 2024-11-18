/* 
 * File:   utils.h
 * Author: Luke Alderson
 *
 * Created on February 29, 2024, 1:04 PM
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef _utils_H
#define	_utils_H

#include <xc.h> // include processor files - each processor file is guarded.  

// Defines
#define BRAKE_LIGHT_INIT        TRISDbits.TRISD4
#define LEFT_LIGHT_INIT         TRISFbits.TRISF0
#define RIGHT_LIGHT_INIT        TRISHbits.TRISH0
#define MAIN_BEAM_INIT          TRISDbits.TRISD3

#define LEFT_LIGHT              LATFbits.LATF0  // 9 pins from vsys is turn left - RF0
#define RIGHT_LIGHT             LATHbits.LATH0  // 13 pins from RST is turn right lights - RH0
#define BRAKE_LIGHT             LATDbits.LATD4  // 12 pins from reset brake - RD4
#define MAIN_BEAM               LATDbits.LATD3  // 10 pins from GND (RST) is mainbeam - RD3)

#endif

