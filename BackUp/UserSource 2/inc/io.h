#ifndef __io_h
#define __io_h

#include "stm32f10x.h"
#include <stdbool.h>

#define IN	  	  (0x00)
#define OUT_10MHz (0x01)
#define OUT_2MHz  (0x02)
#define OUT_50MHz (0x03)

#define OUT_PP   (0x00)
#define OUT_OD   (0x04)
#define OUT_APP  (0x08)
#define OUT_AOD  (0x0C)

typedef enum
{
	io_DIR1 = 0,
	io_EN1,
	io_STEP1,
	io_DIR2,
	io_EN2,
	io_STEP2
} tIOLine;

typedef struct
{
	GPIO_TypeDef* GPIOx;
	uint16_t GPIO_Pin;
	uint8_t MODE;
	uint8_t DefState;
} tGPIO_Line;

typedef enum
{
	OFF = 0,
	ON = 1,
	LOW = 0,
	HIGH =1
} tIOState;

void IO_Init(void);
void IO_SetLine(tIOLine Line, bool State);
void IO_ConfigLine(tIOLine Line, uint8_t Mode, uint8_t State);

#endif
