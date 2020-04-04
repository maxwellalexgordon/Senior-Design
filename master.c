/*
 * master.c
 *
 *  Created on: Mar 24, 2020
 *      Author: Maxwell
 */
#include "state.h"
#include <stdint.h>
#include <stdbool.h>
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
#include "utils/uartstdio.c"
#include <string.h>
#include <ctype.h>
#include "driverlib/debug.h"
#include "driverlib/pin_map.h"


#define plate_0 0
#define plate_1 5
#define plate_2 7
#define mix_on "00020001"
#define mix_off "00020002"


void delayMs(uint32_t ui32Ms)
{

    // 1 clock cycle = 1 / SysCtlClockGet() second
    // 1 SysCtlDelay = 3 clock cycle = 3 / SysCtlClockGet() second
    // 1 second = SysCtlClockGet() / 3
    // 0.001 second = 1 ms = SysCtlClockGet() / 3 / 1000

    SysCtlDelay(ui32Ms * (SysCtlClockGet() / 3 / 1000));
}

//UART x3
void setUpUART0(){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0); //Enable UART 7 in GPIO port A.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA); //Enable port A.
    GPIOPinConfigure(GPIO_PA0_U0RX); //Set port A pin 0 as UART 0 RX.
    GPIOPinConfigure(GPIO_PA1_U0TX); //Set port A pin 1 as UART 0 TX.
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    //Set UART 0 clock to system clock at 115200 baud with 8 bit data length, one stop bit and no parity bits.
    UARTConfigSetExpClk(
            UART0_BASE, SysCtlClockGet(), 115200,
            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
}

void setUpUART5(){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART5); //Enable UART 5 in GPIO port E
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE); //Enable port E.
    GPIOPinConfigure(GPIO_PE4_U5RX); //Set port E pin 4 as UART 5 RX.
    GPIOPinConfigure(GPIO_PE5_U5TX); //Set port E pin 5 as UART 5 TX.
    GPIOPinTypeUART(GPIO_PORTE_BASE, GPIO_PIN_4 | GPIO_PIN_5);
    //Set UART 7 clock to system clock at 115200 baud with 8 bit data length, one stop bit and no parity bits.
    UARTConfigSetExpClk(
            UART5_BASE, SysCtlClockGet(), 115200,
            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
}

void setUpUART7(){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART7); //Enable UART 7 in GPIO port E.
    // SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE); //Enable port E.  -----UART5 will turn it on.
    GPIOPinConfigure(GPIO_PE0_U7RX); //Set port E pin 0 as UART 7 RX.
    GPIOPinConfigure(GPIO_PE1_U7TX); //Set port E pin 1 as UART 7 TX.
    GPIOPinTypeUART(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    //Set UART 7 clock to system clock at 115200 baud with 8 bit data length, one stop bit and no parity bits.
    UARTConfigSetExpClk(
            UART7_BASE, SysCtlClockGet(), 115200,
            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

}

void USART_PutString(char *s, int port){
    switch (port)
    {
    case 0:
        while (*s)
        {
            UARTCharPut(UART0_BASE, *s++);
        }
        break;
    case 5:
        while (*s)
        {
            UARTCharPut(UART5_BASE, *s++);
            //delayMs(4); //recomnded to wait 4ms after eat transmitions
        }
        break;
    case 7:
        while (*s)
        {
            UARTCharPut(UART7_BASE, *s++);
        }
        break;
    default:
        break;
    }

}

/*
Function | Array
----------------
Device | Info
xxxx   | yyyy
-----------------
Plate : 0001
-on: 0001
-off: 0002
-setTemp: XXXX  (remember to * & +)

Mixing Motor : 0002
-on: 0001
-off: 0002
-
*/





int main(void){

    STATE state = STATE_IDLE;
    setUpUART0();
    setUpUART5();

    while(1){
        switch(state)
                {
                    case STATE_IDLE:
                    //turn on plate 1, 2, 3   
                    USART_PutString(mix_off,plate_0);
                    delayMs(1000);
                    USART_PutString(mix_off,plate_0);


                    //mixing off 1, 2, 3



                    //close valve: they shoulbe be all closed alreaady


                    delayMs(4000);
                        break;

                    case STATE_START:
                        //mixing off 1, 2, 3
           
                        break;

                    case STATE_END:
                        break;
                }

    //get next state after running state
    state = StateGetNext(state);
    }




}
