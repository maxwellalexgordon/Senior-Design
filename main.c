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

//headers
void decToHexa(char *hex, int n);

//Variables
uint8_t ui8PinData;        //used for stepper/LED
uint32_t ui32ADC0Value[4]; //Set ui32ADC0Value as a 4 element array of unsigend 32 bit integer data type.
volatile uint32_t adcval;  //Set adcval as the averaged data to be read from ADC.

//variable to be assigned in console (DEFAULTS)
int plate_1_temp = 10;
int plate_2_temp = 12;
int plate_3_temp = 10;
uint32_t plate_1_time = 3000; //ms
int plate_1_pwm = 50;
char mix_on[9] =       "00020001\0";
char mix_off[9] =      "00020002\0";
char mix_enable[9]=    "00020003\0";
char plate_on[9] =     "00010001\0";
char plate_off[9] =    "00010002\0";
char valve_open[9] =   "00030001\0";
char valve_close[9] =  "00030002\0";
char valve_shutdown[9]="00030003\0";

//adjustable command templates
char setTempCmd[9] = "00010000\0";
char setPWMCmd[9] = "00020000\0";

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

void updatePWMCmd(int val){
    setPWMCmd[6] = (val /10) + '0';
    setPWMCmd[7] = (val% 10) +'0';
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

//uses pin PF3
void setUpADC0()
{

    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0); //Enable ADC0.
    //Set the sequencer 1, with 4 sample storage capacity to be triggered by the processor with highest priority.
    ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
    //Set the sequencer 1's first 3 samples to come from ADC pin 0 or PE3.
    ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH0);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_CH0);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_CH0);
    //Set the sequencer 3's sample 3 to be the final sample taken from ADC input pin 0 or PE3 and generate an interrupt when sample is set.
    ADCSequenceStepConfigure(ADC0_BASE, 1, 3, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, 1); //Enable ADC0's sample sequencer 1.
}

uint32_t getADC0_val()
{
    ADCIntClear(ADC0_BASE, 1);                 //Clear the interrupt.
    ADCProcessorTrigger(ADC0_BASE, 1);         //Trigger the ADC to start converting and taking samples.
    while (!ADCIntStatus(ADC0_BASE, 1, false)) //Wait for all samples to be put in the sequencer.
    {
    }
    ADCSequenceDataGet(ADC0_BASE, 1, ui32ADC0Value); //Acquire the value captured in the sequencer.
    //Set adcval to the average of the captured values. Add 2 for rounding off errors of four 0.5s.
    adcval = (ui32ADC0Value[0] + ui32ADC0Value[1] + ui32ADC0Value[2] + ui32ADC0Value[3] + 2) / 4;
    return adcval;
}

