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
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"

//FreeRTOS includes
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

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
    //
    // Loop while there are more characters to send.
    //
    pt=&buff[0];
    size=strlen(buff);
    while(size--)
    {
        //
        // Write the next character to the UART.
        //
        UARTCharPutNonBlocking(UART0_BASE, *(pt++));
    }
}


void Task1Handler(void* pvParam1){


    BaseType_t status;
    /*
    vTaskSuspend(task2);
    strcpy(buff,"task1 is running\n");
    vTaskResume(task2);
    UARTSend();
*/
    while(1){
        vTaskSuspendAll();
        strcpy(buff,"task1 is running\n");
        xTaskResumeAll();
        UARTSend();
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1, ++toggle1 % 2);
        status=xTaskNotifyWait(0,0,NULL,pdMS_TO_TICKS(1600));
        if(pdTRUE==status){
            portENTER_CRITICAL();
            strcpy(buff,"task1 is deleted\n");
            UARTSend();
            next_task_handle = task2;
            GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1, 0x01);
            strcpy(buff,"task1 is deleted\n");
            UARTSend();
            portEXIT_CRITICAL();
            vTaskDelete(NULL);



        }


    }

}

void Task2Handler(void* pvParam2){

    BaseType_t status;

    while(1){

        vTaskSuspendAll();
        strcpy(buff,"task2 is running\n");
        xTaskResumeAll();
        UARTSend();
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_1, ++toggle2 % 2);
        status = xTaskNotifyWait(0,0,NULL,pdMS_TO_TICKS(1000));
        if(pdTRUE == status){
            portENTER_CRITICAL();   //Disable all the hw interrupts
            strcpy(buff,"task2 is deleted\n");
            UARTSend();
            next_task_handle = NULL;
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_1, 0x1);

            portEXIT_CRITICAL();
            vTaskDelete(NULL);



        }

    }

}

void IRQButtonHandler(void){

    BaseType_t pxHigherPriorityTaskWoken;

    pxHigherPriorityTaskWoken = pdFALSE;

    GPIOIntClear(GPIO_PORTJ_BASE, GPIO_INT_PIN_0);//Clear the Pending Bit
    xTaskNotifyFromISR(next_task_handle,0,eNoAction,&pxHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);

   /* strcpy(buff,"Buttton IRQ is running\n");
    UARTSend();
*/


}
uint32_t clk;
void InitSystem(){
    //Set system freq to 120MHz
    clk=SysCtlClockFreqSet(SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480,120000000U);

    //Enable clock for desired periphs
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    //UART Enable
    GPIOPinConfigure(0x001);
    GPIOPinConfigure(0x401);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTClockSourceSet(UART0_BASE,UART_CLOCK_SYSTEM);
    UARTConfigSetExpClk(UART0_BASE, 120000000U, 57600,
                                (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                                 UART_CONFIG_PAR_NONE));



    //LEDs PORTF 0-1
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE , GPIO_PIN_0 | GPIO_PIN_1);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_1 ,GPIO_STRENGTH_12MA , GPIO_PIN_TYPE_STD);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_1,0);

    //LEDs PORTN 0-1
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    GPIOPadConfigSet(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1 ,GPIO_STRENGTH_12MA , GPIO_PIN_TYPE_STD);
    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1, 0);

    //User Button1 PJ0 Interrupt Enable

    GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0);
    GPIOPadConfigSet(GPIO_PORTJ_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOIntEnable(GPIO_PORTJ_BASE,GPIO_INT_PIN_0);
    GPIOIntTypeSet(GPIO_PORTJ_BASE, GPIO_PIN_0, GPIO_RISING_EDGE);
    IntPrioritySet(INT_GPIOJ, 0xD0);
    IntRegister(INT_GPIOJ, IRQButtonHandler);
    IntEnable(INT_GPIOJ);
    IntMasterEnable();




}




int main(void)
{



    InitSystem();
    UARTCharPutNonBlocking(UART0_BASE, 'V');
    UARTCharPutNonBlocking(UART0_BASE, 'V');
    UARTCharPutNonBlocking(UART0_BASE, 'V');


    xTaskCreate(Task1Handler,"Task1",200,NULL,3,&task1);
    xTaskCreate(Task2Handler,"Task2",200,NULL,3,&task2);
    next_task_handle = task1;
    vTaskStartScheduler();





    return 0;
}
void __error__(char *pcFilename, uint32_t ui32Line)
{
    // Place a breakpoint here to capture errors until logging routine is finished
    while (1)
    {
    }
}
