/*
 * state.c
 *
 *  Created on: Feb 26, 2020
 *      Author: maxwe
 */

#include "state.h"

STATE StateGetNext(STATE current)
{
    STATE next = STATE_IDLE;

    switch(current)
    {
        case STATE_IDLE:
            next = STATE_START;
            break;
        case STATE_START:
            next = STATE_END;
            break;
        default:
            next = STATE_IDLE;
    }
    return next;
}


