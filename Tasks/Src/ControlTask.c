#include "includes.h"

WorkState_e WorkState = PREPARE_STATE;
uint16_t prepare_time = 0;

PID_Regulator_t CMRotatePID = CHASSIS_MOTOR_ROTATE_PID_DEFAULT; 
PID_Regulator_t CM1SpeedPID = CHASSIS_MOTOR_SPEED_PID_DEFAULT;
PID_Regulator_t CM2SpeedPID = CHASSIS_MOTOR_SPEED_PID_DEFAULT;
PID_Regulator_t CM3SpeedPID = CHASSIS_MOTOR_SPEED_PID_DEFAULT;
PID_Regulator_t CM4SpeedPID = CHASSIS_MOTOR_SPEED_PID_DEFAULT;

static uint8_t s_yawCount = 0;
static uint8_t s_pitchCount = 0;
static uint8_t s_CMFLCount = 0;
static uint8_t s_CMFRCount = 0;
static uint8_t s_CMBLCount = 0;
static uint8_t s_CMBRCount = 0;
int16_t CMFLIntensity = 0, CMFRIntensity = 0, CMBLIntensity = 0, CMBRIntensity = 0;
int16_t yawIntensity = 0;		
int16_t pitchIntensity = 0;

void CMControlInit(void)
{
	CMRotatePID.Reset(&CMRotatePID);
	CM1SpeedPID.Reset(&CM1SpeedPID);
	CM2SpeedPID.Reset(&CM2SpeedPID);
	CM3SpeedPID.Reset(&CM3SpeedPID);
	CM4SpeedPID.Reset(&CM4SpeedPID);
}

void ControlCMFL(void)
{		
	if(s_CMFLCount == 1)
	{		
		CM1SpeedPID.ref =  ChassisSpeedRef.forward_back_ref*0.075 
											 + ChassisSpeedRef.left_right_ref*0.075 
											 + ChassisSpeedRef.rotate_ref;	
		CM1SpeedPID.ref = 160 * CM1SpeedPID.ref;
			
			
		CM1SpeedPID.fdb = CMFLRx.RotateSpeed;

		CM1SpeedPID.Calc(&CM1SpeedPID);
		CMFLIntensity = CHASSIS_SPEED_ATTENUATION * CM1SpeedPID.output;
			
		s_CMFLCount = 0;
	}
	else
	{
		s_CMFLCount++;
	}
}

void ControlCMFR(void)
{		
	if(s_CMFRCount == 1)
	{		
		CM2SpeedPID.ref = - ChassisSpeedRef.forward_back_ref*0.075 
										 + ChassisSpeedRef.left_right_ref*0.075 
										 + ChassisSpeedRef.rotate_ref;
		CM2SpeedPID.ref = 160 * CM2SpeedPID.ref;
			
			
		CM2SpeedPID.fdb = CMFRRx.RotateSpeed;

		CM2SpeedPID.Calc(&CM2SpeedPID);
		CMFRIntensity = CHASSIS_SPEED_ATTENUATION * CM2SpeedPID.output;
			
		s_CMFRCount = 0;
	}
	else
	{
		s_CMFRCount++;
	}
}

void ControlCMBL(void)
{		
	if(s_CMBLCount == 1)
	{		
		CM3SpeedPID.ref =  ChassisSpeedRef.forward_back_ref*0.075 
											 - ChassisSpeedRef.left_right_ref*0.075 
											 + ChassisSpeedRef.rotate_ref;
		CM3SpeedPID.ref = 160 * CM3SpeedPID.ref;
			
			
		CM3SpeedPID.fdb = CMBLRx.RotateSpeed;

		CM3SpeedPID.Calc(&CM3SpeedPID);
		CMBLIntensity = CHASSIS_SPEED_ATTENUATION * CM3SpeedPID.output;
			
		s_CMBLCount = 0;
	}
	else
	{
		s_CMBLCount++;
	}
}

void ControlCMBR(void)
{		
	if(s_CMBRCount == 1)
	{		
		CM4SpeedPID.ref = - ChassisSpeedRef.forward_back_ref*0.075 
											 - ChassisSpeedRef.left_right_ref*0.075 
											 + ChassisSpeedRef.rotate_ref;
		CM4SpeedPID.ref = 160 * CM4SpeedPID.ref;
			
			
		CM4SpeedPID.fdb = CMBRRx.RotateSpeed;

		CM4SpeedPID.Calc(&CM4SpeedPID);
		CMBRIntensity = CHASSIS_SPEED_ATTENUATION * CM4SpeedPID.output;
			
		s_CMBRCount = 0;
	}
	else
	{
		s_CMBRCount++;
	}
}

