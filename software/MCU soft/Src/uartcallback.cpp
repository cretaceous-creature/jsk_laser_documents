/**************************
	 UART Receive Interface
**************************/
//Head define everything
#include"common.h"

/**************************
	  UART depack data procedure
**************************/

volatile static uint8_t buf[20], pos, size;
volatile static int state;
void static print_to_buf(uint8_t *buff, uint8_t size)
{
  if(size==ONEWORD)
		{
     uint8_t flag=*(uint8_t *)&buff[0];
     if(flag==0x01)
		 {
				// receive RESET
			 short tmp =  *(short *)&buff[1];
			 Pixel_reset();
			 Pixel_sample();
			 Pixel_trig();
		 }
		 else if(flag==0x02)
		 {
			 // receive SAMPLE
			 short tmp =  *(short *)&buff[1];
			 Pixel_sample();
			 Pixel_trig();
			 HAL_HRTIM_SoftwareReset(&hhrtim1,HRTIM_TIMERRESET_TIMER_C|HRTIM_TIMERRESET_TIMER_D);
		 }
		}
  
  else if(size==TWOWORDS)
		{
				//reserved   
    }
}


uint16_t static depacket(uint8_t rxbyte)
{

	switch (state) {
	case 0:
		if (rxbyte == PACKET_START)
			state = 1;
		break;
	case 1:
		size = rxbyte;
		state = size; //datalength
    pos=0;
		break;
  case 2:
		if(rxbyte == PACKET_END)
			print_to_buf(buf, size);
      state=0;
		break;
	case 3:
		buf[pos++] = rxbyte;  //01 +short or 02+short
		if (pos >= size)
			state = 2;
		break;
  case 4:
    buf[pos++] = rxbyte;  //short + short
		if (pos >= size)
			state =2;
                break;
  case 8:
    buf[pos++] = rxbyte;  
		if (pos >= size)
			state =2;
                break;
	default:
						state = 0;
            //USART1_Puts("wrong receive data");      
            return 0;
            break;
	}	
        return 1;
}

/**************************
	  UART receive Interrupt CallBacks
		 	 1. HAL_UART_ERRORCallback
	     where errors of DMA happens.
	     2. HAL_UART_TxHALFCpltCallback
       half of the DMA transfer complete.	
	     3. HAL_UART_TxCpltCallback
       all of the DMA transfer complete.	 (important)	
			 4. HAL_UART_RxHalfCpltCallback
			 half of the data had been received
			 5. HAL_UART_RxCpltCallback
			 all of the data had been received   (important)
**************************/

//DMA uart transmit error occurs
void HAL_UART_ERRORCallback(UART_HandleTypeDef *huart)
{


}
//DMA uart transmit half of the data
void HAL_UART_TxHALFCpltCallback(UART_HandleTypeDef *huart)
{


}
//DMA uart transmit complete
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
//	HAL_ADC_Start_DMA(&hadc2,(uint32_t *)ADC_BUFF_B,272);
	
}
//uart receive half of the data
void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{

}
//uart receive complete
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart==&huart2)
	{
	  if(UART_RE_DATA[0]=='R')
			{
			  Pixel_reset();
			  
			  Pixel_sample();
			  Pixel_trig();
				pulse_num = UART_RE_DATA[1]*10;
			}
		 else if(UART_RE_DATA[0]=='S')
			{
				Pixel_sample();
				Pixel_trig();
				pulse_num = UART_RE_DATA[3]*10;
		
			}
		 else if(UART_RE_DATA[0]=='P')
		 {
				Pixel_sample();
				Pixel_trig();
				HAL_HRTIM_SoftwareReset(&hhrtim1,HRTIM_TIMERRESET_TIMER_C|HRTIM_TIMERRESET_TIMER_D);
			}
		
		}
			
}



