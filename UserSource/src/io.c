#include "io.h"

#define		cIO_COUNT		(14)

const tGPIO_Line IOs[cIO_COUNT] = 	 {{ GPIOB, 12,  OUT_10MHz + OUT_PP, HIGH },  			// Dir1
																			{ GPIOB, 14, OUT_10MHz + OUT_PP, HIGH },	 			// En1
																			{ GPIOB, 13, OUT_10MHz + OUT_PP, HIGH },        // Step1
																			{ GPIOB, 1, OUT_10MHz + OUT_PP, HIGH },					// MS10
																			{ GPIOB, 10, OUT_10MHz + OUT_PP, HIGH },				// MS11
																			{ GPIOB, 11, OUT_10MHz + OUT_PP, HIGH },				// MS12
																			{ GPIOB, 8, IN_PULL, LOW },											// io_SNS1
																			
																			{ GPIOA, 1, OUT_10MHz + OUT_PP, HIGH },					// Dir2
																			{ GPIOA, 2, OUT_10MHz + OUT_PP, HIGH },					// En2
																			{ GPIOA, 0, OUT_10MHz + OUT_PP, HIGH },					// Step2
																			{ GPIOA, 3, OUT_10MHz + OUT_PP, HIGH },					// MS20
																			{ GPIOB, 6, OUT_10MHz + OUT_PP, HIGH },					// MS21
																			{ GPIOB, 7, OUT_10MHz + OUT_PP, HIGH },					// MS22
																			{ GPIOB, 9, IN_PULL, LOW }											// io_SNS2
																			
																			//{ GPIOA, 1, OUT_10MHz + OUT_PP, HIGH },					// Dir2
																			//{ GPIOA, 2, OUT_10MHz + OUT_PP, HIGH },					// En2
																			//{ GPIOA, 0, OUT_10MHz + OUT_PP, HIGH },					// Step2
																			//{ GPIOA, 3, OUT_10MHz + OUT_PP, HIGH },					// MS20
																			//{ GPIOA, 4, OUT_10MHz + OUT_PP, HIGH },					// MS21
																			//{ GPIOA, 5, OUT_10MHz + OUT_PP, HIGH }					// MS22
																			//{ GPIOB, 7, IN_PULL, LOW }											// io_SNS3
};

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

bool IO_GetLine(tIOLine Line)
{
	if (Line <= cIO_COUNT)
		return ((IOs[Line].GPIOx->IDR) & (1 << IOs[Line].GPIO_Pin));
	else
		return false;
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
		IOs[Line].GPIOx->CRH &= ~(0x0F << ((IOs[Line].GPIO_Pin - 8) * 4));
		IOs[Line].GPIOx->CRH |= Mode << ((IOs[Line].GPIO_Pin - 8) * 4);
	}

	IOs[Line].GPIOx->ODR &= ~(1 << IOs[Line].GPIO_Pin);
	IOs[Line].GPIOx->ODR |= State << IOs[Line].GPIO_Pin;
}

void IO_Init(void)
{
	int Line = 0;
		
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

	for (Line = 0; Line < cIO_COUNT; Line++)
	{
		IO_ConfigLine(Line, IOs[Line].MODE, IOs[Line].DefState);
	}
}
