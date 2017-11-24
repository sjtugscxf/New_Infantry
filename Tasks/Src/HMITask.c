/**
  ******************************************************************************
  * File Name          : HMITask.c
  * Description        : 人机交互任务，目前是OLED显示屏和4*4按键
  ******************************************************************************
  *
  * Copyright (c) 2018 Team TPP-Shanghai Jiao Tong University
  * All rights reserved.
  *
  ******************************************************************************
  */
#include "includes.h"

extern fw_PID_Regulator_t pitchPositionPID;
extern fw_PID_Regulator_t yawPositionPID;
extern fw_PID_Regulator_t pitchSpeedPID;
extern fw_PID_Regulator_t yawSpeedPID;

typedef struct {
  uint8_t * category;
  int16_t  data;
}Putboth;

#define Rows  7
#define Pages 2

uint8_t rows = 0;
uint8_t pages = 0;
uint8_t debug_row = 1;

Putboth outpair[Pages][Rows] = {
	{
		{"GMYAWPID",1},
		{"YAWPkp",8},
		{"YAWPki",0},
		{"YAWPkd",0},
		{"YAWSkp",5},
		{"YAWSki",0},
		{"YAWSkd",0}
	},
	{
		{"GMPITCHPID",1},
		{"PITCHPkp",8},
		{"PITCHPki",0},
		{"PITCHPkd",0},
		{"PITCHSkp",5},
		{"PITCHSki",0},
		{"PITCHSkd",0}
	}
};

// ===== Variables =====

// --- Local ---
const uint8_t F6x8[][6];
#define XLevelL		0x00
#define XLevelH		0x10
#define XLevel		((XLevelH&0x0F)*16+XLevelL)
#define OLED_COL	128
#define Max_Row		  64
#define	Brightness	0xCF 

#define X_WIDTH 128
#define Y_WIDTH 64

// ===== Functions Declare =====

// --- Local --- ( No need for user to use ! )

  // Basic Driver 
void Oled_WrDat(uint8_t data);
void Oled_WrCmd(uint8_t cmd);
void Oled_DLY_ms(uint16_t ms);
void Oled_Set_Pos(uint8_t x, uint8_t y);

// Hardware Interface
void Oled_SCL(uint8_t x);
void Oled_SDA(uint8_t x);
void Oled_RST(uint8_t x);
void Oled_DC(uint8_t x);

// ===== APIs Realization =====
void Oled_Putstr(uint8_t y,uint8_t x,uint8_t ch[])
{
  uint8_t c=0,i=0,j=0,t=0; 
  while (ch[t]!='\0')
    t++;
  Oled_Set_Pos(21-x-t,7-y);
  for(j=0;j<t;j++){   
    c = ch[t-1-j]-32;
    for(i=0;i<6;i++)     
      Oled_WrDat(F6x8[c][5-i]);  
  }
}
/*
void print_curse(u8 col,u8 sel){   
  uint8 i;
  for(i=0;i<8;++i){
    if (col == i) {
      if (sel == 0) Oled_Putstr(col,0,">");
      else Oled_Putstr(col,0,"*");
    }
    else Oled_Putstr(i,0," "); 
  }
}
*/
void Oled_Putnum(uint8_t x,uint8_t y,int16_t c){      
  uint8_t cha[7],temp,sig=0;
  
  if (c<0){
    sig=1;
    c = -c;
  }
  
  cha[0]=' ';
  
  temp=c/10000;
  if(temp==0) cha[1]=' ';
  else cha[1]=temp+48;
  
  temp=c%10000/1000;
  if(temp==0 && cha[1]==' ') cha[2]=' ';
  else cha[2]=temp+48;
  
  temp=c%1000/100;
  if(temp==0 && cha[2]==' ') cha[3]=' ';
  else cha[3]=temp+48;
  
  temp=c%100/10;
  if(temp==0 && cha[3]==' ') cha[4]=' ';
  else cha[4]=temp+48; 
  
  temp=c%10;
  cha[5]=temp+48;  
  cha[6]='\0';
  if(sig){
    temp=6;
    while(cha[temp]!=' ')
      temp--;
    cha[temp]='-';
  }
  Oled_Putstr(x,y+1,cha);
}

