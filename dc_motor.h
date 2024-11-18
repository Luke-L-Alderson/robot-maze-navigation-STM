/* 
 * File:   dc_motor.h
 * Author: Luke Alderson
 *
 * Created on February 29, 2024
 */
#ifndef _DC_MOTOR_H
#define _DC_MOTOR_H
#define _XTAL_FREQ 64000000

#include <xc.h>

typedef struct DC_motor { //definition of DC_motor structure
    char power;         //motor power, out of 100
    char direction;     //motor direction, forward(1), reverse(0)
    char brakemode;		// short or fast decay (brake or coast)
    unsigned int PWMperiod; //base period of PWM cycle
    unsigned char *posDutyHighByte; //PWM duty address for motor +ve side
    unsigned char *negDutyHighByte; //PWM duty address for motor -ve side
} DC_motor;

//function prototypes
void initDCmotorsPWM(unsigned int PWMperiod); // function to setup PWM
void setMotorPWM(DC_motor *m);
void stop(DC_motor *mL, DC_motor *mR);
void start(DC_motor *mL, DC_motor *mR, char power);
void turnLeft(DC_motor *mL, DC_motor *mR);
void turnRight(DC_motor *mL, DC_motor *mR);
void turnLeftDeg(DC_motor *mL, DC_motor *mR, unsigned int turnTime, unsigned int deg);
void turnRightDeg(DC_motor *mL, DC_motor *mR, unsigned int turnTime, unsigned int deg);
void forward(DC_motor *mL, DC_motor *mR, unsigned char power);
void reverse(DC_motor *mL, DC_motor *mR, unsigned char power);
void forward_unit(DC_motor *mL, DC_motor *mR, float moveTime);
void reverse_unit(DC_motor *mL, DC_motor *mR, float moveTime);
void custom_delay_ms(unsigned int delayTime);
void red(DC_motor *mL, DC_motor *mR, unsigned int turnTime);
void green(DC_motor *mL, DC_motor *mR, unsigned int turnTime);
void blue(DC_motor *mL, DC_motor *mR, unsigned int turnTime);
void yellow(DC_motor *mL, DC_motor *mR, unsigned int turnTime);
void pink(DC_motor *mL, DC_motor *mR, unsigned int turnTime);
void orange(DC_motor *mL, DC_motor *mR, unsigned int turnTime);
void light_blue(DC_motor *mL, DC_motor *mR, unsigned int turnTime);

//unsigned int calibrateLeft(DC_motor *mL, DC_motor *mR, unsigned int baseTurnTime);
//unsigned int calibrateRight(DC_motor *mL, DC_motor *mR, unsigned int baseTurnTime);

#endif