void charSwap(char *a, char *b)
{
    char temp = *a;
    *a = *b;
    *b = temp;
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

void delayMs(uint32_t ui32Ms)
{

    // 1 clock cycle = 1 / SysCtlClockGet() second
    // 1 SysCtlDelay = 3 clock cycle = 3 / SysCtlClockGet() second
    // 1 second = SysCtlClockGet() / 3
    // 0.001 second = 1 ms = SysCtlClockGet() / 3 / 1000

    SysCtlDelay(ui32Ms * (SysCtlClockGet() / 3 / 1000));
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

//direction on PF1, Step on PF2, enjoys the LEDs
void stepBack()
{
    //turn PF2 on, PF1 off
    ui8PinData = 4;

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
    delayMs(100);
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

void setTemp(float t, int UART_num)
{
    char setTemp[10] = {'*', '1', 'c', '0', '0', '0', '0', '0', '0', '\r'}; //-1.5C

    int temp = (int)(t * 10);
    char hex_temp[4] = "0000";
    decToHexa(&hex_temp, temp);

    //insert temp convertion
    int count = 3;
    int p;
    for (p = 3; p <= 6; p++)
    {
        setTemp[p] = tolower(hex_temp[count--]);
    }

    //calc check sum and add to setTemp string
    calcCheckSum(setTemp);

    USART_PutString(setTemp, UART_num);
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
//UART5
void turnPlateOn()
{
    char turnOn[10] = {'*', '3', '0', '0', '0', '0', '1', '0', '0', '\r'};
    calcCheckSum(turnOn);
    USART_PutString(turnOn, 5);
}
//UART5
void turnPlateOff()
{
    char turnOff[10] = {'*', '3', '0', '0', '0', '0', '0', '0', '0', '\r'};
    calcCheckSum(turnOff);
    USART_PutString(turnOff, 5);
}

//print to UART0
void read_UART5()
{
    char rx;
    if (UARTCharsAvail(UART5_BASE))
    { //If UART 7's Rx pin at E0 has a character available, put it to the variable rx.

        rx = UARTCharGet(UART5_BASE);
        UARTCharPut(UART0_BASE, '\n');
        //UARTCharPut(UART0_BASE, '\r');

        UARTCharPut(UART0_BASE, rx); //Send the character in rx to the Putty terminal through Port 0's UART Tx pin at A0.
    }
}

//eddits cmd shar arr to incllude given temp
void makeTempCmd(int t)
{
    //char setTemp[10] = { '*', '1', 'c', '0', '0', '0', '0', '0', '0', '\r' }; //-1.5C

    int temp = (t * 10);
    char hex_temp[4] = "0000";
    decToHexa(&hex_temp, temp);

    //insert temp convertion
    int count = 3;
    int p;
    for (p = 4; p <= 7; p++)
    {
        setTempCmd[p] = tolower(hex_temp[count--]);
    }
    setTempCmd[8] = NULL;
}

int main(void)
{
    //set up system clock
    SysCtlClockSet(
        SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ); //50Mhz == 400Mhz base /4(sys div)/2(internal)

    //turn on UARTS
    setUpUART0();
    setUpUART5(); //turns on port E for UART5 & UART7
    //setUpUART7();

    //set up LED(used for stepper)
    setUpLED();

    //set up ADC, PF3
    //setUpADC0();

    //set up PWM PF1
    //setUpPWM();

    //////////////////////////////////////////////////////////////////////////////

    //Start FSM
    STATE state = STATE_IDLE;
    while (1)
    {
        switch (state)
        {
        case STATE_IDLE:
         
           
            //turn mixer off & setPWM
            updatePWMCmd(plate_1_pwm);
            USART_PutString(mix_enable, plate_1);
            USART_PutString(setPWMCmd, plate_1);
            USART_PutString(mix_off, plate_1);

            //turn cooler on
            USART_PutString(plate_on, plate_1);

            //set cooler temp
            makeTempCmd(plate_1_temp);
            USART_PutString(setTempCmd, plate_1);

            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
                         2);

            break;

       
        case STATE_START:
            //start mixing
            USART_PutString(mix_on, plate_1);
            


            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 4);
            break;
        case STATE_BAG1_RELEASE:
            //open valve 1
            USART_PutString(valve_open, plate_1);
            //wait
            delayMs(plate_1_time);
            //close valve
            USART_PutString(valve_close, plate_1);

            //turn plate 1 off
            USART_PutString(plate_off, plate_1);

            break;
        case STATE_BAG2_RELEASE:
            //open valve 1
           USART_PutString(valve_open, plate_1);
           //wait
           delayMs(plate_1_time);
           //close valve
           USART_PutString(valve_close, plate_1);

           //turn plate 1 off
           USART_PutString(plate_off, plate_1);

            //turn plate 2 off
            break;
        case STATE_BAG3_RELEASE:
            //open valve 1
           USART_PutString(valve_open, plate_1);
           //wait
           delayMs(plate_1_time);
           //close valve
           USART_PutString(valve_close, plate_1);

           //turn plate 1 off
           USART_PutString(plate_off, plate_1);
            break;
        case STATE_END:
            //turn mix off
            USART_PutString(mix_off, plate_1);

            //turn plate off
            USART_PutString(plate_off, plate_1);

            //open valves
            USART_PutString(valve_open, plate_1);

            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 8);
            break;
        }

        //get next state after running state
        state = StateGetNext(state);

        //delay
        delayMs(3000);
    }
}
