//*******************************************************************
//*																																	*
//* 																																*
//* 			 				CDS55xx舵机控制    串口2  TX                      *
//* 																																*
//* 					void CDS5516( ID, V_pos, V_speed);										*
//* 																																*
//* 函数中自动判断是否已启动串口2且使用独立波特率发生器产生119200MHz*
//* 																																*
//* 																																*
//*******************************************************************

//#ifndef __CDS5516_H_
//#define __CDS5516_H_

#include <reg52.h>

#define FOSC 110592000ul
#define BAUD 19200

//-----------------------------------
//                                7      6      5      4      3      2     1     0        Reset Value
sfr S2CON = 0x9A; //S2 Control  S2SM0  S2SM1  S2SM2  S2REN  S2TB8  S2RB8  S2TI  S2RI      00000000B
sfr S2BUF = 0x9B; //S2 Serial Buffer                                                      xxxx,xxxx
sfr BRT = 0x9C; //S2 Baud-Rate Timer                                                    0000,0000
//--------------------------------------------------------------------------------
//                                        7     6       5      4     3      2      1      0   Reset Value
sfr AUXR  = 0x8E; //Auxiliary Register  T0x12 T1x12 UART_M0x6 BRTR S2SMOD BRTx12 EXTRAM S1BRS  0000,0000
//-----------------------------------
sfr AUXR1 = 0xA2; //Auxiliary Register 1  -  PCA_P4  SPI_P4  S2_P4  GF2    ADRJ   -    DPS  0000,0000

sfr IE2       = 0xAF;  //Auxiliary Interrupt   -     -     -    -     -    -  ESPI  ES2  0000,0000B

sbit beep=P2^0;
typedef unsigned char u8;
typedef unsigned int u16;

u8 B_RI;
u8 B_TI;
/**********函数声明**************/
/********************************************************************************/

void delayms(unsigned char ms);			   	//自适应0-255ms延时

void uart1_init();
void uart1_SendByte(u8 S_Byte);
void uart1_ReceByte();
void SetServoPosition1(unsigned char ID,unsigned int V_pos,unsigned int V_speed);

																										
void init_uart2(void);																																//19200bps@11.0592MHz
void uart2_SendByte(u8 S_Byte);																												//串口2发送
//void uart2_ReceByte();																															//串口2接收,串口2中断函数中没有对接收到的信息进行处理，若要接收请对串口2中断函数修改

void SetServoPosition2(unsigned char ID,unsigned int V_pos,unsigned int V_speed);			//CDS55xx舵机“wreite data” ID，位置，速度


void SetServoSpeed(unsigned char ID,int V_speed);																	//CDS55xx舵机“wreite data” ID，速度

void CDS5516(unsigned char ID,unsigned int V_pos,unsigned int V_speed);

void UART2_RCV (void);			//串口2中断函数



/************************************************************/


/**********函数功能*************/
/***********************************************************************************/

void CDS5516(unsigned char ID,unsigned int V_pos,unsigned int V_speed)
{
	delayms(2);
	if(!IE2) 
	{		//判断是否开启串口2
		init_uart2();
	}
	SetServoPosition2(ID,V_pos,V_speed);
}
/************************************************************************************/
//自适应0-255ms延时
void delayms(unsigned char ms)
{
	unsigned int i;
	do{
		i = FOSC/13000;
		while(--i);
	}while(--ms);
}

/************************************************************************************/
void init_uart2(void)			//19200bps@11.0592MHz
{
	AUXR &= 0xF7;		//波特率不倍速
	S2CON = 0x50;		//8位数据，可变波特率
	AUXR |= 0x04;		//独立波特率发生器时钟为Fosc，即1T
	BRT = 0xEE;		//设定独立波特率发生器重装值
	AUXR |= 0x10;		//启动独立波特率发生器
	IE2 = 1;				//启动串口2
	EA = 1;					//开总中断
}

/************************************************************************************/
//发送
void uart2_SendByte(u8 S_Byte)
{
	 S2BUF=S_Byte;
	 while(B_TI==0);  //等待发送完成		
	 B_TI=0; 
}


/********************************************////


void uart1_init()
{
	PCON &= 0x7F;		
	SCON = 0x50;		
	AUXR |= 0x40;		
	AUXR &= 0xFE;		
	TMOD &= 0x0F;		
	TMOD |= 0x20;	
	TL1 = 0xEE;	
	TH1 = 0xEE;	
	ET1 = 0;		
	TR1 = 1;	
}

void uart1_SendByte(u8 S_Byte)
{
	SBUF=S_Byte;
	 while(!TI);  //等待发送完成	   
	 TI=0;
}

void uart1_ReceByte()
{
	while(RI==0);
   RI = 0;
}



/************************************************************************************/
//接收
//void uart2_ReceByte() 
//{
//   while(B_RI==0);
//   B_RI = 0;
//}




/*********************************************/
//ID，位置，速度


