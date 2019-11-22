#ifndef __stepmotor_h
#define __stepmotor_h

#define configDEBUG	0

#include "stm32f10x.h"
#include "main.h"
#include "usart.h"

typedef struct
{
	tIOLine DIR;
	tIOLine STEP;
	tIOLine EN;
	tIOLine MS0;
	tIOLine MS1;
	tIOLine MS2;
} tStepper;

typedef enum
{
	st_Motor1 = 0,
	st_Motor2 = 1
} tMotor;

typedef enum
{
	CW = 0,
	CCW = 1
} tMotorDir;

typedef enum
{
	FULL_STEP = 0,
	HALF_STEP,
	QUARTER_STEP,
	EIGHTH_STEP,
	SIXTEENTH_STEP
} tMotorMode;

typedef struct
{
  tMotor aMotor;
  tMotorDir aDirection;
  uint16_t aSteps;
  uint8_t aSpeed;
	tMotorMode aMode;
} tRotate;

void Stepper_Init(void);

extern QueueHandle_t qMotor1, qMotor2;

extern SemaphoreHandle_t StopMotor1, StopMotor2;
extern SemaphoreHandle_t OK_Motor1, OK_Motor2;

#endif
