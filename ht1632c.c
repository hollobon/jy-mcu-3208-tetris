/* Driver for the HT1632C LED driver
 *
 * Code from the clock example by "DrJones" at http://club.dx.com/forums/forums.dx/threadid.1118199?page=1
 */

#include "ht1632c.h"

// ADDRESS: MSB first
// DATA: LSB first     transferring a byte (msb first) fills one row of one 8*8-matrix, msb left, starting with the left matrix
// timing: pull strobe LOW, bits evaluated at rising clock edge, strobe high
// commands can be queued: 100-ccccccccc-ccccccccc-ccccccccc-... (ccccccccc: without 100 at front)
// setup: cast startsys, setclock, setlayout, ledon, brightness+(15 << 1), blinkoff

void HTsend(word data, byte bits)
{
    // MSB first
    word bit = ((word)1) << (bits-1);
    while (bit) {
        HTclk0;
        if (data & bit)
            HTdata1;
        else
            HTdata0;
        HTclk1;
        bit >>= 1;
    }
}

void HTcommand(word data)
{
    HTstrobe0;
    HTsend(data,12);
    HTstrobe1;
}

void HTsendscreen(void)
{
    HTstrobe0;
    HTsend(HTwrite,10);
    for (byte mtx = 0; mtx < 4; mtx++)        //sending 8x8-matrices left to right, rows top to bottom, MSB left
        for (byte row = 0; row < 8; row++) {  //while leds[] is organized in columns for ease of use.
            byte q = 0;
            for (byte col = 0; col < 8; col++)
                q = (q << 1) | ((leds[col + (mtx << 3)] >> row) & 1);
            HTsend(q, 8);
        }
    HTstrobe1;
}

void HTsetup()
{  //setting up the display
    HTcommand(HTstartsys);
    HTcommand(HTledon);
    HTcommand(HTsetclock);
    HTcommand(HTsetlayout);
    HTcommand(HTsetbright + (8 << 1));
    HTcommand(HTblinkoff);
}

void HTbrightness(byte b)
{
    HTcommand(HTsetbright + ((b & 15) << 1));
}
