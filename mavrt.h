/*
 * Mieszko Mazurek <mimaz@gmx.com>
 *
 * Apr 2018
 */

#ifndef __MAVRT_H
#define __MAVRT_H

#include <stdint.h>
#include <avr/interrupt.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAVRT_1000HZ 0
#define MAVRT_500HZ 1
#define MAVRT_250HZ 2
#define MAVRT_125HZ 3

#define MAVRT_CONFIG(mode, vect, tcnt, tcntmax) \
    __attribute__((naked)) \
    void vect(void) { __asm("sei"); __asm("jmp mavrt_systick"); } \
    extern volatile uint8_t mavrt_millis_lsb; \
    uint16_t mavrt_context_time(void) { \
        return tcnt + (uint16_t) mavrt_millis_lsb * tcntmax; } \
    uint32_t mavrt_time_millis(void) { \
        return mavrt_system_time() << mode; }

#define MAVRT_ATOMIC(expr) { \
    mavrt_enter_critical(); \
    { expr; } \
    mavrt_exit_critical(); }

typedef struct mavrt_thread     mavrt_thread;

typedef void                  (*mavrt_handler)(void *);


mavrt_thread   *mavrt_current           (void);

mavrt_thread   *mavrt_launch            (mavrt_handler  handler,
                                         void          *data,
                                         void          *memory,
                                         uint16_t       memsiz);

void            mavrt_exit              (void) __attribute__((noreturn));

void            mavrt_enter_critical    (void);

void            mavrt_exit_critical     (void);

void            mavrt_yield             (void);

uint32_t        mavrt_system_time       (void);

uint32_t        mavrt_time_millis       (void);

void            mavrt_kill              (mavrt_thread *thread);

void            mavrt_pause             (mavrt_thread *thread);

void            mavrt_resume            (mavrt_thread *thread);

uint8_t         mavrt_is_paused         (mavrt_thread *thread);

uint8_t         mavrt_is_killed         (mavrt_thread *thread);

#ifdef __cplusplus
}
#endif

#endif
