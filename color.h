/* 
 * File:   color.h
 * Author: Thomas Groom
 *
 * Created on February 23, 2024
 */

#ifndef _color_H
#define _color_H
#define _XTAL_FREQ 64000000

#include <xc.h>

// Definition of RGB structure
struct RGB { 
	unsigned char R;
	unsigned char G;
	unsigned char B;
    unsigned char C;
};

// Definition of HSV structure
struct HSV {
    unsigned char H;
    unsigned char S;
    unsigned char V;
};

struct HSV* HSV(unsigned char H, unsigned char S, unsigned char V);
char getIndexOfMax(void);
void color_click_init(void);  // Function to initialise the colour click module using I2C
void setLEDColor(int r, int g, int b);
void color_writetoaddr(char address, char value);  // Function to write to the colour click module address is the register within the colour click to write to value is the value that will be written to that address
unsigned int  color_readfromaddr(char address);
struct RGB color_read_all(unsigned char gain);  // Function to read the red channel. Returns a 16 bit ADC value representing colour intensity
struct HSV RgbToHsv(struct RGB rgb);
unsigned int euclidean_distance(unsigned char x1, unsigned char y1, unsigned char z1, unsigned char x2, unsigned char y2, unsigned char z2);
unsigned int HSV_Distance(struct HSV hsv1, struct HSV hsv2);
unsigned char segment(struct HSV* colourCentres, unsigned char minS, unsigned char minV, struct HSV col);
void resetColourAveraging(void);
void calibrateGainAndLED(struct HSV* colourCentres, unsigned char* gain);
void calibrateClear(unsigned char gain, unsigned char* minS, unsigned char* minV);
void calibrateKMean(struct HSV* colourCentres, unsigned char gain);
unsigned char senseColour(struct HSV colourCentres[], unsigned char gain, unsigned char minS, unsigned char minV);

#endif
