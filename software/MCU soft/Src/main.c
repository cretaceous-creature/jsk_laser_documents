/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2015 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f3xx_hal.h"
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "hrtim.h"
#include "iwdg.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include <string.h>
#include <math.h>
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
#define DAC_1V 1240
#define DAC_2_125V 2530
//#define LASER850


uint16_t ADC_DATABUFF_A[272];
uint16_t ADC_DATABUFF_B[272];

uint16_t ADC_RESET_A[272];
uint16_t ADC_RESET_B[272];

uint16_t ADC_AMBIENT_A[272];// per 100ms....
uint16_t ADC_AMBIENT_B[272];

float poly[7];


//status, for checking calculation in main while(1)
typedef enum {idle=0,converting=1,finished=2}ADC_STATUS;
typedef enum {ambient=0,normal=1}WORKING_STATUS;
//ADC and temporary working status
volatile ADC_STATUS ADCStatus;
WORKING_STATUS WORKINGStatus;
//distance data
uint16_t DISTANCE_DATA[274]; //use this to store distance data.
uint16_t TEST_DATA[8][274];
//reserved 
volatile uint16_t TXBUFF[274];
int TIMERCOUNTER = 0;
uint8_t UART_RE_DATA[2];
int pulse_num = 0;

float ambient_coef;

