#include "stepmotor.h"

#define STEPPER_COUNT (2)

tStepper Motor[STEPPER_COUNT] ={{io_DIR1, io_STEP1, io_EN1},
                                {io_DIR2, io_STEP2, io_EN2}};

void st_Step(tMotor Number, tMotorDir Direction)
{
	IO_SetLine(Motor[Number].DIR,Direction);

	IO_SetLine(Motor[Number].STEP,LOW);

	taskYIELD();

	IO_SetLine(Motor[Number].STEP,HIGH);
}

void st_Rotate(tMotor Number, tMotorDir Direction, uint32_t N, uint16_t Speed, SemaphoreHandle_t Stopper)
{
	uint32_t i;

	IO_SetLine(Motor[Number].EN,OFF);

	for(i=0;i<N;i++)
	{
		if(xSemaphoreTake(Stopper, 0)) break;
		st_Step(Number,Direction);
		vTaskDelay(Speed);
	}

//	IO_SetLine(Motor[Number].EN,ON);
}

void st_vMotorR (void *pvParameters)
{
	tRotate Rcv;

	while(1)
	{
		xQueueReceive(qMotorR, &Rcv, portMAX_DELAY);
		st_Rotate(Rcv.aMotor, Rcv.aDirection, Rcv.aSteps, Rcv.aSpeed, StopMotorR);
		xSemaphoreGive(OK_MotorR);
	}
}

void st_vMotorL (void *pvParameters)
{
	tRotate Rcv;

	while(1)
	{
		xQueueReceive(qMotorL, &Rcv, portMAX_DELAY);
		st_Rotate(Rcv.aMotor, Rcv.aDirection, Rcv.aSteps, Rcv.aSpeed, StopMotorL);
		xSemaphoreGive(OK_MotorL);
	}
}

void Stepper_Init(void)
{
	if(pdTRUE != xTaskCreate(st_vMotorR, "MotorR", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL))
		ERROR_ACTION(TASK_NOT_CREATE, 0);
	
	qMotorR = xQueueCreate(2, sizeof(tRotate));
	StopMotorR = xSemaphoreCreateBinary();
	OK_MotorR = xSemaphoreCreateBinary();


	if(pdTRUE != xTaskCreate(st_vMotorL, "MotorL", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL))
		ERROR_ACTION(TASK_NOT_CREATE, 0);
	
	qMotorL = xQueueCreate(2, sizeof(tRotate));
	StopMotorL = xSemaphoreCreateBinary();
	OK_MotorL = xSemaphoreCreateBinary();
}
