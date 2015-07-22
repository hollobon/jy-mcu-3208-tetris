/* Driver for the HT1632C LED driver
 *
 * Code from the clock example by "DrJones" at http://club.dx.com/forums/forums.dx/threadid.1118199?page=1
 */

#ifndef HT1632C_H
#define HT1632C_H

#include <stdint.h>

#include <avr/io.h>

#define byte uint8_t
#define word uint16_t

#define HTport   PORTB
#define HTddr    DDRB
#define HTstrobe 3
#define HTclk    4
#define HTdata   5

#define HTclk0    HTport &= ~(1 << HTclk)
#define HTclk1    HTport |= (1 << HTclk)
#define HTstrobe0 HTport &= ~(1 << HTstrobe)
#define HTstrobe1 HTport |= (1 << HTstrobe)
#define HTdata0   HTport &= ~(1 << HTdata)
#define HTdata1   HTport |= (1 << HTdata)
#define HTpinsetup() do{  HTddr |= (1 << HTstrobe) | (1 << HTclk) | (1 << HTdata); HTport |= (1 << HTstrobe) | (1 << HTclk) | (1 << HTdata);  }while(0)
        // set as output and all high

byte leds[32];  //the screen array, 1 byte = 1 column, left to right, lsb at top.

#define HTstartsys   0b100000000010 //start system oscillator
#define HTstopsys    0b100000000000 //stop sytem oscillator and LED duty    <default
#define HTsetclock   0b100000110000 //set clock to master with internal RC  <default
#define HTsetlayout  0b100001000000 //NMOS 32*8 // 0b100-0010-ab00-0  a:0-NMOS,1-PMOS; b:0-32*8,1-24*16   default:ab = 10
#define HTledon      0b100000000110 //start LEDs
#define HTledoff     0b100000000100 //stop LEDs    <default
#define HTsetbright  0b100101000000 //set brightness b = 0..15  add b << 1  //0b1001010xxxx0 xxxx:brightness 0..15 = 1/16..16/16 PWM
#define HTblinkon    0b100000010010 //Blinking on
#define HTblinkoff   0b100000010000 //Blinking off  <default
#define HTwrite      0b1010000000   // 101-aaaaaaa-dddd-dddd-dddd-dddd-dddd-... aaaaaaa:nibble adress 0..3F   (5F for 24*16)

void HTsetup(void);             // set up the display
void HTbrightness(byte b);      // set brightness, 0 <= b <= 15
void HTsendscreen(void);        // send the screen, contained in the leds array

#endif   /* HT1632C_H */
