//home mode headers
#include "state.h"
#include "master.h"



//Constants
#define SW1 GPIO_PIN_4
#define SW2 GPIO_PIN_0
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

//FSM Variables
STATE state = STATE_IDLE;
volatile char flag1 = 0;
volatile char flag2 = 0;

//variable to be assigned in console (DEFAULTS)
int plate_1_temp = 10;
int plate_2_temp = 12;
int plate_3_temp = 10;
int plate_1_time = 500; //ms
int plate_1_pwm = 50;
int plate_2_pwm = 50;
int plate_3_pwm = 50;

//pre built commands
char mix_on[9] = "00020001\0";
char mix_off[9] = "00020002\0";
char mix_enable[9] = "00020003\0";
char plate_on[9] = "00010001\0";
char plate_off[9] = "00010002\0";
char valve_open[9] = "00030001\0";
char valve_close[9] = "00030002\0";
char valve_shutdown[9] = "00030003\0";

//adjustable command templates
char setTempCmd[9] = "00010000\0";
char setPWMCmd[9] = "00020000\0";

void updatePWMCmd(int val)
{
    setPWMCmd[6] = (val / 10) + '0';
    setPWMCmd[7] = (val % 10) + '0';
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

void setUpUART0()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0); //Enable UART 7 in GPIO port A.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA); //Enable port A.
    GPIOPinConfigure(GPIO_PA0_U0RX);            //Set port A pin 0 as UART 0 RX.
    GPIOPinConfigure(GPIO_PA1_U0TX);            //Set port A pin 1 as UART 0 TX.
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
    GPIOPinConfigure(GPIO_PE4_U5RX);            //Set port E pin 4 as UART 5 RX.
    GPIOPinConfigure(GPIO_PE5_U5TX);            //Set port E pin 5 as UART 5 TX.
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
void USART_PutCommand(char *s, int port)
{
    switch (port)
    {
    case 0:
        UARTCharPut(UART0_BASE, '*');
        while (*s)
        {
            UARTCharPut(UART0_BASE, *s++);
        }
        break;
    case 5:
        UARTCharPut(UART5_BASE, '*');
        while (*s)
        {
            UARTCharPut(UART5_BASE, *s++);
        }

        break;
    case 7:
        UARTCharPut(UART7_BASE, '*');
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

//enable SW1(PF4) and interupt(enables)
void setUpButtonPress()
{

    //B3
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_3);
    GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_3, GPIO_STRENGTH_2MA,
    GPIO_PIN_TYPE_STD_WPU);

    //F4 & F0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, SW1);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA,
    GPIO_PIN_TYPE_STD_WPU);
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0x4C4F434B;
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, SW2);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA,
    GPIO_PIN_TYPE_STD_WPU);

    //innterupts
    IntEnable(INT_GPIOF);
    IntMasterEnable();

    // Button 1
    GPIOIntTypeSet(GPIO_PORTF_BASE, SW1, GPIO_RISING_EDGE);
    GPIOIntRegister(GPIO_PORTF_BASE, PORTF_Handler);
    GPIOIntEnable(GPIO_PORTF_BASE, SW1);

    // Button 2
    GPIOIntTypeSet(GPIO_PORTF_BASE, SW2, GPIO_RISING_EDGE);
    GPIOIntRegister(GPIO_PORTF_BASE, PORTF_Handler);
    GPIOIntEnable(GPIO_PORTF_BASE, SW2);

}

