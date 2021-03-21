#include <reg52.h>
#include <intrins.h>
#include "mfrc522.h"
#include "system.h"
#define MAXRLEN 18
//////////////////////////////////
//端口定义
//////////////////////////////////
//RFID-RC522
sbit     MF522_NSS  =    P1^4;			//片选
sbit     MF522_SCK  =    P1^1;
sbit     MF522_SI   =    P1^0;
sbit     MF522_SO   =    P1^2;
sbit     MF522_RST  =    P1^3; 

//////////////////////////////////////////////////////////////////////
//设置RC522的工作方式 
//////////////////////////////////////////////////
char M500PcdConfigISOType(unsigned char type){
   if (type == 'A')                     //ISO14443_A
   { 
       ClearBitMask(Status2Reg,0x08);
       WriteRawRC(ModeReg,0x3D);//3
       WriteRawRC(RxSelReg,0x86);//
       WriteRawRC(RFCfgReg,0x7F);   //4F
   	   WriteRawRC(TReloadRegL,30);//tmoLength);// TReloadVal = 'h6a =tmoLength(dec) 
			 WriteRawRC(TReloadRegH,0);
       WriteRawRC(TModeReg,0x8D);
			 WriteRawRC(TPrescalerReg,0x3E);
	     delay(1);
       PcdAntennaOn();
   }
   else{ return -1; }
   
   return MI_OK;
}

/////////////////////////////////////////////////
//功能：向RC522的寄存器里面写命令——SPI通信（51--->rfid-rc522）
//参数：Address:寄存器的地址
//参数：value：想要写入的命令
/////////////////////////////////////////////////
void WriteRawRC(unsigned char Address,unsigned char value)
{
	unsigned char i,ucAddr;
	MF522_SCK=0;
	MF522_NSS=0;
	ucAddr = (Address<<1)&0x7E;
	for(i=8;i>0;i--)
	{
		MF522_SI = ((ucAddr & 0x80)==0x80);
		MF522_SCK = 1;
		ucAddr<<=1;
		MF522_SCK = 0;
	}
	for(i=8;i>0;i--)
    {
        MF522_SI = ((value&0x80)==0x80);
        MF522_SCK = 1;
        value <<= 1;
        MF522_SCK = 0;
    }
    MF522_NSS = 1;
    MF522_SCK = 1;
}
//////////////////////////////////////////////
//功能：读出寄存器里面的值
//参数说明：Address 寄存器地址
//返回：读出的值
//////////////////////////////////////////////
unsigned char ReadRawRC(unsigned char Address)
{
	unsigned char i,ucAddr;
	unsigned char res;
	MF522_SCK = 0;
	MF522_NSS = 0;
	ucAddr = (((Address<<1)&0x7e)|0x80);
	for(i=8;i>0;i--)
	{
		MF522_SI=((ucAddr&0x80)==0x80);
		MF522_SCK = 1;
		ucAddr<<=1;
		MF522_SCK=0;
	}
	for(i=8;i>0;i--)
	{
		MF522_SCK = 1;
		res<<=1;
		res|=(bit)MF522_SO;
		MF522_SCK=0;
		
	}
	MF522_NSS = 1;
	MF522_SCK = 1;
	return res;
}
/////////////////////////////////////////////////
//功能：置位寄存器
//参数：reg 寄存器地址
//参数：mask 置位值
////////////////////////////////////////////////
void SetBitMask(unsigned char reg,unsigned char mask)
{
	unsigned char tmp = 0x00;
	tmp = ReadRawRC(reg);
	WriteRawRC(reg, tmp | mask);  // clear bit mask
}

/////////////////////////////////////////////////////////////////////
//功    能：清RC522寄存器位
//参数说明：reg[IN]:寄存器地址
//          mask[IN]:清位值
/////////////////////////////////////////////////////////////////////
void ClearBitMask(unsigned char reg,unsigned char mask)  
{
    char tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp & ~mask);  // clear bit mask
} 

