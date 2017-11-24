/**
  ******************************************************************************
  * File Name          : HMITask.h
  * Description        : 人机交互任务，目前是OLED显示屏和4*4按键
  ******************************************************************************
  *
  * Copyright (c) 2018 Team TPP-Shanghai Jiao Tong University
  * All rights reserved.
  *
  ******************************************************************************
  */
#ifndef __HMITASK_H
#define __HMITASK_H

#include "includes.h"

void HMILoop();

void Oled_Init(void);

#endif /*__ HMITASK_H */
