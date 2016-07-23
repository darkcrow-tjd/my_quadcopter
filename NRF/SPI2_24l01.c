#include "24l01.h"
#include "delay.h"

#include "usart.h"


u8 SPI2_ReadWriteByte(u8 TxData)
{
	u8 retry=0;				 	
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET) //���ָ����SPI��־λ�������:���ͻ���ձ�־λ
	{
		retry++;
		if(retry>200)return 0;
	}			  
	SPI_I2S_SendData(SPI2, TxData); //ͨ������SPIx����һ������
	retry=0;

	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET)//���ָ����SPI��־λ�������:���ܻ���ǿձ�־λ
	{
		retry++;
		if(retry>200)return 0;
	}	  					
	return SPI_I2S_ReceiveData(SPI2); //����ͨ��SPIx������յ�����					    
}


void SPI2_SetSpeed(u8 SPI_BaudRatePrescaler)
{
  	assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));
	SPI2->CR1&=0XFFC7;
	SPI2->CR1|=SPI_BaudRatePrescaler;	//����SPI2�ٶ� 
	SPI_Cmd(SPI2,ENABLE); 

} 
//SPI2 ��ʼ��
//SPI2 nrf24l01 ��ʼ��
//PB13 14 15 SCK MISO MOSI �����������
//PG67 CE CS �������
//PG8 IRQ
void SPI2_NRF24L01_Init(void)
{ 	
	GPIO_InitTypeDef GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB |RCC_APB2Periph_GPIOD| RCC_APB2Periph_GPIOG | RCC_APB2Periph_AFIO, ENABLE);	 //ʹ��PB,D,G�˿�ʱ��
	RCC_APB1PeriphClockCmd(	RCC_APB1Periph_SPI2,  ENABLE );//SPI2ʱ��ʹ�� 	    	
	/*
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;				 //PB12���� ��ֹW25X�ĸ���
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);	//��ʼ��ָ��IO
 	GPIO_SetBits(GPIOB,GPIO_Pin_12);//����				
 	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;				 //PD2�����������   ��ֹSD���ĸ���
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 	GPIO_SetBits(GPIOD,GPIO_Pin_2);//��ʼ��ָ��IO
	*/
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;	//PG6 7 ���� 	  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  //������� 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
 	GPIO_Init(GPIOG, &GPIO_InitStructure);//��ʼ��ָ��IO
  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;  
 	GPIO_Init(GPIOG, &GPIO_InitStructure);//��ʼ��ָ��IO
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //PB13/14/15����������� 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIOB

//GPIO_ResetBits(GPIOG,GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8);//PG6,7,8����					

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //SPI����Ϊ˫��˫��ȫ˫��
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//SPI����
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//���ͽ���8λ֡�ṹ
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//ʱ�����յ�
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	//���ݲ����ڵ�1��ʱ����
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS�ź����������
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;		//���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ16
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//���ݴ����MSBλ��ʼ
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRCֵ����Ķ���ʽ
	SPI_Init(SPI2, &SPI_InitStructure);  //����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ���
	SPI_Cmd(SPI2, ENABLE); //ʹ��SPI����
	
	SPI2_ReadWriteByte(0xff);//��������		 
 
			 
	SPI2_NRF24L01_CE=0; 			//ʹ��24L01
	SPI2_NRF24L01_CSN=1;			//SPIƬѡȡ��  
	 		 	 
}



u8 SPI2_NRF24L01_Check(void)
{
	u8 buf[5]={0XA5,0XA5,0XA5,0XA5,0XA5};
	u8 i;
	SPI2_SetSpeed(SPI_BaudRatePrescaler_8); //spi�ٶ�Ϊ9Mhz��24L01�����SPIʱ��Ϊ10Mhz��   	 
	SPI2_NRF24L01_Write_Buf(WRITE_REG_NRF+TX_ADDR,buf,5);//д��5���ֽڵĵ�ַ.	
	SPI2_NRF24L01_Read_Buf(TX_ADDR,buf,5); //����д��ĵ�ַ  
	for(i=0;i<5;i++)if(buf[i]!=0XA5)break;	 							   
	if(i!=5)return 1;//���24L01����	
	return 0;		 //��⵽24L01
}	 	 

