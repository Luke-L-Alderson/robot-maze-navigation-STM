/* 
 * File:   color.c
 * Author: Thomas Groom
 *
 * Created on February 23, 2024
 */

#include <xc.h>
#include <stdio.h>
#include "color.h"
#include "i2c.h"
#include "LCD.h"

// This array of char pointers helps convert between a numeric representation
// of the colour, and a visual representation for the LCD to display
static char* COLOUR[11] = { "-               ", "--              ", "---             ",
                            "White           ", "Red             ", "Pink            ",
                            "Orange          ", "Yellow          ", "Green           ",
                            "Light Blue      ", "Blue            " };

// This tally keeps track of the most likley colour based on the previous n samples
static unsigned char runningTallyCol[11] = {0,0,0,0,0,0,0,0,0,0,0};
volatile unsigned char RED_BRIGHTNESS = 160;    // The PWM of the Red LED
volatile unsigned char GREEN_BRIGHTNESS = 100;  // The PWM of the Green LED
volatile unsigned char BLUE_BRIGHTNESS = 255;   // The PWM of the Blue LED

/************************************
 * Description:
 * The constructor of the HSV structure
 * Inputs:
 * Hue, Saturation and Value as unsigned chars
 * Outputs:
 * A pointer to the memory location which now stores a new HSV structure
 ************************************/
struct HSV* HSV(unsigned char H, unsigned char S, unsigned char V) { 
    struct HSV* c = malloc(sizeof(struct HSV));  // Allocate memory equal to the memory size required of the structure
    c->H = H;  // Set the hue of this structure
    c->S = S;  // Set the saturation of this structure
    c->V = V;  // Set the value of this structure
    return c;
};

/************************************
 * Description:
 * Initialisation function for the colour click
 * The I2C is initialised and the desired gain and integration time settings are set
 ************************************/
void color_click_init(void) {
    // Set the red LED pin as output
    LATGbits.LATG0 = 0;
    TRISGbits.TRISG0 = 0;
    
    // Set the green LED pin as output
    LATEbits.LATE7 = 0;
    TRISEbits.TRISE7 = 0;
    
    // Set the blue LED pin as output
    LATAbits.LATA3 = 0;
    TRISAbits.TRISA3 = 0;
    
    // Setup colour sensor via I2C interface
    I2C_2_Master_Init();  // Initialise I2C as master
    __delay_ms(10);
    
    // This loop will inform the user that there is a problem with the colour sensor on the I2C bus
    while(I2C_2_Master_Idle()){
        LCD_sendstring("I2C Busy...     ", 0, 0);
    }
    
    // Set device PON
	color_writetoaddr(0x00, 0x01);
    __delay_ms(5);  // We need to wait 5ms for everything to start up
    color_writetoaddr(0x00, 0x03); // Turn on device ADC. Write 1 to the PON bit in the device enable register
    __delay_ms(5);
    color_writetoaddr(0x0F, 0x11); // Gain setting: 00 1× gain, 01 4× gain, 10 16× gain, 11 60× gain
    __delay_ms(5);
    color_writetoaddr(0x01, 0x90); // Integration time ATIME: 0xFF 2.4 ms, 0xF6 24 ms, 0xD5 101 ms, 0xC0 154 ms, 0x00 700 ms 
}

/************************************
 * Description:
 * Writes a value to the desired address of the colour click
 * Inputs:
 * The address byte to write too, and the value byte to write
 ************************************/
void color_writetoaddr(char address, char value) {
    I2C_2_Master_Start();                // Start condition
    I2C_2_Master_Write(0x52 | 0x00);     // 7 bit device address + Write mode
    I2C_2_Master_Write(0x80 | address);  // Command + register address
    I2C_2_Master_Write(value);           // Write the value byte
    I2C_2_Master_Stop();                 // Stop condition
}

/************************************
 * Description:
 * This function will read the colour sensor values for RGB and Clear
 * Inputs:
 * A gain value which scales the output to an appropriate range given the lighting conditions
 * Outputs:
 * An RGB structure holding the RGBC information as 8-bit values
 ************************************/
struct RGB color_read_all(unsigned char gain) {
    struct RGB out;
    