void PORTF_Handler()
{
    if (GPIOIntStatus(GPIO_PORTF_BASE, 1) == 16)
    {
        // Clear Flag
        GPIOIntClear(GPIO_PORTF_BASE, SW1);

        /* Handler Code Below */

        flag1 = 1;

    }
    else if (GPIOIntStatus(GPIO_PORTF_BASE, 1) == 1)
    {
        // Clear Flag
        GPIOIntClear(GPIO_PORTF_BASE, SW2);

        /* Handler Code Below */

        flag2 = 1;

    }
}
char** str_split(char *a_str, const char a_delim)
{
    char **result = 0;
    size_t count = 0;
    char *tmp = a_str;
    char *last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
     knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx = 0;
        char *token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

void printIntro()
{  
    char* intro = "-------------------------------------------------------------------------------------------------------------\n\r-   										                            -\n\r- 	                                                                                                    -\n\r-    .----------------.  .----------------.  .-----------------. .----------------.  .----------------.     -\n\r-    | .--------------. || .--------------. || .--------------. || .--------------. || .--------------. |   -\n\r-    | | ____    ____ | || |     _____    | || | ____  _____  | || |     _____    | || |      __      | |   -\n\r-    | ||_   \  /   _|| || |    |_   _|   | || ||_   \|_   _| | || |    |_   _|   | || |     /  \     | |   -\n\r-    | |  |   \/   |  | || |      | |     | || |  |   \ | |   | || |      | |     | || |    / /\ \    | |   -\n\r-    | |  | |\  /| |  | || |      | |     | || |  | |\ \| |   | || |      | |     | || |   / ____ \   | |   -\n\r-    | | _| |_\/_| |_ | || |     _| |_    | || | _| |_\   |_  | || |     _| |_    | || | _/ /    \ \_ | |   -\n\r-    | ||_____||_____|| || |    |_____|   | || ||_____|\____| | || |    |_____|   | || ||____|  |____|| |   -\n\r-    | |              | || |              | || |              | || |              | || |              | |   -\n\r-    | '--------------' || '--------------' || '--------------' || '--------------' || '--------------' |   -\n\r-    '----------------'  '----------------'  '----------------'  '----------------'  '----------------'     -\n\r-                                                                                                           -\n\r-                                        CREATED BY: DU TERUMO BCT                         	            -\n\r-               	    								                    -\n\r-              										                    -\n\r-                   	                  To Use:							    -\n\r-                           	          Device  Plate  Value			                            -\n\r-                           	          ====================				                    -\n\r-                           	          TEMP	  1/2/3  -20->20C		                            -\n\r-                           	          PWM	  1/2/3  0->100	%		                            -\n\r-                   										            -\n\r-                   	                  Type 'EXIT 0 0' when done		                            -\n\r-   									                                    -\n\r-------------------------------------------------------------------------------------------------------------\n\r";
     USART_PutString(intro, plate_0);
}

int runSetUp()
{

    int exit = 0;
    char rx;
    int count;
    int value;
    char valueAsString[3];
    int device;

    //printIntro();


    while (exit == 0)
    {

        //read in string
        char buf[50];
        rx = 0;
        count = 0;
        while (rx != 13)
        {

            //debug
            UARTCharPut(UART0_BASE, rx);

            //read avilible character
            rx = UARTCharGet(UART0_BASE);

            //check for backspace
            if(rx == 127){
                //set cursor back 1, not less than zero
                if(count > 0){
                    count--;
                }
                buf[count] = NULL;
            }else{
                //store
                buf[count++] = rx;
            }

        }

        //print new line
        USART_PutString("\n\r", plate_0);

        //fill in rest
        int var;
        for (var = count - 1; var < 50; var++)
        {
            buf[var] = 'z';
        }

        //split up inputs
        char data[3][10];
        int count = 1;
        char *pch = strtok(buf, " ,.-");
        strcpy(data[0], pch);
        while (pch != NULL)
        {
            pch = strtok(NULL, " ,.z");
            strcpy(data[count++], pch);
        }


        //CHECK IF NOT ALL THREE FIELDS ARE FILLED


        

        //find variable
        //find device: mixer or plate
        value = 0;
        device = 0;
        device = data[1][0] - '0';
        value = atoi(data[2]);
        itoa(value,valueAsString,10);  //repetitive MAY WANT TO REMOVE

        //update variables
        if (!strcmp(data[0], "PWM"))
        {
            USART_PutString("-->PWM ", plate_0);
            USART_PutString(data[1], plate_0);
            USART_PutString(" set to: ", plate_0);
            USART_PutString(valueAsString,plate_0);
            USART_PutString("\n\r", plate_0);

            //set value
            switch (device)
            {
            case 1:
                plate_1_pwm = value;
                break;
            case 2:
                plate_2_pwm = value;
                break;
            case 3:
                plate_3_pwm = value;
                break;
            default:
                break;
            }
        }
        else if (!strcmp(data[0], "TEMP"))
        {
            USART_PutString("-->TEMP ", plate_0);
           USART_PutString(data[1], plate_0);
           USART_PutString(" set to: ", plate_0);
           USART_PutString(valueAsString,plate_0);
           USART_PutString("\n\r", plate_0);

            
            //set value
            switch (device)
            {
            case 1:
                plate_1_temp = value;
                break;
            case 2:
                plate_2_temp = value;
                break;
            case 3:
                plate_3_temp = value;
                break;
            default:
                break;
            }

        }else if(!strcmp(data[0], "EXIT")){
            exit = 1;
        }else{
            USART_PutString("ERROR\n\r", plate_0);
        }

        
    }
    return 0;

}



void showFinalValue(){
    //used to hold atoi transformation
    char tempBuffer[4];

    //display final value

    //PLATE 1
    USART_PutString("\n\rPlate 1::: PWM = ",plate_0);
    itoa(plate_1_pwm,tempBuffer,10);
    USART_PutString(tempBuffer,plate_0);
    USART_PutString("  TEMP = ",plate_0);
    itoa(plate_1_temp,tempBuffer,10);
    USART_PutString(tempBuffer,plate_0);

    //PLATE 2
    USART_PutString("\n\rPlate 2::: PWM = ",plate_0);
    itoa(plate_2_pwm,tempBuffer,10);
    USART_PutString(tempBuffer,plate_0);
    USART_PutString("  TEMP = ",plate_0);
    itoa(plate_2_temp,tempBuffer,10);
    USART_PutString(tempBuffer,plate_0);

    //PLATE 3
    USART_PutString("\n\rPlate 3::: PWM = ",plate_0);
    itoa(plate_3_pwm,tempBuffer,10);
    USART_PutString(tempBuffer,plate_0);
    USART_PutString("  TEMP = ",plate_0);
    itoa(plate_3_temp,tempBuffer,10);
    USART_PutString(tempBuffer,plate_0);

    //ask if values are correct
    USART_PutString("\n\n\rPress 1 to confirm OR 0 to modify values: ",plate_0);
    char rx = UARTCharGet(UART0_BASE);
    if(rx == 48){  //"0"
        USART_PutString("\n\n\rEnter value commands..\n\r", plate_0);
        runSetUp();
        showFinalValue();
    }

    USART_PutString("\n\rSystem booting up...\n\r", plate_0);
}


// inline function to swap two numbers
inline void swap(char *x, char *y) 
{
	char t = *x; *x = *y; *y = t;
}

// function to reverse buffer[i..j]
char* reverse(char *buffer, int i, int j)
{
	while (i < j)
		swap(&buffer[i++], &buffer[j--]);

	return buffer;
}

// Iterative function to implement itoa() function in C
char* itoa(int value, char* buffer, int base)
{
	// invalid input
	if (base < 2 || base > 32)
		return buffer;

	// consider absolute value of number
	int n = abs(value);

	int i = 0;
	while (n)
	{
		int r = n % base;

		if (r >= 10) 
			buffer[i++] = 65 + (r - 10);
		else
			buffer[i++] = 48 + r;

		n = n / base;
	}

	// if number is 0
	if (i == 0)
		buffer[i++] = '0';

	// If base is 10 and value is negative, the resulting string 
	// is preceded with a minus sign (-)
	// With any other base, value is always considered unsigned
	if (value < 0 && base == 10)
		buffer[i++] = '-';

	buffer[i] = '\0'; // null terminate string

	// reverse the string and return it
	return reverse(buffer, 0, i - 1);
}

int main(void)
{
    //set up system clock
    SysCtlClockSet(
    SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ); //50Mhz == 400Mhz base /4(sys div)/2(internal)

    //enable buttons
    setUpButtonPress();

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



    printIntro();
    runSetUp();
    showFinalValue();


    /*
     //Start FSM
     while (1)
     {
     switch (state)
     {
     case STATE_IDLE:


     //turn mixer off & setPWM
     updatePWMCmd(plate_1_pwm);
     USART_PutCommand(mix_enable, plate_1);
     USART_PutCommand(setPWMCmd, plate_1);
     USART_PutCommand(mix_off, plate_1);

     //turn cooler on
     USART_PutCommand(plate_on, plate_1);

     //set cooler temp
     makeTempCmd(plate_1_temp);
     USART_PutCommand(setTempCmd, plate_1);

     GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
     2);

     while(flag1 != 1){}
     flag1 = 0;

     break;


     case STATE_START:
     //start mixing
     USART_PutCommand(mix_on, plate_1);


     GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 4);
     while(flag2 != 1){}
     flag2 = 0;


     break;
     case STATE_BAG1_RELEASE:
     //open valve 1
     USART_PutCommand(valve_open, plate_1);
     //wait
     delayMs(plate_1_time);
     //close valve
     USART_PutCommand(valve_close, plate_1);

     //turn plate 1 off
     USART_PutCommand(plate_off, plate_1);

     GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
     6);
     break;
     case STATE_BAG2_RELEASE:
     //open valve 1
     USART_PutCommand(valve_open, plate_1);
     //wait
     delayMs(plate_1_time);
     //close valve
     USART_PutCommand(valve_close, plate_1);

     //turn plate 1 off
     USART_PutCommand(plate_off, plate_1);

     //turn plate 2 off

     GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
     8);
     break;
     case STATE_BAG3_RELEASE:
     //open valve 1
     USART_PutCommand(valve_open, plate_1);
     //wait
     delayMs(plate_1_time);
     //close valve
     USART_PutCommand(valve_close, plate_1);

     //turn plate 1 off
     USART_PutCommand(plate_off, plate_1);

     GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
     10);
     break;
     case STATE_END:
     //turn mix off
     USART_PutCommand(mix_off, plate_1);

     //turn plate off
     USART_PutCommand(plate_off, plate_1);

     //open valves
     USART_PutCommand(valve_open, plate_1);

     GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 12);


     break;
     }

     //get next state after running state
     state = StateGetNext(state);

     }
     */
}