void Oled_Clear(void)
{
	uint8_t y,x;	
	for(y=0;y<8;y++)
	{
		Oled_WrCmd(0xb0+y);
		Oled_WrCmd(0x01);
		Oled_WrCmd(0x10); 
		for(x=0;x<X_WIDTH;x++)
			Oled_WrDat(0);
	}
}

void Oled_Init(void)        
{  
  Oled_SCL(1);
  //Oled_CS(1);	 	
	
  Oled_RST(0);
  Oled_DLY_ms(50);
  Oled_RST(1);

  Oled_WrCmd(0xae);//--turn off oled panel
  Oled_WrCmd(0x00);//---set low column address
  Oled_WrCmd(0x10);//---set high column address
  Oled_WrCmd(0x40);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
  Oled_WrCmd(0x81);//--set contrast control register
  Oled_WrCmd(0xcf); // Set SEG Output Current Brightness
  Oled_WrCmd(0xa1);//--Set SEG/Column Mapping     0xa0ͶʅԽӵ 0xa1гЭ
  Oled_WrCmd(0xc8);//Set COM/Row Scan Direction   0xc0УɺԽӵ 0xc8гЭ
  Oled_WrCmd(0xa6);//--set normal display
  Oled_WrCmd(0xa8);//--set multiplex ratio(1 to 64)
  Oled_WrCmd(0x3f);//--1/64 duty
  Oled_WrCmd(0xd3);//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
  Oled_WrCmd(0x00);//-not offset
  Oled_WrCmd(0xd5);//--set display clock divide ratio/oscillator frequency
  Oled_WrCmd(0xf0);//--set divide ratio, Set Clock as 100 Frames/Sec
  Oled_WrCmd(0xd9);//--set pre-charge period
  Oled_WrCmd(0xf1);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
  Oled_WrCmd(0xda);//--set com pins hardware configuration
  Oled_WrCmd(0x12);
  Oled_WrCmd(0xdb);//--set vcomh
  Oled_WrCmd(0x40);//Set VCOM Deselect Level
  Oled_WrCmd(0x20);//-Set Page Addressing Mode (0x00/0x01/0x02)
  Oled_WrCmd(0x02);//
  Oled_WrCmd(0x8d);//--set Charge Pump enable/disable
  Oled_WrCmd(0x14);//--set(0x10) disable
  Oled_WrCmd(0xa4);// Disable Entire Display On (0xa4/0xa5)
  Oled_WrCmd(0xa6);// Disable Inverse Display On (0xa6/a7) 
  Oled_WrCmd(0xaf);//--turn on oled panel
  Oled_Clear(); 
  Oled_Set_Pos(0,0);  
	
	Oled_Putstr(0,7,"TPP2018");
	Oled_Putstr(2,0," S7 ENTER S15 QUIT");
	Oled_Putstr(3,0,"S11 UP    S12 DOWN");
	Oled_Putstr(4,0,"S16 LEFT  S8  RIGHT");
	Oled_Putstr(5,0," S3 ADD   S4  SUB");
} 

// --- Basic Driver ---

void Oled_DLY_ms(uint16_t ms)
{                         
  uint16_t a;
  while(ms)
  {
    a=1335;
    while(a--);
    ms--;
  }
  return;
}

void Oled_WrDat(uint8_t data)
{
  uint8_t i=8;
  //Oled_CS(0);;
  Oled_DC(1);;
  Oled_SCL(0);;   ;
  while(i--)
  {
    if(data&1){
      Oled_SDA(1);__nop;__nop;
    }
    else{
      Oled_SDA(0);__nop;__nop;
    }
    Oled_SCL(1); 
    __nop;__nop;__nop;__nop;   
    Oled_SCL(0);; 
    __nop;__nop;
    data>>=1;;    
  }
  //Oled_CS=1;
}

