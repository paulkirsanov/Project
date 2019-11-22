#ifndef MAIN_H_
#define MAIN_H_

#include "stm32f10x.h"

#include "led.h"
#include "usart.h"
#include "delay_tim4.h"
#include "stepmotor.h"
#include "io.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#define	ERROR_ACTION(CODE,POS)		do{}while(0)

#endif
