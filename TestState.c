/*
 * TestState.c
 *
 *  Created on: Feb 26, 2020
 *      Author: maxwe
 */


#include "state.h"

int main(void){

    STATE state = STATE_IDLE;

    while(1){
        switch(state)
                {
                    case STATE_IDLE:
                        //turn coolers on
                        //do not mix
                        //blink LED
                        break;

                    case STATE_START:
                        //start mixing
                        //next LED
                        break;

                    case STATE_END:
                        break;
                }

    //get next state after running state
    state = StateGetNext(state);
    }




}