void Oled_WrCmd(uint8_t cmd)
{
  uint8_t i=8;
  
  //Oled_CS(0);;
  Oled_DC(0);;
  Oled_SCL(0);;
  //asm("nop");   
  while(i--)
  {
    if(cmd&0x80){Oled_SDA(1);;}
    else{Oled_SDA(0);;}
    Oled_SCL(1);;
    __nop;__nop;   
    Oled_SCL(0);;    
    cmd<<=1;;   
  }
  //Oled_CS=1;
}
void Oled_Set_Pos(uint8_t x, uint8_t y)
{ 
  uint8_t xt,msb,lsb;
  //Oled_WrCmd(0xb0+y);
  //Oled_WrCmd(((x&0xf0)>>4)|0x10);
  //Oled_WrCmd((x&0x0f)|0x01); 
  xt = 1 + x * 6;
  lsb = xt % 16;
  msb = xt / 16 + 0x10;
  Oled_WrCmd(0xb0 + y);
  Oled_WrCmd(lsb);
  Oled_WrCmd(msb); 
} 

// --- Hardware Interface ---

void Oled_SCL(uint8_t x){                     //d0
  if(x)
    HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, GPIO_PIN_SET);
  else
    HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, GPIO_PIN_RESET);
}
void Oled_SDA(uint8_t x){                             //d1
  if(x)
    HAL_GPIO_WritePin(Oled_SDA_GPIO_Port, Oled_SDA_Pin, GPIO_PIN_SET);
  else
    HAL_GPIO_WritePin(Oled_SDA_GPIO_Port, Oled_SDA_Pin, GPIO_PIN_RESET);
}
void Oled_RST(uint8_t x){                     //res
  if(x)
    HAL_GPIO_WritePin(Oled_RST_GPIO_Port, Oled_RST_Pin, GPIO_PIN_SET);
  else
    HAL_GPIO_WritePin(Oled_RST_GPIO_Port, Oled_RST_Pin, GPIO_PIN_RESET);
}
void Oled_DC(uint8_t x){                      //dc
  if(x)
    HAL_GPIO_WritePin(Oled_DC_GPIO_Port, Oled_DC_Pin, GPIO_PIN_SET);
  else
    HAL_GPIO_WritePin(Oled_DC_GPIO_Port, Oled_DC_Pin, GPIO_PIN_RESET);
}


