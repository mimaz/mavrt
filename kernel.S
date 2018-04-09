/*
 * Mieszko Mazurek <mimaz@gmx.com>
 *
 * Apr 2018
 */

SPL     = 0x3D
SPH     = 0x3E
SREG    = 0x3F

rzero   = 1

        .macro push_volatile
            push    r0
            push    r1
            in      r0,     SREG
            push    r0
            push    r18
            push    r19
            push    r20
            push    r21
            push    r22
            push    r23
            push    r24
            push    r25
            push    r26
            push    r27
            push    r30
            push    r31
            clr     rzero
        .endm

        .macro pop_volatile
            pop     r31
            pop     r30
            pop     r27
            pop     r26
            pop     r25
            pop     r24
            pop     r23
            pop     r22
            pop     r21
            pop     r20
            pop     r19
            pop     r18
            pop     r0
            out     SREG,   r0
            pop     r1
            pop     r0
        .endm

        .section .bss

        .global mavrt_millis_lsb

mavrt_millis_lsb:
timems:
        .byte   0, 0, 0, 0

prelock:
        .byte   0

lasttime:
        .byte   0, 0

        .section .text

        .global mavrt_enter_critical
        .global mavrt_exit_critical
        .global mavrt_launch
        .global mavrt_system_time
        .global mavrt_yield
        .global mavrt_schedule
        .global mavrt_systick

        ; r24:r25 handler pointer
        ; r22:r23 handler data
        ; r20:r21 memory
        ; r18:r19 memory size
mavrt_launch:
        movw    r30,    r20
        add     r30,    r18
        adc     r31,    r19

        in      r0,     SREG
        ldi     r18,    pm_lo8(.child)
        ldi     r19,    pm_hi8(.child)

        st      -Z,     r18
        st      -Z,     r19
        st      -Z,     r0
        st      -Z,     r1
        st      -Z,     r0
        st      -Z,     r18
        st      -Z,     r19
        st      -Z,     r20
        st      -Z,     r21
        st      -Z,     r22
        st      -Z,     r23
        st      -Z,     r24
        st      -Z,     r25
        st      -Z,     r26
        st      -Z,     r27
        st      -Z,     rzero
        st      -Z,     rzero
        sbiw    r30,    18+1 ; volatile & offset

        movw    r22,    r30
        movw    r24,    r20

        rcall   mavrt_enter_critical
        call    mavrt_register
        rjmp    mavrt_exit_critical

.child:
        movw    r30,    r24
        movw    r24,    r22
        icall
        call    mavrt_current
        jmp     mavrt_kill

mavrt_enter_critical:
        push    r16
        ser     r16
        sts     prelock,    r16
        pop     r16
        ret

mavrt_exit_critical:
mavrt_schedule:
        push_volatile
        clr     r24
        rjmp    .ctxswitch

mavrt_yield:
        push_volatile
        ser     r24
        rjmp    .ctxswitch

mavrt_system_time:
        lds     r18,    timems+0
        lds     r19,    timems+1
        lds     r20,    timems+2
        lds     r21,    timems+3

        lds     r22,    timems+0
        lds     r23,    timems+1
        lds     r24,    timems+2
        lds     r25,    timems+3

        cp      r18,    r22
        cpc     r19,    r23
        cpc     r20,    r24
        cpc     r21,    r25
        brne    mavrt_system_time
        ret

mavrt_systick:
        push_volatile

        ; increment time
        ldi     r18,    1
        lds     r22,    timems+0
        lds     r23,    timems+1
        lds     r24,    timems+2
        lds     r25,    timems+3
        add     r22,    r18
        adc     r23,    rzero
        adc     r24,    rzero
        adc     r25,    rzero
        sts     timems+3,   r25
        sts     timems+2,   r24
        sts     timems+1,   r23
        sts     timems+0,   r22

        lds     r18,    prelock
        cpse    r18,    rzero
        rjmp    .skip_ctxswitch

        clr     r24

        ; r24 putback flag
.ctxswitch:
        ser     r18
        sts     prelock,    r18

        push    r24 ; push putback flag
        call    mavrt_context_time
        lds     r18,    lasttime+0
        lds     r19,    lasttime+1
        sub     r24,    r18
        sbc     r25,    r19

        pop     r20 ; pop putback flag
        movw    r22,    r24
        in      r24,    SPL
        in      r25,    SPH
        sbiw    r24,    18

        call    mavrt_switch

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

        call    mavrt_context_time
        sts     lasttime+1, r25
        sts     lasttime+0, r24

        sts     prelock,    rzero

.skip_ctxswitch:
        pop_volatile
        ret
