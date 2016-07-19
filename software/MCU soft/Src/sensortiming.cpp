/**************************
	 Sensor Timing Generation
**************************/
//Head define everything
#include"common.h"

#define TRIG_ON HAL_GPIO_WritePin(GPIOA,GPIO_PIN_10,GPIO_PIN_SET);
#define TRIG_OFF HAL_GPIO_WritePin(GPIOA,GPIO_PIN_10,GPIO_PIN_RESET);

#define P_RES_ON HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_SET);
#define P_RES_OFF HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_RESET);

#define P_HIS_ON HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_SET);
#define P_HIS_OFF HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_RESET);


void Pixel_reset()
{
	P_RES_ON;
	DelayUs(20);  //10us delay,
	P_RES_OFF;
	DelayUs(2); //delay for 1us   not necessary
}

void Pixel_sample()
{
	P_HIS_ON;
	DelayUs(4);
	P_HIS_OFF;
	DelayUs(4);
}	

void Pixel_trig()
{
	TRIG_ON;
	TRIG_OFF;
	Delay250ns(2);
}