/////////////////////////////////////////////////////////////////////
//功    能：选定卡片
//参数说明: pSnr[IN]:卡片序列号，4字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdSelect(unsigned char *pSnr)
{
    char status;
    unsigned char i;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (i=0; i<4; i++)
    {
    	ucComMF522Buf[i+2] = *(pSnr+i);
    	ucComMF522Buf[6]  ^= *(pSnr+i);
    }
    CalulateCRC(ucComMF522Buf,7,&ucComMF522Buf[7]);
  
    ClearBitMask(Status2Reg,0x08);

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,9,ucComMF522Buf,&unLen);
    
    if ((status == MI_OK) && (unLen == 0x18))
    {   status = MI_OK;  }
    else
    {   status = MI_ERR;    }

    return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：验证卡片密码
//参数说明: auth_mode[IN]: 密码验证模式
//                 0x60 = 验证A密钥
//                 0x61 = 验证B密钥 
//          addr[IN]：块地址
//          pKey[IN]：密码
//          pSnr[IN]：卡片序列号，4字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////               
char PcdAuthState(unsigned char auth_mode,unsigned char addr,unsigned char *pKey,unsigned char *pSnr)
{
    char status;
    unsigned int  unLen;
    unsigned char i,ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = auth_mode;
    ucComMF522Buf[1] = addr;
    for (i=0; i<6; i++)
    {    ucComMF522Buf[i+2] = *(pKey+i);   }
    for (i=0; i<6; i++)
    {    ucComMF522Buf[i+8] = *(pSnr+i);   }
 //   memcpy(&ucComMF522Buf[2], pKey, 6); 
 //   memcpy(&ucComMF522Buf[8], pSnr, 4); 
    
    status = PcdComMF522(PCD_AUTHENT,ucComMF522Buf,12,ucComMF522Buf,&unLen);
    if ((status != MI_OK) || (!(ReadRawRC(Status2Reg) & 0x08)))
    {   status = MI_ERR;   }   
    return status;
}


