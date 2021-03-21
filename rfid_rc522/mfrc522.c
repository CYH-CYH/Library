#include <reg52.h>
#include <intrins.h>
#include "mfrc522.h"
#include "system.h"
#define MAXRLEN 18
//////////////////////////////////
//�˿ڶ���
//////////////////////////////////
//RFID-RC522
sbit     MF522_NSS  =    P1^4;			//Ƭѡ
sbit     MF522_SCK  =    P1^1;
sbit     MF522_SI   =    P1^0;
sbit     MF522_SO   =    P1^2;
sbit     MF522_RST  =    P1^3; 

//////////////////////////////////////////////////////////////////////
//����RC522�Ĺ�����ʽ 
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
//���ܣ���RC522�ļĴ�������д�����SPIͨ�ţ�51--->rfid-rc522��
//������Address:�Ĵ����ĵ�ַ
//������value����Ҫд�������
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
//���ܣ������Ĵ��������ֵ
//����˵����Address �Ĵ�����ַ
//���أ�������ֵ
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
//���ܣ���λ�Ĵ���
//������reg �Ĵ�����ַ
//������mask ��λֵ
////////////////////////////////////////////////
void SetBitMask(unsigned char reg,unsigned char mask)
{
	unsigned char tmp = 0x00;
	tmp = ReadRawRC(reg);
	WriteRawRC(reg, tmp | mask);  // clear bit mask
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ���RC522�Ĵ���λ
//����˵����reg[IN]:�Ĵ�����ַ
//          mask[IN]:��λֵ
/////////////////////////////////////////////////////////////////////
void ClearBitMask(unsigned char reg,unsigned char mask)  
{
    char tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp & ~mask);  // clear bit mask
} 

