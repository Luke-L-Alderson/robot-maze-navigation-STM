/* 
 * File:   main.c
 * Author: Thomas Groom & Luke Alderson
 *
 * Created on February 23, 2024
 */

// CONFIG1L
#pragma config FEXTOSC = HS     // External Oscillator mode Selection bits (HS (crystal oscillator) above 8 MHz; PFM set to high power)
#pragma config RSTOSC = EXTOSC_4PLL// Power-up default value for COSC bits (EXTOSC with 4x PLL, with EXTOSC operating per FEXTOSC bits)

// CONFIG3L
#pragma config WDTCPS = WDTCPS_31// WDT Period Select bits (Divider ratio 1:65536; software control of WDTPS)
#pragma config WDTE = OFF        // WDT operating mode (WDT enabled regardless of sleep)

#pragma config CLKOUTEN = OFF  // Disable CLKOUT function
#pragma config CSWEN = OFF     // Clock switch is disabled
#pragma config FCMEN = OFF     // Fail-Safe Clock Monitor is disabled

#include <xc.h>
#include <stdio.h>
#include "ADC.h"
#include "LCD.h"
#include "dc_motor.h"
#include "interrupts.h"
#include "timers.h"
#include "utils.h"
#include "i2c.h"
#include "color.h"

volatile unsigned int deltaTime;


// Defines
#define MOVES_ARRAY_SIZE 80
#define SPEED 6

#define LOW_POWER 3*SPEED
#define MED_POWER (unsigned char)(3.5*SPEED)
#define HIGH_POWER 4*SPEED

#define BUTTONF2 !PORTFbits.RF2
#define BUTTONF3 !PORTFbits.RF3

struct action{
    unsigned char type;
    unsigned int time;
};

