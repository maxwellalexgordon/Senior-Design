/*
 * slave.c
 *
 *  Created on: Mar 24, 2020
 *      Author: Maxwell
 */

#include "state.h"
#include "slave.h"

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

//Constants
#define plate_0 0
#define plate_1 5
#define plate_2 7
#define mix_on "00020001"
#define mix_off "00020002"

//Variables
uint8_t ui8PinData;        //used for stepper/LED
uint32_t ui32ADC0Value[4]; //Set ui32ADC0Value as a 4 element array of unsigend 32 bit integer data type.
volatile uint32_t adcval;  //Set adcval as the averaged data to be read from ADC.
char command[9] = "00000000\0";



//functions
//direction on PF1, Step on PF2, enjoys the LEDs
void stepBack()
{
    //turn PF2 on, PF1 off
    int ui8PinData = 4;

    //update
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
                 ui8PinData);

    //delay
    delayMs(1);

    //turn Pf2 off, PF1 off
    ui8PinData = 0;

    //update
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
                 ui8PinData);

    //delay
    delayMs(1);
}

//direction on PF1, Step on PF2, enjoys the LEDs
void stepForward()
{
    //turn PF2 on, PF1 off
    ui8PinData = 6;

    //update
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
                 ui8PinData);

    //delay
    delayMs(1);

    //turn Pf2 off, PF1 off
    ui8PinData = 2;

    //update
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
                 ui8PinData);

    //delay
    delayMs(1);
}


int baseToDec(int base, char *value)
{
    int val = 0;
    int pow = 0;
    int mult = 0;
    int i;
    for (i = strlen(value) - 1; i >= 0; i--)
    {
        if (value[i] - '0' > 9)
        {
            mult = value[i] - '0' - 7;
        }
        else
        {
            mult = value[i] - '0';
        }
        val = val + (mult * power(base, pow));
        pow++;
    }
    return val;
}

void calcCheckSum(char *arr)
{
    int total = 0;
    int var;
    for (var = 1; var <= 6; ++var)
    {

        total = total + arr[var];
    }

    //convert checksum value to hex
    char FULLCS[4] = "0000";
    decToHexa(&FULLCS, total);

    //add 2 LSB of check sum to main statement arr
    arr[7] = tolower(FULLCS[1]);
    arr[8] = tolower(FULLCS[0]);
}

//NEED T0 CHECK!!!
int getTempSensor()
{
    char getTemp[10] = {'*', '0', '1', '0', '0', '0', '0', '2', '1', '\r'};
    calcCheckSum(getTemp);
    //ask for temp
    USART_PutString(getTemp, 0);

    char temp_responce[8];
    int var = 0;
    //read temp at UART5, max 8 char
    for (var = 0; var < 8; var++)
    {
        if (UARTCharsAvail(UART5_BASE))
        {
            temp_responce[var] = UARTCharGet(UART5_BASE);
        }
    }
    //temp stored at temp_responce[4-7]
    char temp_hex[4];
    temp_hex[0] = temp_responce[4];
    temp_hex[1] = temp_responce[5];
    temp_hex[2] = temp_responce[6];
    temp_hex[3] = temp_responce[7];

    //convert to an int
    int val = baseToDec(16, temp_hex);
    return val;
}


void turnPlateOn(int plate)
{
    char turnOn[10] = {'*', '3', '0', '0', '0', '0', '1', '0', '0', '\r'};
    calcCheckSum(turnOn);
    USART_PutString(turnOn,plate);
}

void turnPlateOff(int plate)
{
    char turnOff[10] = {'*', '3', '0', '0', '0', '0', '0', '0', '0', '\r'};
    calcCheckSum(turnOff);
    USART_PutString(turnOff,plate);
}

void setTemp(char** t, int plate)
{
    char setTemp[10] = {'*', '1', 'c', '0', '0', '0', '0', '0', '0', '\r'}; //-1.5C

    //insert temp convertion
    int count = 3;
    int p;
    for (p = 3; p <= 6; p++)
    {
        setTemp[p] = tolower(t[count--]) - '0';
    }

    //calc check sum and add to setTemp string
    calcCheckSum(setTemp);

    USART_PutString(setTemp, plate);
}

