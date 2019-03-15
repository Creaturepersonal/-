//*******************************************************************
//*																																	*
//* 																																*
//* 			 				CDS55xx�������    ����2  TX                      *
//* 																																*
//* 					void CDS5516( ID, V_pos, V_speed);										*
//* 																																*
//* �������Զ��ж��Ƿ�����������2��ʹ�ö��������ʷ���������119200MHz*
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
/**********��������**************/
/********************************************************************************/

void delayms(unsigned char ms);			   	//����Ӧ0-255ms��ʱ

void uart1_init();
void uart1_SendByte(u8 S_Byte);
void uart1_ReceByte();
void SetServoPosition1(unsigned char ID,unsigned int V_pos,unsigned int V_speed);

																										
void init_uart2(void);																																//19200bps@11.0592MHz
void uart2_SendByte(u8 S_Byte);																												//����2����
//void uart2_ReceByte();																															//����2����,����2�жϺ�����û�жԽ��յ�����Ϣ���д�����Ҫ������Դ���2�жϺ����޸�

void SetServoPosition2(unsigned char ID,unsigned int V_pos,unsigned int V_speed);			//CDS55xx�����wreite data�� ID��λ�ã��ٶ�


void SetServoSpeed(unsigned char ID,int V_speed);																	//CDS55xx�����wreite data�� ID���ٶ�

void CDS5516(unsigned char ID,unsigned int V_pos,unsigned int V_speed);

void UART2_RCV (void);			//����2�жϺ���



/************************************************************/


/**********��������*************/
/***********************************************************************************/

void CDS5516(unsigned char ID,unsigned int V_pos,unsigned int V_speed)
{
	delayms(2);
	if(!IE2) 
	{		//�ж��Ƿ�������2
		init_uart2();
	}
	SetServoPosition2(ID,V_pos,V_speed);
}
/************************************************************************************/
//����Ӧ0-255ms��ʱ
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
	AUXR &= 0xF7;		//�����ʲ�����
	S2CON = 0x50;		//8λ���ݣ��ɱ䲨����
	AUXR |= 0x04;		//���������ʷ�����ʱ��ΪFosc����1T
	BRT = 0xEE;		//�趨���������ʷ�������װֵ
	AUXR |= 0x10;		//�������������ʷ�����
	IE2 = 1;				//��������2
	EA = 1;					//�����ж�
}

/************************************************************************************/
//����
void uart2_SendByte(u8 S_Byte)
{
	 S2BUF=S_Byte;
	 while(B_TI==0);  //�ȴ��������		
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
	 while(!TI);  //�ȴ��������	   
	 TI=0;
}

void uart1_ReceByte()
{
	while(RI==0);
   RI = 0;
}



/************************************************************************************/
//����
//void uart2_ReceByte() 
//{
//   while(B_RI==0);
//   B_RI = 0;
//}




/*********************************************/
//ID��λ�ã��ٶ�


void SetServoPosition1(unsigned char ID,unsigned int V_pos,unsigned int V_speed)
{
	unsigned int temp_speed = 0;//��ʱ�ٶȣ����ڽ��з����б�
	unsigned int temp_pos = 0;
	unsigned char temp_speed_h = 0;//����������Hλ
	unsigned char temp_speed_l = 0;//����������Lλ
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
	//����ָ���
	uart1_SendByte(0xff);
	//delayms(20);
	uart1_SendByte(0xff);
	
	uart1_SendByte(ID);//����id
	
	uart1_SendByte(7);//�������ݳ���
	
	uart1_SendByte(0x03);//����wirte data����
	
	uart1_SendByte(0x1E);//������ƼĴ����׵�ַ
	
	uart1_SendByte(temp_pos_l);//����λ�õ��ֽ�
	
	uart1_SendByte(temp_pos_h);//����λ�ø��ֽ�
	
	uart1_SendByte(temp_speed_l);//�����ٶȵ��ֽ�
	
	uart1_SendByte(temp_speed_h);//�����ٶȸ��ֽ�
	
	temp_sum = ID + 7 + 0x03 + 0x1E + temp_speed_l + temp_speed_h + temp_pos_l + temp_pos_h;
	
	temp_sum = ~temp_sum;
	
	uart1_SendByte(temp_sum);
	//uart1_ReceByte();
	delayms(2);
}



