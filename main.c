#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include <stdio.h>
#include <ctype.h>



//math help


void charSwap(char* a, char* b){
    char temp = *a;
    *a = *b;
    *b = temp;
}

// function to convert decimal to hexadecimal
void decToHexa(char* hex, int n)
{
    //multiply num by 10
     n = n * 10;


    //check if neg
    if(n<0){
       n = 65536 + n; //2**16 + n;
   }

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
void setTemp(float t, int UART_num){
     char setTemp[10] = {'*','1','c','0','0','0','0','0','0','\r'};  //-1.5C

     int temp = (int)(t * 10);
     char hex_temp[4] = "0000";
     decToHexa(&hex_temp, temp);

     //print temp conversion
     // for(int i = 3; i>= 0; i--){
     // printf("%c\n",hex_temp[i]);
     //}
     //printf("\n\n\n");


     //insert temp convertion
     int count = 3;
     int p;
     for(p = 3; p<=6; p++){
    setTemp[p] = tolower(hex_temp[count--]);
    }



    //calc check sum and add to setTemp string
    calcCheckSum(setTemp);

    //print full string
    //printf("sending string: ");
    //int i;
    //for(i = 0; i<= 10; i++){
    //    printf("%c",setTemp[i]);
    //}
    //printf("\n");
    USART_PutString(setTemp, UART_num);

}

void calcCheckSum(char* arr){
    int total = 0;
    int var;
    for (var = 1; var <= 6; ++var) {
          //printf("Adding: %c   %x\n",arr[var], arr[var]);
           total = total + arr[var];
    }

    //convert checksum value to hex
    char FULLCS[4]= "0000";
    decToHexa(&FULLCS, total);


    //print cmd
    //printf("\nfullcs: \n");
    //for(int i = 0; i<= 3; i++){
    //  printf("%c\n",FULLCS[i]);
    //}

    //add 2 LSB of check sum to main statement arr
    arr[7] = tolower(FULLCS[1]);
    arr[8] = tolower(FULLCS[0]);
}

void getTempSensor(){
   char getTemp[10] ={'*','0','1','0','0','0','0','2','1','\r'};
}


int main(void) {
    //set up system clock
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    //turn on UARTS
    setUpUART0();
    setUpUART5();
    //float i;
    //for(i = -5; i <5; i = i + 0.5){
    //       setTemp(i);
    //   }


    delayMs(100);
    setTemp(10,5);
    setTemp(10,0);


    //infinite loop
    while(1){
      //  USART_PutString("hello",0);
        delayMs(100);
    }




}
