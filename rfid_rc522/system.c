#include <reg52.h>
#include "uart.h"
#include <mfrc522.h>
#include "system.h"

char code DefaultKey[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; //初始密码
unsigned char g_ucTempbuf1[4];  //返回卡号	
static char w_ucTempbuf[17]={'2','0','1','8','3','1','0','8','2','4','0','5',-1};//待写入的数据
static unsigned char g_ucTempbuf2[17]; //返回块中16位数据 
sfr   RCAP2LH =   0xCA;
sfr   T2LH    =   0xCC;
sbit f = P2^5;
sbit led = P2^0;
unsigned char addr;
unsigned char len;
unsigned char dataLen=17;//读取或者写入卡信息的长度
///////////////////////////////////////////
//功能：进行通信系统的初始化
//////////////////////////////////////////
void initSystem()
{
	initUart();					//串口通信初始化
	PcdReset();					//rfid_rc522通信初始化
  PcdAntennaOff(); 
  PcdAntennaOn();
	//M500PcdConfigISOType('A');
	addr = 0;
}

///////////////////////////////////////////////////////////////////////
// Delay 10ms
///////////////////////////////////////////////////////////////////////
void delay(unsigned int _10ms)
{
#ifndef NO_TIMER2
    RCAP2LH = RCAP2_10ms;
    T2LH    = RCAP2_10ms;
    
    TR2 = TRUE;
    while (_10ms--)
    {
	    while (!TF2);
	    TF2 = FALSE;
    }
    TR2 = FALSE;
#else
    while (_10ms--)
    {
	    delay_50us(19);
	    if (CmdValid)
	        return;
	    delay_50us(20);
	    if (CmdValid)
	        return;
	    delay_50us(20);
	    if (CmdValid)
	        return;
	    delay_50us(20);
	    if (CmdValid)
	        return;
	    delay_50us(20);
	    if (CmdValid )
	        return;
	    delay_50us(20);
	    if (CmdValid)
	        return;
	    delay_50us(20);
	    if (CmdValid)
	        return;
	    delay_50us(20);
	    if (CmdValid)
	        return;
	    delay_50us(20);
	    if (CmdValid)
	        return;
	    delay_50us(19);
	    if (CmdValid)
	        return;
    }
#endif
}

///////////////////////////////////////////////
//功能：蜂鸣器响应
///////////////////////////////////////////////
void feeb()
{
	f=1;
	delay(1); 		//延时大约100ms   通过修改此延时时间达到不同的发声效果	
	f=0;
	delay(1);
	
}


void LED()
{
	led=0;
	delay(1); 		//延时大约100ms   通过修改此延时时间达到不同的发声效果	
	led=1;
	delay(1);
	
}
///////////////////////////////////////////////////
//中断服务函数
//功能：将从RC522中接收到的数据填入SBUF中
//参数：接收到的数据
//////////////////////////////////////////////////
void isr_UART() interrupt 4 using 1
{
	unsigned char i;
	if(TI){
		TI=0;
		for(i=0;i<dataLen;i++){
			SBUF=g_ucTempbuf2[i];
			while(!TI);
			TI=0;			
		}
		REN=1;
	}
}
///////////////////////////////////////////////
//功能：寻卡
/////////////////////////////////////////////
void readCart()
{
	char status;
	while(1)
	{
		unsigned char status;
		
    status = PcdRequest(PICC_REQALL, g_ucTempbuf1);//*PICC_REQALL=0x52:寻天线区内所有符合14443A标准的卡  PICC_REQIDL=0x26:只寻未进入休眠状态的卡  	            															//寻卡失败
		if (status == MI_OK)
		{
			status = PcdAnticoll(g_ucTempbuf1);  					//防冲撞处理，输出卡片序列号，4字节 
		}
		
		if (status == MI_OK)
		{
			status = PcdSelect(g_ucTempbuf1);    //选择卡片，输入卡片序列号，4字节
		}
	  
		if (status == MI_OK) 
		{
			status = PcdAuthState(PICC_AUTHENT1A, 1, DefaultKey, g_ucTempbuf1);//打开天线
			feeb();
		}
		if (status == MI_OK)   
		{
			status = PcdRead(1, g_ucTempbuf2);//读卡---第一个扇区的数据块一
		}
		if(status == MI_OK)
		{
			g_ucTempbuf2[16]=0x11;
			CALL_isr_UART();
			LED();
		}
  } 
}
//////////////////////////////////////////////////////////////////////////
//写卡------（注释：我的程序是只更改一次信息，也可以用中断直接写入信息）
//////////////////////////////////////////////////////////////////////////
void writeCart()
{
			unsigned char status;
		
    status = PcdRequest(PICC_REQALL, g_ucTempbuf1);//*PICC_REQALL=0x52:寻天线区内所有符合14443A标准的卡  PICC_REQIDL=0x26:只寻未进入休眠状态的卡  	            															//寻卡失败
		if (status == MI_OK)
		{
			status = PcdAnticoll(g_ucTempbuf1);  					//防冲撞处理，输出卡片序列号，4字节 
		}
		
		if (status == MI_OK)
		{
			status = PcdSelect(g_ucTempbuf1);    //选择卡片，输入卡片序列号，4字节
		}
	  
		if (status == MI_OK) 
		{
			status = PcdAuthState(PICC_AUTHENT1A, 1, DefaultKey, g_ucTempbuf1);//打开天线
			feeb();
		}
		if (status == MI_OK)   
		{
			status = PcdWrite(1, w_ucTempbuf);//读卡---第一个扇区的数据块一
		}
		if (status == MI_OK)   
		{
			status = PcdRead(1, g_ucTempbuf2);//读卡---第一个扇区的数据块一
		}
		if(status == MI_OK)
		{
			g_ucTempbuf2[16]=0x11;
			CALL_isr_UART();
			LED();
		}
}
	
