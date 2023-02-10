//
// Created by Isabel Song on 2/4/23.
//
// Turns on all 4 LEDs
#include <avr/io.h>
void q1(void) {
    DDRB |= 0x1E;
    PORTB |= 0x1E;
}