#include "main.h"

QueueHandle_t qMotor1, qMotor2, qMotor3;
QueueHandle_t qError1, qError2, qError3;

SemaphoreHandle_t StopMotor1 = NULL, StopMotor2 = NULL, StopMotor3 = NULL;
SemaphoreHandle_t OK_Motor1 = NULL, OK_Motor2 = NULL, OK_Motor3 = NULL;
SemaphoreHandle_t ISRFromUSARTHandle;

TaskHandle_t hBootLoader, hBlinker, hSender, hBlinkerBootLoader;

char buffer_rx[RX_BUFFER_SIZE] = "\0";
volatile uint8_t rx_index = 0;
uint8_t x = 0;

volatile static float CPU_USAGE = 0;

static tMotor sensor = st_Motor1;

void rcc_init(void);
void vBlinker(void *pvParameters);
void vSender(void *pvParameters);
void vBootLoader(void *pvParameters);
void vBlinkerBootLoader(void *pvParameters);

int main(void)
{
	rcc_init();
	usart1_init();
	led_init();
	spi1_init();
	
	IO_Init();
	Stepper_Init();

	timer2_init();
	timer3_init();
	
	if(pdTRUE != xTaskCreate(vBootLoader, "BootLoader", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &hBootLoader))
		ERROR_ACTION(TASK_NOT_CREATE, 0);
	
	if(pdTRUE != xTaskCreate(vBlinkerBootLoader, "BlinkerBootLoader", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &hBlinkerBootLoader))
		ERROR_ACTION(TASK_NOT_CREATE, 0);
	
	if(pdTRUE != xTaskCreate(vBlinker, "Blinker", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &hBlinker))
		ERROR_ACTION(TASK_NOT_CREATE, 0);
	
	if(pdTRUE != xTaskCreate(vSender, "Sender", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &hSender))
		ERROR_ACTION(TASK_NOT_CREATE, 0);
	
	vTaskStartScheduler();
	
	return 0;
}

void rcc_init(void)
{
	RCC->CR |= ((uint32_t)RCC_CR_HSEON);												//Enable HSE
	while(!(RCC->CR & RCC_CR_HSERDY));													//Ready start HSE
	
	FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY;					//Clock flash memory
	
	RCC->CFGR |= RCC_CFGR_HPRE_DIV1;														//AHB = SYSCLK/1
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;														//APB1 = HCLK/4
	RCC->CFGR |= RCC_CFGR_PPRE2_DIV4;														//APB2 = HCLK/4
	
	RCC->CFGR &= ~RCC_CFGR_PLLMULL;															//clear PLLMULL bits
	RCC->CFGR &= ~RCC_CFGR_PLLSRC;															//clear PLLSRC bits
	RCC->CFGR &= ~RCC_CFGR_PLLXTPRE;														//clear PLLXPRE bits
	
	RCC->CFGR |= RCC_CFGR_PLLSRC;																//source HSE
	RCC->CFGR |= RCC_CFGR_PLLXTPRE_HSE;													//sorce HSE/1 = 8MHz
	RCC->CFGR |= RCC_CFGR_PLLMULL9;															//PLL x9: clock = 8MHz * 9 = 72 MHz
	
	RCC->CR |= RCC_CR_PLLON;																		//enable PLL
	while(!(RCC->CR & RCC_CR_PLLRDY));													//wait till PLL is ready
	
	RCC->CFGR &= ~RCC_CFGR_SW;																	//clear SW bits
	RCC->CFGR |= RCC_CFGR_SW_PLL;																//select surce SYSCLK = PLL
	while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_1){}				//wait till PLL is used
}

void vBlinker(void *pvParameters)
{
	while(1)
	{
		GPIOC->BSRR |= GPIO_BSRR_BS13;
		vTaskDelay(500);
		GPIOC->BSRR |= GPIO_BSRR_BR13;
		vTaskDelay(500);
	}
}

void vBlinkerBootLoader(void *pvParameters)
{
	while(1)
	{
		GPIOC->BSRR |= GPIO_BSRR_BS13;
		vTaskDelay(100);
		GPIOC->BSRR |= GPIO_BSRR_BR13;
		vTaskDelay(100);
	}
}

