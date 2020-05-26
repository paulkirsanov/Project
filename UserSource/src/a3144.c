#include "a3144.h"

//tHall Sensor[SENSOR_COUNT] = {io_SNS1};

/*void a3144_init(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
	GPIOB->CRH &= ~(GPIO_CRH_MODE9 | GPIO_CRH_CNF9);
	GPIOB->CRH |= GPIO_CRH_CNF9_1;
	GPIOB->ODR &= ~GPIO_ODR_ODR9;
}

uint8_t a3144_read(void)
{
	if(!(GPIOB->IDR & GPIO_IDR_IDR9))
		return 0;
	else
		return 1;
}*/

/*uint8_t a3144_read(hSensor Number)
{	
	if(IO_GetLine(Sensor[Number].PIN) == true)
		return 0;
	else
		return 1;
}*/
