#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_gpio.h"
#include "driverlib/timer.h"
#include "driverlib/comp.h"
#include <stdio.h>
#include "driverlib/pwm.h"
#include "driverlib/pin_map.h"


int main(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ); //Set system clock to 40 MHz.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); //Enable port F for PWM module 1 PWM 4 to PWM 7.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1); //Enable PWM module 1.
    SysCtlPWMClockSet(SYSCTL_PWMDIV_1); //Set PWM frequency to the same as System Clock.
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) =  0x4C4F434B; //Code for unlocking port F pins that are locked.
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;

    GPIOPinConfigure(GPIO_PF0_M1PWM4); //Set the pins in PortF pins 0, 1, 2 and 3 as PWM pins on PWM module 1.
    GPIOPinConfigure(GPIO_PF1_M1PWM5);
    GPIOPinConfigure(GPIO_PF2_M1PWM6);
    GPIOPinConfigure(GPIO_PF3_M1PWM7);
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

    PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC); //Configures PWM generator 2 (pins 4, 5) and PWM generator
    PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC); //3 (pins 6, 7) as down counting generators with no
                                                                                    //syncing with the clock required.
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, 400); //Set the total period of the PWM generators in clock ticks.
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, 400);

    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_4, 300); //Set the on time pulse width.
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, 300);
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, 300);
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, 300);

    PWMGenEnable(PWM1_BASE, PWM_GEN_2); //Enable each PWM generator.
    PWMGenEnable(PWM1_BASE, PWM_GEN_3);

    PWMOutputState(PWM1_BASE, (PWM_OUT_4_BIT | PWM_OUT_5_BIT | PWM_OUT_6_BIT | PWM_OUT_7_BIT ), true); //Enable all the PWM pins.

    while(1){
/*    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_4,300);
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5,300);
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6,300);
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7,300);
    SysCtlDelay(10000);*/
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_4,100);
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5,150);
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6,200);
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7,250);
    SysCtlDelay(10000);
    }



}
