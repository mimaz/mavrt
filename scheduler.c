/*
 * Mieszko Mazurek <mimaz@gmx.com>
 *
 * Apr 2018
 */

#include "mavrt.h"

#define MAX_THREADS 16

#ifndef NULL
#define NULL ((void *) 0)
#endif

#define DEFAULT_PRIORITY MAVRT_MEDIUM

enum
{
    FLAG_PAUSED = 0x01,
    FLAG_KILLED = 0x02,
};

struct mavrt_thread
{
    uint32_t proctim;
    void *sptr;
    uint8_t flags;
};

static mavrt_thread mainctx;

static mavrt_thread *ctxheap[MAX_THREADS] = { &mainctx };
static uint8_t ctxheapsiz = 1;
static uint32_t maxproctime;

static mavrt_thread *volatile mavrt_context = &mainctx;

#define head ctxheap[0]

static mavrt_thread *volatile tail = &mainctx;

static uint8_t childid(uint8_t id)
{
    return id * 2 + 1;
}

static void swapctx(uint8_t pid, uint8_t qid)
{
    mavrt_thread *tmp = ctxheap[pid];

    ctxheap[pid] = ctxheap[qid];
    ctxheap[qid] = tmp;
}

static void bubble(void)
{
    uint8_t idx = 0, chl = childid(idx);

    while (chl < ctxheapsiz)
    {
        if (chl + 1 < ctxheapsiz 
                && ctxheap[chl + 1]->proctim < ctxheap[chl]->proctim)
        {
            chl++;
        }

        if (ctxheap[chl]->proctim < ctxheap[idx]->proctim)
        {
            swapctx(idx, chl);

            idx = chl;
            chl = childid(idx);

            continue;
        }

        break;
    }
}

static void putback(void)
{
    head->proctim = ++maxproctime;

    swapctx(0, ctxheapsiz - 1);

    bubble();
}

mavrt_thread *mavrt_current(void)
{
    return head;
}

void mavrt_exit(void)
{
    mavrt_kill(NULL);

    while (1);
}

mavrt_thread *mavrt_register(mavrt_thread *node, void *sptr)
{
    node->sptr = sptr;
    node->proctim = maxproctime + 1;
    node->flags = 0;

    ctxheap[ctxheapsiz++] = node;

    bubble();

    return node;
}

void *mavrt_switch(void *sptr, uint16_t usedtim, uint8_t pback)
{
    head->sptr = sptr;
    
    if (pback)
    {
        putback();
    }
    else
    {
        head->proctim += usedtim;

        if (head->proctim > maxproctime)
            maxproctime = head->proctim;
        
        bubble();
    }


    while (head->flags & (FLAG_PAUSED | FLAG_KILLED))
    {
        putback();

        if (head->flags & FLAG_KILLED)
            ctxheapsiz--;
    }


    return head->sptr;
}

void mavrt_kill(mavrt_thread *node)
{
    if (node == NULL)
        node = mavrt_current();

    MAVRT_ATOMIC(node->flags |= FLAG_KILLED)

    __asm("call mavrt_schedule");
}

void mavrt_pause(mavrt_thread *node)
{
    if (node == NULL)
        node = mavrt_current();

    MAVRT_ATOMIC(node->flags |= FLAG_PAUSED)

    __asm("call mavrt_schedule");
}

void mavrt_resume(mavrt_thread *node)
{
    if (node == NULL)
        node = mavrt_current();

    MAVRT_ATOMIC(node->flags &= ~FLAG_PAUSED)
}

uint8_t mavrt_is_paused(mavrt_thread *node)
{
    if (node == NULL)
        node = mavrt_current();

    return (node->flags & FLAG_PAUSED) != 0;
}

uint8_t mavrt_is_killed(mavrt_thread *node)
{
    if (node == NULL)
        node = mavrt_current();

    return (node->flags & FLAG_KILLED) != 0;
}
