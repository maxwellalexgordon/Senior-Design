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
            next = STATE_BAG1_RELEASE;
            break;
        case STATE_BAG1_RELEASE:
            next = STATE_BAG2_RELEASE;
            break;
        case STATE_BAG2_RELEASE:
            next = STATE_BAG3_RELEASE;
            break;
        case STATE_BAG3_RELEASE:
            next = STATE_END;
            break;
        case STATE_END:
            next = STATE_IDLE;
        default:
            next = STATE_IDLE;
    }
    return next;
}


