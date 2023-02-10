//
// Created by Isabel Song on 2/4/23.
//
#include <avr/io.h>
#include <util/delay.h>

void q3(void) {
    int ctr = 0;
    DDRD &= ~(1<<DDD7); // configure input pin
    PORTD |= (1<<PORTD7); // pull high
    DDRB |= 0x1E; // configure output pins
    PORTB = 0x00;
    PORTB |= (1<<PORTB1); // start first LED on
    while (1) {
        while (!(PIND & (1 << PIND7))) {}
        ctr = (ctr + 1) % 4;
        switch(ctr) {
            case 0:
                PORTB = 0x02;
                break;
            case 1:
                PORTB = 0x04;
                break;
            case 2:
                PORTB = 0x08;
                break;
            case 3:
                PORTB = 0x10;
                break;
        }
        _delay_ms(500);
    }
}