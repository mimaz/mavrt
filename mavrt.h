/*
 * Mieszko Mazurek <mimaz@gmx.com>
 *
 * Apr 2018
 */

#ifndef __MAVRT_H
#define __MAVRT_H

#include <stdint.h>
#include <avr/interrupt.h>

#define MAVRT_CONFIGURE(mode, vect, tcnt, tcntmax) \
    __attribute__((naked, interrupt)) \
    void vect(void) { __asm("jmp mavrt_systick"); } \
    extern volatile uint8_t mavrt_millis_lsb; \
    uint16_t mavrt_context_time(void) { \
        return tcnt + (uint16_t) mavrt_millis_lsb * tcntmax; } \
    __attribute__((constructor)) \
    void mavrt_configure(void); \
    void mavrt_initialize(void) { \
        __asm("cli"); mavrt_configure(); __asm("sei"); } \
    void mavrt_configure(void)

#define MAVRT_DEFINE_THREAD(name, ssiz) \
    mavrt_thread *volatile name; \
    uint8_t name##_stack[ssiz]; \
    void name##_handler2(void); \
    void name##_handler(void *unused) { \
        (void) unused; name##_handler2(); } \
    __attribute__((constructor)) \
    void name##_initializer(void) { \
        name = mavrt_launch(name##_handler, \
                NULL, name##_stack, ssiz); } \
    void name##_handler2(void)

#ifdef __cplusplus
extern "C" {
#endif
    
typedef struct mavrt_thread     mavrt_thread;

typedef void                  (*mavrt_handler)(void *);

extern mavrt_thread            *mavrt_context;


mavrt_thread   *mavrt_launch            (mavrt_handler  handler,
                                         void          *data,
                                         void          *memory,
                                         uint16_t       memsiz);

void            mavrt_exit              (void) 
                __attribute__((noreturn));

void            mavrt_schedule          (void);

void            mavrt_yield             (void);

uint32_t        mavrt_system_time       (void);

uint32_t        mavrt_time_millis       (void);

void            mavrt_sleep             (uint32_t delay);

void            mavrt_continue_sleep    (uint32_t delay);

void            mavrt_pause             (void);

void            mavrt_resume            (mavrt_thread *thread);

#ifdef __cplusplus
}
#endif

#endif
