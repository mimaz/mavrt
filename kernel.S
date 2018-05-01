/*
 * Mieszko Mazurek <mimaz@gmx.com>
 *
 * Apr 2018
 */

SPL     = 0x3D
SPH     = 0x3E
SREG    = 0x3F

rzero   = 1

        .section .bss

        .global mavrt_timems
        .global mavrt_intflag
        .extern mavrt_tcntmax

mavrt_timems:
timems:
        .byte   0, 0, 0, 0

mavrt_intflag:
intflag:
        .byte   0

        .section .text

        .global main
        .weak main

        .global mavrt_launch
        .global mavrt_time
        .global mavrt_schedule
        .global mavrt_lock
        .global mavrt_unlock
        .global mavrt_systick

main:
        sei
        jmp     mavrt_exit

        ; r24:r25 handler pointer
        ; r22:r23 memory
        ; r20:r21 memory size
mavrt_launch:
        movw    r30,    r22
        add     r30,    r20
        adc     r31,    r21

        ldi     r18,    pm_lo8(.child)
        ldi     r19,    pm_hi8(.child)

        ; handler
        st      -Z,     r24
        st      -Z,     r25

        ; return address
        st      -Z,     r18
        st      -Z,     r19

        ; fixed + offset
        sbiw    r30,    16+1 

        movw    r24,    r22
        movw    r22,    r30

        jmp     mavrt_register

.child:
        pop     r25
        pop     r24

        icall

        jmp     mavrt_exit

mavrt_time:
        sts     intflag,    rzero

        lds     r22,    timems+0
        lds     r23,    timems+1
        lds     r24,    timems+2
        lds     r25,    timems+3

        lds     r20,    intflag
        cpse    r20,    rzero
        rjmp    mavrt_time
        ret

mavrt_schedule:
        in      r24,    SPL
        in      r25,    SPH

        ; 17 - number of left registers to store below
        sbiw    r24,    18 

        ; run scheduler
        call    mavrt_switch

        ; store fixed registers
        push    r2
        push    r3
        push    r4
        push    r5
        push    r6
        push    r7
        push    r8
        push    r9
        push    r10
        push    r11
        push    r12
        push    r13
        push    r14
        push    r15
        push    r16
        push    r17
        push    r28
        push    r29

        cli
        out     SPH,    r25
        out     SPL,    r24
        sei

        pop     r29
        pop     r28
        pop     r17
        pop     r16
        pop     r15
        pop     r14
        pop     r13
        pop     r12
        pop     r11
        pop     r10
        pop     r9
        pop     r8
        pop     r7
        pop     r6
        pop     r5
        pop     r4
        pop     r3
        pop     r2

        ret

mavrt_lock:
        push    r28
        push    r29
        movw    r28,    r24
.check:
        ld      r20,    Y
        cpse    r20,    rzero
        rjmp    .locked
        ser     r18
        st      Y,      r18
        pop     r29
        pop     r28
        ret
.locked:
        call    mavrt_schedule
        rjmp    .check

mavrt_unlock:
        movw    r26,    r24
        st      X,      rzero
        ret

mavrt_systick:
        push    r16
        push    r20
        in      r20,    SREG
        push    r20

        ldi     r20,    1
        sts     intflag,    r20

        lds     r16,    timems+0
        add     r16,    r20
        sts     timems+0,   r16

        clr     r20
        lds     r16,    timems+1
        adc     r16,    r20
        sts     timems+1,   r16

        lds     r16,    timems+2
        adc     r16,    r20
        sts     timems+2,   r16

        lds     r16,    timems+3
        adc     r16,    r20
        sts     timems+3,   r16

        pop     r20
        out     SREG,   r20
        pop     r20
        pop     r16
        reti
