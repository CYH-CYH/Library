#include <reg52.h>
#include "uart.h"
#include <mfrc522.h>
#include "system.h"

char code DefaultKey[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; //��ʼ����
unsigned char g_ucTempbuf1[4];  //���ؿ���	
static char w_ucTempbuf[17]={'2','0','1','8','3','1','0','8','2','4','0','5',-1};//��д�������
static unsigned char g_ucTempbuf2[17]; //���ؿ���16λ���� 
sfr   RCAP2LH =   0xCA;
sfr   T2LH    =   0xCC;
sbit f = P2^5;
sbit led = P2^0;
unsigned char addr;
unsigned char len;
unsigned char dataLen=17;//��ȡ����д�뿨��Ϣ�ĳ���
///////////////////////////////////////////
//���ܣ�����ͨ��ϵͳ�ĳ�ʼ��
//////////////////////////////////////////
void initSystem()
{
	initUart();					//����ͨ�ų�ʼ��
	PcdReset();					//rfid_rc522ͨ�ų�ʼ��
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
//���ܣ���������Ӧ
///////////////////////////////////////////////
void feeb()
{
	f=1;
	delay(1); 		//��ʱ��Լ100ms   ͨ���޸Ĵ���ʱʱ��ﵽ��ͬ�ķ���Ч��	
	f=0;
	delay(1);
	
}


void LED()
{
	led=0;
	delay(1); 		//��ʱ��Լ100ms   ͨ���޸Ĵ���ʱʱ��ﵽ��ͬ�ķ���Ч��	
	led=1;
	delay(1);
	
}
///////////////////////////////////////////////////
//�жϷ�����
//���ܣ�����RC522�н��յ�����������SBUF��
//���������յ�������
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
//���ܣ�Ѱ��
/////////////////////////////////////////////
void readCart()
{
	char status;
	while(1)
	{
		unsigned char status;
		
    status = PcdRequest(PICC_REQALL, g_ucTempbuf1);//*PICC_REQALL=0x52:Ѱ�����������з���14443A��׼�Ŀ�  PICC_REQIDL=0x26:ֻѰδ��������״̬�Ŀ�  	            															//Ѱ��ʧ��
		if (status == MI_OK)
		{
			status = PcdAnticoll(g_ucTempbuf1);  					//����ײ���������Ƭ���кţ�4�ֽ� 
		}
		
		if (status == MI_OK)
		{
			status = PcdSelect(g_ucTempbuf1);    //ѡ��Ƭ�����뿨Ƭ���кţ�4�ֽ�
		}
	  
		if (status == MI_OK) 
		{
			status = PcdAuthState(PICC_AUTHENT1A, 1, DefaultKey, g_ucTempbuf1);//������
			feeb();
		}
		if (status == MI_OK)   
		{
			status = PcdRead(1, g_ucTempbuf2);//����---��һ�����������ݿ�һ
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
//д��------��ע�ͣ��ҵĳ�����ֻ����һ����Ϣ��Ҳ�������ж�ֱ��д����Ϣ��
//////////////////////////////////////////////////////////////////////////
void writeCart()
{
			unsigned char status;
		
    status = PcdRequest(PICC_REQALL, g_ucTempbuf1);//*PICC_REQALL=0x52:Ѱ�����������з���14443A��׼�Ŀ�  PICC_REQIDL=0x26:ֻѰδ��������״̬�Ŀ�  	            															//Ѱ��ʧ��
		if (status == MI_OK)
		{
			status = PcdAnticoll(g_ucTempbuf1);  					//����ײ���������Ƭ���кţ�4�ֽ� 
		}
		
		if (status == MI_OK)
		{
			status = PcdSelect(g_ucTempbuf1);    //ѡ��Ƭ�����뿨Ƭ���кţ�4�ֽ�
		}
	  
		if (status == MI_OK) 
		{
			status = PcdAuthState(PICC_AUTHENT1A, 1, DefaultKey, g_ucTempbuf1);//������
			feeb();
		}
		if (status == MI_OK)   
		{
			status = PcdWrite(1, w_ucTempbuf);//����---��һ�����������ݿ�һ
		}
		if (status == MI_OK)   
		{
			status = PcdRead(1, g_ucTempbuf2);//����---��һ�����������ݿ�һ
		}
		if(status == MI_OK)
		{
			g_ucTempbuf2[16]=0x11;
			CALL_isr_UART();
			LED();
		}
}
	
