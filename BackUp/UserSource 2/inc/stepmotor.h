#ifndef __stepmotor_h
#define __stepmotor_h

#include "stm32f10x.h"
#include "main.h"

typedef struct
{
	tIOLine DIR;
	tIOLine STEP;
	tIOLine EN;
} tStepper;

typedef enum
{
	st_MotorR = 0,
	st_MotorL = 1
} tMotor;

typedef enum
{
	CW = 0,
	CCW = 1
} tMotorDir;

typedef struct
{
  tMotor aMotor;
  tMotorDir aDirection;
  uint16_t aSteps;
  uint8_t aSpeed;
} tRotate;

typedef enum
{
	FULL_STEP = 0,
	HALF_STEP,
	QUARTER_STEP,
	EIGHTH_STEP,
	SIXTEENTH_STEP
} tMotorMode;

void Stepper_Init(void);

static QueueHandle_t qMotorR, qMotorL;

static SemaphoreHandle_t StopMotorR, StopMotorL;
static SemaphoreHandle_t OK_MotorR, OK_MotorL;

#endif
