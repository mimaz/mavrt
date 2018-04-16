/*
 * Mieszko Mazurek <mimaz@gmx.com>
 *
 * Apr 2018
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/delay.h>
#include <stdlib.h>

#include "mavrt.h"

MAVRT_CONFIG(MAVRT_250HZ, TIMER2_COMPA_vect, TCNT2, 250)

void delay(uint16_t ms)
{
    uint32_t timeout = mavrt_time_millis() + ms;

    while (mavrt_time_millis() < timeout)
        mavrt_yield();
}

mavrt_thread *volatile task1, *volatile task2, *volatile task3;

void blink1(void *data)
{
    mavrt_pause(mavrt_current());

    while (1)
    {
        MAVRT_NO_SCHEDULE(PORTB |= _BV(5))
        delay(250);
        MAVRT_NO_SCHEDULE(PORTB &= ~_BV(5))
        delay(250);
    }
}

void blink2(void *data)
{
    while (1)
    {
        MAVRT_NO_SCHEDULE(PORTB |= _BV(4))
        delay(49);
        MAVRT_NO_SCHEDULE(PORTB &= ~_BV(4))
        delay(951);

        if (mavrt_time_millis() > 3000)
            mavrt_resume(task1);
    }
}

void blink3(void *data)
{
    while (1)
    {
        MAVRT_NO_SCHEDULE(PORTB |= _BV(3))
        delay(279);
        MAVRT_NO_SCHEDULE(PORTB &= ~_BV(3))
        delay(53);
    }
}

int main()
{
    static uint8_t stack[256];
    static uint8_t stack1[256];

    cli();
    DDRB = 0xFF;
    PORTB = 0;

    TCCR2A = _BV(WGM21);
    TCCR2B = _BV(CS22) | _BV(CS21);
    TIMSK2 = _BV(OCIE2A);
    OCR2A = 250;
    sei();

    task1 = mavrt_current();
    task2 = mavrt_launch(blink2, NULL, stack, sizeof(stack));
    task3 = mavrt_launch(blink3, NULL, stack1, sizeof(stack1));

    blink1((void *) 250);

    return 0;
}
