/*
 * Mieszko Mazurek <mimaz@gmx.com>
 *
 * Apr 2018
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdlib.h>

#include "mavrt.h"

MAVRT_CONFIGURE(MAVRT_250HZ, TIMER2_COMPA_vect, TCNT2, 250)
{
    TCCR2A = _BV(WGM21);
    TCCR2B = _BV(CS22) | _BV(CS21);
    TIMSK2 = _BV(OCIE2A);
    OCR2A = 250;

    DDRB = 0xFF;
    PORTB = 0;
}

MAVRT_DEFINE_THREAD(task1, 256)
{
    mavrt_pause();

    while (1)
    {
        MAVRT_NO_SCHEDULE(PORTB |= _BV(5))
        mavrt_continue_sleep(250);
        MAVRT_NO_SCHEDULE(PORTB &= ~_BV(5))
        mavrt_continue_sleep(250);
    }
}

MAVRT_DEFINE_THREAD(blink2, 256)
{
    while (1)
    {
        MAVRT_NO_SCHEDULE(PORTB |= _BV(4))
        mavrt_continue_sleep(49);
        MAVRT_NO_SCHEDULE(PORTB &= ~_BV(4))
        mavrt_continue_sleep(951);

        if (mavrt_time_millis() > 5000)
            mavrt_resume(task1);
    }
}

MAVRT_DEFINE_THREAD(blink3, 256)
{
    while (1)
    {
        MAVRT_NO_SCHEDULE(PORTB |= _BV(3))
        mavrt_continue_sleep(279);
        MAVRT_NO_SCHEDULE(PORTB &= ~_BV(3))
        mavrt_continue_sleep(53);
    }
}
