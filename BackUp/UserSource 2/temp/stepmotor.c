#include "stepmotor.h"

void stepmotor_init(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN;
	
	GPIOB->CRH &= ~(GPIO_CRH_CNF12 | GPIO_CRH_CNF13 | GPIO_CRH_CNF14 | GPIO_CRH_CNF15);
	GPIOB->CRH |= (GPIO_CRH_MODE12_0 | GPIO_CRH_MODE13_0 | GPIO_CRH_MODE14_0 | GPIO_CRH_MODE15_0);
	
	GPIOA->CRH &= ~(GPIO_CRH_CNF8 | GPIO_CRH_CNF11);
	GPIOA->CRH |= (GPIO_CRH_MODE8_0 | GPIO_CRH_MODE11_0);
}

void st_Step(tMotor Number, tMotorDir Direction)
{
	IO_SetLine(Motor[Number].DIR,Direction);
	IO_SetLine(Motor[Number].STEP,LOW);
	taskYIELD();
	IO_SetLine(Motor[Number].STEP,HIGH);
}

void st_Rotate(tMotor Number, tMotorDir Direction, uint32_t N, uint16_t Speed)
{
	uint32_t i = 0;
	IO_SetLine(Motor[Number].EN, ON);
	for(i = 0; i < N; i++)
	{
		st_Step(Number, Direction);
		vTaskDelay(Speed);
	}
}

void st_vMotorR(void *pvParameters)
{
	st_Rotate(st_MotorR, CCW, 1000, 1);
	vTaskDelete(NULL);
}

void st_vMotorL(void *pvParameters)
{
	st_Rotate(st_MotorL, CCW, 1000, 1);
	vTaskDelete(NULL);
}