    // Read the clear sensor information
    unsigned int clear;
	I2C_2_Master_Start();               // Start condition
	I2C_2_Master_Write(0x52 | 0x00);    // 7 bit address + Write mode
	I2C_2_Master_Write(0xA0 | 0x14);    // Auto-increment protocol transaction + start at CLEAR low register
	I2C_2_Master_RepStart();			// Start a repeated transmission
	I2C_2_Master_Write(0x52 | 0x01);    // 7 bit address + Read (1) mode
	clear = I2C_2_Master_Read(1);		// Read the CLEAR LSB
	clear |= ((unsigned int)I2C_2_Master_Read(0)<<8);  // Read the CLEAR MSB (don't acknowledge as this is the last read)
	I2C_2_Master_Stop();                // Stop condition
    
    // Read the red sensor information
	unsigned int red;
	I2C_2_Master_Start();               // Start condition
	I2C_2_Master_Write(0x52 | 0x00);    // 7 bit address + Write mode
	I2C_2_Master_Write(0xA0 | 0x16);    // Auto-increment protocol transaction + start at RED low register
	I2C_2_Master_RepStart();			// Start a repeated transmission
	I2C_2_Master_Write(0x52 | 0x01);    // 7 bit address + Read (1) mode
	red = I2C_2_Master_Read(1);			// Read the RED LSB
	red |= ((unsigned int)I2C_2_Master_Read(0)<<8);  // Read the RED MSB (don't acknowledge as this is the last read)
	I2C_2_Master_Stop();                // Stop condition
	
    // Read the green sensor information
    unsigned int green;
	I2C_2_Master_Start();               // Start condition
	I2C_2_Master_Write(0x52 | 0x00);    // 7 bit address + Write mode
	I2C_2_Master_Write(0xA0 | 0x18);    // Auto-increment protocol transaction + start at GREEN low register
	I2C_2_Master_RepStart();			// Start a repeated transmission
	I2C_2_Master_Write(0x52 | 0x01);    // 7 bit address + Read (1) mode
	green = I2C_2_Master_Read(1);		// Read the GREEN LSB
	green |= ((unsigned int)I2C_2_Master_Read(0)<<8);  // Read the GREEN MSB (don't acknowledge as this is the last read)
	I2C_2_Master_Stop();                // Stop condition
    
    // Read the blue sensor information
    unsigned int blue;
	I2C_2_Master_Start();               // Start condition
	I2C_2_Master_Write(0x52 | 0x00);    // 7 bit address + Write mode
	I2C_2_Master_Write(0xA0 | 0x1A);    // Auto-increment protocol transaction + start at BLUE low register
	I2C_2_Master_RepStart();			// Start a repeated transmission
	I2C_2_Master_Write(0x52 | 0x01);    // 7 bit address + Read (1) mode
	blue = I2C_2_Master_Read(1);		// Read the BLUE LSB
	blue |= ((unsigned int)I2C_2_Master_Read(0)<<8);  // Read the BLUE MSB (don't acknowledge as this is the last read)
	I2C_2_Master_Stop();                // Stop condition
    
    // Scale the values by the gain amount and cast to 8-bits
    out.R = (unsigned char)(red >> (gain + 1));  // Devision by 2 as RED channel is more sensitive
    out.G = (unsigned char)(green >> gain);
    out.B = (unsigned char)(blue >> gain);
    out.C = (unsigned char)(clear >> (gain + 2));  // Division by 4 as CLEAR channel is more sensitive
    
    // Make sure values don't overflow but instead max out at 255
    if((red >> (8 + (gain + 1))) > 0){
        out.R = 0xFF;
    }
    if((green >> (8 + gain)) > 0){
        out.G = 0xFF;
    }
    if((blue >> (8 + gain)) > 0){
        out.B = 0xFF;
    }
    if((clear >> (8 + (gain + 2))) > 0){
        out.C = 0xFF;
    }
    return out;
}

/************************************
 * Description:
 * Return the index of the largest value in runningTallyCol
 * Used to average over the colours detected
 * Outputs:
 * The index corresponding to colour with the highest votes
 ************************************/
