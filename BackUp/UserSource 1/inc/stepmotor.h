#ifndef __stepmotor_h
#define __stepmotor_h

#include "stm32f10x.h"
#include "stdio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "io.h"

//#define FULL_STEP									1
//#define HALF_STEP									2
//#define QUARTER_STEP							4
//#define EIGHTH_STEP								8
//#define SIXTEENTH_STEP						16

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

void st_Rotate(tMotor Number, tMotorDir Direction, uint32_t N, uint16_t Speed);

#endif