u8 SPI2_NRF24L01_Write_Reg(u8 reg,u8 value)
{
	u8 status;	
   	SPI2_NRF24L01_CSN=0;                 //ʹ��SPI����
  	status =SPI2_ReadWriteByte(reg);//���ͼĴ����� 
  	SPI2_ReadWriteByte(value);      //д��Ĵ�����ֵ
  	SPI2_NRF24L01_CSN=1;                 //��ֹSPI����	   
  	return(status);       			//����״ֵ̬
}

u8 SPI2_NRF24L01_Read_Reg(u8 reg)
{
	u8 reg_val;	    
 	SPI2_NRF24L01_CSN = 0;          //ʹ��SPI����		
  	SPI2_ReadWriteByte(reg);   //���ͼĴ�����
  	reg_val=SPI2_ReadWriteByte(0XFF);//��ȡ�Ĵ�������
  	SPI2_NRF24L01_CSN = 1;          //��ֹSPI����		    
  	return(reg_val);           //����״ֵ̬
}	

u8 SPI2_NRF24L01_Read_Buf(u8 reg,u8 *pBuf,u8 len)
{
	u8 status,u8_ctr;	       
  	SPI2_NRF24L01_CSN = 0;           //ʹ��SPI����
  	status=SPI2_ReadWriteByte(reg);//���ͼĴ���ֵ(λ��),����ȡ״ֵ̬   	   
 	for(u8_ctr=0;u8_ctr<len;u8_ctr++)pBuf[u8_ctr]=SPI2_ReadWriteByte(0XFF);//��������
  	SPI2_NRF24L01_CSN=1;       //�ر�SPI����
  	return status;        //���ض�����״ֵ̬
}

u8 SPI2_NRF24L01_Write_Buf(u8 reg, u8 *pBuf, u8 len)
{
	u8 status,u8_ctr;	    
 	SPI2_NRF24L01_CSN = 0;          //ʹ��SPI����
  	status = SPI2_ReadWriteByte(reg);//���ͼĴ���ֵ(λ��),����ȡ״ֵ̬
  	for(u8_ctr=0; u8_ctr<len; u8_ctr++)SPI2_ReadWriteByte(*pBuf++); //д������	 
  	SPI2_NRF24L01_CSN = 1;       //�ر�SPI����
  	return status;          //���ض�����״ֵ̬
}				   

u8 SPI2_NRF24L01_TxPacket(u8 *txbuf)
{
	u8 sta;
// 	SPI2_SetSpeed(SPI_BaudRatePrescaler_8);//spi�ٶ�Ϊ9Mhz��24L01�����SPIʱ��Ϊ10Mhz��   
	SPI2_NRF24L01_CE=0;
  	SPI2_NRF24L01_Write_Buf(WR_TX_PLOAD,txbuf,TX_PLOAD_WIDTH);//д���ݵ�TX BUF  32���ֽ�
 	SPI2_NRF24L01_CE=1;//��������	   
	while(SPI2_NRF24L01_IRQ!=0);//�ȴ��������
	sta=SPI2_NRF24L01_Read_Reg(STATUS);  //��ȡ״̬�Ĵ�����ֵ	   
	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+STATUS,sta); //���TX_DS��MAX_RT�жϱ�־
	if(sta&MAX_TX)//�ﵽ����ط�����
	{
		SPI2_NRF24L01_Write_Reg(FLUSH_TX,0xff);//���TX FIFO�Ĵ��� 
		return MAX_TX; 
	}
	if(sta&TX_OK)//�������
	{
		return TX_OK;
	}
	return 0xff;//����ԭ����ʧ��
}

