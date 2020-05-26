#ifndef __stepmotor_h
#define __stepmotor_h

#define configDEBUG	0

#include "stm32f10x.h"
#include "main.h"
#include "usart.h"

#define STEPPER_COUNT (2)

typedef struct
{
	tIOLine DIR;
	tIOLine STEP;
	tIOLine EN;
	tIOLine MS0;
	tIOLine MS1;
	tIOLine MS2;
	tIOLine SNS;
} tStepper;

typedef enum
{
	st_Motor1 = 0,
	st_Motor2 = 1,
	st_Motor3 = 2
} tMotor;

typedef enum
{
	CW = 0,
	CCW = 1,
	NONE = 2
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

typedef struct
{
  tMotorDir eDirection;
} tError;

void Stepper_Init(void);
bool sensor_read(tMotor Number);

extern QueueHandle_t qMotor1, qMotor2, qMotor3;
extern QueueHandle_t qError1, qError2, qError3;

extern SemaphoreHandle_t StopMotor1, StopMotor2, StopMotor3;
extern SemaphoreHandle_t OK_Motor1, OK_Motor2, OK_Motor3;

#endif