/////////////////////////////////////////////////////////////////////
//功    能：读取M1卡一块数据
//参数说明: addr[IN]：块地址
//          pData[OUT]：读出的数据，16字节
//返    回: 成功返回MI_OK
///////////////////////////////////////////////////////////////////// 
char PcdRead(unsigned char addr,unsigned char *pData)
{
    char status;
    unsigned int  unLen;
    unsigned char i,ucComMF522Buf[MAXRLEN]; 
		
	
    ucComMF522Buf[0] = PICC_READ;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
   
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
    if ((status == MI_OK)&& unLen == 0x90)
    {
        for (i=0; i<16; i++)
        {    
					*(pData+i) = ucComMF522Buf[i];
				}
    }
    else
    {

				status = MI_ERR;   
		}
    
    return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：写数据到M1卡一块
//参数说明: addr[IN]：块地址
//          pData[IN]：写入的数据，16字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////                  
char PcdWrite(unsigned char addr,unsigned char *pData)
{
    char status;
    unsigned int  unLen;
    unsigned char i,ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
		
	
		//选地址
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {   status = MI_ERR;   }
     

		//发数据	
    if (status == MI_OK)
    {
        //memcpy(ucComMF522Buf, pData, 16);
        for (i=0; i<16; i++)
        {    ucComMF522Buf[i] = *(pData+i);   }
        CalulateCRC(ucComMF522Buf,16,&ucComMF522Buf[16]);

        status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,18,ucComMF522Buf,&unLen);
        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {   status = MI_ERR;   }
    }
    
    return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：寻卡
//参数说明: req_code[IN]:寻卡方式
//                0x52 = 寻感应区内所有符合14443A标准的卡
//                0x26 = 寻未进入休眠状态的卡
//          pTagType[OUT]：卡片类型代码
//                0x4400 = Mifare_UltraLight
//                0x0400 = Mifare_One(S50)
//                0x0200 = Mifare_One(S70)
//                0x0800 = Mifare_Pro(X)
//                0x4403 = Mifare_DESFire
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdRequest(unsigned char req_code,unsigned char *pTagType)
{
   char status;  
   unsigned int  unLen;
   unsigned char ucComMF522Buf[MAXRLEN]; 

   ClearBitMask(Status2Reg,0x08);//软件清零标志位
   WriteRawRC(BitFramingReg,0x07);//停止发送，设置接收到的LSB放在位0，接收到的第2 位放在位1，D2D1D0=111表示最后一个字节发7位
   SetBitMask(TxControlReg,0x03);//对天线是否发送调制数据设置，开启天线
 
   ucComMF522Buf[0] = req_code;

   status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,1,ucComMF522Buf,&unLen);
   
   if ((status == MI_OK) && (unLen == 0x10))		//传回16位卡片类型代码
   {    
       *pTagType     = ucComMF522Buf[0];
       *(pTagType+1) = ucComMF522Buf[1];
   }
   else
   {   status = MI_ERR;   }
   
   return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：防冲撞
//参数说明: pSnr[OUT]:卡片序列号，4字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////  
char PcdAnticoll(unsigned char *pSnr)
{
    char status;
    unsigned char i,snr_check=0;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 
    

    ClearBitMask(Status2Reg,0x08);
    WriteRawRC(BitFramingReg,0x00);
    ClearBitMask(CollReg,0x80);
 
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x20;

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,2,ucComMF522Buf,&unLen);

    if (status == MI_OK)
    {
    	 for (i=0; i<4; i++)
         {   
             *(pSnr+i)  = ucComMF522Buf[i];
             snr_check ^= ucComMF522Buf[i];
         }
         if (snr_check != ucComMF522Buf[i])
         {   status = MI_ERR;    }
    }
    
    SetBitMask(CollReg,0x80);
    return status;
}
/////////////////////////////////////////////////////////////////////
//功    能：通过RC522和ISO14443卡通讯
//参数说明：Command[IN]:RC522命令字
//          pInData[IN]:通过RC522发送到卡片的数据
//          InLenByte[IN]:发送数据的字节长度
//          pOutData[OUT]:接收到的卡片返回数据
//          *pOutLenBit[OUT]:返回数据的位长度
/////////////////////////////////////////////////////////////////////
char PcdComMF522(unsigned char Command, 
                 unsigned char *pInData, 
                 unsigned char InLenByte,
                 unsigned char *pOutData, 
                 unsigned int  *pOutLenBit)
{
    char status = MI_ERR;
    unsigned char irqEn   = 0x00;
    unsigned char waitFor = 0x00;
    unsigned char lastBits;
    unsigned char n;
    unsigned int i;
    switch (Command)
    {
       case PCD_AUTHENT:   
          irqEn   = 0x12;
          waitFor = 0x10;
          break;
       case PCD_TRANSCEIVE:
          irqEn   = 0x77;
          waitFor = 0x30;
          break;
       default:
         break;
    }
   
    WriteRawRC(ComIEnReg,irqEn|0x80);			//设置各中断请求传递到IRQ引脚，D7=1:有中断时IRQ为低，
    ClearBitMask(ComIrqReg,0x80);					//ComIrqReg屏蔽位清零
    WriteRawRC(CommandReg,PCD_IDLE);			//取消当前命令
    SetBitMask(FIFOLevelReg,0x80);				//FIFO缓冲区读和写指针以及FIFO缓冲区溢出错误标志BufferOvfl清零
    
    for (i=0; i<InLenByte; i++)
    {   
			WriteRawRC(FIFODataReg, pInData[i]);		//要发送的数据写入FIFO缓冲区
    }		
    WriteRawRC(CommandReg, Command);
   
    
    if (Command == PCD_TRANSCEIVE)
    {    
			SetBitMask(BitFramingReg,0x80);  				//start TX
		}		
		
    i = 600;//根据时钟频率调整，操作M1卡最大等待时间25ms
    do 
    {
         n = ReadRawRC(ComIrqReg);		//读中断请求标志
         i--;
    }
    while((i!=0) && !(n&0x01) && !(n&waitFor));
		
    ClearBitMask(BitFramingReg,0x80);		//stop TX
	      
    if (i!=0)
    {    
         if(!(ReadRawRC(ErrorReg)&0x1B))		//检测是否有出现的种错误
         {
             status = MI_OK;
             if (n & irqEn & 0x01)
             {   status = MI_NOTAGERR;   }
             if (Command == PCD_TRANSCEIVE)
             {
               	n = ReadRawRC(FIFOLevelReg);		//读FIFO缓冲区中的字节数
              	lastBits = ReadRawRC(ControlReg) & 0x07;		//查看最后接收到的字节有效位的数目，如果为0则整个字节有效
                if (lastBits)
                {   *pOutLenBit = (n-1)*8 + lastBits;   }
                else
                {   *pOutLenBit = n*8;   }
                if (n == 0)
                {   n = 1;    }
                if (n > MAXRLEN)
                {   n = MAXRLEN;   }
                for (i=0; i<n; i++)
                {   pOutData[i] = ReadRawRC(FIFODataReg);    }		//获取接收到的数据
            }
         }
         else
         {   status = MI_ERR;   }
        
   }
   SetBitMask(ControlReg,0x80);           // stop timer now
   WriteRawRC(CommandReg,PCD_IDLE); 			//取消当前命令
   return status;
}
/////////////////////////////////////////////////////////////////////
//用MF522计算CRC16函数
/////////////////////////////////////////////////////////////////////
void CalulateCRC(unsigned char *pIndata,unsigned char len,unsigned char *pOutData)
{
    unsigned char i,n;
    ClearBitMask(DivIrqReg,0x04);			///清CRC完成标志位
    WriteRawRC(CommandReg,PCD_IDLE);	//取消当前命令
    SetBitMask(FIFOLevelReg,0x80);		//FIFO缓冲区读和写指针以及FIFO缓冲区溢出错误标志BufferOvfl清零
    for (i=0; i<len; i++)
    {   WriteRawRC(FIFODataReg, *(pIndata+i));   }
    WriteRawRC(CommandReg, PCD_CALCCRC);		//进行CRC计算
    i = 0xFF;
    do 
    {
        n = ReadRawRC(DivIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x04));	//检测CRC完成标志位
    pOutData[0] = ReadRawRC(CRCResultRegL);		//读CRC计算结果
    pOutData[1] = ReadRawRC(CRCResultRegM);
}

