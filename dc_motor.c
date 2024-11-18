/* 
 * File:   dc_motor.c
 * Author: Luke Alderson
 *
 * Created on February 29, 2024
 */
#include <xc.h>
#include <math.h>
#include "dc_motor.h"
#include "utils.h"
#include "timers.h"

#define UNIT_TIME 2130

// Function initialise T2 and CCP for DC motor control
void initDCmotorsPWM(unsigned int PWMperiod) {
    // Initialise your TRIS and LAT registers for PWM  
    TRISEbits.TRISE2 = 0;
    TRISEbits.TRISE4 = 0;
    TRISCbits.TRISC7 = 0;
    TRISGbits.TRISG6 = 0;
    
    LATEbits.LATE2 = 0;
    LATEbits.LATE4 = 0;
    LATCbits.LATC7 = 0;
    LATGbits.LATG6 = 0;
    
    // Configure PPS to map CCP modules to pins
    RE2PPS=0x05; //CCP1 on RE2
    RE4PPS=0x06; //CCP2 on RE4
    RC7PPS=0x07; //CCP3 on RC7 // something wrong here?
    RG6PPS=0x08; //CCP4 on RG6

    // Timer 2 config
    T2CONbits.CKPS = 3;         // 1:8 prescaler
    T2HLTbits.MODE = 0b00000;   // Free Running Mode, software gate only
    T2CLKCONbits.CS = 0b0001;   // Fosc/4

    // Tpwm*(Fosc/4)/prescaler - 1 = PTPER
    T2PR = (unsigned char)((16000000 / (PWMperiod * 8)) - 1); //Period reg 10kHz base period
    T2CONbits.ON=1;
    
    // Setup CCP modules to output PMW signals
    // Initial duty cycles of 100%
    CCPR1H = T2PR; 
    CCPR2H = T2PR; 
    CCPR3H = T2PR; 
    CCPR4H = T2PR; 
    
    // Use tmr2 for all CCP modules used
    CCPTMRS0bits.C1TSEL=0;
    CCPTMRS0bits.C2TSEL=0;
    CCPTMRS0bits.C3TSEL=0;
    CCPTMRS0bits.C4TSEL=0;
    
    // Configure each CCP
    CCP1CONbits.FMT=1; // left aligned duty cycle (we can just use high byte)
    CCP1CONbits.CCP1MODE=0b1100; //PWM mode  
    CCP1CONbits.EN=1; //turn on
    
    CCP2CONbits.FMT=1; // left aligned
    CCP2CONbits.CCP2MODE=0b1100; //PWM mode  
    CCP2CONbits.EN=1; //turn on
    
    CCP3CONbits.FMT=1; // left aligned
    CCP3CONbits.CCP3MODE=0b1100; //PWM mode  
    CCP3CONbits.EN=1; //turn on
    
    CCP4CONbits.FMT=1; // left aligned
    CCP4CONbits.CCP4MODE=0b1100; //PWM mode  
    CCP4CONbits.EN=1; //turn on
}

// Function to set CCP PWM output from the values in the motor structure
void setMotorPWM(struct DC_motor *m) {
	unsigned char posDuty, negDuty;  // Duty cycle values for different sides of the motor

	if(m->brakemode) {
		posDuty = (unsigned char)(m->PWMperiod - ((unsigned int)(m->power) * (m->PWMperiod)) / 100);  // Inverted PWM duty
		negDuty = (unsigned char)(m->PWMperiod);  // Other side of motor is high all the time
	}
	else {
		posDuty = (unsigned char)(((unsigned int)(m->power) * (m->PWMperiod)) / 100);  // PWM duty
		negDuty = 0;  // Other side of motor is low all the time
	}

	if (m->direction) {
		*(m->posDutyHighByte) = posDuty;  // Assign values to the CCP duty cycle registers
		*(m->negDutyHighByte) = negDuty;       
	} else {
		*(m->posDutyHighByte)=negDuty;  // Do it the other way around to change direction
		*(m->negDutyHighByte)=posDuty;
	}
}

