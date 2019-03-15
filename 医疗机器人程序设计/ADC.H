
/*
模数转换接口为P1^0 口

本头文件为ADC转换程序，直接调用GetADCResult（0）函数，返回值为十六进制
的数值 可以返回的数值范围为3-10cm  以cm为最小单位
3     4     5     6     7     8     9      10
0x03  0x04	0x05  0x06	0x07  0x08	0x09   0x0A

*/
#include<reg52.h>
//#include "stc12c5a.h"
#include "intrins.h"

sbit KEY=P3^2 ;
sbit BEEP=P0^6 ;
typedef unsigned char uchar;
typedef unsigned int uint;

uchar code HEX_VALUE[]=

{54,57,59,63,68,73,78,84,90,96,103,113,123,135,141};  //15

float code LENGTH[]={10, 9.5, 9, 8.5, 8, 7.5, 7, 6.5, 6, 5.5, 5, 4.5, 4,

3.5, 3};


sfr ADC_CONTR   =   0xBC;           //ADC control register
sfr ADC_RES     =   0xBD;           //ADC high 8-bit result register
sfr ADC_LOW2    =   0xBE;           //ADC low 2-bit result register
sfr P1ASF       =   0x9D;           //P1 secondary function control register

//定义ADC 赋值操作数
#define ADC_POWER   0x80            //ADC 电源控制位
#define ADC_FLAG    0x10            //ADC 完成标志
#define ADC_START   0x08            //ADC 开始控制位
#define ADC_SPEEDLL 0x00            //420 clocks
#define ADC_SPEEDL  0x20            //280 clocks
#define ADC_SPEEDH  0x40            //140 clocks
#define ADC_SPEEDHH 0x60            //70 clocks

void InitUart(); //串口初始化函数
void InitADC();	 //ADC初始化函数
void SendData(float dat); 
uchar GetADCResult(uchar ch);
void Delay(uint n);
void ShowResult(uchar ch);
float Return_length(uint num);


//void main()
//{
//    InitUart();                     //Init UART, use to show ADC result
//    InitADC();                      //Init ADC sfr
//    while (1)
//    {
//	    if(KEY==0)
//		{
//		    Delay(100);
//	        ShowResult(0); 		
//		}
//	
//    }
//}

/*----------------------------
发送ADC结果	  ch传入参数0 定义P1_0口为模数转换通道
----------------------------*/
void ShowResult(uchar ch)
{
    SendData(ch);                   //显示通道数
    SendData(GetADCResult(ch));     //显示高8位ADC转换值
}

/*----------------------------
获取ADC转换的结果
----------------------------*/
uchar GetADCResult(uchar ch)
{
    ADC_CONTR = ADC_POWER | ADC_SPEEDLL | ch | ADC_START;
    _nop_();                       
    _nop_();
    _nop_();
    _nop_();
    while (!(ADC_CONTR & ADC_FLAG));//等待完成标志
    ADC_CONTR &= ~ADC_FLAG;         //关闭ADC

    return Return_length(ADC_RES);                //返回ADC转换值
}

/*----------------------------
初始化串口
----------------------------*/
void InitUart()
{
    SCON = 0x5a;                    
    TMOD = 0x20;                    //T1 8位装值

    TH1=0xfd ;
	TL1=0xfd ;
    TR1 = 1;                        //T1 启动
}

/*----------------------------
初始化ADC寄存器
----------------------------*/
void InitADC()
{
    P1ASF = 0x01;                   //P1_0设为模拟电压输入端口
    ADC_RES = 0;                    //清除之前的数值
    ADC_CONTR = ADC_POWER | ADC_SPEEDLL;
    Delay(2);                       //开启ADC电源
}

/*----------------------------
发送数据给上位机PC
----------------------------*/
void SendData(float dat)
{
    while (!TI);                    
    TI = 0;                         
    SBUF = dat;                    
}

/*----------------------------
软件延时
----------------------------*/
void Delay(uint n)
{
    uint x;       

    while (n--)
    {
        x = 5000;
        while (x--);
    }
}
//查询函数，根据AD转换数值，查表找出长度，红外测距函数
float Return_length(uint num)
{
	uint mid_num,high=14,mid=7,low=0;
	while(low<high && low<mid)
	{
		mid_num=HEX_VALUE[mid];

		if(num==HEX_VALUE[mid])
		{
			return (uchar)LENGTH[mid];
		}
		if(num==HEX_VALUE[low])
        {
			return (uchar)LENGTH[low];

        }
        if(num==HEX_VALUE[high])
        {
			return (uchar)LENGTH[high];
        }
		else if(num < mid_num)
		{
			high=mid;
			mid=(mid+low)/2;
			low=low;
		}
		else if(num > mid_num)
		{
			low=mid;
			mid=(high+mid)/2;
			high=high;
		}
	}
	return (uchar)((LENGTH[low]+LENGTH[high])/2.0);
}
