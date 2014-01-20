/**
 * This is the main file
 */

#include <timer.h>
#include "mainapplication.h"

uint32 testVar = 2345u;

int main(void)
{
    uint8 ledTiming;
    
    SystemCoreClockUpdate();
   
    MainApplication_initializeHardware();
    MainApplication_initializeVariables();
    
    ledTiming = 0u;
    
    for (;;)
    {
        MainApplication_mainTask();
        MainApplication_buttonTask();
        
        if (ledTiming == 200u)   // led have to be changed only every 1000ms
        {
            MainApplication_ledTask();
            ledTiming = 0u;
        }
        ledTiming++;
        
        Timer_delayMs(5u);
     }
     
    return 0;
}