char getIndexOfMax(void) {
    unsigned char index = 0;
    unsigned char max = runningTallyCol[0];
    // Loop over all tally heaps in the array
    for (unsigned char i = 0; i < 11; i++){
        // If this heap is larger than the largest heap so far
        if (runningTallyCol[i] > max) {
            max = runningTallyCol[i];  // Set this heap as the new target to beat
            index = i;  // Set the index of this heap
        }
    }
    return index;
}

/************************************
 * Description:
 * This function converts RGBC values into the HSV colour space
 * Modified version of: https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
 * Inputs:
 * An RGB value
 * Outputs:
 * An HSV structure containing the converter output
 ************************************/
struct HSV RgbToHsv(struct RGB rgb) {
    struct HSV hsv;
    unsigned char rgbMin, rgbMax;
    
    // Determine the minimum and maximum value between R, G and B
    rgbMin = rgb.R < rgb.G ? (rgb.R < rgb.B ? rgb.R : rgb.B) : (rgb.G < rgb.B ? rgb.G : rgb.B);
    rgbMax = rgb.R > rgb.G ? (rgb.R > rgb.B ? rgb.R : rgb.B) : (rgb.G > rgb.B ? rgb.G : rgb.B);
    
    // Value is simply the clear RGBC channel
    hsv.V = rgb.C;
    if (hsv.V == 0) {
        // If the value is 0 (black), then the hue and saturation are irrelevant and will both be set to 0
        hsv.H = 0;
        hsv.S = 0;
        return hsv;
    }
    
    // Saturation is the difference between the min and max as a "percentage" of the value of the colour
    hsv.S = (unsigned char)(255 * (long)(rgbMax - rgbMin) / (255 - abs(2*hsv.V - 255)));
    if (hsv.S == 0) {
        // If the saturation is 0 (grey scale), then the hue is irrelevant and therefore set to 0 also
        hsv.H = 0;
        return hsv;
    }

    // Hue is the scaled combination of the three RGB values at 120 degrees apart
    if (rgbMax == rgb.R)
        hsv.H = 0 + 43 * (rgb.G - rgb.B) / (rgbMax - rgbMin);
    else if (rgbMax == rgb.G)
        hsv.H = 85 + 43 * (rgb.B - rgb.R) / (rgbMax - rgbMin);
    else
        hsv.H = 171 + 43 * (rgb.R - rgb.G) / (rgbMax - rgbMin);

    return hsv;
}

/************************************
 * Description:
 * Returns the square of the Euclidean distance between point (x1, y1, z1) and
 * (x2, y2, z2) as a 16 bit unsigned integer
 * Inputs:
 * Two sets of x,y,z coordinates
 * Outputs:
 * The distance between the given points
 ************************************/
unsigned int euclidean_distance(unsigned char x1, unsigned char y1, unsigned char z1, unsigned char x2, unsigned char y2, unsigned char z2) {
    // Calculating the sum of squares of the differences between the points
    unsigned int x_sq = (unsigned int)((int)(x2 - x1) * (int)(x2 - x1));
    unsigned int y_sq = (unsigned int)((int)(y2 - y1) * (int)(y2 - y1));
    unsigned int z_sq = (unsigned int)((int)(z2 - z1) * (int)(z2 - z1));
    
    // Scaling the values so that their sum fits into 16 bits
    x_sq = x_sq >> 2;
    y_sq = y_sq >> 2;
    z_sq = z_sq >> 2;
    
    return x_sq + y_sq + z_sq;    
}

/************************************
 * Description:
 * Returns the distance between two HSV values in the HSV colour space taking the wrapping from 360->0 into account
 * Inputs:
 * Two HSV colours
 * Outputs:
 * The smallest distance between these HSV values in HSV colour space
 ************************************/
