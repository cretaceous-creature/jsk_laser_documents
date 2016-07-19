/**************************
	 Interrupt Callback Functions
**************************/
//Head define everything
#include"common.h"
#include <string.h>
/**************************
	 DMA_AD Interrupt CallBacks
	 1. HAL_ADC_ErrorCallback 
	     where errors of DMA happens.
	 2. HAL_ADC_ConvHalfCpltCallback
       half of the DMA transfer complete.	
	 3. HAL_ADC_ConvCpltCallback
       all of the DMA transfer complete.	 (important)			 
**************************/
//TIM1 update callback

static int FRAMECOUNT = 0;
//When reach this pulse number, processor will calculate the 
//distance data, set 10
//#define HIGHFREQ

#ifdef HIGHFREQ
#define ptcbuffsize 2
static int Pulse_to_calcu[ptcbuffsize] = {10,100};
#define stoppulse 200
#else
#define ptcbuffsize 6
static int Pulse_to_calcu[ptcbuffsize] = {10,100,200,400,550,700};
#define stoppulse 1000
#endif

#define framenum 2   //every 10 frame one ambient measure



//DMA error happens
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef* hadc)
{
		if(hadc->Instance==hadc1.Instance){
			//ADC1
			while(1);
		}
		else{
			//ADC2
			while(1);
		}

}
//DMA half transfered
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
	//	if(hadc->Instance==hadc1.Instance){
	//		ADCStatus = converting;
	//	}
	//	else{
	//		ADCStatus = converting;
	//	}	

}
//DMA transfer complete and ADC finish status
volatile char flag1, flag2, flag3;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
//		if(hadc->Instance==hadc2.Instance){
//			//ADC1
//			flag1 = 1;
//		  HAL_ADC_Stop_DMA(&hadc2);
//		//	memcpy(&TXBUFF[2],&ADC_BUFF_A,sizeof(ADC_BUFF_A));
//		}
//		else if(hadc->Instance==hadc1.Instance){
//			//ADC2
//			flag2 = 1;
//			HAL_ADC_Stop_DMA(&hadc2);
//		//	memcpy(&TXBUFF[2+sizeof(ADC_BUFF_A)],&ADC_BUFF_B,sizeof(ADC_BUFF_B));
//		}
//		
//		if(flag1&&flag2){
//			flag1 = 0;
//			flag2 = 0;
//			ADCStatus = idle;			
//			if(flag3){
//				flag3 = 0;
//			//status change, state machine model
//				ADCStatus = finished;
//				volatile int k = TIMERCOUNTER;
//			}

//		}
}

static uint16_t Cover_data[272]; 
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	  if(htim->Instance==htim1.Instance){

			//HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
     
								//ALL channel A, C and D.
				HAL_HRTIM_SoftwareReset(&hhrtim1,HRTIM_TIMERRESET_TIMER_A
				|HRTIM_TIMERRESET_TIMER_C|HRTIM_TIMERRESET_TIMER_D);
				if(!TIMERCOUNTER++){ //first time
					// put the data into RESET BUFFer
					flag3 = 0;
				  memcpy(&DISTANCE_DATA[1],Cover_data,sizeof(Cover_data));
					HAL_ADC_Start_DMA(&hadc1,(uint32_t *)ADC_RESET_A,272);
					HAL_ADC_Start_DMA(&hadc2,(uint32_t *)ADC_RESET_B,272);
					Pixel_reset();
					Pixel_sample();
					Pixel_trig();

					ADCStatus = idle;
				}
				else if(ADC_RESET_B[100]<3940){
					TIMERCOUNTER = 0;
				}
				else{
					for(int i=0;i<ptcbuffsize;i++){
						if(TIMERCOUNTER == Pulse_to_calcu[i]){
							//put the data into Ambient buff
							HAL_ADC_Start_DMA(&hadc1,(uint32_t *)ADC_DATABUFF_A,272);
							HAL_ADC_Start_DMA(&hadc2,(uint32_t *)ADC_DATABUFF_B,272);
							flag3 = 1;
							Pixel_sample();
							Pixel_trig();
							
							DelayUs(80);
							ADCStatus = finished;
							ambient_coef = (float)(TIMERCOUNTER * 0.053);  //10 pulse minus 0.53 
							break;
						}
					}			
					//end the the calculation and send the data to PC
					if(TIMERCOUNTER > stoppulse){ 
						//transmit the data
						if(FRAMECOUNT++>framenum){					
							FRAMECOUNT = 0;
						}
					  DISTANCE_DATA[177] = DISTANCE_DATA[176];
						DISTANCE_DATA[17] = DISTANCE_DATA[18];
						memcpy(TXBUFF,DISTANCE_DATA,sizeof(DISTANCE_DATA));
						
					  if(DISTANCE_DATA[100] < 1500)
						{	volatile int kk = 0;
							kk++;
						}
						HAL_UART_Transmit_DMA(&huart2,(uint8_t *)TXBUFF,sizeof(TXBUFF));
					  TIMERCOUNTER = 0;
					}
				}
				
			
		
		}
	
}

