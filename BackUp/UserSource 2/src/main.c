#include "main.h"

void rcc_init(void);
void vBlinker(void *arg);
void Sender (void *pvParameters);

int main(void)
{
	rcc_init();
	usart1_init();
	led_init();
	
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
	
	GPIOB->CRL &= ~GPIO_CRL_CNF1;
	GPIOB->CRL |= GPIO_CRL_MODE1_0;
	
	GPIOB->CRH &= ~(GPIO_CRH_CNF10 | GPIO_CRH_CNF11);
	GPIOB->CRH |= (GPIO_CRH_MODE10_0 | GPIO_CRH_MODE11_0);
	
	GPIOB->BSRR |= GPIO_BSRR_BR1;
	GPIOB->BSRR |= GPIO_BSRR_BS10;
	GPIOB->BSRR |= GPIO_BSRR_BR11;
	
	IO_Init();
	Stepper_Init();

	if(pdTRUE != xTaskCreate(vBlinker, "Blinker", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL))
		ERROR_ACTION(TASK_NOT_CREATE, 0);
	
	if(pdTRUE != xTaskCreate(Sender, "Sender", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL))
		ERROR_ACTION(TASK_NOT_CREATE,0);
	
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

void Sender (void *pvParameters)
{
	tRotate Sendit;

	Sendit.aMotor = st_MotorL;
	Sendit.aDirection = CCW;
	Sendit.aSteps = 300;
	Sendit.aSpeed = 1;

	xQueueSend(qMotorL, &Sendit, 4);

	Sendit.aMotor = st_MotorR;
	Sendit.aDirection = CCW;
	Sendit.aSteps = 200;
	Sendit.aSpeed = 1;

	xQueueSend(qMotorR, &Sendit, 4);
	
	vTaskDelete(NULL);
}

void USART1_IRQHandler(void)
{
	if(USART1->SR & USART_SR_RXNE)
	{
		USART1->SR &= ~USART_SR_RXNE;
		
		if(USART1->DR == '0')
			usart_send_string(USART1, "Receive character zero\r\n");
	}
}