// A lot of parameters used to calculate the distance
float SR = 1.04; // sensitivity ratio
float Dofs = 2.0;  // distance offset
float Tilt = 1.0;   //tilt value 
float top,bot;
int record[8];
//these values could be changed by usart

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
extern void DelayUs(uint32_t dwTime);
extern void Delay250ns(uint32_t dwTime);
/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */
  DISTANCE_DATA[0] = 0xa1a2;
	DISTANCE_DATA[273] = 0x2b2a;
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_DAC1_Init();
  MX_HRTIM1_Init();
  MX_IWDG_Init();
  MX_TIM1_Init();
  MX_USART2_UART_Init();

  /* USER CODE BEGIN 2 */
	
	/****************************
	HRTIM CONFIGURATION:
			PB12=C1=VTX1    PB13=C2=VTX2 
			PB14=D1=VTX3    PA9=A2=PULSE	
	******************************/
	{
	//channel C TC1 and TC2    as VTX1 and VTX2
		//test!!! should be in a function
		{
			int hrtim_offset = -40;
#ifdef LASER850
			hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP2xR = 100 + 139;
			hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP3xR = 100 + 139 + 139;
			hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CMP3xR = 100 + 139 + 139 + 1;
		//pulse = 30nm
			hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CMP2xR = 139;
			hrtim_offset = 20;
			
#endif
			
			// using offset to define delay after the pulse......      905nm 20   850nm 0?  need to be test
			hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP1xR += hrtim_offset;
			hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP2xR += hrtim_offset;
			hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP3xR += hrtim_offset;
			
			// 5 is for the offset of VTX3  should be faster and slower than VTX1 and VTX2`
			hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CMP1xR += hrtim_offset-5;
			
						//this is the pulse remain constant
			hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CMP2xR += 0;
			
			hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CMP3xR += hrtim_offset+5;
		}	
		HAL_HRTIM_WaveformOutputStart(&hhrtim1,HRTIM_OUTPUT_TC1);	
		HAL_HRTIM_WaveformOutputStart(&hhrtim1,HRTIM_OUTPUT_TC2);	
		//channel D TD1   as VTX3   channel D TA2 as PULSE
		HAL_HRTIM_WaveformOutputStart(&hhrtim1,HRTIM_OUTPUT_TD1);
		HAL_HRTIM_WaveformOutputStart(&hhrtim1,HRTIM_OUTPUT_TA2);
		// start timer		
		HAL_HRTIM_WaveformCounterStart(&hhrtim1,HRTIM_TIMERID_TIMER_A|HRTIM_TIMERID_TIMER_C|HRTIM_TIMERID_TIMER_D);
	}
	

	/****************************
	DAC CONFIGURATION:
			PA5=DAC2=4.25V/2=2.125V	=	2638*3.3/4096
			PA4=DAC1=1.0V		=		1241*3.3/4096
	******************************/
{
	HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_1,DAC_ALIGN_12B_R,DAC_1V);
	HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_2,DAC_ALIGN_12B_R,DAC_2_125V);
	HAL_DAC_Start(&hdac1,DAC_CHANNEL_1);
	HAL_DAC_Start(&hdac1,DAC_CHANNEL_2);
}
	
	/****************************
		ADC_CONFIGURATION:
		BUFFER:    ADC_BUFF_A[272] and  ADC_BUFF_B[272];
	******************************/
{
	  //start calibration first
		HAL_ADCEx_Calibration_Start(&hadc1,ADC_SINGLE_ENDED);
		HAL_ADCEx_Calibration_Start(&hadc2,ADC_SINGLE_ENDED);
		HAL_Delay(1);	
		//then start DMA
	  HAL_ADC_Start_DMA(&hadc1,(uint32_t *)ADC_RESET_A,272);
		HAL_ADC_Start_DMA(&hadc2,(uint32_t *)ADC_RESET_B,272);
}
	/****************************
		UART Transmit and Receive
	******************************/
{

	//HAL_UART_Transmit_DMA(&huart1,(uint8_t *)ADC_BUFF_B,544);
	HAL_UART_Receive_IT(&huart2,UART_RE_DATA,sizeof(UART_RE_DATA));
//	HAL_UART_DMAPause(&huart2);

}
	/****************************
		TIM1 100us interrupt...
	******************************/
	 HAL_TIM_Base_Start_IT(&htim1);
	 WORKINGStatus = ambient;
   ADCStatus = idle;
		/****************************
	Independent Watch Dog
	******************************/
		// HAL_IWDG_Start(&hiwdg);
  int calcu_num = 0;
   poly[0] = 1.648001258107309e+06;
   poly[1] = -5.305835891559632e+06;
   poly[2] = 7.127857366587162e+06;
   poly[3] = -5.110547144753641e+06;
   poly[4] = 2.060999435889212e+06;
   poly[5] = -4.423848603771256e+05;
   poly[6] = 3.937815633389474e+04;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
		
		//HAL_Delay(1);
		if(ADCStatus==finished){
			ADCStatus = idle;  //idle mode...
			//switch status, to do the calculation....
			
			for(int i=0;i<272;i++){
				//Vreset2 - Vout2 - Vambient2     Cap2 discharge mount
				 top = ADC_RESET_B[i] - ADC_DATABUFF_B[i] - ambient_coef ;
				//(Vreset1 - Vout1 - Vambient1)*SR     Cap1 discharge mount
				 bot = SR * (ADC_RESET_A[i] - ADC_DATABUFF_A[i]) - ambient_coef;
				//light speed in air = 2.9952816
								if(i==176)  //conpensate for pixel 176
                    bot *= 1.04;
				float k = top/(bot+top);

				float dist;
				if(bot>2500||bot<0)
                       dist= 0;
        else if(bot+top>100 + (ambient_coef/0.053) * 0.4){ 
					dist = poly[0]*k*k*k*k*k*k + poly[1]*k*k*k*k*k + poly[2]*k*k*k*k
                               + poly[3]*k*k*k + poly[4]*k*k + poly[5]*k + poly[6]  + 3.4 - 10*k;
			}
				else
					dist = 0;
				//dist cant be minus
				dist = dist>0?dist:0;
				
				//then we need to consider to get the average distance data,
				//perhaps the last one is much more accurate....
				//need to be tested.
//			if(ADC_DATABUFF_A[i]<3500||ADC_DATABUFF_B[i]<3500){
//				dist = (dist + (float)DISTANCE_DATA[i]/10000)/2;
//			}
				//here we need to consider the saturation... 
				//1500 = 1.2V  saturation, will use the previous data
				if(ADC_DATABUFF_A[i]>2200&&ADC_DATABUFF_B[i]>2200){
					//&&ADC_DATABUFF_A[i]<3700&&ADC_DATABUFF_B[i]<3700){
					DISTANCE_DATA[i+1] = (uint16_t)(dist * 100);  //0~60000,  at most 6 meters....
					}
			  	//ADC_DATABUFF_B[i] = 0;
					//ADC_DATABUFF_A[i] = 0;
					
			} 
			record[calcu_num] = TIMERCOUNTER;
			memcpy(&TEST_DATA[calcu_num][274],DISTANCE_DATA,sizeof(DISTANCE_DATA));
			calcu_num++;
			if(calcu_num>=6)
				calcu_num = 0;
		}
			
	//	HAL_HRTIM_SoftwareReset(&hhrtim1,HRTIM_TIMERRESET_TIMER_A|HRTIM_TIMERRESET_TIMER_C|HRTIM_TIMERRESET_TIMER_D);
	 // HAL_UART_Receive_IT(&huart2,UART_RE_DATA,sizeof(UART_RE_DATA));
		
		//HAL_IWDG_Refresh(&hiwdg);
  }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_HRTIM1|RCC_PERIPHCLK_USART2
                              |RCC_PERIPHCLK_TIM1|RCC_PERIPHCLK_ADC12;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_SYSCLK;
  PeriphClkInit.Adc12ClockSelection = RCC_ADC12PLLCLK_DIV1;
  PeriphClkInit.Tim1ClockSelection = RCC_TIM1CLK_HCLK;
  PeriphClkInit.Hrtim1ClockSelection = RCC_HRTIM1CLK_PLLCLK;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

  HAL_RCC_MCOConfig(RCC_MCO, RCC_MCOSOURCE_HSE, RCC_MCO_DIV2);

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 1, 0);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
