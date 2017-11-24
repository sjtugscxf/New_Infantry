/**
  ******************************************************************************
  * File Name          : ShootTask.h
  * Description        : 射击任务（摩擦轮控制）
  ******************************************************************************
  *
  * Copyright (c) 2018 Team TPP-Shanghai Jiao Tong University
  * All rights reserved.
  *
  ******************************************************************************
  */
#ifndef __SHOOTTASK_H
#define __SHOOTTASK_H

#include "includes.h"

#define FRICTION_RAMP_TICK_COUNT				100
#define FRICTION_WHEEL_MAX_DUTY         1350



typedef __packed enum
{
	FRICTION_WHEEL_OFF = 0,
	FRICTION_WHEEL_START_TURNNING = 1,
	FRICTION_WHEEL_ON = 2,
}FrictionWheelState_e;

typedef __packed enum
{
	NOSHOOTING = 0,
	SHOOTING = 1,
}Shoot_State_e;

//斜坡部分开始
typedef struct RampGen_t
{
	int32_t count;
	int32_t XSCALE;
	float out;
	void (*Init)(struct RampGen_t *ramp, int32_t XSCALE);
	float (*Calc)(struct RampGen_t *ramp);
	void (*SetCounter)(struct RampGen_t *ramp, int32_t count);
	void (*ResetCounter)(struct RampGen_t *ramp);
	void (*SetScale)(struct RampGen_t *ramp, int32_t scale);
	uint8_t (*IsOverflow)(struct RampGen_t *ramp);
}RampGen_t;
#define RAMP_GEN_DEFAULT \
{ \
							.count = 0, \
							.XSCALE = 0, \
							.out = 0, \
							.Init = &RampInit, \
							.Calc = &RampCalc, \
							.SetCounter = &RampSetCounter, \
							.ResetCounter = &RampResetCounter, \
							.SetScale = &RampSetScale, \
							.IsOverflow = &RampIsOverflow, \
						} \
					
void RampInit(RampGen_t *ramp, int32_t XSCALE);
float RampCalc(RampGen_t *ramp);
void RampSetCounter(struct RampGen_t *ramp, int32_t count);
void RampResetCounter(struct RampGen_t *ramp);
void RampSetScale(struct RampGen_t *ramp, int32_t scale);
uint8_t RampIsOverflow(struct RampGen_t *ramp);
//结束

void RemoteShootControl(RemoteSwitch_t *sw, uint8_t val);
void MouseShootControl(Mouse *mouse);

#endif /* __SHOOTTASK_H */