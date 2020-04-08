/*
 * slave.h
 *
 *  Created on: Apr 4, 2020
 *      Author: Maxwell
 */
#include <stdint.h>
#ifndef SLAVE_H_
#define SLAVE_H_


void delayMs(uint32_t ui32Ms);
void USART_PutString(char *s, int port);
void setUpUART0();
void setUpLED();
void UARTIntHandler(void);
void setUpUART5();
void setUpPWM();
void turnOnPWM();
void turnOffPWM();
void setPWM_Percent(int percent);
int power(int base, int exponent);
void charSwap(char *a, char *b);
void decToHexa(char *hex, int n);
void runCommand();
void setTemp(char** t, int plate);
int getTempSensor();
void turnPlateOn();
void turnPlateOff();
void calcCheckSum(char *arr);
int baseToDec(int base, char *value);
void stepBack();
void stepForward();

#endif /* SLAVE_H_ */