// Function to make the robot go straight back in reverse
void forward(DC_motor *mL, DC_motor *mR, unsigned char power) {
    // Set direction of mL, mR to forward. mL = 1 is forward, as is mR = 0
    mL -> direction = 1;
    mR -> direction = 0;
    
    // Full power both wheels
    mL -> power = (unsigned char)(power*1.1);
    mR -> power = power;
    
    setMotorPWM(mL);
    setMotorPWM(mR);
}

// Function to make the robot go straight
void reverse(DC_motor *mL, DC_motor *mR, unsigned char power)
{
    // Set direction of mL to reverse, mR to forward
    mL -> direction = 0;
    mR -> direction = 1;
    
    // Full power both wheels
    mL -> power = (unsigned char)(power*1.1);
    mR -> power = power;
    
    setMotorPWM(mL);
    setMotorPWM(mR);
}

// Function to stop the robot gradually 
void stop(DC_motor *mL, DC_motor *mR)
{
    for (char j = mL -> power; j > 0; j--){
        // Gradually reduce power from the current power to 0%
        mL -> power = j;
        mR -> power = j;
        
        setMotorPWM(mL);
        setMotorPWM(mR);
        __delay_us(1000);
    }
    mL -> power = 0;
    mR -> power = 0;
    setMotorPWM(mL);
    setMotorPWM(mR);
}

// Function to start the robot gradually 
void start(DC_motor *mL, DC_motor *mR, char power)
{
    for (char j = 0; j <= power; j++){
        // Gradually increase power from 0% to specified amount
        mL -> power = j;
        mR -> power = j;
        
        setMotorPWM(mL);
        setMotorPWM(mR);
        __delay_us(1000);
    }
}

// Function to make the robot turn left 
void turnLeft(DC_motor *mL, DC_motor *mR) {
    // Set direction of mL to reverse, mR to forward
    mL -> direction = 0;
    mR -> direction = 0;
    
    start(mL, mR, 40);
}

// Function to make the robot turn right 
void turnRight(DC_motor *mL, DC_motor *mR) {
    // Set direction of mL to forward, mR to reverse
    mL -> direction = 1;
    mR -> direction = 1;
    
    start(mL, mR, 40);
}

void turnLeftDeg(DC_motor *mL, DC_motor *mR, unsigned int turnTime, unsigned int deg) {
    int q = 0;
    turnLeft(mL, mR);
    
    while (q < (turnTime * (deg / 90)))
    {
        if (q % 100 == 0)
        {
            LEFT_LIGHT = ~LEFT_LIGHT;
        }
        __delay_ms(1);
        q++;
    }
    LEFT_LIGHT = 0;
    stop(mL, mR);
}

void turnRightDeg(DC_motor *mL, DC_motor *mR, unsigned int turnTime, unsigned int deg) {
    int q = 0;
    turnRight(mL, mR);
    
    while (q < turnTime * (deg / 90))
    {
        if (q % 100 == 0)
        {
            RIGHT_LIGHT = ~RIGHT_LIGHT;
        }
        __delay_ms(1);
        q++;
    }
    RIGHT_LIGHT = 0;
    stop(mL, mR);
}


void forward_unit(DC_motor *mL, DC_motor *mR, float moveTime) {
    forward(mL, mR, 40);
    
    custom_delay_ms((unsigned int)(moveTime*UNIT_TIME));
    
    stop(mL, mR);
}

void reverse_unit(DC_motor *mL, DC_motor *mR, float moveTime) {
    reverse(mL, mR, 40);
    
    custom_delay_ms((unsigned int)(moveTime*UNIT_TIME));
    
    stop(mL, mR);
}

/*
 * Function to pass a variable into a delay, replacing the macro __delay_ms().
 * 
 * Inputs: ms: rounds to nearest 10ms to save memory.
 * Outputs: None.
 */
void custom_delay_ms(unsigned int delayTime)
{
    unsigned long tic = 0;
    while(tic <= delayTime)
    {
        tic++;
        __delay_ms(1);
    }
}