void SetServoPosition1(unsigned char ID,unsigned int V_pos,unsigned int V_speed)
{
	unsigned int temp_speed = 0;//临时速度，用于进行方向判别
	unsigned int temp_pos = 0;
	unsigned char temp_speed_h = 0;//待发送数据H位
	unsigned char temp_speed_l = 0;//待发送数据L位
	unsigned char temp_pos_h = 0;
	unsigned char temp_pos_l = 0;
	unsigned char temp_sum = 0;
	
	
	if(V_speed > 1023)
	{
		temp_speed = 1023;
	}
	else temp_speed = V_speed;
	
	if(V_pos > 1023)
		temp_pos = 1023;
	else temp_pos = V_pos;
	
	
	temp_speed_h = (unsigned char)(V_speed>>8);
	temp_speed_l = (unsigned char)(V_speed);
	
	temp_pos_h = (unsigned char)(V_pos>>8);
	temp_pos_l = (unsigned char)(V_pos);
	//发送指令包
	uart1_SendByte(0xff);
	//delayms(20);
	uart1_SendByte(0xff);
	
	uart1_SendByte(ID);//发送id
	
	uart1_SendByte(7);//发送数据长度
	
	uart1_SendByte(0x03);//发送wirte data命令
	
	uart1_SendByte(0x1E);//舵机控制寄存器首地址
	
	uart1_SendByte(temp_pos_l);//发送位置低字节
	
	uart1_SendByte(temp_pos_h);//发送位置高字节
	
	uart1_SendByte(temp_speed_l);//发送速度低字节
	
	uart1_SendByte(temp_speed_h);//发送速度高字节
	
	temp_sum = ID + 7 + 0x03 + 0x1E + temp_speed_l + temp_speed_h + temp_pos_l + temp_pos_h;
	
	temp_sum = ~temp_sum;
	
	uart1_SendByte(temp_sum);
	//uart1_ReceByte();
	delayms(2);
}



/************************************************************************************/
//ID，位置，速度
void SetServoPosition2(unsigned char ID,unsigned int V_pos,unsigned int V_speed)
{
	unsigned int temp_speed = 0;//临时速度，用于进行方向判别
	unsigned int temp_pos = 0;	//位置
	unsigned char temp_speed_h = 0;//待发送数据H位
	unsigned char temp_speed_l = 0;//待发送数据L位
	unsigned char temp_pos_h = 0;		
	unsigned char temp_pos_l = 0;
	unsigned char temp_sum = 0;			//校验和
	
	if(V_speed > 1023)							//不能大于1023
	{
		temp_speed = 1023;
	}
	else temp_speed = V_speed;
	
	if(V_pos > 1023)
		temp_pos = 1023;
	else temp_pos = V_pos;
	
	temp_speed_h = (unsigned char)(V_speed>>8);		//取高八位
	temp_speed_l = (unsigned char)(V_speed);			//取低八位
	
	temp_pos_h = (unsigned char)(V_pos>>8);
	temp_pos_l = (unsigned char)(V_pos);

	//发送指令包
	uart2_SendByte(0xff);
	//delayms(1);
	uart2_SendByte(0xff);
	uart2_SendByte(ID);//发送id
	
	uart2_SendByte(7);//发送数据长度
	
	uart2_SendByte(0x03);//发送wirte data命令
	
	uart2_SendByte(0x1E);//舵机控制寄存器首地址
	
	uart2_SendByte(temp_pos_l);//发送位置低字节
	
	uart2_SendByte(temp_pos_h);//发送位置高字节
	
	uart2_SendByte(temp_speed_l);//发送速度低字节
	
	uart2_SendByte(temp_speed_h);//发送速度高字节
	
	temp_sum = ID + 7 + 0x03 + 0x1E + temp_speed_l + temp_speed_h + temp_pos_l + temp_pos_h;
	
	temp_sum = ~temp_sum;				//校验和等于各项和取反
	
	uart2_SendByte(temp_sum);
	
	delayms(2);
}

/************************************************************************************/

void SetServoSpeed(unsigned char ID,int V_speed)								//CDS55xx舵机“wreite data” ID，速度
{
	unsigned char temp_sign 		= 	0;						//临时符号，用于判别方向
	unsigned int 	temp_speed 		= 	0;						//临时速度用于进行方向判别
	unsigned char temp_speed_h 	= 	0;						//待发送速度高位
	unsigned char temp_speed_l 	= 	0;						//待发送速度低位
	unsigned char temp_sum = 0;										//校验和寄存变量
	
	if(V_speed < 0)
	{
		temp_speed = -V_speed;											//速度为负，取绝对值
		temp_sign = 1;															//标记为1负方向
	}
	else
	{
		temp_speed = V_speed;
		temp_sign = 0;															//0正方向
	}
	
	if(V_speed > 1023)
		temp_speed = 1023;													//相知速度范围
	
	//	BIT   15-11   10			  9 8 7 6 5 4 3 2 1 0
	//VALUE			0		  0/1						SPEED VALUE(0-1023)
	//
	//bit10为方向位。
	temp_speed |=(temp_sign)<<10;									//设置bit10为方向位，这是temp_speed就是待发送的速度数据了
	
	temp_speed_h = (unsigned char)temp_speed>>8;	//取高八位
	temp_speed_l = (unsigned char)temp_speed;			//取低八位
	
	//发送指令包
	uart2_SendByte(0xff);
	//delayms(1);
	uart2_SendByte(0xff);
	
	uart2_SendByte(ID);//发送id
	
	uart2_SendByte(5);//发送数据长度
	
	uart2_SendByte(0x03);//发送wirte data命令
	
	uart2_SendByte(0x20);//舵机控制寄存器首地址
	
	uart2_SendByte(temp_speed_l);//发送速度低字节
	
	uart2_SendByte(temp_speed_h);//发送速度高字节
	
	temp_sum = ID + 7 + 0x03 + 0x20 + temp_speed_l + temp_speed_h;
	
	temp_sum = ~temp_sum;				//校验和等于各项和取反
	
	uart2_SendByte(temp_sum);
	
	delayms(2);
}



//串口2中断函数
void UART2_RCV (void) interrupt 8
{
	
	if(S2CON & 0x01)
	{
		S2CON &= ~0x01;         //清除接收完成标志
		B_RI = 1;
	}
	if(S2CON & 0x02)
	{
		S2CON &= ~0x02;         //清除发送完成标志
		B_TI = 1;
	}
}