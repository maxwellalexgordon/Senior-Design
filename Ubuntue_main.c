#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
/*
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
*/
#include <stdio.h>



//math help

int hexadecimalToDecimal(char hexVal[]) 
{    
    int len = strlen(hexVal); 
      
    // Initializing base value to 1, i.e 16^0 
    int base = 1; 
      
    int dec_val = 0; 
      
    // Extracting characters as digits from last character 
    for (int i=len-1; i>=0; i--) 
    {    
        // if character lies in '0'-'9', converting  
        // it to integral 0-9 by subtracting 48 from 
        // ASCII value. 
        if (hexVal[i]>='0' && hexVal[i]<='9') 
        { 
            dec_val += (hexVal[i] - 48)*base; 
                  
            // incrementing base by power 
            base = base * 16; 
        } 
  
        // if character lies in 'A'-'F' , converting  
        // it to integral 10 - 15 by subtracting 55  
        // from ASCII value 
        else if (hexVal[i]>='A' && hexVal[i]<='F') 
        { 
            dec_val += (hexVal[i] - 55)*base; 
          
            // incrementing base by power 
            base = base*16; 
        } 
    } 
      
    return dec_val; 
} 

// function to convert decimal to hexadecimal
void decToHexa(char* hex, int n)
{
    //check negative
    if(n<0){
 	n = 65536 + n;
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
    //	printf("%c\n",FULLCS[i]);
    //} 
   
    //add 2 LSB of check sum to main statement arr
    arr[7] = tolower(FULLCS[1]);
    arr[8] = tolower(FULLCS[0]);
   
    
   
   
}

void setTemp(float t){
     char setTemp[10] = {'*','1','c','0','0','0','0','0','0','*'};  //-1.5C
     
     int temp = (int)(t * 10);
     char hex_temp[4] = "0000";
     decToHexa(&hex_temp, temp);
	
     //print temp conversion
     // for(int i = 3; i>= 0; i--){
     //	printf("%c\n",hex_temp[i]);
     //} 
     //printf("\n\n\n");


     //insert temp convertion
     int count = 3;
     for(int i = 3; i<=6; i++){
	setTemp[i] = tolower(hex_temp[count--]);
    } 
	


    //calc check sum and add to setTemp string
    calcCheckSum(setTemp);

    //print full string
    printf("sending string: ");
    for(int i = 0; i<= 10; i++){
    	printf("%c",setTemp[i]);
    }
    printf("\n"); 

   
}

void getTempSensor(){
   char getTemp[10] ={'*','0','1','0','0','0','0','2','1','\r'};
   //read info

   //convert to int
   
   //print value
}


int main(void) {
   
    

  

}
