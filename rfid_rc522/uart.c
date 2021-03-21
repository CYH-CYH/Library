#include "uart.h"
#include "reg52.h"
///////////////////////////////////////////////////
//功能：初始化串口中断
//////////////////////////////////////////////////
void initUart()
{
	SCON=0X50;			//设置串口工作模式---设置为工作方式1
	TMOD=0X20;			//设置时钟中断1的工作模式---工作方式二，8位自动重装计时器
	PCON=0X00;			//波特率不加倍
	TH1=0XFA;		    //计数器初始值设置，注意波特率是4800的
	REN = 1;
	ES=1;						//打开接收中断
	EA=1;						//打开总中断
	TR1=1;				 //启动时钟中断1---TR（TCON）
}
