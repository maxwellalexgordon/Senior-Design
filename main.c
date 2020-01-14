#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include <stdio.h>



//math help


void charSwap(char* a, char* b){
    char temp = *a;
    *a = *b;
    *b = temp;
}

// function to convert decimal to hexadecimal
void decToHexa(char* hex, int n, int size)
{
    //multiply num by 10
     n = n * 10;


    //check if neg
   // if(n<0){
    //    n = 65536 - n;//2**16 - n;
    //}

    // counter for hexadecimal number array
    int i = 0;
    while(n!=0)
    {
        // temporary variable to store remainder
        int temp  = 0;

        // storing remainder in temp variable.
        temp = n % 16;

        // check if temp < 10
        if(temp < 10)
        {
            hex[i] = temp + 48;
            i++;
        }
        else
        {
            hex[i] = temp + 55;
            i++;
        }

        n = n/16;
    }

    /*
    //reverse characters
    int start = 0;
    int end = size;
    while(start < end){
        charSwap(hex+start, hex+end);
        start++;
        end--;
    }
    */

}

void delayMs(uint32_t ui32Ms) {

    // 1 clock cycle = 1 / SysCtlClockGet() second
    // 1 SysCtlDelay = 3 clock cycle = 3 / SysCtlClockGet() second
    // 1 second = SysCtlClockGet() / 3
    // 0.001 second = 1 ms = SysCtlClockGet() / 3 / 1000

    SysCtlDelay(ui32Ms * (SysCtlClockGet() / 3 / 1000));
}

void config_LED(){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);
}

void setUpUART0(){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0); //Enable UART 7 in GPIO port A.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA); //Enable port A.
    GPIOPinConfigure(GPIO_PA0_U0RX); //Set port A pin 0 as UART 0 RX.
    GPIOPinConfigure(GPIO_PA1_U0TX); //Set port A pin 1 as UART 0 TX.
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    //Set UART 0 clock to system clock at 115200 baud with 8 bit data length, one stop bit and no parity bits.
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
}

void setUpUART5(){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART5); //Enable UART 5 in GPIO port E
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE); //Enable port E.
    GPIOPinConfigure(GPIO_PE4_U5RX); //Set port E pin 4 as UART 5 RX.
    GPIOPinConfigure(GPIO_PE5_U5TX); //Set port E pin 5 as UART 5 TX.
    GPIOPinTypeUART(GPIO_PORTE_BASE, GPIO_PIN_3 | GPIO_PIN_4); //#-1 for each port
    //Set UART 7 clock to system clock at 115200 baud with 8 bit data length, one stop bit and no parity bits.
    UARTConfigSetExpClk(UART5_BASE, SysCtlClockGet(), 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
}

//send array of chars
void USART_PutString(char *s, int port)
{
    switch (port) {
        case 0:
            while(*s){
                    UARTCharPut(UART0_BASE, *s++);
                }
            break;
        case 5:
            while(*s){
                   UARTCharPut(UART5_BASE, *s++);
               }
        default:
            break;
    }

}
void setTemp(int temp){
    char setTemp[11] = {'*','1','c','f','f','f','1','f','7','\r'};  //-1.5C

    /*UART TEMP CONTROL COMMANDS
    stx relates to '*'
    the command code is '1','c'
    data being transmitted is '0','0','6','4' = 10 degrees
    checksum is '5','3'  last two digits of sum (hex) (not including start and stop)
    return is '\r'
    */

    USART_PutString(setTemp,0);
}

void calcCheckSum(char* arr){
    int total = 0;
    int var;
    for (var = 0; var <= 8; ++var) {
           total = total + (int)arr[var];
    }
    total =  total + 0;

}


void getTempSensor(){
   char getTemp[10] ={'*','0','1','0','0','0','0','2','1','\r'};
}


int main(void) {
    //set up system clock
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    //turn on UARTS
    setUpUART0();

   // int temperature = 1;
    //int size_of_hex = 4;
    //char hex_temp[4] = "0000";
    //decToHexa(&hex_temp, temperature, size_of_hex);


    //setTemp(100);

   // calcCheckSum(hex_temp);

    setTemp(10);
    USART_PutString("\n", 0);






    //infinite loop
    while(1){
      //  USART_PutString("hello",0);
        delayMs(100);
    }




}
