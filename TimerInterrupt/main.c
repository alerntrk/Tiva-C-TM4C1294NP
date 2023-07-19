#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"

#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
    while(1);
}
#endif


uint32_t freq;
uint8_t toggle;
uint32_t count;
void Timer1IntHandler(void){
    count++;
    toggle = (count % 2);

    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT); //Clear Timeout Flag

    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1, toggle);





}
void Timer_Init(){


    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);//Timer 1 clock hattý aktif et

    IntMasterEnable();//Ýþlemci IRQ i kabul etsin

    //TimerDisable(TIMER1_BASE,TIMER_A);//Timeri konfigur etmeden önce kapalý tut



    /* Timer1A konfigurasyýnlarý*/
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER1_BASE,TIMER_A , 6000000);// TimerLoadSet64 olabilir?

    IntEnable(INT_TIMER1A); // NVIC interrupt Enable
    TimerIntEnable(TIMER1_BASE,TIMER_TIMA_TIMEOUT);//TIM OUT flagýndan gelen interruptý aktif etsin

    TimerEnable(TIMER1_BASE, TIMER_A);//Timer Saymaya Baþlasýn


}
void GPIO_Init(){


    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0| GPIO_PIN_1);


}


uint32_t freq;

int
main(void)
{


    freq=SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);

    GPIO_Init();
    Timer_Init();

    while(1)
    {

    }
}
