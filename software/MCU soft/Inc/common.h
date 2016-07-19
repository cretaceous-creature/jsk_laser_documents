#ifndef COMMON_H
#define COMMON_H

#include "stm32f3xx_hal.h"
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "hrtim.h"
//#include "iwdg.h"
#include "usart.h"
#include "gpio.h"
#include "tim.h"

//ADC_BUFF
extern uint16_t ADC_DATABUFF_A[272];
extern uint16_t ADC_DATABUFF_B[272];
//ADC_RESETBUFF
extern uint16_t ADC_RESET_A[272];
extern uint16_t ADC_RESET_B[272];
//ADC_AMBIENTBUFF
extern uint16_t ADC_AMBIENT_A[272];// per 100ms....
extern uint16_t ADC_AMBIENT_B[272];
//Distance Buff
extern uint16_t DISTANCE_DATA[274];
extern uint16_t TEST_DATA[8][274];
extern int record[8];
//buff to transmit
extern uint16_t TXBUFF[274];
extern int TIMERCOUNTER;
	
extern char debug_flag;

extern float ambient_coef;

extern uint8_t UART_RE_DATA[2]; // receive data pointer..
extern int pulse_num;
void DelayUs(uint32_t dwTime);
void Delay250ns(uint32_t dwTime);

void Pixel_sample();
void Pixel_trig();
void Pixel_reset();

//status, for checking calculation in main while(1)
typedef enum {idle=0,converting=1,finished=2}ADC_STATUS;
typedef enum {ambient=0,normal=1}WORKING_STATUS;
extern ADC_STATUS ADCStatus;
extern WORKING_STATUS WORKINGStatus;

#define PACKET_START (uint8_t)0xf8
#define PACKET_END   (uint8_t)0x8f

#define ONEWORD 3
#define TWOWORDS 4

#endif
