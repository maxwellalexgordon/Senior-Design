/*
 * slave.c
 *
 *  Created on: Mar 24, 2020
 *      Author: Maxwell
 */

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
#include "state.h"

//Constants
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




//TO CONSOLE FOR DEBUG
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


void UARTIntHandler(void)
{

	unsigned long ulStatus;

	ulStatus = UARTIntStatus(UART5_BASE, true); //get interrupt status
	UARTIntClear(UART5_BASE, ulStatus); //clear the asserted interrupts

	while(UARTCharsAvail(UART5_BASE)) //loop while there are chars
	{
        //echo character
		UARTCharPutNonBlocking(UART0_BASE, UARTCharGetNonBlocking(UART5_BASE));
		//USART_PutString("a", 0);

        //VISUAL DEBUG
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2); //blink LED
		SysCtlDelay(SysCtlClockGet() / (1000 * 3));
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0); //turn off LED
	}
}

//communication to master, enable interupt
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
            // Enable processor interrupts, then enable the UART interrupt, and then select which individual UART interrupts to enable
    UARTIntRegister(UART5_BASE, UARTIntHandler);
    IntMasterEnable();
	IntEnable(INT_UART5);
	UARTIntEnable(UART5_BASE, UART_INT_RX | UART_INT_RT);
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
    // CLOCK SETUP - System Clock to 50 MHz
	SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);


    setUpLED();
    setUpUART0();
    setUpUART5();  //read data and print it out to UART0
    



    USART_PutString("Start: \n \r", 0);
    /*
    char rx;
    while (1)
    {
        if (UARTCharsAvail(UART5_BASE)){ //If UART 7's Rx pin at E0 has a character available, put it to the variable rx.
            rx = UARTCharGet(UART5_BASE);
            //UARTCharPut(UART0_BASE, '\n');
            UARTCharPut(UART0_BASE, '\r');
            UARTCharPut(UART0_BASE, rx); //Send the character in rx to the Putty terminal through Port 0's UART Tx pin at A0.

        }

    }
    */

    while(1){}


}









