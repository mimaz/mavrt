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

enum
{
    FLAG_PAUSED = 0x01,
    FLAG_KILLED = 0x02,
    FLAG_SLEEP = 0x04,
};

struct mavrt_thread
{
    uint32_t waketim;
    mavrt_thread *next;
    void *sptr;
    uint8_t flags;
};


static mavrt_thread mainctx;

static mavrt_thread *rootnd = &mainctx;
static mavrt_thread *currnd = &mainctx;
static mavrt_thread *prevnd = NULL;

static void nextctx(void)
{
    prevnd = currnd;
    currnd = currnd->next;

    if (currnd == NULL)
    {
        prevnd = NULL;
        currnd = rootnd;
    }
}

static void remctx(void)
{
    if (prevnd != NULL)
    {
        if (currnd->next != NULL)
        {
            prevnd->next = currnd->next;
            currnd = currnd->next;
        }
        else
        {
            prevnd->next = NULL;
            currnd = rootnd;
        }
    }
    else
    {
        currnd = currnd->next;
        rootnd = currnd;
    }
}

void mavrt_exit(void)
{
    currnd->flags |= FLAG_KILLED;

    while (1)
        mavrt_schedule();
}

void *mavrt_register(mavrt_thread *node, void *sptr)
{
    node->waketim = 0;
    node->next = rootnd;
    node->sptr = sptr;
    node->flags = 0;

    rootnd = node;

    return node;
}

void *mavrt_switch(void *sptr)
{
    currnd->sptr = sptr;

    nextctx();


    while (currnd->flags & (FLAG_SLEEP | FLAG_PAUSED | FLAG_KILLED))
    {
        if (currnd->flags & FLAG_SLEEP && currnd->waketim < mavrt_time())
        {
            currnd->flags &= ~FLAG_SLEEP;
            break;
        }

        if (currnd->flags & FLAG_KILLED)
            remctx();

        nextctx();
    }


    return currnd->sptr;
}

void mavrt_pause(void)
{
    currnd->flags |= FLAG_PAUSED;
}

void mavrt_resume(mavrt_thread *node)
{
    node->flags &= ~FLAG_PAUSED;
}

void mavrt_sleep(uint32_t delay)
{
    // TODO
    uint32_t waketim = mavrt_time() + delay + 1;

    currnd->flags |= FLAG_SLEEP; 
    currnd->waketim = waketim;

    mavrt_schedule();
}

void mavrt_continue_sleep(uint32_t delay)
{
    // TODO
    currnd->flags |= FLAG_SLEEP; 
    currnd->waketim += delay + 1;

    mavrt_schedule();
}
