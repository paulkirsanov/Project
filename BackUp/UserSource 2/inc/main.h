#ifndef MAIN_H_
#define MAIN_H_

#include "stm32f10x.h"

#include <stdbool.h>
#include "inttypes.h"

#include "led.h"
#include "usart.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "io.h"
#include "stepmotor.h"

#define	ERROR_ACTION(CODE,POS)		do{}while(0)

#endif
