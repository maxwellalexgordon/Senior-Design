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

uint8_t ui8PinData = 2;

void setUpUART0()
{
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

void delayMs(uint32_t ui32Ms)
{

    // 1 clock cycle = 1 / SysCtlClockGet() second
    // 1 SysCtlDelay = 3 clock cycle = 3 / SysCtlClockGet() second
    // 1 second = SysCtlClockGet() / 3
    // 0.001 second = 1 ms = SysCtlClockGet() / 3 / 1000

    SysCtlDelay(ui32Ms * (SysCtlClockGet() / 3 / 1000));
}

//led flash
int main(void)
{
    SysCtlClockSet(
    SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

    //set up port F (LEDS)
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,
    GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

    //enable ADC
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_TS);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_TS);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_TS);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 3,
                             ADC_CTL_TS | ADC_CTL_IE | ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, 1);

    uint32_t ui32ADC0Value[4];
    volatile uint32_t ADC0_val;

    //turn all LED off
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0x00);

    while (1)
    {
        //clear interupt
        ADCIntClear(ADC0_BASE, 1);
        //start ADC capture
        ADCProcessorTrigger(ADC0_BASE, 1);
        //wait until capture complete
        while (!ADCIntStatus(ADC0_BASE, 1, false)) {}
        //read captured data
        ADCSequenceDataGet(ADC0_BASE, 1, ui32ADC0Value);
        //average data
        ADC0_val = (ui32ADC0Value[0] + ui32ADC0Value[1] + ui32ADC0Value[2]
                + ui32ADC0Value[3] + 2) / 4;



        /*
         //turn PF2 on
         ui8PinData = 4;

         //update
         GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
         ui8PinData);

         //delay
         delayMs(1);

         //turn Pf2 off
         ui8PinData = 0;

         //update
         GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
         ui8PinData);

         //delay
         delayMs(1);
         */

    }
}