//α˫�����շ�����һ��ack���ݰ�
void SPI2_NRF24L01_TxPacket_AP(uint8_t * tx_buf)
{	
	SPI2_NRF24L01_CE=0;	 //StandBy Iģʽ	
	//command word 1010 1xxx ���ͨ��4 ͨ��0װ������
	SPI2_NRF24L01_Write_Buf(0xa8, tx_buf, TX_PLOAD_WIDTH); 			
	SPI2_NRF24L01_CE=1;	 //�ø�CE
}
u8 SPI2_NRF24L01_RxPacket(u8 *rxbuf)
{
	u8 sta;		    							   
//	SPI2_SetSpeed(SPI_BaudRatePrescaler_8); //spi�ٶ�Ϊ9Mhz��24L01�����SPIʱ��Ϊ10Mhz��   
	sta=SPI2_NRF24L01_Read_Reg(STATUS);  //��ȡ״̬�Ĵ�����ֵ    	 
	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+STATUS,sta); //���TX_DS��MAX_RT�жϱ�־
	if(sta&RX_OK)//���յ�����
	{
		SPI2_NRF24L01_Read_Buf(RD_RX_PLOAD,rxbuf,RX_PLOAD_WIDTH);//��ȡ����
		SPI2_NRF24L01_TxPacket_AP(rxbuf);
//		SPI2_NRF24L01_Write_Reg(FLUSH_RX,0xff);//���RX FIFO�Ĵ��� 
		return 0; 
	}	   
	return 1;//û�յ��κ�����
}					
//���ռ��ص����ݰ���
void SPI2_NRF24L01_RxPacket_AP(u8 *rxbuf)
{
	u8 sta;
	sta = SPI2_NRF24L01_Read_Reg(READ_REG_NRF + STATUS);
	if(sta & RX_OK)
	{
		u8 rx_len = SPI2_NRF24L01_Read_Reg(0x60);
		if(rx_len<33)
		{
			SPI2_NRF24L01_Read_Buf(RD_RX_PLOAD,rxbuf,rx_len);// read receive payload from RX_FIFO buffer
			usart1_send_char(0x55);	
			
		}
		else 
		{
			SPI2_NRF24L01_Write_Reg(FLUSH_RX,0xff);//��ջ�����
		}
	}
	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF + STATUS, sta);
}

void SPI2_NRF24L01_RX_Mode(void)
{
	SPI2_NRF24L01_CE=0;	  
  	SPI2_NRF24L01_Write_Buf(WRITE_REG_NRF+RX_ADDR_P0,(u8*)RX_ADDRESS,RX_ADR_WIDTH);//дRX�ڵ��ַ
	  
  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+EN_AA,0x01);    //ʹ��ͨ��0���Զ�Ӧ��    
  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+EN_RXADDR,0x01);//ʹ��ͨ��0�Ľ��յ�ַ  	 
  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+RF_CH,40);	     //����RFͨ��Ƶ��		  
  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+RX_PW_P0,RX_PLOAD_WIDTH);//ѡ��ͨ��0����Ч���ݿ�� 	    
  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+RF_SETUP,0x0f);//����TX�������,0db����,2Mbps,���������濪��   
  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+CONFIG, 0x0f);//���û�������ģʽ�Ĳ���;PWR_UP,EN_CRC,16BIT_CRC,����ģʽ 
  SPI2_NRF24L01_CE = 1; //CEΪ��,�������ģʽ 
}			


void SPI2_NRF24L01_TX_Mode(void)
{														 
	SPI2_NRF24L01_CE=0;	    
  	SPI2_NRF24L01_Write_Buf(WRITE_REG_NRF+TX_ADDR,(u8*)TX_ADDRESS,TX_ADR_WIDTH);//дTX�ڵ��ַ 
  	SPI2_NRF24L01_Write_Buf(WRITE_REG_NRF+RX_ADDR_P0,(u8*)RX_ADDRESS,RX_ADR_WIDTH); //����TX�ڵ��ַ,��ҪΪ��ʹ��ACK	  

  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+EN_AA,0x01);     //ʹ��ͨ��0���Զ�Ӧ��    
  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+EN_RXADDR,0x01); //ʹ��ͨ��0�Ľ��յ�ַ  
  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+SETUP_RETR,0xfa);//�����Զ��ط����ʱ��:500us + 86us;����Զ��ط�����:10��
  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+RF_CH,40);       //����RFͨ��Ϊ40
  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+RF_SETUP,0x0f);  //����TX�������,0db����,2Mbps,���������濪��   
  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+CONFIG,0x0e);    //���û�������ģʽ�Ĳ���;PWR_UP,EN_CRC,16BIT_CRC,����ģʽ,���������ж�
	SPI2_NRF24L01_CE=1;//CEΪ��,10us����������
}		  



