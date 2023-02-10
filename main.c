#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "uart.h"
#include <util/delay.h>

#define F_CPU 16000000UL
#define BAUD_RATE 9600
#define BAUD_PRESCALER (((F_CPU / (BAUD_RATE * 16UL))) - 1)

// - is just root, # are placeholders, ! is out of bounds
const char tree[] = {'-', 'E', 'T', 'I', 'A', 'N', 'M', 'S', 'U', 'R', 'W',
                    'D', 'K', 'G', 'O', 'H', 'V', 'F', '#', 'L',
                    '#', 'P', 'J', 'B', 'X', 'C', 'Y', 'Z', 'Q',
                    '#', '#', '5', '4', '#', '3', '#', '#',
                    '#', '2', '#', '#', '#', '#', '#', '#',
                    '#', '1', '6', '#', '#', '#', '#', '#', '#',
                    '#', '7', '#', '#', '#', '8', '#', '9', '0',
                    '!'};

#define CLK_FREQ 62500
int rise = 0;
int fall = -1;
int ovfCount = 0;
float lowTime = 0.0;
float highTime = 0.0;
int current = 0; // where in character tree, start at root

char String[25];

int addDot(int i) { // left child is dot
    if (2 * (i + 1) <= 61) {
        return 2 * i + 1;
    }
    return 62; // '!' in tree = out of bounds
}

int addDash(int i) { // right child is dash
    if (2 * (i + 1) + 1 <= 61) {
        return 2 * (i + 1);
    }
    return 62; // '!' in tree = out of bounds
}

void Initialize() {
    cli();

    DDRB &= ~(1<<DDB0); // (ICP1) input pin

    // TOTAL PRESCALING: divide by 256 --> frequency = 62.5 kHz
    // prescale timer clock by factor of 256
    TCCR1B |= (1<<CS12);
    TCCR1B &= ~(1<<CS11);
    TCCR1B &= ~(1<<CS10);

    // normal mode
    TCCR1A &= ~(1<<WGM10);
    TCCR1A &= ~(1<<WGM11);
    TCCR1B &= ~(1<<WGM12);
    TCCR1B &= ~(1<<WGM13);

    TIMSK1 |= (1<<TOIE1); // enable timer overflow interrupt
    TCCR1B |= (1<<ICES1); // detect rising edge
    TIFR1 |= (1<<ICF1); // clear input capture flag
    TIMSK1 |= (1<<ICIE1); // enable interrupt capture

    sei();
}

ISR(TIMER1_OVF_vect) {
    // flag cleared automatically
    ovfCount++;
}

ISR(TIMER1_CAPT_vect) {
    cli();
    if (PINB & (1<<PINB0)) { // caught rising edge
        rise = ICR1; // record timer value of rising edge
        TIFR1 |= (1<<ICF1); // clear interrupt flag
        // compare fall to rise time
        lowTime = (float)(rise - fall) * 1000 / CLK_FREQ + ovfCount * 1000; // how long button not pressed (ms)
        if (lowTime >= 800 && fall >= 0) {
            // space = not pressed for at least 400 x 2 ms
            sprintf(String, "%c\n", tree[current]);
            UART_putstring(String);
            current = 0;
        }
        ovfCount = 0; // reset overflow count
        _delay_ms(50);
        sei();
        TCCR1B &= ~(1<<ICES1); // detect falling edge
    } else { // caught falling edge
        fall = ICR1; // record timer value of falling edge
        TIFR1 |= (1<<ICF1); // clear interrupt flag
        // compare rise to fall time
        highTime = (float)(fall - rise) * 1000 / CLK_FREQ + ovfCount * 1000; // how long button pressed (ms)
        if (highTime >= 400 && highTime <= 800) {
            // dash = pressed for 200x2 ms to 400x2 ms
            current = addDash(current);
        } else if (highTime >= 30 && highTime <= 400) {
            // dot = pressed for 30 ms to 200 x 2 ms
            current = addDot(current);
        }
        else {
            current = 0;
        }
        ovfCount = 0; // reset overflow count
        _delay_ms(50);
        sei();
        TCCR1B |= (1<<ICES1); // detect rising edge
    }
}

int main(void)
{
    Initialize();
    UART_init(BAUD_PRESCALER);
    sprintf(String, "%s\n", "\n\n\n\n\n");
    UART_putstring(String);
    while (1) {}
}