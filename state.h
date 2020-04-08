/*
 * state.h
 *
 *  Created on: Feb 26, 2020
 *      Author: maxwe
 */

#ifndef STATE_H_
#define STATE_H_



typedef enum {
    STATE_IDLE,
    STATE_START,
    STATE_BAG1_RELEASE,
    STATE_BAG2_RELEASE,
    STATE_BAG3_RELEASE,
    STATE_END
} STATE;

/**
    Get the next state based on the current state.
*/
STATE StateGetNext(STATE current);




#endif /* STATE_H_ */
