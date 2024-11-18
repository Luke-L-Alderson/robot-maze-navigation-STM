/* 
 * File:   i2c.h
 * Author: Thomas Groom
 *
 * Created on February 23, 2024
 */

#ifndef _i2c_H
#define _i2c_H
#define _XTAL_FREQ 64000000 //note intrinsic _delay function is 62.5ns at 64,000,000Hz  
#define _I2C_CLOCK 100000 //100kHz for I2C

#include <xc.h>

/********************************************//**
 *  Function to inialise I2C module and pins
 ***********************************************/
void I2C_2_Master_Init(void);

/********************************************//**
 *  Function to wait until I2C is idle
 ***********************************************/
unsigned char I2C_2_Master_Idle(void);

/********************************************//**
 *  Function to send start bit
 ***********************************************/
void I2C_2_Master_Start(void);

/********************************************//**
 *  Function to send repeated start bit
 ***********************************************/
void I2C_2_Master_RepStart(void);

/********************************************//**
 *  Function to send stop bit
 ***********************************************/
void I2C_2_Master_Stop(void);

/********************************************//**
 *  Function to send a byte on the I2C interface
 ***********************************************/
void I2C_2_Master_Write(unsigned char data_byte);

/********************************************//**
 *  Function to read a byte on the I2C interface
 ***********************************************/
unsigned char I2C_2_Master_Read(unsigned char ack);


#endif
