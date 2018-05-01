/*
 * Mieszko Mazurek <mimaz@gmx.com>
 *
 * Apr 2018
 */

#ifndef __MAVRT_H
#define __MAVRT_H

#include <stdint.h>
#include <avr/interrupt.h>

#define MAVRT_CONFIGURE(vect, tcnt, tcntmax) \
    __attribute__((naked, interrupt)) \
    void vect(void) { __asm("jmp mavrt_systick"); } \
    extern volatile uint32_t mavrt_timems; \
    extern volatile uint8_t mavrt_intflag; \
    __attribute__((constructor)) \
    static void mavrt_configure(void); \
    void mavrt_initialize(void) { \
        __asm("cli"); mavrt_configure(); __asm("sei"); } \
    static void mavrt_configure(void)

#define MAVRT_DEFINE_THREAD(name, ssiz) \
    mavrt_thread *volatile name; \
    uint8_t name##_stack[ssiz]; \
    void name##_handler(void); \
    __attribute__((constructor)) \
    void name##_initializer(void) { \
        name = mavrt_launch(name##_handler, \
                name##_stack, ssiz); } \
    void name##_handler(void)

#define MAVRT_UNLOCKED_MUTEX ((mavrt_mutex) 0)

#ifdef __cplusplus
extern "C" {
#endif
    
typedef struct mavrt_thread     mavrt_thread;

typedef void                  (*mavrt_handler)(void);

typedef uint8_t                 mavrt_mutex;


mavrt_thread   *mavrt_launch            (mavrt_handler  handler,
                                         void          *memory,
                                         uint16_t       memsiz);

void            mavrt_exit              (void) 
                __attribute__((noreturn));

void            mavrt_schedule          (void);

uint32_t        mavrt_system_time       (void);

uint32_t        mavrt_time              (void);

void            mavrt_sleep             (uint32_t delay);

void            mavrt_continue_sleep    (uint32_t delay);

void            mavrt_lock              (mavrt_mutex *mtx);

void            mavrt_unlock            (mavrt_mutex *mtx);

void            mavrt_pause             (void);

void            mavrt_resume            (mavrt_thread *thread);

#ifdef __cplusplus
}
#endif

#endif
