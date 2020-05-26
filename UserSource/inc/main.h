#ifndef MAIN_H_
#define MAIN_H_

#include "stm32f10x.h"

#include <stdbool.h>
#include "inttypes.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "io.h"
#include "stepmotor.h"

#include "timer.h"
#include "led.h"
#include "usart.h"
#include "spi.h"
#include "w25q16.h"

#define	ERROR_ACTION(CODE,POS)		do{}while(0)

#define RX_BUFFER_SIZE						9
	
#define configDEBUG_BOOTLOADER		1

#endif
