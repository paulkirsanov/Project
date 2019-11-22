#include "stepmotor.h"

#define STEPPER_COUNT (2)

tStepper Motor[STEPPER_COUNT] ={{io_DIR1, io_STEP1, io_EN1, io_MS10, io_MS11, io_MS12},
                                {io_DIR2, io_STEP2, io_EN2, io_MS20, io_MS21, io_MS22}};

void st_Step(tMotor Number, tMotorDir Direction)
{
	IO_SetLine(Motor[Number].DIR, Direction);

	IO_SetLine(Motor[Number].STEP, LOW);

	taskYIELD();

	IO_SetLine(Motor[Number].STEP, HIGH);
}

void set_resolution(tMotor Number, tMotorMode resolution)
{
	switch(resolution)
	{
		case FULL_STEP:
			IO_SetLine(Motor[Number].MS0, LOW);
			IO_SetLine(Motor[Number].MS1, LOW);
			IO_SetLine(Motor[Number].MS2, LOW);
			break;
		case HALF_STEP:
			IO_SetLine(Motor[Number].MS0, HIGH);
			IO_SetLine(Motor[Number].MS1, LOW);
			IO_SetLine(Motor[Number].MS2, LOW);
			break;
		case QUARTER_STEP:
			IO_SetLine(Motor[Number].MS0, LOW);
			IO_SetLine(Motor[Number].MS1, HIGH);
			IO_SetLine(Motor[Number].MS2, LOW);
			break;
		case EIGHTH_STEP:
			IO_SetLine(Motor[Number].MS0, HIGH);
			IO_SetLine(Motor[Number].MS1, HIGH);
			IO_SetLine(Motor[Number].MS2, LOW);
			break;
		case SIXTEENTH_STEP:
			IO_SetLine(Motor[Number].MS0, HIGH);
			IO_SetLine(Motor[Number].MS1, HIGH);
			IO_SetLine(Motor[Number].MS2, HIGH);
			break;
		default:
			break;
	}
}

void st_Rotate(tMotor Number, tMotorDir Direction, uint32_t N, uint16_t Speed, SemaphoreHandle_t Stopper, tMotorMode Mode)
{
	uint32_t i;

	IO_SetLine(Motor[Number].EN, OFF);
	
	set_resolution(Number, Mode);

	for(i = 0; i < N; i++)
	{
//		if(xSemaphoreTake(Stopper, 0))
//			break;
		st_Step(Number,Direction);
		vTaskDelay(Speed);
	}

//	IO_SetLine(Motor[Number].EN,ON);
}

void st_vMotor1(void *pvParameters)
{
	tRotate Rcv;

	while(1)
	{
		if(xQueueReceive(qMotor1, &Rcv, portMAX_DELAY))
		{
			st_Rotate(Rcv.aMotor, Rcv.aDirection, Rcv.aSteps, Rcv.aSpeed, StopMotor1, Rcv.aMode);
			xSemaphoreGive(OK_Motor1);
		}
	}
}

void st_vMotor2(void *pvParameters)
{
	tRotate Rcv;

	while(1)
	{
		if(xQueueReceive(qMotor2, &Rcv, portMAX_DELAY))
		{
			st_Rotate(Rcv.aMotor, Rcv.aDirection, Rcv.aSteps, Rcv.aSpeed, StopMotor2, Rcv.aMode);
			xSemaphoreGive(OK_Motor2);
		}
	}
}

void Stepper_Init(void)
{
	if(pdTRUE != xTaskCreate(st_vMotor1, "Motor1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL))
		ERROR_ACTION(TASK_NOT_CREATE, 0);
	
	qMotor1 = xQueueCreate(2, sizeof(tRotate));
	if(qMotor1 != NULL)
	{
		#if(configDEBUG)
			usart_send_string(USART1, "Queue qMotor1 was create\r\n");
		#endif
	}
		
	StopMotor1 = xSemaphoreCreateBinary();
	if( StopMotor1 != NULL )
	{
		#if(configDEBUG)
			usart_send_string(USART1, "StopMotor1 was create\r\n");
		#endif
	}
	OK_Motor1 = xSemaphoreCreateBinary();
	if( OK_Motor1 != NULL )
	{
		#if(configDEBUG)
			usart_send_string(USART1, "OK_Motor1 was create\r\n");
		#endif
	}

	if(pdTRUE != xTaskCreate(st_vMotor2, "Motor2", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL))
		ERROR_ACTION(TASK_NOT_CREATE, 0);
	
	qMotor2 = xQueueCreate(2, sizeof(tRotate));
	if(qMotor2 != NULL)
	{
		#if(configDEBUG)
			usart_send_string(USART1, "Queue qMotor2 was create\r\n");
		#endif
	}
	
	StopMotor2 = xSemaphoreCreateBinary();
	if( StopMotor2 != NULL )
	{
		#if(configDEBUG)
			usart_send_string(USART1, "StopMotor2 was create\r\n");
		#endif
	}
	OK_Motor2 = xSemaphoreCreateBinary();
	if( OK_Motor2 != NULL )
	{
		#if(configDEBUG)
			usart_send_string(USART1, "OK_Motor2 was create\r\n");
		#endif
	}
}
