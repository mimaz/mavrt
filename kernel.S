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
        .extern mavrt_tcntmax

mavrt_timems:
timems:
        .byte   0, 0, 0, 0

lasttime:
        .byte   0, 0, 0, 0

        .section .text

        .global main
        .weak main

        .global mavrt_launch
        .global mavrt_time_millis
        .global mavrt_yield
        .global mavrt_schedule
        .global mavrt_systick

main:
        sei
        call    mavrt_yield
        rjmp    main

        ; r24:r25 handler pointer
        ; r22:r23 handler data
        ; r20:r21 memory
        ; r18:r19 memory size
mavrt_launch:
        movw    r30,    r20
        add     r30,    r18
        adc     r31,    r19

        ldi     r18,    pm_lo8(.child)
        ldi     r19,    pm_hi8(.child)

        ; start data
        st      -Z,     r22
        st      -Z,     r23
        st      -Z,     r24
        st      -Z,     r25

        ; return address
        st      -Z,     r18
        st      -Z,     r19

        ; fixed + offset
        sbiw    r30,    16+1 

        movw    r22,    r30
        movw    r24,    r20

        push    r20
        push    r21

        call    mavrt_register

        pop     r25
        pop     r24
        ret

.child:
        pop     r31
        pop     r30
        pop     r25
        pop     r24

        icall

        jmp     mavrt_exit

mavrt_time_millis:
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
        brne    mavrt_time_millis
        ret

mavrt_yield:
        lds     r18,    mavrt_tcntmax+0
        lds     r19,    mavrt_tcntmax+1
    
        lds     r20,    lasttime+0
        lds     r21,    lasttime+1
        lds     r22,    lasttime+2
        lds     r23,    lasttime+3

        sub     r20,    r18
        sbc     r21,    r19
        sbc     r22,    rzero
        sbc     r23,    rzero

        sts     lasttime+3, r23
        sts     lasttime+2, r22
        sts     lasttime+1, r21
        sts     lasttime+0, r20

mavrt_schedule:
        call    mavrt_context_time
        lds     r18,    lasttime+0
        lds     r19,    lasttime+1
        lds     r20,    lasttime+2
        lds     r21,    lasttime+3

        sts     lasttime+3, r25
        sts     lasttime+2, r24
        sts     lasttime+1, r23
        sts     lasttime+0, r22

        sub     r22,    r18
        sbc     r23,    r19
        sbc     r24,    r20
        sbc     r25,    r21

        movw    r20,    r22
        movw    r22,    r24

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

        out     SPH,    r25
        out     SPL,    r24

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

mavrt_systick:
        push    r16
        push    r20
        in      r20,    SREG
        push    r20

        ldi     r20,    1
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
