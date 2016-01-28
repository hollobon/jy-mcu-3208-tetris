"""Compute lookup tables and generate header file"""

import sys
from math import cos, pi

def main():
    print "#ifndef LOOKUP_H"
    print "#define LOOKUP_H"
    steps = 16
    print
    print "#include <avr/pgmspace.h>"
    print
    print "const uint8_t _cos_s16_r1_15[] PROGMEM = {" + ", ".join(
        "%d" % round(1 + (1 - cos(pi * x / steps)) * 7)
        for x in range(steps)) + "};"
    print "#define cos_s16_r1_15(n) pgm_read_byte(&(_cos_s16_r1_15[(n)]))"
    print "#endif // LOOKUP_H"
    return 0

if __name__ == "__main__":
    sys.exit(main())