unsigned int HSV_Distance(struct HSV hsv1, struct HSV hsv2) {
    hsv1.S >>= 1; // Make saturation play less of a role in the distance calculation
    hsv2.S >>= 1;
    hsv1.V >>= 4; // Make value play less of a role in the distance calculation
    hsv2.V >>= 4;
    unsigned int dist1 = euclidean_distance(hsv1.H, hsv1.S, hsv1.V, hsv2.H, hsv2.S, hsv2.V);
    unsigned int dist2;
    if(hsv1.H < hsv2.H){
        dist2 = euclidean_distance(0, hsv1.S, hsv1.V, hsv1.H + (255 - hsv2.H), hsv2.S, hsv2.V);
    } else {
        dist2 = euclidean_distance(0, hsv1.S, hsv1.V, hsv2.H + (255 - hsv1.H), hsv2.S, hsv2.V);
    }
    return dist1 < dist2 ? dist1 : dist2;  // Return the lowest distance
}

/************************************
 * Description:
 * The function will find the best fitting estimate for the given colour sensor
 * measurement using the HSV_Distance function
 * Inputs:
 * Settings for the colour centres and min saturation and min value to segment
 * black and white colours
 * Outputs:
 * The colour/proximity represented as a numeric value
 ************************************/
unsigned char segment(struct HSV colourCentres[], unsigned char minS, unsigned char minV,  struct HSV col) {
    unsigned char colour_out;
    unsigned int minDist = 65535;
    
    unsigned int proximity = ((col.S * col.S) >> 2) + ((col.V * col.V) >> 2);
    unsigned int proximity1 = ((minS * minS) >> 2) + ((minV * minV) >> 2) + 100;
    unsigned int proximity2 = proximity1 + 400;
    if (proximity < proximity1) {
        colour_out = 0; // BLACK (no proximity)
    } else if (proximity < proximity2) {
        colour_out = 1; // Low proximity
    } else {
        colour_out = 2; // High proximity
    }
    
    // As blue is very dark, it is the only colour that is not clipped by the global min value limit
    if(HSV_Distance(colourCentres[7], col) < 100){ // Within acceptable range of colour centre
        colour_out = 10;
    }

    if(col.V > minV){
        if(col.S > minS){
            for (unsigned char i = 0; i < 8; i++){
                unsigned int dist = HSV_Distance(colourCentres[i], col);
                if(dist < minDist){
                    minDist = dist;
                    if(minDist < 300){ // Within acceptable range of colour centre
                        colour_out = i + 3;
                    }
                }
            }
        } else {
            if(col.V > 105){
                colour_out = 3; // WHITE
            }
        }
    }
    return colour_out;
    
}

/************************************
 * Description:
 * This function will calibrate the gain, LED colour and white k-mean centre colours
 * Inputs:
 * A pointer to the colourCenter variable to calibrate, and the pointer to the gain to calibrate
 ************************************/
void calibrateGainAndLED(struct HSV* colourCentres, unsigned char* gain){
    char buf[16];

    struct RGB colRGB;
    struct HSV colHSV;
    
    // GAIN CALIBRATION AND
    // LED COLOUR CALIBRATION
    while(PORTFbits.RF2){
        unsigned char rgbMax = colRGB.R > colRGB.G ? (colRGB.R > colRGB.B ? colRGB.R : colRGB.B) : (colRGB.G > colRGB.B ? colRGB.G : colRGB.B);
        unsigned char max = rgbMax > colRGB.C ? rgbMax : colRGB.C;
        if(max >= 255){
            (*gain)+=1;
        }
        colRGB = color_read_all(*gain);
        if((int)(colRGB.B - colRGB.R) > 3 && RED_BRIGHTNESS < 255){
            RED_BRIGHTNESS++;
        } else if ((int)(colRGB.B - colRGB.R) < -3 && RED_BRIGHTNESS > 0){
            RED_BRIGHTNESS--;
        }
        if((int)(colRGB.B - colRGB.G) > 3 && GREEN_BRIGHTNESS < 255){
            GREEN_BRIGHTNESS++;
        } else if ((int)(colRGB.B - colRGB.G) < -3 && GREEN_BRIGHTNESS > 0){
            GREEN_BRIGHTNESS--;
        }
        *(colourCentres) = RgbToHsv(colRGB);
        sprintf(buf, "WHITE   Gain: %01d ", *gain);
        LCD_sendstring(buf, 0, 0);
        sprintf(buf, "RGB: %03d %03d %03d", RED_BRIGHTNESS, GREEN_BRIGHTNESS, BLUE_BRIGHTNESS);
        LCD_sendstring(buf, 1, 0);
    }
    while(!PORTFbits.RF2){
        __delay_ms(100);
    }
}