/******************************************************************************
 * Description:
 * This is a high level set of of functions mapping the output of the vision
 * system to motor commands.
 * 
 *  RED         - turn right 90 degrees.
 *  GREEN       - turn left 90 degrees.
 *  BLUE        - turn 180 degrees (moving clockwise in this implementation)
 *  YELLOW      - reverse 1 unit and turn right 90 degrees.
 *  PINK        - reverse 1 unit and turn left 90 degrees.
 *  ORANGE      - Turn right 135 degrees.
 *  LIGHT BLUE  - Turn left 135 degrees.
 *  WHITE       - Finish, returning home.
 *  BLACK       - indicates a maze wall - somethings gone wrong!
 * 
 * Inputs:
 * Motor structures for left and right sides.
 * 
 * Outputs:
 * None.
 * 
 ******************************************************************************/
void red(DC_motor *mL, DC_motor *mR, unsigned int turnTime){
    reverse_unit(mL, mR, 0.5);
    __delay_ms(100);
    turnRightDeg(mL, mR, turnTime, 90);
}

void green(DC_motor *mL, DC_motor *mR, unsigned int turnTime){
    reverse_unit(mL, mR, 0.5);
    __delay_ms(100);
    turnLeftDeg(mL, mR, turnTime, 90);
}

void blue(DC_motor *mL, DC_motor *mR, unsigned int turnTime){
    reverse_unit(mL, mR, 0.5);
    __delay_ms(100);
    turnRightDeg(mL, mR, turnTime, 180);
}

void yellow(DC_motor *mL, DC_motor *mR, unsigned int turnTime){
    reverse_unit(mL, mR, 1.5);
    __delay_ms(100);
    turnRightDeg(mL, mR, turnTime, 90);
}

void pink(DC_motor *mL, DC_motor *mR, unsigned int turnTime){
    reverse_unit(mL, mR, 1.5);
    __delay_ms(100);
    turnLeftDeg(mL, mR, turnTime, 90);
}

void orange(DC_motor *mL, DC_motor *mR, unsigned int turnTime){
    reverse_unit(mL, mR, 0.5);
    __delay_ms(100);
    turnRightDeg(mL, mR, turnTime, 135);
}

void light_blue(DC_motor *mL, DC_motor *mR, unsigned int turnTime){
    reverse_unit(mL, mR, 0.5);
    __delay_ms(100);
    turnLeftDeg(mL, mR, turnTime, 135);
}


/* Description:
 * Calibrating the angular and forward distances.
 *
 * Instructions:
 * Enter routine by pressing RF3.
 * Buggy will cycle through.
 *  - left 90
 *  - right 90
 *  - forward
 *  - reverse
 * At each stage, buggy will perform action then pause.
 * The user can then increment or decrement the distance by tapping RF2 or RF3
 * Move to the next condition by pressing RF2 and RF3 at the same time.
 * 
 * 
 */
/*
unsigned int calibrateLeft(DC_motor *mL, DC_motor *mR, unsigned int baseTurnTime)
{
    unsigned int turnTime = baseTurnTime;
    unsigned int period = 300;
    
    LATHbits.LATH3 = 1;
    
    __delay_ms(1000);
    turnLeft90(mL, mR, turnTime);
    
    while(1)
    {
        if(BUTTONF2)
        {
            turnTime += 10;
            turnLeft90(mL, mR, turnTime);
        }
        else if(BUTTONF3)
        {
            turnTime -= 10;
            turnLeft90(mL, mR, turnTime);
        }
    }
    
    LATHbits.LATH3 = 0;
    
    return turnTime;
}

unsigned int calibrateRight(DC_motor *mL, DC_motor *mR, unsigned int baseTurnTime)
{
    unsigned int turnTime = baseTurnTime;
    
    LATHbits.LATH3 = 1;
    
    __delay_ms(1000);
    turnRight90(mL, mR, turnTime);
    
    while(1)
    {
        // how to break? on colour?
        
        if(BUTTONF2)
        {
            turnTime += 10;
            turnRight90(mL, mR, turnTime);
        }
        else if(BUTTONF3)
        {
            turnTime -= 10;
            turnRight90(mL, mR, turnTime);
        }
    }
    
    LATHbits.LATH3 = 0;
    
    return turnTime;
}
*/