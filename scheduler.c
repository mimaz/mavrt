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

#define context mavrt_context

extern uint8_t mavrt_tcnt(void);

enum
{
    FLAG_PAUSED = 0x01,
    FLAG_KILLED = 0x02,
    FLAG_SLEEP = 0x04,
};

struct mavrt_thread
{
    uint32_t proctim;
    uint32_t waketim;
    uint8_t waketcn;
    void *sptr;
    uint8_t flags;
};


static mavrt_thread mainctx;

mavrt_thread *mavrt_context = &mainctx;

static mavrt_thread *ctxheap[MAX_THREADS] = { &mainctx };
static uint8_t ctxheapsiz = 1;
static uint32_t maxproctim;


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

static void updateheap(void)
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

    context = ctxheap[0];
}

void mavrt_exit(void)
{
    context->flags |= FLAG_KILLED;

    while (1)
        mavrt_schedule();
}

void *mavrt_register(mavrt_thread *node, void *sptr)
{
    node->proctim = maxproctim + 1;
    node->waketim = 0;
    node->sptr = sptr;
    node->flags = 0;

    ctxheap[ctxheapsiz++] = node;

    return node;
}

void *mavrt_switch(void *sptr, uint32_t usedtim)
{
    context->sptr = sptr;
    context->proctim += usedtim;

    if (context->proctim > maxproctim)
        maxproctim = context->proctim;
    
    updateheap();


    while (context->flags & (FLAG_PAUSED | FLAG_KILLED | FLAG_SLEEP))
    {
        if (context->flags & FLAG_SLEEP)
        {
            uint32_t crtim = mavrt_time_millis();
            uint8_t crtcn = mavrt_tcnt();

            uint32_t wktim = context->waketim;
            uint8_t wktcn = context->waketcn;

            if (wktim < crtim || (wktim == crtim && wktcn < crtcn))
            {
                context->flags &= ~FLAG_SLEEP;
                break;
            }
        }


        context->proctim = ++maxproctim;

        swapctx(0, ctxheapsiz - 1);

        if (context->flags & FLAG_KILLED)
            ctxheapsiz--;

        updateheap();
    }


    return context->sptr;
}

void mavrt_pause(void)
{
    context->flags |= FLAG_PAUSED;
}

void mavrt_resume(mavrt_thread *node)
{
    node->flags &= ~FLAG_PAUSED;
}

void mavrt_sleep(uint32_t delay)
{
    context->flags |= FLAG_SLEEP; 
    context->waketim = mavrt_time_millis() + delay;
    context->waketcn = mavrt_tcnt();

    mavrt_schedule();
}

void mavrt_continue_sleep(uint32_t delay)
{
    context->flags |= FLAG_SLEEP; 
    context->waketim += delay;

    mavrt_schedule();
}