void SPI2_TX2_Mode(void)
{
	SPI2_NRF24L01_CE=0;	    
  	SPI2_NRF24L01_Write_Buf(WRITE_REG_NRF+TX_ADDR,(u8*)TX_ADDRESS,TX_ADR_WIDTH);//дTX�ڵ��ַ 
  	SPI2_NRF24L01_Write_Buf(WRITE_REG_NRF+RX_ADDR_P0,(u8*)RX_ADDRESS,RX_ADR_WIDTH); //����TX�ڵ��ַ,��ҪΪ��ʹ��ACK	  

  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+EN_AA,0x01);     //ʹ��ͨ��0���Զ�Ӧ��    
  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+EN_RXADDR,0x01); //ʹ��ͨ��0�Ľ��յ�ַ  
  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+SETUP_RETR,0xfa);//�����Զ��ط����ʱ��:500us + 86us;����Զ��ط�����:10��
  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+RF_CH,40);       //����RFͨ��Ϊ40
  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+RF_SETUP,0x0f);  //����TX�������,0db����,2Mbps,���������濪��   
  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+CONFIG,0x0e);    //���û�������ģʽ�Ĳ���;PWR_UP,EN_CRC,16BIT_CRC,����ģʽ,���������ж�
	
		//α˫����������
		SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF + CONFIG, 0x0e);   		 // IRQ�շ�����жϿ���,16λCRC,������
		SPI2_NRF24L01_Write_Reg(FLUSH_TX,0xff);
		SPI2_NRF24L01_Write_Reg(FLUSH_RX,0xff);
		SPI2_ReadWriteByte(0x50);
		SPI2_ReadWriteByte(0x73);
		SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+0x1c,0x01);
		SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+0x1d,0x06);
	SPI2_NRF24L01_CE=1;//CEΪ��,10us����������

}
void SPI2_RX2_Mode(void)
{
	SPI2_NRF24L01_CE=0;
		SPI2_NRF24L01_Write_Buf(WRITE_REG_NRF+RX_ADDR_P0,(u8*)RX_ADDRESS,RX_ADR_WIDTH);	//дRX�ڵ��ַ 
		SPI2_NRF24L01_Write_Buf(WRITE_REG_NRF+TX_ADDR,(u8*)TX_ADDRESS,TX_ADR_WIDTH); 		//дTX�ڵ��ַ  
		   
		SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+EN_AA,0x01); 													//ʹ��ͨ��0���Զ�Ӧ�� 
		SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+EN_RXADDR,0x01);											//ʹ��ͨ��0�Ľ��յ�ַ 
		SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+RF_CH,40);														//����RFͨ��ΪCHANAL	
		   
		SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+RF_SETUP,0x0f); 												//����TX�������,0db����,2Mbps,���������濪��	
		SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+SETUP_RETR,0x1a);											//�����Զ��ط����ʱ��:500us;����Զ��ط�����:10�� 
		//α˫������
		SPI2_NRF24L01_Write_Reg(FLUSH_TX,0xff);
		SPI2_NRF24L01_Write_Reg(FLUSH_RX,0xff);
		SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF + CONFIG, 0x0f);   		 // IRQ�շ�����жϿ���,16λCRC,������
		SPI2_ReadWriteByte(0x50);
		SPI2_ReadWriteByte(0x73);
		SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+0x1c,0x01);
		SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+0x1d,0x06);
	SPI2_NRF24L01_CE = 1;
}	







