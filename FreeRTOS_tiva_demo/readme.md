# Tiva TM4C1294 Launchpad FreeRTOS Demo #

This firmware package is intended to provide a starting point for creating a FreeRTOS 10.2 based project in Code Composer Studio for the Tiva TM4C1294 launchpad, which is currently not supplied by either FreeRTOS or Texas Instruments.

## Required Components ##
* Code Composer Studio (v9.1 used for this project)
* [TM4C1294 Connected Launchpad](http://www.ti.com/tool/ek-tm4c1294xl)
* FreeRTOS Kernel


## Expected Output ##
There are two task each of them is supposed to be blink a user LED(on board) and button interrupt is activated .If any button press is detected IRQ handler notify task1 and task1 deletes itself.Similarly if a second button press is occured then task2 is deleted.

## Notes ##
This software is provided as-is!