/************************************************************************************/
//ID��λ�ã��ٶ�
void SetServoPosition2(unsigned char ID,unsigned int V_pos,unsigned int V_speed)
{
	unsigned int temp_speed = 0;//��ʱ�ٶȣ����ڽ��з����б�
	unsigned int temp_pos = 0;	//λ��
	unsigned char temp_speed_h = 0;//����������Hλ
	unsigned char temp_speed_l = 0;//����������Lλ
	unsigned char temp_pos_h = 0;		
	unsigned char temp_pos_l = 0;
	unsigned char temp_sum = 0;			//У���
	
	if(V_speed > 1023)							//���ܴ���1023
	{
		temp_speed = 1023;
	}
	else temp_speed = V_speed;
	
	if(V_pos > 1023)
		temp_pos = 1023;
	else temp_pos = V_pos;
	
	temp_speed_h = (unsigned char)(V_speed>>8);		//ȡ�߰�λ
	temp_speed_l = (unsigned char)(V_speed);			//ȡ�Ͱ�λ
	
	temp_pos_h = (unsigned char)(V_pos>>8);
	temp_pos_l = (unsigned char)(V_pos);

	//����ָ���
	uart2_SendByte(0xff);
	//delayms(1);
	uart2_SendByte(0xff);
	uart2_SendByte(ID);//����id
	
	uart2_SendByte(7);//�������ݳ���
	
	uart2_SendByte(0x03);//����wirte data����
	
	uart2_SendByte(0x1E);//������ƼĴ����׵�ַ
	
	uart2_SendByte(temp_pos_l);//����λ�õ��ֽ�
	
	uart2_SendByte(temp_pos_h);//����λ�ø��ֽ�
	
	uart2_SendByte(temp_speed_l);//�����ٶȵ��ֽ�
	
	uart2_SendByte(temp_speed_h);//�����ٶȸ��ֽ�
	
	temp_sum = ID + 7 + 0x03 + 0x1E + temp_speed_l + temp_speed_h + temp_pos_l + temp_pos_h;
	
	temp_sum = ~temp_sum;				//У��͵��ڸ����ȡ��
	
	uart2_SendByte(temp_sum);
	
	delayms(2);
}

/************************************************************************************/

void SetServoSpeed(unsigned char ID,int V_speed)								//CDS55xx�����wreite data�� ID���ٶ�
{
	unsigned char temp_sign 		= 	0;						//��ʱ���ţ������б���
	unsigned int 	temp_speed 		= 	0;						//��ʱ�ٶ����ڽ��з����б�
	unsigned char temp_speed_h 	= 	0;						//�������ٶȸ�λ
	unsigned char temp_speed_l 	= 	0;						//�������ٶȵ�λ
	unsigned char temp_sum = 0;										//У��ͼĴ����
	
	if(V_speed < 0)
	{
		temp_speed = -V_speed;											//�ٶ�Ϊ����ȡ����ֵ
		temp_sign = 1;															//���Ϊ1������
	}
	else
	{
		temp_speed = V_speed;
		temp_sign = 0;															//0������
	}
	
	if(V_speed > 1023)
		temp_speed = 1023;													//��֪�ٶȷ�Χ
	
	//	BIT   15-11   10			  9 8 7 6 5 4 3 2 1 0
	//VALUE			0		  0/1						SPEED VALUE(0-1023)
	//
	//bit10Ϊ����λ��
	temp_speed |=(temp_sign)<<10;									//����bit10Ϊ����λ������temp_speed���Ǵ����͵��ٶ�������
	
	temp_speed_h = (unsigned char)temp_speed>>8;	//ȡ�߰�λ
	temp_speed_l = (unsigned char)temp_speed;			//ȡ�Ͱ�λ
	
	//����ָ���
	uart2_SendByte(0xff);
	//delayms(1);
	uart2_SendByte(0xff);
	
	uart2_SendByte(ID);//����id
	
	uart2_SendByte(5);//�������ݳ���
	
	uart2_SendByte(0x03);//����wirte data����
	
	uart2_SendByte(0x20);//������ƼĴ����׵�ַ
	
	uart2_SendByte(temp_speed_l);//�����ٶȵ��ֽ�
	
	uart2_SendByte(temp_speed_h);//�����ٶȸ��ֽ�
	
	temp_sum = ID + 7 + 0x03 + 0x20 + temp_speed_l + temp_speed_h;
	
	temp_sum = ~temp_sum;				//У��͵��ڸ����ȡ��
	
	uart2_SendByte(temp_sum);
	
	delayms(2);
}



//����2�жϺ���
void UART2_RCV (void) interrupt 8
{
	
	if(S2CON & 0x01)
	{
		S2CON &= ~0x01;         //���������ɱ�־
		B_RI = 1;
	}
	if(S2CON & 0x02)
	{
		S2CON &= ~0x02;         //���������ɱ�־
		B_TI = 1;
	}
}