void vSender(void *pvParameters)
{
	tError Err1, Err2;
	
	tRotate Sendit;
	portBASE_TYPE xStatus1 = 0, xStatus2 = 0, xStatus3 = 0;
	
	Err1.eDirection = NONE;
	Err2.eDirection = NONE;
	
	ISRFromUSARTHandle = xSemaphoreCreateBinary();
	
	while(1)
	{
		if(xSemaphoreTake(ISRFromUSARTHandle, portMAX_DELAY))
		{
			if((int)buffer_rx[0] == 0xF8)
			{
				if(buffer_rx[1] == 0x01)
				{
					Sendit.aMotor = st_Motor1;
				}
				else if(buffer_rx[1] == 0x02)
				{
					Sendit.aMotor = st_Motor2;
				}
				else if(buffer_rx[1] == 0x03)
				{
					Sendit.aMotor = st_Motor3;
				}
				else
				{
					break;
				}
				
				if(buffer_rx[2] == 0x52)
				{
					Sendit.aDirection = CW;
				}
				else if(buffer_rx[2] == 0x55)
				{
					Sendit.aDirection = CCW;
				}
				else
				{
					break;
				}
				
				if(buffer_rx[3] == 0x46)
				{
					Sendit.aMode = FULL_STEP;
				}
				else if(buffer_rx[3] == 0x48)
				{
					Sendit.aMode = HALF_STEP;
				}
				else if(buffer_rx[3] == 0x51)
				{
					Sendit.aMode = QUARTER_STEP;
				}
				else if(buffer_rx[3] == 0x45)
				{
					Sendit.aMode = EIGHTH_STEP;
				}
				else if(buffer_rx[3] == 0x53)
				{
					Sendit.aMode = SIXTEENTH_STEP;
				}
				else
				{
					break;
				}
				
				Sendit.aSteps = buffer_rx[4] << 8 | buffer_rx[5];
				Sendit.aSpeed = buffer_rx[6] << 8 | buffer_rx[7];
				
				if((int)buffer_rx[8] == 0xE0)
				{
					if(Sendit.aMotor == st_Motor1)
					{
						/*if(xQueueReceive(qError1, &Err1, 0))
						{
							if(Sendit.aDirection == Err1.eDirection)
							{
								usart_send_data(USART1, 0x15);
							}
						}
						else
						{*/
							xStatus1 = xQueueSendToBack(qMotor1, &Sendit, portMAX_DELAY);
							if(xStatus1 != pdPASS)
							{
								usart_send_string(USART1, "Could not send to the queue.\r\n");
							}
						//}
					} else if(Sendit.aMotor == st_Motor2)
					{
						/*if(xQueuePeek(qError2, &Err2, 0))
						{
							if(Sendit.aDirection == Err2.eDirection)
							{
								usart_send_data(USART1, 0x16);
							}
							else
							{*/
								/*------------------- something here to do ----------------------*/
								xStatus2 = xQueueSendToBack(qMotor2, &Sendit, portMAX_DELAY);
								if(xStatus2 != pdPASS)
								{
									usart_send_string(USART1, "Could not send to the queue.\r\n");
								}
							//}
						/*}
						else
						{
							xStatus2 = xQueueSendToBack(qMotor2, &Sendit, portMAX_DELAY);
							if(xStatus2 != pdPASS)
							{
								usart_send_string(USART1, "Could not send to the queue.\r\n");
							}
						}*/
					} else if(Sendit.aMotor == st_Motor3)
					{
						xStatus3 = xQueueSendToBack(qMotor3, &Sendit, portMAX_DELAY);
						if(xStatus3 != pdPASS)
						{
							usart_send_string(USART1, "Could not send to the queue.\r\n");
						}
					}
				}
				else
				{
					break;
				}
				
				for(x = 0; x < 8; x++)
				{
					buffer_rx[x] = 0;
				}
				rx_index = 0;
			}
			else
			{
				for(x = 0; x < 8; x++)
				{
					buffer_rx[x] = 0;
				}
				rx_index = 0;
			}
		}
	}
}

void vBootLoader(void *pvParameters)
{
	#if(configDEBUG_BOOTLOADER)
//	tError Err;
//	tRotate Sendit;
//	tMotor i = st_Motor1;
	
	tError Error;
	uint32_t DeviceID = 0;
	uint32_t FlashID = 0;
	#endif
	
	NVIC_DisableIRQ(TIM2_IRQn);
	NVIC_DisableIRQ(TIM3_IRQn);
	vTaskSuspend(hBlinker);
	vTaskSuspend(hSender);
	
	vTaskDelay(2000);
	
	Error.eDirection = NONE;
	xQueueSendToBack(qError1, &Error, portMAX_DELAY);
	xQueueSendToBack(qError2, &Error, portMAX_DELAY);
	xQueueSendToBack(qError3, &Error, portMAX_DELAY);
	
	#if(configDEBUG_BOOTLOADER)
	
	DeviceID = w25q16_ReadDeviceID();
	FlashID = w25q16_ReadID();
	if(FlashID == sFLASH_ID)
	{
		usart_send_data(USART1, 0x25);
	}
	else
	{
		usart_send_data(USART1, 0x26);
	}
	
	/*for(i = st_Motor1; i < st_Motor2; i++)
	{
		Sendit.aMotor = i;
		Sendit.aDirection = CW;
		Sendit.aMode = QUARTER_STEP;
		Sendit.aSpeed = 1;
		Sendit.aSteps = 5000;
		
		xQueueSendToBack(qMotor1, &Sendit, portMAX_DELAY);
		
		if(i == st_Motor1)
		{
			xQueueReceive(qError1, &Err, 0);
			if(Sendit.aDirection == Err.eDirection)
			{
				break;
			}
		} else if(i == st_Motor2)
		{
			xQueueReceive(qError2, &Err, 0);
			if(Sendit.aDirection == Err.eDirection)
			{
				break;
			}
		}
	}*/
	#endif
	
	NVIC_EnableIRQ(TIM2_IRQn);
	NVIC_EnableIRQ(TIM3_IRQn);
	vTaskResume(hBlinker);
	vTaskResume(hSender);
	vTaskSuspend(hBlinkerBootLoader);
	vTaskSuspend(hBootLoader);
}