/************************************
 * Description:
 * The function will calibrate the min saturation and min value variables by measureing
 * the colour sensor when no colour card is present
 * Inputs:
 * Gain and variables to calibrate
 ************************************/
void calibrateClear(unsigned char gain, unsigned char* minS, unsigned char* minV){
    char buf[16];
    struct RGB colRGB;
    struct HSV colHSV;
    
    while(PORTFbits.RF2){
        colRGB = color_read_all(gain);
        colHSV = RgbToHsv(colRGB);
        // Add an offset to ensure no colour is measured sporadically
        *minV = colHSV.V < 240 ? colHSV.V + 10 : 250;
        *minS = colHSV.S < 240 ? colHSV.S + 10 : 250;
        
        // Show these values on the LCD
        LCD_sendstring("CLEAR Calibrat.", 0, 0);
        sprintf(buf, "Min S: %02d V: %02d ", *minS, *minV);
        LCD_sendstring(buf, 1, 0);
    }
    while(!PORTFbits.RF2){
        __delay_ms(100);
    } 
}

/************************************
 * Description:
 * Calibrates the colour centres by cycling through all colours
 * Inputs:
 * The colour centre variable to calibrate and the gain
 ************************************/
void calibrateKMean(struct HSV* colourCentres, unsigned char gain){
    struct HSV colHSV;
    char buf[16];

    for(unsigned char currentColour = 1; currentColour < 8; currentColour++){
        while(PORTFbits.RF2){
            colHSV = RgbToHsv(color_read_all(gain));
            *(colourCentres + currentColour) = colHSV;
            LCD_sendstring("K-Mean ", 0, 0);
            LCD_sendstring(COLOUR[currentColour + 3], 0, 7);
            sprintf(buf, "HSV: %03d %03d %03d", colHSV.H, colHSV.S, colHSV.V);
            LCD_sendstring(buf, 1, 0);
        }
        while(!PORTFbits.RF2){
            __delay_ms(100);
        }
        while(PORTFbits.RF2){
            LCD_sendstring("HOLD   ", 0, 0);
            __delay_ms(100);
        }
        while(!PORTFbits.RF2){
            __delay_ms(100);
        }
    }
}

/************************************
 * Description:
 * Resets all heaps in the tally array to 0
 ************************************/
void resetColourAveraging(void){
    runningTallyCol[0] = 8;
    for (int i = 1; i < 11; i++){
        runningTallyCol[i] = 0;
    }
}

/************************************
 * Description:
 * Used to sense and average the result post segmentation
 * Inputs:
 * All calibration values
 * Outputs:
 * The averaged colour/proximity represented as a numeric value
 ************************************/
unsigned char senseColour(struct HSV* colourCentres, unsigned char gain, unsigned char minS, unsigned char minV){
    char buf[16];
    
    struct RGB colRGB;
    struct HSV colHSV;
    
    // Read the colour sensor value and convert to HSV colour space
    colRGB = color_read_all(gain);
    colHSV = RgbToHsv(colRGB);

    unsigned char colour_index = segment(colourCentres, minS, minV,  colHSV);
    // sprintf(buf,"%03d %03d %03d %03d", colRGB.R, colRGB.G, colRGB.B, colRGB.C);
    sprintf(buf,"HSV %03d %03d %03d ", colHSV.H, colHSV.S, colHSV.V);

    // Averaging
    if(runningTallyCol[colour_index] < 8){ // This value /2 is the number of measurements to average over
        runningTallyCol[colour_index] += 2;
    }
    
    // Lower all votes in the tally by 1 to allow forgetting
    for (int i = 0; i < 11; i++){
        if(runningTallyCol[i] > 0){
            runningTallyCol[i]--;
        }
    }

    // Get the most voted colour
    unsigned char colour_out = getIndexOfMax();
    
    // Display what the best guess for the colour is on the LCD
    LCD_sendstring(buf, 0, 0);
    LCD_sendstring(COLOUR[colour_out], 1, 0);
    
    return colour_out;
}
