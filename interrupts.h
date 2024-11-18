/* 
 * File:   interrupts.c
 * Author: Thomas Groom & Luke Alderson
 *
 * Created on March 1, 2024
 */

#ifndef _interrupts_H
#define _interrupts_H
#define _XTAL_FREQ 64000000

#include <xc.h>

void Interrupts_init(void);
void __interrupt(high_priority) HighISR();

#endif