void WorkStateFSM(void)
{
	switch (WorkState)
	{
		case PREPARE_STATE:
		{
			if (inputmode == STOP) WorkState = STOP_STATE;
			
			if(prepare_time<5000) prepare_time++;
			if(prepare_time == 3000) GYRO_RST();
			if(prepare_time == 5000)
			{
				WorkState = NORMAL_STATE;
				prepare_time = 0;
			}
		}break;
		case NORMAL_STATE:
		{
			if (inputmode == STOP) WorkState = STOP_STATE;
		}break;
		case STOP_STATE:
		{
			if (inputmode == REMOTE_INPUT)
			{
				WorkState = PREPARE_STATE;
				RemoteTaskInit();
			}
		}break;
	}
}

void setCMMotor()
{
	CanTxMsgTypeDef pData;
	CMGMMOTOR_CAN.pTxMsg = &pData;
	
	CMGMMOTOR_CAN.pTxMsg->StdId = CM_TXID;
	CMGMMOTOR_CAN.pTxMsg->ExtId = 0;
	CMGMMOTOR_CAN.pTxMsg->IDE = CAN_ID_STD;
	CMGMMOTOR_CAN.pTxMsg->RTR = CAN_RTR_DATA;
	CMGMMOTOR_CAN.pTxMsg->DLC = 0x08;
	
	CMGMMOTOR_CAN.pTxMsg->Data[0] = (uint8_t)(CMFLIntensity >> 8);
	CMGMMOTOR_CAN.pTxMsg->Data[1] = (uint8_t)CMFLIntensity;
	CMGMMOTOR_CAN.pTxMsg->Data[2] = (uint8_t)(CMFRIntensity >> 8);
	CMGMMOTOR_CAN.pTxMsg->Data[3] = (uint8_t)CMFRIntensity;
	CMGMMOTOR_CAN.pTxMsg->Data[4] = (uint8_t)(CMBLIntensity >> 8);
	CMGMMOTOR_CAN.pTxMsg->Data[5] = (uint8_t)CMBLIntensity;
	CMGMMOTOR_CAN.pTxMsg->Data[6] = (uint8_t)(CMBRIntensity >> 8);
	CMGMMOTOR_CAN.pTxMsg->Data[7] = (uint8_t)CMBRIntensity;

//	__disable_irq() ;
	//HAL_NVIC_DisableIRQ(CAN1_TX_IRQn);
	//HAL_NVIC_DisableIRQ(CAN2_TX_IRQn);
	HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
	HAL_NVIC_DisableIRQ(CAN2_RX0_IRQn);
	HAL_NVIC_DisableIRQ(USART1_IRQn);
	HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);
	if(HAL_CAN_Transmit_IT(&CMGMMOTOR_CAN) != HAL_OK)
	{
		Error_Handler();
	}
	//HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);
	//HAL_NVIC_EnableIRQ(CAN2_TX_IRQn);
	HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
	HAL_NVIC_EnableIRQ(CAN2_RX0_IRQn);
	HAL_NVIC_EnableIRQ(USART1_IRQn);
	HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
//	__enable_irq() ;
}