const uint8_t F6x8[][6] =
{
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // sp
    { 0x00, 0x00, 0x00, 0x2f, 0x00, 0x00 },   // !
    { 0x00, 0x00, 0x07, 0x00, 0x07, 0x00 },   // "
    { 0x00, 0x14, 0x7f, 0x14, 0x7f, 0x14 },   // #
    { 0x00, 0x24, 0x2a, 0x7f, 0x2a, 0x12 },   // $
    { 0x00, 0x62, 0x64, 0x08, 0x13, 0x23 },   // %
    { 0x00, 0x36, 0x49, 0x55, 0x22, 0x50 },   // &
    { 0x00, 0x00, 0x05, 0x03, 0x00, 0x00 },   // '
    { 0x00, 0x00, 0x1c, 0x22, 0x41, 0x00 },   // (
    { 0x00, 0x00, 0x41, 0x22, 0x1c, 0x00 },   // )
    { 0x00, 0x14, 0x08, 0x3E, 0x08, 0x14 },   // *
    { 0x00, 0x08, 0x08, 0x3E, 0x08, 0x08 },   // +
    { 0x00, 0x00, 0x00, 0xA0, 0x60, 0x00 },   // ,
    { 0x00, 0x08, 0x08, 0x08, 0x08, 0x08 },   // -
    { 0x00, 0x00, 0x60, 0x60, 0x00, 0x00 },   // .
    { 0x00, 0x20, 0x10, 0x08, 0x04, 0x02 },   // /
    { 0x00, 0x3E, 0x51, 0x49, 0x45, 0x3E },   // 0 16
    { 0x00, 0x00, 0x42, 0x7F, 0x40, 0x00 },   // 1
    { 0x00, 0x42, 0x61, 0x51, 0x49, 0x46 },   // 2
    { 0x00, 0x21, 0x41, 0x45, 0x4B, 0x31 },   // 3
    { 0x00, 0x18, 0x14, 0x12, 0x7F, 0x10 },   // 4
    { 0x00, 0x27, 0x45, 0x45, 0x45, 0x39 },   // 5
    { 0x00, 0x3C, 0x4A, 0x49, 0x49, 0x30 },   // 6
    { 0x00, 0x01, 0x71, 0x09, 0x05, 0x03 },   // 7
    { 0x00, 0x36, 0x49, 0x49, 0x49, 0x36 },   // 8
    { 0x00, 0x06, 0x49, 0x49, 0x29, 0x1E },   // 9
    { 0x00, 0x00, 0x36, 0x36, 0x00, 0x00 },   // :
    { 0x00, 0x00, 0x56, 0x36, 0x00, 0x00 },   // ;
    { 0x00, 0x08, 0x14, 0x22, 0x41, 0x00 },   // <
    { 0x00, 0x14, 0x14, 0x14, 0x14, 0x14 },   // =
    { 0x00, 0x00, 0x41, 0x22, 0x14, 0x08 },   // >
    { 0x00, 0x02, 0x01, 0x51, 0x09, 0x06 },   // ?
    { 0x00, 0x32, 0x49, 0x59, 0x51, 0x3E },   // @
    { 0x00, 0x7C, 0x12, 0x11, 0x12, 0x7C },   // A
    { 0x00, 0x7F, 0x49, 0x49, 0x49, 0x36 },   // B
    { 0x00, 0x3E, 0x41, 0x41, 0x41, 0x22 },   // C
    { 0x00, 0x7F, 0x41, 0x41, 0x22, 0x1C },   // D
    { 0x00, 0x7F, 0x49, 0x49, 0x49, 0x41 },   // E
    { 0x00, 0x7F, 0x09, 0x09, 0x09, 0x01 },   // F
    { 0x00, 0x3E, 0x41, 0x49, 0x49, 0x7A },   // G
    { 0x00, 0x7F, 0x08, 0x08, 0x08, 0x7F },   // H
    { 0x00, 0x00, 0x41, 0x7F, 0x41, 0x00 },   // I
    { 0x00, 0x20, 0x40, 0x41, 0x3F, 0x01 },   // J
    { 0x00, 0x7F, 0x08, 0x14, 0x22, 0x41 },   // K
    { 0x00, 0x7F, 0x40, 0x40, 0x40, 0x40 },   // L
    { 0x00, 0x7F, 0x02, 0x0C, 0x02, 0x7F },   // M
    { 0x00, 0x7F, 0x04, 0x08, 0x10, 0x7F },   // N
    { 0x00, 0x3E, 0x41, 0x41, 0x41, 0x3E },   // O
    { 0x00, 0x7F, 0x09, 0x09, 0x09, 0x06 },   // P
    { 0x00, 0x3E, 0x41, 0x51, 0x21, 0x5E },   // Q
    { 0x00, 0x7F, 0x09, 0x19, 0x29, 0x46 },   // R
    { 0x00, 0x46, 0x49, 0x49, 0x49, 0x31 },   // S
    { 0x00, 0x01, 0x01, 0x7F, 0x01, 0x01 },   // T
    { 0x00, 0x3F, 0x40, 0x40, 0x40, 0x3F },   // U
    { 0x00, 0x1F, 0x20, 0x40, 0x20, 0x1F },   // V
    { 0x00, 0x3F, 0x40, 0x38, 0x40, 0x3F },   // W
    { 0x00, 0x63, 0x14, 0x08, 0x14, 0x63 },   // X
    { 0x00, 0x07, 0x08, 0x70, 0x08, 0x07 },   // Y
    { 0x00, 0x61, 0x51, 0x49, 0x45, 0x43 },   // Z
    { 0x00, 0x00, 0x7F, 0x41, 0x41, 0x00 },   // [
    { 0x00, 0x55, 0x2A, 0x55, 0x2A, 0x55 },   // 55
    { 0x00, 0x00, 0x41, 0x41, 0x7F, 0x00 },   // ]
    { 0x00, 0x04, 0x02, 0x01, 0x02, 0x04 },   // ^
    { 0x00, 0x40, 0x40, 0x40, 0x40, 0x40 },   // _
    { 0x00, 0x00, 0x01, 0x02, 0x04, 0x00 },   // '
    { 0x00, 0x20, 0x54, 0x54, 0x54, 0x78 },   // a
    { 0x00, 0x7F, 0x48, 0x44, 0x44, 0x38 },   // b
    { 0x00, 0x38, 0x44, 0x44, 0x44, 0x20 },   // c
    { 0x00, 0x38, 0x44, 0x44, 0x48, 0x7F },   // d
    { 0x00, 0x38, 0x54, 0x54, 0x54, 0x18 },   // e
    { 0x00, 0x08, 0x7E, 0x09, 0x01, 0x02 },   // f
    { 0x00, 0x18, 0xA4, 0xA4, 0xA4, 0x7C },   // g
    { 0x00, 0x7F, 0x08, 0x04, 0x04, 0x78 },   // h
    { 0x00, 0x00, 0x44, 0x7D, 0x40, 0x00 },   // i
    { 0x00, 0x40, 0x80, 0x84, 0x7D, 0x00 },   // j
    { 0x00, 0x7F, 0x10, 0x28, 0x44, 0x00 },   // k
    { 0x00, 0x00, 0x41, 0x7F, 0x40, 0x00 },   // l
    { 0x00, 0x7C, 0x04, 0x18, 0x04, 0x78 },   // m
    { 0x00, 0x7C, 0x08, 0x04, 0x04, 0x78 },   // n
    { 0x00, 0x38, 0x44, 0x44, 0x44, 0x38 },   // o
    { 0x00, 0xFC, 0x24, 0x24, 0x24, 0x18 },   // p
    { 0x00, 0x18, 0x24, 0x24, 0x18, 0xFC },   // q
    { 0x00, 0x7C, 0x08, 0x04, 0x04, 0x08 },   // r
    { 0x00, 0x48, 0x54, 0x54, 0x54, 0x20 },   // s
    { 0x00, 0x04, 0x3F, 0x44, 0x40, 0x20 },   // t
    { 0x00, 0x3C, 0x40, 0x40, 0x20, 0x7C },   // u
    { 0x00, 0x1C, 0x20, 0x40, 0x20, 0x1C },   // v
    { 0x00, 0x3C, 0x40, 0x30, 0x40, 0x3C },   // w
    { 0x00, 0x44, 0x28, 0x10, 0x28, 0x44 },   // x
    { 0x00, 0x1C, 0xA0, 0xA0, 0xA0, 0x7C },   // y
    { 0x00, 0x44, 0x64, 0x54, 0x4C, 0x44 },   // z
    { 0x14, 0x14, 0x14, 0x14, 0x14, 0x14 }    // horiz lines
};