/////////////////////////////////////////////////////////////////////
//��    �ܣ�ѡ����Ƭ
//����˵��: pSnr[IN]:��Ƭ���кţ�4�ֽ�
//��    ��: �ɹ�����MI_OK
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
//��    �ܣ���֤��Ƭ����
//����˵��: auth_mode[IN]: ������֤ģʽ
//                 0x60 = ��֤A��Կ
//                 0x61 = ��֤B��Կ 
//          addr[IN]�����ַ
//          pKey[IN]������
//          pSnr[IN]����Ƭ���кţ�4�ֽ�
//��    ��: �ɹ�����MI_OK
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
//��    �ܣ���ȡM1��һ������
//����˵��: addr[IN]�����ַ
//          pData[OUT]�����������ݣ�16�ֽ�
//��    ��: �ɹ�����MI_OK
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
//��    �ܣ�д���ݵ�M1��һ��
//����˵��: addr[IN]�����ַ
//          pData[IN]��д������ݣ�16�ֽ�
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////                  
char PcdWrite(unsigned char addr,unsigned char *pData)
{
    char status;
    unsigned int  unLen;
    unsigned char i,ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
		
	
		//ѡ��ַ
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {   status = MI_ERR;   }
     

		//������	
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
//��    �ܣ�Ѱ��
//����˵��: req_code[IN]:Ѱ����ʽ
//                0x52 = Ѱ��Ӧ�������з���14443A��׼�Ŀ�
//                0x26 = Ѱδ��������״̬�Ŀ�
//          pTagType[OUT]����Ƭ���ʹ���
//                0x4400 = Mifare_UltraLight
//                0x0400 = Mifare_One(S50)
//                0x0200 = Mifare_One(S70)
//                0x0800 = Mifare_Pro(X)
//                0x4403 = Mifare_DESFire
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////
char PcdRequest(unsigned char req_code,unsigned char *pTagType)
{
   char status;  
   unsigned int  unLen;
   unsigned char ucComMF522Buf[MAXRLEN]; 

   ClearBitMask(Status2Reg,0x08);//��������־λ
   WriteRawRC(BitFramingReg,0x07);//ֹͣ���ͣ����ý��յ���LSB����λ0�����յ��ĵ�2 λ����λ1��D2D1D0=111��ʾ���һ���ֽڷ�7λ
   SetBitMask(TxControlReg,0x03);//�������Ƿ��͵����������ã���������
 
   ucComMF522Buf[0] = req_code;

   status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,1,ucComMF522Buf,&unLen);
   
   if ((status == MI_OK) && (unLen == 0x10))		//����16λ��Ƭ���ʹ���
   {    
       *pTagType     = ucComMF522Buf[0];
       *(pTagType+1) = ucComMF522Buf[1];
   }
   else
   {   status = MI_ERR;   }
   
   return status;
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ�����ײ
//����˵��: pSnr[OUT]:��Ƭ���кţ�4�ֽ�
//��    ��: �ɹ�����MI_OK
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
//��    �ܣ�ͨ��RC522��ISO14443��ͨѶ
//����˵����Command[IN]:RC522������
//          pInData[IN]:ͨ��RC522���͵���Ƭ������
//          InLenByte[IN]:�������ݵ��ֽڳ���
//          pOutData[OUT]:���յ��Ŀ�Ƭ��������
//          *pOutLenBit[OUT]:�������ݵ�λ����
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
   
    WriteRawRC(ComIEnReg,irqEn|0x80);			//���ø��ж����󴫵ݵ�IRQ���ţ�D7=1:���ж�ʱIRQΪ�ͣ�
    ClearBitMask(ComIrqReg,0x80);					//ComIrqReg����λ����
    WriteRawRC(CommandReg,PCD_IDLE);			//ȡ����ǰ����
    SetBitMask(FIFOLevelReg,0x80);				//FIFO����������дָ���Լ�FIFO��������������־BufferOvfl����
    
    for (i=0; i<InLenByte; i++)
    {   
			WriteRawRC(FIFODataReg, pInData[i]);		//Ҫ���͵�����д��FIFO������
    }		
    WriteRawRC(CommandReg, Command);
   
    
    if (Command == PCD_TRANSCEIVE)
    {    
			SetBitMask(BitFramingReg,0x80);  				//start TX
		}		
		
    i = 600;//����ʱ��Ƶ�ʵ���������M1�����ȴ�ʱ��25ms
    do 
    {
         n = ReadRawRC(ComIrqReg);		//���ж������־
         i--;
    }
    while((i!=0) && !(n&0x01) && !(n&waitFor));
		
    ClearBitMask(BitFramingReg,0x80);		//stop TX
	      
    if (i!=0)
    {    
         if(!(ReadRawRC(ErrorReg)&0x1B))		//����Ƿ��г��ֵ��ִ���
         {
             status = MI_OK;
             if (n & irqEn & 0x01)
             {   status = MI_NOTAGERR;   }
             if (Command == PCD_TRANSCEIVE)
             {
               	n = ReadRawRC(FIFOLevelReg);		//��FIFO�������е��ֽ���
              	lastBits = ReadRawRC(ControlReg) & 0x07;		//�鿴�����յ����ֽ���Чλ����Ŀ�����Ϊ0�������ֽ���Ч
                if (lastBits)
                {   *pOutLenBit = (n-1)*8 + lastBits;   }
                else
                {   *pOutLenBit = n*8;   }
                if (n == 0)
                {   n = 1;    }
                if (n > MAXRLEN)
                {   n = MAXRLEN;   }
                for (i=0; i<n; i++)
                {   pOutData[i] = ReadRawRC(FIFODataReg);    }		//��ȡ���յ�������
            }
         }
         else
         {   status = MI_ERR;   }
        
   }
   SetBitMask(ControlReg,0x80);           // stop timer now
   WriteRawRC(CommandReg,PCD_IDLE); 			//ȡ����ǰ����
   return status;
}
/////////////////////////////////////////////////////////////////////
//��MF522����CRC16����
/////////////////////////////////////////////////////////////////////
void CalulateCRC(unsigned char *pIndata,unsigned char len,unsigned char *pOutData)
{
    unsigned char i,n;
    ClearBitMask(DivIrqReg,0x04);			///��CRC��ɱ�־λ
    WriteRawRC(CommandReg,PCD_IDLE);	//ȡ����ǰ����
    SetBitMask(FIFOLevelReg,0x80);		//FIFO����������дָ���Լ�FIFO��������������־BufferOvfl����
    for (i=0; i<len; i++)
    {   WriteRawRC(FIFODataReg, *(pIndata+i));   }
    WriteRawRC(CommandReg, PCD_CALCCRC);		//����CRC����
    i = 0xFF;
    do 
    {
        n = ReadRawRC(DivIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x04));	//���CRC��ɱ�־λ
    pOutData[0] = ReadRawRC(CRCResultRegL);		//��CRC������
    pOutData[1] = ReadRawRC(CRCResultRegM);
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ����Ƭ��������״̬
//��    ��: �ɹ�����MI_OK
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
//��    �ܣ���λRC522
//��    ��: �ɹ�����MI_OK
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
    
    WriteRawRC(ModeReg,0x3D);       		//������������SIGIN����Ч  ����Mifare��ͨѶ��CRC��ʼֵ0x6363
    WriteRawRC(TReloadRegL,30);       	//��ʱ����װ��ֵ    
    WriteRawRC(TReloadRegH,0);
    WriteRawRC(TModeReg,0x8D);					//��ʱ�����ã���Ƶֵ
    WriteRawRC(TPrescalerReg,0x3E);			//��ʱ������
    WriteRawRC(TxAutoReg,0x40);				
   
    return MI_OK;
}


/////////////////////////////////////////////////////////////////////
//��������  
//ÿ��������ر����շ���֮��Ӧ������1ms�ļ��
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
//�ر�����
/////////////////////////////////////////////////////////////////////
void PcdAntennaOff()
{
    ClearBitMask(TxControlReg, 0x03);
}