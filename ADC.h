/* 
 * File:   ADC.h
 * Author: Thomas Groom & Luke Alderson
 *
 * Created on March 1, 2024
 */

#ifndef _ADC_H
#define _ADC_H
#define _XTAL_FREQ 64000000

#include <xc.h>

void ADC_init(void);  // Function used to initialise ADC module
unsigned int ADC_getval(void);  // Measures the voltage of the ADC pin as an 8 bit value

#endif