typedef struct
{
	uint8_t key_state;
	uint8_t key_time;
	uint16_t pressed_value;
	uint16_t long_pressed_value;
}tKEY_INFO;

static tKEY_INFO key_matrix;

void set_MK_CO_RI()
{
	GPIO_InitTypeDef gpio;
	gpio.Mode = GPIO_MODE_INPUT;
	gpio.Pull = GPIO_PULLUP;
	
	gpio.Pin = ROW0_Pin;
	HAL_GPIO_Init(ROW0_GPIO_Port, &gpio);
	gpio.Pin = ROW1_Pin;
	HAL_GPIO_Init(ROW1_GPIO_Port, &gpio);
	gpio.Pin = ROW2_Pin;
	HAL_GPIO_Init(ROW2_GPIO_Port, &gpio);
	gpio.Pin = ROW3_Pin;
	HAL_GPIO_Init(ROW3_GPIO_Port, &gpio);
	
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
	
	gpio.Pin = COL0_Pin;
  HAL_GPIO_Init(COL0_GPIO_Port, &gpio);
	gpio.Pin = COL1_Pin;
  HAL_GPIO_Init(COL1_GPIO_Port, &gpio);
	gpio.Pin = COL2_Pin;
  HAL_GPIO_Init(COL2_GPIO_Port, &gpio);
	gpio.Pin = COL3_Pin;
  HAL_GPIO_Init(COL3_GPIO_Port, &gpio);
	
	HAL_GPIO_WritePin(COL0_GPIO_Port,COL0_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(COL1_GPIO_Port,COL1_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(COL2_GPIO_Port,COL2_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(COL3_GPIO_Port,COL3_Pin,GPIO_PIN_RESET);
}

void set_MK_CI_RO()
{
	GPIO_InitTypeDef gpio;
	gpio.Mode = GPIO_MODE_INPUT;
	gpio.Pull = GPIO_PULLUP;
	
	gpio.Pin = COL0_Pin;
	HAL_GPIO_Init(COL0_GPIO_Port, &gpio);
	gpio.Pin = COL1_Pin;
	HAL_GPIO_Init(COL1_GPIO_Port, &gpio);
	gpio.Pin = COL2_Pin;
	HAL_GPIO_Init(COL2_GPIO_Port, &gpio);
	gpio.Pin = COL3_Pin;
	HAL_GPIO_Init(COL3_GPIO_Port, &gpio);
	
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
	
	gpio.Pin = ROW0_Pin;
  HAL_GPIO_Init(ROW0_GPIO_Port, &gpio);
	gpio.Pin = ROW1_Pin;
  HAL_GPIO_Init(ROW1_GPIO_Port, &gpio);
	gpio.Pin = ROW2_Pin;
  HAL_GPIO_Init(ROW2_GPIO_Port, &gpio);
	gpio.Pin = ROW3_Pin;
  HAL_GPIO_Init(ROW3_GPIO_Port, &gpio);
	
	HAL_GPIO_WritePin(ROW0_GPIO_Port,ROW0_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(ROW1_GPIO_Port,ROW1_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(ROW2_GPIO_Port,ROW2_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(ROW3_GPIO_Port,ROW3_Pin,GPIO_PIN_RESET);
}

uint8_t mk_read_row()
{
	uint8_t hr = 0;
	
	hr = (HAL_GPIO_ReadPin(ROW0_GPIO_Port,ROW0_Pin)<<3)|
			 (HAL_GPIO_ReadPin(ROW1_GPIO_Port,ROW1_Pin)<<2)|
			 (HAL_GPIO_ReadPin(ROW2_GPIO_Port,ROW2_Pin)<<1)|
				(HAL_GPIO_ReadPin(ROW3_GPIO_Port,ROW3_Pin)<<0);
	
	return hr;
}

uint8_t mk_read_col()
{
	uint8_t hr = 0;
	
	hr = (HAL_GPIO_ReadPin(COL0_GPIO_Port,COL0_Pin)<<3)|
			 (HAL_GPIO_ReadPin(COL1_GPIO_Port,COL1_Pin)<<2)|
			 (HAL_GPIO_ReadPin(COL2_GPIO_Port,COL2_Pin)<<1)|
				(HAL_GPIO_ReadPin(COL3_GPIO_Port,COL3_Pin)<<0);
	
	return hr;
}

uint8_t mk_decode(uint8_t press)
{
	uint8_t hr = 0;
	switch(press)
	{
		case 0x7e:hr=1;break;
		case 0xbe:hr=2;break;
		case 0xde:hr=3;break;
		case 0xee:hr=0;break;
		
		case 0x7d:hr=4;break;
		case 0xbd:hr=5;break;
		case 0xdd:hr=6;break;
		case 0xed:hr=11;break;
		
		case 0x7b:hr=7;break;
		case 0xbb:hr=8;break;
		case 0xdb:hr=9;break;
		case 0xeb:hr=12;break;
		
		case 0x77:hr=13;break;
		case 0xb7:hr=10;break;
		case 0xd7:hr=14;break;
		case 0xe7:hr=15;break;
	}
	return hr;
}

int key;
//uint8_t key_valuet = 0;
uint8_t key_value = 0;
uint8_t last_key_value = 0;

uint8_t Read_MatrixKey()
{
	static uint8_t row_press[2]={0,0};
	static uint8_t col_press[2]={0,0};
	
	key_matrix.pressed_value = 0;
	key = 0;
	switch(key_matrix.key_state)
	{
		case 0:
		{
			set_MK_CO_RI();
			row_press[0] = mk_read_row();
			
			if(row_press[0]!=0x0f) key_matrix.key_state = 1;
			
			break;
		}
		case 1:
		{
			set_MK_CO_RI();
			row_press[1] = mk_read_row();
			if(row_press[1] == row_press[0]) key_matrix.key_state = 2;
			else key_matrix.key_state = 0;
			break;
		}
		case 2:
		{
			set_MK_CI_RO();
			col_press[0] = mk_read_col();
			
			if(col_press[0]!=0x0f) key_matrix.key_state = 3;
			
			break;
		}
		case 3:
		{
			set_MK_CI_RO();
			col_press[1] = mk_read_col();
			if(col_press[1] == col_press[0])
			{
				key_matrix.key_state = 4;
				key_matrix.pressed_value = mk_decode(col_press[1]<<4 | row_press[1]);
				key = key_matrix.pressed_value;
			}
			else key_matrix.key_state = 0;
			break;
		}
		case 4:
		{
			set_MK_CI_RO();
			col_press[1] = mk_read_col();
			
			if(col_press[1]==0x0f) key_matrix.key_state = 0;
			break;
		}
	}
	return key_matrix.pressed_value;
}

void displayParameters()
{
	Oled_Clear(); 
	Oled_Set_Pos(0,0); 
	Oled_Putstr(0,3,outpair[pages][0].category);
	for(int i = 1; i < 7; i++)
	{
		Oled_Putstr(i,0,outpair[pages][i].category);
		Oled_Putnum(i,11,outpair[pages][i].data);
	}
	Oled_Putstr(7,0,outpair[pages][debug_row].category);
	Oled_Putnum(7,11,outpair[pages][debug_row].data);
}

uint8_t debug_mode = 0;
void HMILoop()
{
	key_value = Read_MatrixKey();
	if(key_value == 2 && last_key_value == 0)
	{
		debug_mode = 0;
		Oled_Clear(); 
		Oled_Set_Pos(0,0); 
		Oled_Putstr(0,7,"TPP2018");
		Oled_Putstr(2,0," S7 ENTER S15 QUIT");
		Oled_Putstr(3,0,"S11 UP    S12 DOWN");
		Oled_Putstr(4,0,"S16 LEFT  S8  RIGHT");
		Oled_Putstr(5,0," S3 ADD   S4  SUB");
	}
	if(key_value == 8 && last_key_value == 0) debug_mode = 1;
	if(debug_mode == 1)
	{
		if(key_value == 7 && last_key_value == 0) pages = (pages+1)%Pages;
		if(key_value == 1 && last_key_value == 0) 
		{
			if(pages == 0) pages = Pages - 1;
			else pages = pages - 1;
		}
		if(key_value == 4 && last_key_value == 0) 
		{
			if(debug_row == 6) debug_row = 1;
			else debug_row = debug_row + 1;
		}
		if(key_value == 5 && last_key_value == 0) 
		{
			if(debug_row == 1) debug_row = 6;
			else debug_row = debug_row - 1;
		}
		if(key_value == 10 && last_key_value == 0) 
		{
			//加变量的值
		}
		if(key_value == 13 && last_key_value == 0) 
		{
			//减变量的值
		}
		displayParameters();
	}
	//if(key_valuet != 0) key_value = key_valuet;
	
	last_key_value = key_value;
}
