#ifndef STEPMOTOR_H_
#define STEPMOTOR_H_

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

typedef struct
{
	tIOLine DIR;
	tIOLine STEP;
	tIOLine EN;
} tStepper;

void stepmotor_init(void);
void st_vMotorR(void *pvParameters);
void st_vMotorL(void *pvParameters);
																
#endif