/*void Sender(void *pvParameters)
{
	tRotate Sendit;
	portBASE_TYPE xStatus1 = 0, xStatus2 = 0;
	
	Sendit.aMotor = st_Motor1;
	Sendit.aDirection = CW;
	Sendit.aSteps = 500;
	Sendit.aSpeed = 1;
	Sendit.aMode = QUARTER_STEP;

	xStatus1 = xQueueSendToBack(qMotor1, &Sendit, portMAX_DELAY);
	if(xStatus1 != pdPASS)
	{
		usart_send_string(USART1, "Could not send to the queue.\r\n");
	}
	
	Sendit.aMotor = st_Motor2;
	Sendit.aDirection = CCW;
	Sendit.aSteps = 500;
	Sendit.aSpeed = 1;
	Sendit.aMode = FULL_STEP;
	
	xStatus2 = xQueueSendToBack(qMotor2, &Sendit, portMAX_DELAY);
	if(xStatus2 != pdPASS)
	{
		usart_send_string(USART1, "Could not send to the queue.\r\n");
	}
	
	vTaskDelay(1000);
 
	xSemaphoreGive(StopMotor2);
 
	vTaskDelay(5000);
 
	xSemaphoreGive(StopMotor1);
	
	vTaskDelete(NULL);
}*/

void USART1_IRQHandler(void)
{
	static portBASE_TYPE xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;
	
	if(USART1->SR & USART_SR_RXNE)
	{
		USART1->SR &= ~USART_SR_RXNE;
		buffer_rx[rx_index++] = USART1->DR;
		
		if(rx_index == RX_BUFFER_SIZE)
		{
			rx_index = 0;
			
			xSemaphoreGiveFromISR(ISRFromUSARTHandle, &xHigherPriorityTaskWoken);

			if( xHigherPriorityTaskWoken == pdTRUE )
			{
				portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
			}
		}
	}
}

void TIM2_IRQHandler(void)
{
	static portBASE_TYPE xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;
	
	if( TIM2->SR & TIM_SR_UIF )
	{
		TIM2->SR &= ~TIM_SR_UIF;
		
		/*if( sensor_read(sensor) == false )
		{
			if( sensor == st_Motor1 )
				xSemaphoreGiveFromISR(StopMotor1, &xHigherPriorityTaskWoken);
			else if( sensor == st_Motor2 )
				xSemaphoreGiveFromISR(StopMotor2, &xHigherPriorityTaskWoken);
			
			if( xHigherPriorityTaskWoken == pdTRUE )
				portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
		}
		
		if( sensor == st_Motor2 )
			sensor = st_Motor1;
		else
			sensor++;*/
	}
}

void TIM3_IRQHandler(void)
{
	if( TIM3->SR & TIM_SR_UIF )
	{
		TIM3->SR &= ~TIM_SR_UIF;
		
		if(xSemaphoreTake(OK_Motor1, 0))
		{
			usart_send_data(USART1, 0x27);
		} else if(xSemaphoreTake(OK_Motor2, 0))
		{
			usart_send_data(USART1, 0x28);
		} else if(xSemaphoreTake(OK_Motor3, 0))
		{
			usart_send_data(USART1, 0x29);
		}
	}
}

void vApplicationIdleHook(void)
{
	static portTickType LastTick;
	static int count;
	static int max_count;
	
	count++;
	if(xTaskGetTickCount()- LastTick > 1000)
	{
		LastTick = xTaskGetTickCount();
		if(count > max_count) max_count = count;
		CPU_USAGE = 100 -(100 * ((float)count / (float)max_count));
		
//		usart_send_data(USART1, 0x02);
//		usart_send_data(USART1, 0x72);
		
		count = 0;
	}
}
