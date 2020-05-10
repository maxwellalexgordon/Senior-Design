/*
 * master.h
 *
 *  Created on: Apr 9, 2020
 *      Author: Maxwell
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "stdlib.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_uart.h"
#include "inc/hw_gpio.h"
#include "inc/hw_pwm.h"
#include "inc/hw_types.h"
#include "driverlib/adc.h"
#include "driverlib/timer.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/udma.h"
#include "driverlib/pwm.h"
#include "driverlib/ssi.h"
#include "driverlib/systick.h"
#include "driverlib/adc.h"
#include "utils/uartstdio.h"
#include <assert.h>
#include "utils/uartstdio.c"

#include <string.h>

#include <ctype.h>
#include "driverlib/debug.h"
#include "driverlib/pin_map.h"



#ifndef MASTER_H_
#define MASTER_H_

//comman generation
void updatePWMCmd(int val);
void makeTempCmd(int t);
void calcCheckSum(char *arr);

//character stuff
char* itoa(int value, char* buffer, int base);
char* reverse(char *buffer, int i, int j);
inline void swap(char *x, char *y);


//math and character help
int power(int base, int exponent);
void charSwap(char *a, char *b);
void decToHexa(char *hex, int n);
int baseToDec(int base, char *value);

//system
void delayMs(uint32_t ui32Ms);
void setUpLED();

//set up console
void printIntro();
int runSetUp();
void parseArgs(char *buffer, char** args, int argsSize, int *nargs);

//UART
void setUpUART0();
void setUpUART5();
void setUpUART7();
void USART_PutString(char *s, int port);
void USART_PutCommand(char *s, int port);
void read_UART5();

//buttons
void setUpButtonPress();
void PORTF_Handler();
void PORTB_Handler();






#endif /* MASTER_H_ */
