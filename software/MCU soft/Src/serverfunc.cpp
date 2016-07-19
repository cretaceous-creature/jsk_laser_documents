/**************************
	 Serve File
**************************/
#include"common.h"

/****************************************************
Name:     Ivoid DelayUs(INT32U dwTime)
Function: Delay time in us's
Input:     INT32U dwTime: Delay times
Output:   
****************************************************/

void DelayUs(uint32_t dwTime)
{
    uint32_t dwCurCounter=0;                                
    uint32_t dwPreTickVal=SysTick->VAL;                    
    uint32_t dwCurTickVal;                                 
    dwTime=dwTime*72;    
    for(;;){
        dwCurTickVal=SysTick->VAL;
        if(dwCurTickVal<dwPreTickVal){
            dwCurCounter=dwCurCounter+dwPreTickVal-dwCurTickVal;
        }
        else{
            dwCurCounter=dwCurCounter+dwPreTickVal+SysTick->LOAD-dwCurTickVal;
        }
        dwPreTickVal=dwCurTickVal;
        if(dwCurCounter>=dwTime){
            return;
        }
    }
}


void Delay250ns(uint32_t dwTime)
{
    uint32_t dwCurCounter=0;                                
    uint32_t dwPreTickVal=SysTick->VAL;                    
    uint32_t dwCurTickVal;                                 
    dwTime=dwTime*18;    
    for(;;){
        dwCurTickVal=SysTick->VAL;
        if(dwCurTickVal<dwPreTickVal){
            dwCurCounter=dwCurCounter+dwPreTickVal-dwCurTickVal;
        }
        else{
            dwCurCounter=dwCurCounter+dwPreTickVal+SysTick->LOAD-dwCurTickVal;
        }
        dwPreTickVal=dwCurTickVal;
        if(dwCurCounter>=dwTime){
            return;
        }
    }
}