void runCommand()
{

  //  USART_PutString(command,plate_0);

    //componets of a command
    int device = 0; //sum 0:3
    int action = 0; //sum 4:7

    int i;
    for (i = 0; i <= 3; i++)
    {
        device += (command)[i] - '0';  //same
    }
    for (i = 4; i <= 7; i++)
    {
        action += (command)[i] - '0';  //need to switch to be real
    }

    switch (device)
    {
    case 1:
        USART_PutString("PLATE: ", plate_0);
        switch (action)
        {
        case 1:
            USART_PutString("ON \n \r", plate_0);
            turnPlateOn(plate_0);
            break;
        case 2:
            USART_PutString("OFF \n \r", plate_0);
            break;
        default:
            USART_PutString("SETTEMP:  ", plate_0);
            USART_PutString(command, plate_0);
            USART_PutString("\n \r", plate_0);
            //setTemp(action,0);

            break;
        }
        break;
    case 2:

        USART_PutString("Mixer: ", plate_0);
        switch (action)
        {
        case 1:
            USART_PutString("ON \n \r", plate_0);
            //turnOnPWM();
            break;
        case 2:
            USART_PutString("OFF \n \r", plate_0);
            //turnOffPWM();
            break;
        case 3:
             USART_PutString("ENABLE \n \r", plate_0);
             //setUpPWM();
             break;
        default:
            USART_PutString("SET_PWM \n \r", plate_0);
            //conver 0x00XX
            int temp = 0;
            temp += ((command)[6] - '0') * 10;
            temp += (command)[7] - '0';
            //setPWM_Percent(temp);

            break;
        }
        break;
    case 3:
        USART_PutString("Valve: ", plate_0);
        switch (action)
        {
        case 1:
            USART_PutString("OPEN \n \r", plate_0);
            
            break;
        case 2:
            USART_PutString("CLOSE \n \r", plate_0);
            break;
        case 3:
             USART_PutString("SHUTDOWN \n \r", plate_0);
             break;
        default:
            USART_PutString("UNKNOWN \n \r", plate_0);
            break;
        }
        break;
    default:
        USART_PutString("UNKNOWN DEVICE \n \r \n\n\n", plate_0);
        USART_PutString(command, plate_0);
        break;
    }
    



}

// function to convert decimal to hexadecimal
void decToHexa(char *hex, int n)
{
    //multiply num by 10

    //check if neg
    if (n < 0)
    {
        n = 65536 + n; //2**16 + n;
    }

    // counter for hexadecimal number array
    int i = 0;
    while (n != 0)
    {
        // temporary variable to store remainder
        int temp = 0;

        // storing remainder in temp variable.
        temp = n % 16;

        // check if temp < 10
        if (temp < 10)
        {
            hex[i] = temp + 48;
            i++;
        }
        else
        {
            hex[i] = temp + 55;
            i++;
        }

        n = n / 16;
    }
}

void charSwap(char *a, char *b)
{
    char temp = *a;
    *a = *b;
    *b = temp;
}

int power(int base, int exponent)
{
    int result = 1;
    for (exponent; exponent > 0; exponent--)
    {
        result = result * base;
    }
    return result;
}

//set up PWM on LED PF1
void setUpPWM()
{

    // Enable the GPIO peripheral and wait for it to be ready.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
    {
    }

    // Enable the PWM peripheral and wait for it to be ready.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_PWM1))
    {
    }

    //Set PWM clock base frequency....means sys cllk divide by 8
    SysCtlPWMClockSet(SYSCTL_PWMDIV_8); //   50/8=6.25MhzPWM HZ

    // Configure the internal multiplexer to connect the PWM peripheral to PF1
    GPIOPinConfigure(GPIO_PF1_M1PWM5);

    // Set up the PWM module on pin PF3
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1);

    //just some junk, base1 works for gen2 and gen3
    PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);

    /*
     *
     *6.25Mh/desired HZ of PWM = 250Hz
     *
     */
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, 25000);

    //period / N.   1/N = duty cycle/
    //Default 50% == 2
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, PWMGenPeriodGet(PWM1_BASE, PWM_GEN_2) / 90);

    // Enable the PWM peripheral
    PWMGenEnable(PWM1_BASE, PWM_GEN_2);

    // Enable the PWM output signal
    PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT, false);
}

