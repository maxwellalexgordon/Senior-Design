#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
int main(void)
{

    uint32_t ui32ADC0Value [4]; //Set ui32ADC0Value as a 4 element array of unsigend 32 bit integer data type.
    volatile uint32_t adcval; //Set adcval as the averaged data to be read from ADC.
    SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0); //Enable ADC0.
    //Set the sequencer 1, with 4 sample storage capacity to be triggered by the processor with highest priority.
    ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
    //Set the sequencer 1's first 3 samples to come from ADC pin 0 or PE3.
    ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH0);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_CH0);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_CH0);
    //Set the sequencer 3's sample 3 to be the final sample taken from ADC input pin 0 or PE3 and generate an interrupt when sample is set.
    ADCSequenceStepConfigure(ADC0_BASE, 1, 3, ADC_CTL_CH0|ADC_CTL_IE|ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, 1); //Enable ADC0's sample sequencer 1.
    while(1)
    {
        ADCIntClear(ADC0_BASE, 1); //Clear the interrupt.
        ADCProcessorTrigger(ADC0_BASE, 1); //Trigger the ADC to start converting and taking samples.
        while(!ADCIntStatus(ADC0_BASE, 1, false)) //Wait for all samples to be put in the sequencer.
        {
        }
        ADCSequenceDataGet(ADC0_BASE, 1, ui32ADC0Value); //Acquire the value captured in the sequencer.
        //Set adcval to the average of the captured values. Add 2 for rounding off errors of four 0.5s.
        adcval = (ui32ADC0Value[0] + ui32ADC0Value[1] + ui32ADC0Value[2] + ui32ADC0Value[3] + 2)/4;

    }
}