void setGMMotor()
{
	CanTxMsgTypeDef pData;
	CMGMMOTOR_CAN.pTxMsg = &pData;
	
	CMGMMOTOR_CAN.pTxMsg->StdId = GM_TXID;
	CMGMMOTOR_CAN.pTxMsg->ExtId = 0;
	CMGMMOTOR_CAN.pTxMsg->IDE = CAN_ID_STD;
	CMGMMOTOR_CAN.pTxMsg->RTR = CAN_RTR_DATA;
	CMGMMOTOR_CAN.pTxMsg->DLC = 0x08;
	
	CMGMMOTOR_CAN.pTxMsg->Data[0] = (uint8_t)(yawIntensity >> 8);
	CMGMMOTOR_CAN.pTxMsg->Data[1] = (uint8_t)yawIntensity;
	CMGMMOTOR_CAN.pTxMsg->Data[2] = (uint8_t)(pitchIntensity >> 8);
	CMGMMOTOR_CAN.pTxMsg->Data[3] = (uint8_t)pitchIntensity;
	CMGMMOTOR_CAN.pTxMsg->Data[4] = 0;
	CMGMMOTOR_CAN.pTxMsg->Data[5] = 0;
	CMGMMOTOR_CAN.pTxMsg->Data[6] = 0;
	CMGMMOTOR_CAN.pTxMsg->Data[7] = 0;

//	__disable_irq() ;
	//HAL_NVIC_DisableIRQ(CAN1_TX_IRQn);
	//HAL_NVIC_DisableIRQ(CAN2_TX_IRQn);
	HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
	HAL_NVIC_DisableIRQ(CAN2_RX0_IRQn);
	HAL_NVIC_DisableIRQ(USART1_IRQn);
	HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);
	if(HAL_CAN_Transmit_IT(&CMGMMOTOR_CAN) != HAL_OK)
	{
		Error_Handler();
	}
	//HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);
	//HAL_NVIC_EnableIRQ(CAN2_TX_IRQn);
	HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
	HAL_NVIC_EnableIRQ(CAN2_RX0_IRQn);
	HAL_NVIC_EnableIRQ(USART1_IRQn);
	HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
//	__enable_irq() ;
}

#define NORMALIZE_ANGLE180(angle) angle = ((angle) > 180) ? ((angle) - 360) : (((angle) < -180) ? (angle) + 360 : angle)
fw_PID_Regulator_t pitchPositionPID = fw_PID_INIT(8.0, 0.0, 0.0, 10000.0, 10000.0, 10000.0, 10000.0);
fw_PID_Regulator_t yawPositionPID = fw_PID_INIT(5.0, 0.0, 0.5, 10000.0, 10000.0, 10000.0, 10000.0);
fw_PID_Regulator_t pitchSpeedPID = fw_PID_INIT(40.0, 0.0, 15.0, 10000.0, 10000.0, 10000.0, 3500.0);
fw_PID_Regulator_t yawSpeedPID = fw_PID_INIT(30.0, 0.0, 5, 10000.0, 10000.0, 10000.0, 4000.0);
#define yaw_zero 4708  //100
#define pitch_zero 6400
float yawRealAngle = 0.0;
float pitchRealAngle = 0.0;
float gap_angle = 0.0;
void ControlRotate(void)
{
	gap_angle  = (GMYAWRx.angle - yaw_zero) * 360 / 8192.0f;
  NORMALIZE_ANGLE180(gap_angle);	
	
	if(WorkState == NORMAL_STATE) 
	{
		CMRotatePID.ref = 0;
		CMRotatePID.fdb = gap_angle;
		CMRotatePID.Calc(&CMRotatePID);   
		ChassisSpeedRef.rotate_ref = CMRotatePID.output;
	}
}

void ControlYaw(void)
{
	if(s_yawCount == 1)
	{
		uint16_t yawZeroAngle = yaw_zero;
			
		yawRealAngle = (GMYAWRx.angle - yawZeroAngle) * 360 / 8192.0f;
		NORMALIZE_ANGLE180(yawRealAngle);
			
		if(WorkState == NORMAL_STATE) 
		{
			yawRealAngle = -ZGyroModuleAngle;
		}
							
		yawIntensity = ProcessYawPID(yawAngleTarget, yawRealAngle, -gYroZs);
		s_yawCount = 0;
			
		ControlRotate();
	}
	else
	{
		s_yawCount++;
	}
}

void ControlPitch(void)
{
	if(s_pitchCount == 1)
	{
		uint16_t pitchZeroAngle = pitch_zero;
				
		pitchRealAngle = -(GMPITCHRx.angle - pitchZeroAngle) * 360 / 8192.0;
		NORMALIZE_ANGLE180(pitchRealAngle);

		MINMAX(pitchAngleTarget, -9.0f, 32);
				
		pitchIntensity = ProcessPitchPID(pitchAngleTarget,pitchRealAngle,-gYroXs);
				
		s_pitchCount = 0;
	}
	else
	{
		s_pitchCount++;
	}
}

void controlLoop()
{
	WorkStateFSM();
	
	if(WorkState != STOP_STATE)
	{
		ControlYaw();
		ControlPitch();
		
		setGMMotor();
		
		//ChassisSpeedRef.rotate_ref = 0;
		ControlCMFL();
		ControlCMFR();
		ControlCMBL();
		ControlCMBR();
		
		setCMMotor();
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == htim6.Instance)
	{
		controlLoop();
	}
}