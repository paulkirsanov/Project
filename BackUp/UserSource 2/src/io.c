#include "io.h"

#define		cIO_COUNT		(6)

const tGPIO_Line IOs[cIO_COUNT] = 	 {{ GPIOB, 12,  OUT_10MHz + OUT_PP, HIGH},  			// Dir1
																			{ GPIOB, 14, OUT_10MHz + OUT_PP, HIGH},	 				// En1
																			{ GPIOB, 13, OUT_10MHz + OUT_PP, HIGH},         // Step1
																			{ GPIOA, 1, OUT_10MHz + OUT_PP, HIGH},					// Dir2
																			{ GPIOA, 2, OUT_10MHz + OUT_PP, HIGH},					// En2
																			{ GPIOA, 0, OUT_10MHz + OUT_PP, HIGH}}; 		    // Step2

void IO_SetLine(tIOLine Line, bool State)
{
	if (State)
	{
		IOs[Line].GPIOx->BSRR = 1 << (IOs[Line].GPIO_Pin);
	}
	else
	{
		IOs[Line].GPIOx->BRR = 1 << (IOs[Line].GPIO_Pin);
	}
}

void IO_ConfigLine(tIOLine Line, uint8_t Mode, uint8_t State)
{
	if(IOs[Line].GPIO_Pin < 8)
		{
		IOs[Line].GPIOx->CRL &= ~(0x0F << (IOs[Line].GPIO_Pin * 4));
		IOs[Line].GPIOx->CRL |= Mode<<(IOs[Line].GPIO_Pin * 4);
		}
	else
		{
		IOs[Line].GPIOx->CRH &= ~(0x0F << ((IOs[Line].GPIO_Pin - 8)* 4));
		IOs[Line].GPIOx->CRH |= Mode<<((IOs[Line].GPIO_Pin - 8)* 4);
		}

	IOs[Line].GPIOx->ODR &= ~(1<<IOs[Line].GPIO_Pin);
	IOs[Line].GPIOx->ODR |= State<<IOs[Line].GPIO_Pin;
}

void IO_Init(void)
{
	int Line = 0;
		
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

//	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

	for (Line = 0; Line < cIO_COUNT; Line++)
	{
		IO_ConfigLine(Line, IOs[Line].MODE, IOs[Line].DefState);
	}
}
