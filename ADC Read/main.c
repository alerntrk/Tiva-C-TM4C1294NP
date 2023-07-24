#include <stdbool.h>
#include <stdint.h>
#include <string.h>

//Tiva ware Includes
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/debug.h"
#include "driverlib/interrupt.h"
#include "driverlib/uart.h"
#include "driverlib/adc.h"
#include "driverlib/timer.h"
#include "utils/uartstdio.h"
#include "rom.h"
#include "rom_map.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"

//FreeRTOS includes
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"

uint8_t toggle1=0;
uint8_t toggle2=0;

#define BUFF_SIZE   50
TaskHandle_t volatile next_task_handle = NULL;
TaskHandle_t task1,task2;

uint8_t size;
char buff[BUFF_SIZE]="";
uint8_t* pt;

void UARTSend()
{
    pt=&buff[0];
    size=strlen(buff);
    while(size--)
    {

        UARTCharPutNonBlocking(UART0_BASE, *(pt++));
    }
}


void Task1Handler(void* pvParam1){

    while(1){
            GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0 , 0x1);
             GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_2,0x1); //PE2 pin is high



            // GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0 , ++toggle2 % 2);
            // vTaskDelay(pdMS_TO_TICKS(200));

             GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0 , 0x0);


        }


    }



void Task2Handler(void* pvParam2){


    while(1){

        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0 , 0x1);
        GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2,0x1); //PE2 pin is high



       // GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0 , ++toggle2 % 2);
       // vTaskDelay(pdMS_TO_TICKS(200));

        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0 , 0x0);

    }

}

void IRQButtonHandler(void){

    BaseType_t pxHigherPriorityTaskWoken;

    pxHigherPriorityTaskWoken = pdFALSE;

    GPIOIntClear(GPIO_PORTJ_BASE, GPIO_INT_PIN_0);//Clear the Pending Bit
    xTaskNotifyFromISR(next_task_handle,0,eNoAction,&pxHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);




}
uint32_t clk;
// ****** Constants ******
const int sampleFreq = 8000; // Sample Frequency

// ****** Variables ******
uint32_t ADC0Value[1];
uint16_t samplePeriod;
void InitSystem(){
    //Set system freq to 120MHz
    clk=SysCtlClockFreqSet(SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480,16000000U);

    //Enable clock for desired periphs
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    //UART Enable
    GPIOPinConfigure(0x001);
    GPIOPinConfigure(0x401);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTClockSourceSet(UART0_BASE,UART_CLOCK_SYSTEM);
/*    UARTConfigSetExpClk(UART0_BASE, 120000000U, 57600,
                                (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                                 UART_CONFIG_PAR_NONE));
                                 */
    UARTStdioConfig(0, 57600, 16000000U);



    //LEDs PORTF 0-1
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE , GPIO_PIN_0 | GPIO_PIN_1);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_1 ,GPIO_STRENGTH_12MA , GPIO_PIN_TYPE_STD);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_1,0);

    //LEDs PORTN 0-1
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    GPIOPadConfigSet(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1 ,GPIO_STRENGTH_12MA , GPIO_PIN_TYPE_STD);
    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1, 0);

    //Configure PE2 and PE1 as output for ossiloscope measurement
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE , GPIO_PIN_2 | GPIO_PIN_1);
    GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_2 | GPIO_PIN_1,GPIO_STRENGTH_12MA , GPIO_PIN_TYPE_STD);
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2 | GPIO_PIN_1,0);


    //User Button1 PJ0 Interrupt Enable

    GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0);
    GPIOPadConfigSet(GPIO_PORTJ_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOIntEnable(GPIO_PORTJ_BASE,GPIO_INT_PIN_0);
    GPIOIntTypeSet(GPIO_PORTJ_BASE, GPIO_PIN_0, GPIO_RISING_EDGE);
    IntPrioritySet(INT_GPIOJ, 0xD0);
    IntRegister(INT_GPIOJ, IRQButtonHandler);
    IntEnable(INT_GPIOJ);
    IntMasterEnable();




    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

    // *** ADC0
    ROM_ADCHardwareOversampleConfigure(ADC0_BASE,4);
    ROM_ADCReferenceSet(ADC0_BASE,ADC_REF_INT);
    ROM_ADCSequenceConfigure(ADC0_BASE,3,ADC_TRIGGER_TIMER,0);
    ROM_ADCSequenceStepConfigure(ADC0_BASE,3,0,ADC_CTL_TS | ADC_CTL_IE | ADC_CTL_END);
    ROM_ADCSequenceEnable(ADC0_BASE,3);

    // *** Timer0
    ROM_TimerConfigure(TIMER0_BASE,TIMER_CFG_PERIODIC);
    samplePeriod = 120000000U/sampleFreq;
    ROM_TimerLoadSet(TIMER0_BASE,TIMER_A,samplePeriod - 1);
    ROM_TimerControlTrigger(TIMER0_BASE,TIMER_A,true);

    // *** GPIO
    ROM_GPIOPinTypeADC(GPIO_PORTD_BASE,GPIO_PIN_0);

    // *** Interrupts
    ROM_IntEnable(INT_TIMER0A);
    ROM_TimerIntEnable(TIMER0_BASE,TIMER_TIMA_TIMEOUT);
    ROM_IntEnable(INT_ADC0SS3);
    ROM_ADCIntEnable(ADC0_BASE,3);

    ROM_IntMasterEnable();
    ROM_TimerEnable(TIMER0_BASE,TIMER_A);




}



int main(void)

{



    InitSystem();


    xTaskCreate(Task1Handler,"Task1",200,NULL,3,&task1);
    xTaskCreate(Task2Handler,"Task2",200,NULL,3,&task2);
    next_task_handle = task1;
    vTaskStartScheduler();




    while(1)
        {
        }


    return 0;
}
// Interrupts
void Timer0IntHandler(void){
    ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}
float TempValueC;
void ADC0SS3IntHandler(void){
    ROM_ADCIntClear(ADC0_BASE,3);

    ROM_ADCSequenceDataGet(ADC0_BASE,3,ADC0Value); // Get Data from ADC and store it in ADC0Value

    TempValueC = (147.5 - (float)((75.0*3.3 *(float)ADC0Value[0])) / 4096.0);
    UARTprintf("Raw ADC value is  = %3d*C \r", ADC0Value[0]);
}
void __error__(char *pcFilename, uint32_t ui32Line)
{
    // Place a breakpoint here to capture errors until logging routine is finished
    while (1)
    {
    }
}

