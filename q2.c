//
// Created by Isabel Song on 2/4/23.
//
// Turns on LED when button pressed, off when button released
#include <avr/io.h>
void q2(void) {
    DDRD &= ~(1<<DDD7);
    PORTD |= (1<<PORTD7);
    DDRB |= (1<<DDB1);
    while(1) {
        if (PIND & (1<<PIND7)) { // button pressed
            PORTB |= (1<<PORTB1);
        } else { // button not pressed
            PORTB &= ~(1<<PORTB1);
        }
    }
}