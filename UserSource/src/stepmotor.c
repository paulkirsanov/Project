#include "stepmotor.h"

TaskHandle_t hMotor1, hMotor2, hMotor3;

tStepper Motor[STEPPER_COUNT] ={{ io_DIR1, io_STEP1, io_EN1, io_MS10, io_MS11, io_MS12, io_SNS1 },
                                { io_DIR2, io_STEP2, io_EN2, io_MS20, io_MS21, io_MS22, io_SNS2 }};

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
	tError Error;
	portBASE_TYPE eStatus = 0;
	
	IO_SetLine(Motor[Number].EN, OFF);
	
	set_resolution(Number, Mode);

	for(i = 0; i < N; i++)
	{
		if(xSemaphoreTake(Stopper, 0))
		{
			if(Number == st_Motor1)
				xQueuePeek(qError1, &Error, 0);
			else if(Number == st_Motor2)
				xQueuePeek(qError2, &Error, 0);
			else if(Number == st_Motor3)
				xQueuePeek(qError3, &Error, 0);
			
			if(Error.eDirection == Direction)
			{
				usart_send_data(USART1, 0x77);
			}
			else if(Error.eDirection != Direction)
			{
				if(Stopper == StopMotor1)
				{
					usart_send_data(USART1, 0x70);
					xQueueReceive(qError1, &Error, 0);
					Error.eDirection = Direction;
					eStatus = xQueueSendToBack(qError1, &Error, portMAX_DELAY);
					if(eStatus != pdPASS)
					{
						usart_send_string(USART1, "Could not send to the queue.\r\n");
					}
				}
				else if(Stopper == StopMotor2)
				{
					usart_send_data(USART1, 0x71);
					xQueueReceive(qError2, &Error, 0);
					Error.eDirection = Direction;
					eStatus = xQueueSendToBack(qError2, &Error, portMAX_DELAY);
					if(eStatus != pdPASS)
					{
						usart_send_string(USART1, "Could not send to the queue.\r\n");
					}
				}
				else if(Stopper == StopMotor3)
				{
					usart_send_data(USART1, 0x72);
					eStatus = xQueueSendToBack(qError3, &Error, portMAX_DELAY);
					if(eStatus != pdPASS)
					{
						usart_send_string(USART1, "Could not send to the queue.\r\n");
					}
				}
				break;
			}
		}
		else if(!xSemaphoreTake(Stopper, 0) && Error.eDirection != Direction)
		{
			usart_send_data(USART1, 0x78);
		}
		
		st_Step(Number,Direction);
		vTaskDelay(Speed);
	}
}

bool sensor_read(tMotor Number)
{	
	if(IO_GetLine(Motor[Number].SNS) == true)
		return true;
	else
		return false;
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

void st_vMotor3(void *pvParameters)
{
	tRotate Rcv;

	while(1)
	{
		if(xQueueReceive(qMotor3, &Rcv, portMAX_DELAY))
		{
			st_Rotate(Rcv.aMotor, Rcv.aDirection, Rcv.aSteps, Rcv.aSpeed, StopMotor3, Rcv.aMode);
			xSemaphoreGive(OK_Motor3);
		}
	}
}

void Stepper_Init(void)
{
	if(pdTRUE != xTaskCreate(st_vMotor1, "Motor1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, hMotor1))
		ERROR_ACTION(TASK_NOT_CREATE, 0);
	
	qMotor1 = xQueueCreate(2, sizeof(tRotate));
	if( qMotor1 != NULL )
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
	
	qError1 = xQueueCreate(1, sizeof(tError));
	if( qError1 != NULL )
	{
		#if(configDEBUG)
			usart_send_string(USART1, "Queue qError1 was create\r\n");
		#endif
	}

	if(pdTRUE != xTaskCreate(st_vMotor2, "Motor2", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, hMotor2))
		ERROR_ACTION(TASK_NOT_CREATE, 0);
	
	qMotor2 = xQueueCreate(2, sizeof(tRotate));
	if( qMotor2 != NULL )
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
	
	qError2 = xQueueCreate(1, sizeof(tError));
	if( qError2 != NULL )
	{
		#if(configDEBUG)
			usart_send_string(USART1, "Queue qError2 was create\r\n");
		#endif
	}

	if(pdTRUE != xTaskCreate(st_vMotor3, "Motor3", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, hMotor3))
		ERROR_ACTION(TASK_NOT_CREATE, 0);
	
	qMotor3 = xQueueCreate(2, sizeof(tRotate));
	if( qMotor3 != NULL )
	{
		#if(configDEBUG)
			usart_send_string(USART1, "Queue qMotor3 was create\r\n");
		#endif
	}
	
	StopMotor3 = xSemaphoreCreateBinary();
	if( StopMotor3 != NULL )
	{
		#if(configDEBUG)
			usart_send_string(USART1, "StopMotor3 was create\r\n");
		#endif
	}
	OK_Motor3 = xSemaphoreCreateBinary();
	if( OK_Motor3 != NULL )
	{
		#if(configDEBUG)
			usart_send_string(USART1, "OK_Motor3 was create\r\n");
		#endif
	}
	
	qError3 = xQueueCreate(1, sizeof(tError));
	if( qError3 != NULL )
	{
		#if(configDEBUG)
			usart_send_string(USART1, "Queue qError3 was create\r\n");
		#endif
	}
}
