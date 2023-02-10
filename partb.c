//
// Created by Isabel Song on 2/4/23.
//

#include <avr/io.h>
#include <avr/interrupt.h>

void Initialize() {
    cli();

    DDRB |= (1<<DDB5); // output pin
    DDRB &= ~(1<<DDB0); // (ICP1) input pin

    TCCR1A &= ~(1<<WGM10);
    TCCR1A &= ~(1<<WGM11);
    TCCR1B &= ~(1<<WGM12);
    TCCR1B &= ~(1<<WGM13);

    TIMSK1 |= (1<<ICIE1); // enable interrupt capture
    TCCR1B |= (1<<ICES1); // detect rising edge
    TIFR1 |= (1<<ICF1); // clear interrupt flag

    sei();
}

ISR(TIMER1_CAPT_vect) {
    TIFR1 |= (1<<ICF1); // clear interrupt flag
    if (PINB & (1<<PINB0)) {
        PORTB |= (1<<PORTB5); // turn on LED
        TCCR1B &= ~(1<<ICES1); // detect falling edge
    } else {
        PORTB &= ~(1<<PORTB5); // turn off LED
        TCCR1B |= (1<<ICES1); // detect rising edge
    }
}

void partb(void) {
    Initialize();
    while (1) {}
}