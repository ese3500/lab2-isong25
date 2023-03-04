#include <avr/delay.h>
#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>

/*
 * PD6 (pin 6) = trigger
 * PD5 (pin 5) = buzzer
 * PD4 (pin 4) = button
 * ICP1 (pin 8) = echo
 * PA0 = photoresistor
 */

// timer 1 clock frequency is 2000000 (prescaler of 8)
int echoStart = 0;
int echoEnd = 0;
int ovfCount = 0;
double cm = 0.0;

void InitializeSensor() {
    cli();

    DDRB &= ~(1<<DDB0); // configure input capture pin ICP1
    DDRD |= (1<<DDD6); // configure output pin D6
    PORTD &= ~(1<<PORTD6);

    // timer 1 to normal mode
    TCCR1A &= ~(1<<WGM10);
    TCCR1A &= ~(1<<WGM11);
    TCCR1B &= ~(1<<WGM12);
    TCCR1B &= ~(1<<WGM13);

    // prescale timer clock by factor of 8
    TCCR1B &= ~(1<<CS12);
    TCCR1B |= (1<<CS11);
    TCCR1B &= ~(1<<CS10);

    TIMSK1 |= (1<<TOIE1); // enable timer overflow interrupt

    TIMSK1 |= (1<<ICIE1); // enable input capture
    TCCR1B |= (1<<ICES1); // detect rising edge
    TIFR1 |= (1<<ICF1); // clear interrupt flag

    sei();
}

void InitializePhotoresistor() {
    cli();

    PRR &= ~(1<<PRADC); // turn on ADC

    // prescale ADC timer by 128
    ADCSRA |= (1<<ADPS0);
    ADCSRA |= (1<<ADPS1);
    ADCSRA |= (1<<ADPS2);

    // AVcc as reference
    ADMUX |= (1<<REFS0);
    ADMUX &= ~(1<<REFS1);

    // input at ADC0
    ADMUX &= ~(1<<MUX3);
    ADMUX &= ~(1<<MUX2);
    ADMUX &= ~(1<<MUX1);
    ADMUX &= ~(1<<MUX0);

    // free running mode
    ADCSRB &= ~(1<<ADTS0);
    ADCSRB &= ~(1<<ADTS1);
    ADCSRB &= ~(1<<ADTS2);

    DIDR0 |= (1<<ADC0D); // digital input disable for ADC0 pin

    ADCSRA |= (1<<ADATE); // auto trigger enable
    ADCSRA |= (1<<ADEN); // ADC enable
    ADCSRA |= (1<<ADSC); // ADC start conversion

    sei();
}

void InitializeBuzzer() {
    cli();

    DDRD |= (1<<DDD5); // output on PD5 (OC0B) for buzzer
    DDRD &= ~(1<<DDD4); // input on PD4 for mode button

    // timer 0 to phase correct PWM mode
    TCCR0A |= (1<<WGM00);
    TCCR0A &= ~(1<<WGM01);
    TCCR0B |= (1<<WGM02);

    // prescale timer by 64
    TCCR0B &= ~(1<<CS02);
    TCCR0B |= (1<<CS01);
    TCCR0B |= (1<<CS00);

    OCR0A = 71;
    OCR0B = OCR0A * 0.5; // default about 50% duty cycle

    // clear when up counting, set when down counting
    TCCR0A |= (1<<COM0B1);
    TCCR0A &= ~(1<<COM0B0);

    sei();
}

ISR(TIMER1_OVF_vect) {
    if (PINB & (1<<PINB0)) {
        ovfCount++;
    }
}

ISR(TIMER1_CAPT_vect) {
    if (PINB & (1<<PINB0)) { // rising edge detected
        echoStart = ICR1;
        ovfCount = 0;
        TCCR1B &= ~(1<<ICES1); // detect falling edge
    } else { // falling edge detected
        echoEnd = ICR1;
        // 2 ticks per us, 0.0343 cm/us, extra division by 2 per datasheet
        cm = ((double)(echoEnd - echoStart) + ovfCount * 65536) / 116.618;
        ovfCount = 0;
        TCCR1B |= (1<<ICES1); // detect rising edge
    }
}

int getOCR0A[8] = {59, 62, 70, 79, 89, 94, 106, 119};

int setOC(void) {
    int calc;
    int adc = ADC;
    // tone (frequency)
    if (PIND & (1<<PIND4)) { // button pressed = continuous mode
        // continuous distance range from about 3-63cm
        calc = (int)cm + 59;
        if (calc > 119) {
            OCR0A = 119;
        } else {
            OCR0A = calc;
        }
    } else { // button not pressed = discrete mode
        // 8 discrete ranges in increments of 9cm from the sensor
        calc = (int)(cm / 9);
        if (calc >= 8) {
            OCR0A = 119;
        } else {
            OCR0A = getOCR0A[calc];
        }
    }
    // volume control (duty cycle)
    if (adc >= 1000) {
        OCR0B = OCR0A * 0.5; // max duty cycle of about 50%
    } else {
        OCR0B = OCR0A * ((double)(adc) / 2000.0); // continuous scale duty cycle
    }
}

int main(void) {
    InitializeSensor();
    InitializePhotoresistor();
    InitializeBuzzer();
    while (1) {
        while (!(TCCR1B & (1<<ICES1))); // wait till set to detect rising edge (wait till edge falls)
        setOC();
        _delay_ms(50);

        PORTD |= (1<<PORTD6);
        _delay_us(10);
        PORTD &= ~(1<<PORTD6);

        while ((TCCR1B & (1<<ICES1))); // wait till set to detect falling edge (wait till edge rises)
    }
}