void main(void){
    // Initialisation Function Calls
    LCD_Init();
    color_click_init();
    Interrupts_init();
    Timer_init();
    initDCmotorsPWM(10000);
    ADC_init();
    
    // Initialise RF2 as go button
    TRISFbits.TRISF2 = 1; // Set TRIS value for pin (input)
    ANSELFbits.ANSELF2 = 0; // Turn off analogue input on pin
    
    // Initialise RF3 as control button
    TRISFbits.TRISF3 = 1;
    ANSELFbits.ANSELF3 = 0;
    
    // Initialise LED for battery status
    TRISDbits.TRISD7 = 0;
    LATDbits.LATD7 = 0;

    unsigned char gain = 5;
    unsigned char minVal = 10;
    unsigned char minSat = 10;
    struct HSV colourCentres[8];
    
    // Initialise default values
    colourCentres[0] = *HSV(120,  60,  60);  // WHITE
    colourCentres[1] = *HSV(250, 150,  80);  // RED
    colourCentres[2] = *HSV(245,  40, 100);  // PINK
    colourCentres[3] = *HSV(0,   100, 100);  // ORANGE
    colourCentres[4] = *HSV(70,  100, 100);  // GREEN
    colourCentres[5] = *HSV(20,   80, 110);  // YELLOW
    colourCentres[6] = *HSV(130,  40, 100);  // LIGHT BLUE
    colourCentres[7] = *HSV(155, 110,  60);  // BLUE


    // buggy LEDs
    BRAKE_LIGHT_INIT = 0;
    LEFT_LIGHT_INIT = 0;
    RIGHT_LIGHT_INIT = 0;
    MAIN_BEAM_INIT = 0;
    BRAKE_LIGHT = 0;
    LEFT_LIGHT = 0;
    RIGHT_LIGHT = 0;
    MAIN_BEAM = 0;
    
    /*
     * CARPET SETTINGS:
     * leftTurnTime90 = 740;
     * rightTurnOffset = 0;
     * travelTime = 1500;
     * 
     * HARD FLOOR SETTINGS:
     * leftTurnTime90 = 740;
     * rightTurnOffset = 0;
     * travelTime = 1500;
     */
    
    unsigned char goFlag = 1;
    unsigned char colourState = 0;
    unsigned char previousState = 0;

    unsigned int leftTurnTime90 = 760;
    unsigned int rightTurnTime90 = 802;

    char buf[16];
    unsigned int moveCounter = 0;
    
    struct action moves[MOVES_ARRAY_SIZE];
    for(int i = 0; i < MOVES_ARRAY_SIZE; i++){
        moves[i].type = 3;  // Default action is white (ignores actions)
        moves[i].time = 0;  // Default action has zero duration
    }

    // Battery voltage sensing block. Lights LED if BatVolts is less than 3.75V
    
    while (1) { // Warn if battery is less than 3.75 V
        unsigned int value = ADC_getval() * 3;
        unsigned int int_part = (unsigned int)(value * (3.3/255));
        unsigned int frac_part = (unsigned int)(value * 100 * (3.3/255) - int_part * 100);
        sprintf(buf,"%d.%02d V          ", int_part, frac_part);
        LCD_sendstring(buf, 0, 0);
        if (value * (3.3/255) > 3.75) {
            break;
        }
        LATDbits.LATD7 = 1;
        LCD_sendstring(" LOW BATT.", 0, 6);
    }
    __delay_ms(1000);
    // define motor structures and populate fields.
    DC_motor motorL, motorR; 
    
    motorL.power = 0;                                       //zero power to start
    motorL.direction = 1;                                   //set default motor direction
    motorL.brakemode = 1;                                   // brake mode (slow decay)
    motorL.posDutyHighByte = (unsigned char *)(&CCPR1H);    //store address of CCP1 duty high byte
    motorL.negDutyHighByte = (unsigned char *)(&CCPR2H);    //store address of CCP2 duty high byte
    motorL.PWMperiod = T2PR;                                //store PWMperiod for motor (value of T2PR in this case)
   
    motorR.power = 0;                                       //zero power to start
    motorR.direction = 1;                                   //set default motor direction
    motorR.brakemode = 1;                                   // brake mode (slow decay)
    motorR.posDutyHighByte = (unsigned char *)(&CCPR4H);    //store address of CCP3 duty high byte
    motorR.negDutyHighByte = (unsigned char *)(&CCPR3H);    //store address of CCP4 duty high byte
    motorR.PWMperiod = T2PR;                                //store PWMperiod for motor (value of T2PR in this case)
    
    calibrateGainAndLED(&colourCentres[0], &gain);
    calibrateClear(gain, &minSat, &minVal);
    
    while(!BUTTONF3 && !BUTTONF2){  // Wait for input
        LCD_sendstring("<- Skip         ", 0, 0);
        LCD_sendstring("<- Calib. K-Mean", 1, 0);
        __delay_ms(100);
    }
    
    if (BUTTONF3){ 
        calibrateKMean(&colourCentres[0], gain);
    }
    
    while(BUTTONF3 || BUTTONF2){
        __delay_ms(100);
    }
    
    while(!BUTTONF3 && !BUTTONF2){  // Wait for input
        LCD_sendstring("<- START        ", 0, 0);
        LCD_sendstring("<- Calib. Motors", 1, 0);
        __delay_ms(100);
    }

    //ENTER MOTOR CALIBRATION MODE
    if (BUTTONF3){ 
        while(1){
            __delay_ms(1000);
            turnLeftDeg(&motorL, &motorR, leftTurnTime90, 90);
            LCD_sendstring("LEFT            ", 1, 0);
            while(!BUTTONF2){  // Wait for input
                __delay_ms(100);
            }
            __delay_ms(1000);
            turnRightDeg(&motorL, &motorR, rightTurnTime90, 90);
            LCD_sendstring("RIGHT           ", 1, 0);
            while(!BUTTONF2){  // Wait for input
                __delay_ms(100);
            }
        }
        //leftTurnTime90 = calibrateLeft(&motorL, &motorR, leftTurnTime90);
        //rightTurnTime90 = calibrateRight(&motorL, &motorR, rightTurnTime90);
    }
    
    //ENTER SPELUNKING MODE
    MAIN_BEAM = 1;
    __delay_ms(1000);
    deltaTime = 0; // Reset the timer
     
    // Navigate the maze.
    while (goFlag) {
        colourState = senseColour(colourCentres, gain, minSat, minVal); // Takes a long duration (~100ms)
        switch(colourState){
            case 0:
                forward(&motorL, &motorR, HIGH_POWER);
                break;
            case 1:
                forward(&motorL, &motorR, MED_POWER);
                break;
            case 2:
                forward(&motorL, &motorR, LOW_POWER);
                break;
            case 3:
                stop(&motorL, &motorR);  // Stop moving
                resetColourAveraging();
                goFlag = 0; // Exit while loop
                break;
            case 4:
                red(&motorL, &motorR, rightTurnTime90);
                break;
            case 5:
                pink(&motorL, &motorR, leftTurnTime90);
                break;
            case 6:
                orange(&motorL, &motorR, rightTurnTime90);
                break;
            case 7:
                yellow(&motorL, &motorR, rightTurnTime90);
                break;
            case 8:
                green(&motorL, &motorR, leftTurnTime90);
                break;
            case 9:
                light_blue(&motorL, &motorR, leftTurnTime90);
                break;
            case 10:
                blue(&motorL, &motorR, rightTurnTime90);
                break;        
        }
        if(colourState != previousState){
            moveCounter = moveCounter < (MOVES_ARRAY_SIZE - 1) ? (moveCounter + 1) : (MOVES_ARRAY_SIZE - 1);  // Keep within range
        }
        // Add the move to the moveArray
        if(colourState >= 4){
            stop(&motorL, &motorR); // TODO: Think carefully about where to put this
            resetColourAveraging();
            moves[moveCounter].type = colourState;
            moves[moveCounter].time = 0;
        } else if(colourState <= 2){
            moves[moveCounter].type = colourState;
            moves[moveCounter].time = deltaTime;
        }
        if(colourState != previousState){ // Reset the timer
            deltaTime = 0;
        }
        previousState = colourState;
    }
    __delay_ms(1000);
    
    // Navigate the maze in reverse
    for (int currentMoveIndex = (int)moveCounter; currentMoveIndex >= 0; currentMoveIndex--) // TODO: Should this be >= 0?
    {
        struct action currentAction = moves[currentMoveIndex];
        sprintf(buf, "Action #: %d     ", currentMoveIndex);
        LCD_sendstring(buf, 0, 0);

        switch (currentAction.type){
            case 0:
                for(deltaTime = 0; deltaTime < currentAction.time;){
                    reverse(&motorL, &motorR, HIGH_POWER);
                    sprintf(buf, "Time: %05d     ", deltaTime);
                    LCD_sendstring(buf, 1, 0);
                }
                stop(&motorL, &motorR);
                break;
            case 1:
                for(deltaTime = 0; deltaTime < currentAction.time;){
                    reverse(&motorL, &motorR, MED_POWER);
                    sprintf(buf, "Time: %05d     ", deltaTime);
                    LCD_sendstring(buf, 1, 0);
                }
                stop(&motorL, &motorR);
                break;
            case 2:
                for(deltaTime = 0; deltaTime < currentAction.time;){
                    reverse(&motorL, &motorR, LOW_POWER);
                    sprintf(buf, "Time: %05d    ", deltaTime);
                    LCD_sendstring(buf, 1, 0);
                }
                stop(&motorL, &motorR);
                break;
            case 3:
                // White: Ignore
                break;
            case 4: // Had seen Red
                turnLeftDeg(&motorL, &motorR, leftTurnTime90, 90);
                break;
            case 5: // Had seen Pink: turn right 90 then move forward 1 square 
                turnRightDeg(&motorL, &motorR, rightTurnTime90, 90);
                __delay_ms(100);
                forward_unit(&motorL, &motorR, 1.5);
                break;
            case 6: // Had seen Orange: turn left 135
                turnLeftDeg(&motorL, &motorR, leftTurnTime90, 135);
                break;
            case 7: // Had seen Yellow: turn left 90 then move forward 1 square 
                turnLeftDeg(&motorL, &motorR, leftTurnTime90, 90);
                __delay_ms(100);
                forward_unit(&motorL, &motorR, 1.5);
                break;
            case 8: // Had seen Green
                turnRightDeg(&motorL, &motorR, rightTurnTime90, 90);
                break;
            case 9: // Had seen Light Blue: turn right 135
                turnRightDeg(&motorL, &motorR, rightTurnTime90, 135);
                break;
            case 10: // Had seen Blue: turn 180
                turnRightDeg(&motorL, &motorR, rightTurnTime90, 180);
                break;
        }
    }
    stop(&motorL, &motorR);
    return;
}


/************************************
 * Description:
 * 
 * Inputs:
 * 
 * Outputs:
 * 
 ************************************/