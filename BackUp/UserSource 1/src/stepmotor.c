#include "stepmotor.h"

#define STEPPER_COUNT (4)

tStepper Motor[STEPPER_COUNT] ={{io_DIR1, io_STEP1, io_EN1},
                                {io_DIR2, io_STEP2, io_EN2}};

void st_Step(tMotor Number, tMotorDir Direction)
{
	IO_SetLine(Motor[Number].DIR,Direction);

	IO_SetLine(Motor[Number].STEP,LOW);

	taskYIELD();

	IO_SetLine(Motor[Number].STEP,HIGH);
}

void st_Rotate(tMotor Number, tMotorDir Direction, uint32_t N, uint16_t Speed)
{
	uint32_t i;

	IO_SetLine(Motor[Number].EN,OFF);

	for(i=0;i<N;i++)
	{
		st_Step(Number,Direction);
		vTaskDelay(Speed);
	}

//	IO_SetLine(Motor[Number].EN,ON);
}