void turnOnPWM()
{
    // Enable the PWM output signal
    PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT, true);
}

void turnOffPWM()
{
    // Disable the PWM output signal
    PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT, false);
}

void setPWM_Percent(int percent)
{
    float N = (1.0 / (percent / 100.0));

    //duty cycle must be below 90%
    if (N < 1.11)
    {
        N = 1.11;
    }
    //period / N.   1/N = duty cycle/
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, 25000.0 / N);
}

void delayMs(uint32_t ui32Ms)
{
    // 1 clock cycle = 1 / SysCtlClockGet() second
    // 1 SysCtlDelay = 3 clock cycle = 3 / SysCtlClockGet() second
    // 1 second = SysCtlClockGet() / 3
    // 0.001 second = 1 ms = SysCtlClockGet() / 3 / 1000
    SysCtlDelay(ui32Ms * (SysCtlClockGet() / 3 / 1000));
}

//send array of chars
void USART_PutString(char *s, int port)
{
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

void setUpUART7()
{
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

//TO CONSOLE FOR DEBUG
void setUpUART0()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0); //Enable UART 7 in GPIO port A.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA); //Enable port A.
    GPIOPinConfigure(GPIO_PA0_U0RX);             //Set port A pin 0 as UART 0 RX.
    GPIOPinConfigure(GPIO_PA1_U0TX);             //Set port A pin 1 as UART 0 TX.
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    //Set UART 0 clock to system clock at 115200 baud with 8 bit data length, one stop bit and no parity bits.
    UARTConfigSetExpClk(
        UART0_BASE, SysCtlClockGet(), 115200,
        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
}

//F1,F2,F3
void setUpLED()
{
    //set up port F (LEDS)
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,
                          GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    //turn all LED off
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0x00);
}

void UARTIntHandler(void)
{

    unsigned long ulStatus;
    //char command[9] = "00000000";
    int count = 0;
    char rx;

    ulStatus = UARTIntStatus(UART5_BASE, true); //get interrupt status
    UARTIntClear(UART5_BASE, ulStatus);         //clear the asserted interrupts

    while (UARTCharsAvail(UART5_BASE)) //loop while there are chars
    {

        for (count = 0; count < 8; count++)
        {
            rx = UARTCharGet(UART5_BASE);

            //echo character
             UARTCharPutNonBlocking(UART0_BASE, rx);
            //VISUAL DEBUG
            ///  GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2); //blink LED
            // SysCtlDelay(SysCtlClockGet() / (1000 * 3));
            //  GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0); //turn off LED

            //store data
            command[count] = rx;
        }
        break;
    }

    //print to terminal
    //USART_PutString(command,plate_0);
    //USART_PutString("\n \r",plate_0);

    //find command and run it
    runCommand(&command);
   // USART_PutString("\n \r",plate_0);

}

//communication to master, enable interupt
void setUpUART5()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART5); //Enable UART 5 in GPIO port E
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE); //Enable port E.
    GPIOPinConfigure(GPIO_PE4_U5RX);             //Set port E pin 4 as UART 5 RX.
    GPIOPinConfigure(GPIO_PE5_U5TX);             //Set port E pin 5 as UART 5 TX.
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
-enable: 0003
-set_pwm: 00XX

valve motor: 0003
-open: 00001
-close: 0002
-shutdown: 0003
*/

int main(void)
{
    // CLOCK SETUP - System Clock to 50 MHz
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    setUpLED();
    setUpPWM();
    setUpUART0();
    setUpUART5(); //read data and print it out to UART0

    USART_PutString("Start: \n \r", 0);
    while (1)
    {
    }
}
