/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __INCLUDES_H
#define __INCLUDES_H

#include "main.h"
#include "stm32f4xx_hal.h"
#include "can.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

#include "RemoteTask.h"
#include "ControlTask.h"
#include "IMUTask.h"
#include "CANTask.h"
#include "pid_regulator.h"

#endif /* __INCLUDES_H */