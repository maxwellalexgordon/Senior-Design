/*
 * slave.h
 *
 *  Created on: Apr 4, 2020
 *      Author: Maxwell
 */

#ifndef SLAVE_H_
#define SLAVE_H_

#include <stdint.h>

int power(int base, int exponet);
void charSwap(char* a, char* b);
void setUpPWM();
void turnOnPWM();
void turnOffPWM();
void setPWM_Percent(int percent);
void setUpADC0();
uint32_t getADC0_val();
void setTemp(float t, int UART_num);
void calcCheckSum(char* arr);
int baseToDec(int base, char* value);
void decToHexa(char* hex, int n);
int getTempSensor();
void turnPlateOn();
void turnPlateOff();
void setUpLED();
void stepBack();
void stepForward();
void delayMs(uint32_t ui32Ms);
void setUpUART0();
void UARTIntHandler(void);
void setUpUART5();



#endif /* SLAVE_H_ */
