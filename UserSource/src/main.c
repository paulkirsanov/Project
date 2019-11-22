#include "main.h"

QueueHandle_t qMotor1, qMotor2;

SemaphoreHandle_t StopMotor1, StopMotor2;
SemaphoreHandle_t OK_Motor1, OK_Motor2;
SemaphoreHandle_t ISRFromUSARTHandle;

char buffer_rx[RX_BUFFER_SIZE] = "\0";
volatile uint8_t rx_index = 0;
uint8_t x = 0;

void rcc_init(void);
void vBlinker(void *pvParameters);
void Sender(void *pvParameters);
void Sender_1(void *pvParameters);

int main(void)
{
	rcc_init();
	usart1_init();
	led_init();
	
	IO_Init();
	Stepper_Init();

	if(pdTRUE != xTaskCreate(vBlinker, "Blinker", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL))
		ERROR_ACTION(TASK_NOT_CREATE, 0);
	
	if(pdTRUE != xTaskCreate(Sender_1, "Sender_1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL))
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

void Sender_1(void *pvParameters)
{
	tRotate Sendit;
	portBASE_TYPE xStatus1 = 0, xStatus2 = 0;
	
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
				
				if(buffer_rx[2] == 0x52)
				{
					Sendit.aDirection = CW;
				}
				else if(buffer_rx[2] == 0x4C)
				{
					Sendit.aDirection = CCW;
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
				
				Sendit.aSteps = buffer_rx[4] << 8 | buffer_rx[5];
				
				Sendit.aSpeed = buffer_rx[6] << 8 | buffer_rx[7];
				
				if((int)buffer_rx[8] == 0xE0)
				{
					if(Sendit.aMotor == st_Motor1)
					{
						xStatus1 = xQueueSendToBack(qMotor1, &Sendit, portMAX_DELAY);
						if(xStatus1 != pdPASS)
						{
							usart_send_string(USART1, "Could not send to the queue.\r\n");
						}
					} else
					{
						xStatus2 = xQueueSendToBack(qMotor2, &Sendit, portMAX_DELAY);
						if(xStatus2 != pdPASS)
						{
							usart_send_string(USART1, "Could not send to the queue.\r\n");
						}
					}
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

void Sender(void *pvParameters)
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
}

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