/////////////////////////////////////////////////////////////////////
//功    能：命令卡片进入休眠状态
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdHalt(void)
{
    char status;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = PICC_HALT;
    ucComMF522Buf[1] = 0;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    return MI_OK;
}

/////////////////////////////////////////////////////////////////////
//功    能：复位RC522
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdReset(void)
{
    MF522_RST=1;
    _nop_();
    MF522_RST=0;
    _nop_();
    MF522_RST=1;
     _nop_();
    WriteRawRC(CommandReg,PCD_RESETPHASE);
    _nop_();
    
    WriteRawRC(ModeReg,0x3D);       		//启动发送器，SIGIN高有效  ，和Mifare卡通讯，CRC初始值0x6363
    WriteRawRC(TReloadRegL,30);       	//定时器重装载值    
    WriteRawRC(TReloadRegH,0);
    WriteRawRC(TModeReg,0x8D);					//定时器设置，分频值
    WriteRawRC(TPrescalerReg,0x3E);			//定时器设置
    WriteRawRC(TxAutoReg,0x40);				
   
    return MI_OK;
}


/////////////////////////////////////////////////////////////////////
//开启天线  
//每次启动或关闭天险发射之间应至少有1ms的间隔
/////////////////////////////////////////////////////////////////////
void PcdAntennaOn(){
    unsigned char i;
    i = ReadRawRC(TxControlReg);
    if (!(i & 0x03))
    {
        SetBitMask(TxControlReg, 0x03);
    }
}


/////////////////////////////////////////////////////////////////////
//关闭天线
/////////////////////////////////////////////////////////////////////
void PcdAntennaOff()
{
    ClearBitMask(TxControlReg, 0x03);
}