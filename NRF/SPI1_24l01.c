#include "24l01.h"
#include "delay.h"
#include "usart.h"

const u8 TX_ADDRESS[TX_ADR_WIDTH]={0x12,0x34,0x56,0x78,0x9a}; //���͵�ַ
const u8 RX_ADDRESS[RX_ADR_WIDTH]={0x12,0x34,0x56,0x78,0x9a}; //���͵�ַ

u8 SPI1_ReadWriteByte(u8 TxData)
{
	u8 retry=0;				 	
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) //���ָ����SPI��־λ�������:���ͻ���ձ�־λ
	{
		retry++;
		if(retry>200)return 0;
	}			  
	SPI_I2S_SendData(SPI1, TxData); //ͨ������SPIx����һ������
	retry=0;

	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)//���ָ����SPI��־λ�������:���ܻ���ǿձ�־λ
	{
		retry++;
		if(retry>200)return 0;
	}	  					
	return SPI_I2S_ReceiveData(SPI1); //����ͨ��SPIx������յ�����					    
}


void SPI1_SetSpeed(u8 SPI_BaudRatePrescaler)
{
  	assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));
	SPI1->CR1&=0XFFC7;
	SPI1->CR1|=SPI_BaudRatePrescaler;	//����SPI2�ٶ� 
	SPI_Cmd(SPI1,ENABLE); 

} 

//spi1��ʼ�� 
//spi1��nrf24L01�ĳ�ʼ��
//PA567 SCK MISO MOSI �����������
//PA4 PC4 CE CS �������
//PC5 INT 
/*
   GND       3.3V
PA4 CE   PC4  CS
PA5 SCK  PA7 MOSI
PA6 MISO PC5 INT
*/
void SPI1_NRF24L01_Init(void)
{ 	
	GPIO_InitTypeDef GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;

	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1 | RCC_APB2Periph_AFIO, ENABLE );//PORTCʱ��ʹ�� SPI1ʱ��ʹ�� 	

	
	GPIO_InitStructure.GPIO_Pin =   GPIO_Pin_5  | GPIO_Pin_6 | GPIO_Pin_7;  //PA567 �����������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //PA57����������� 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4 ;   			//PA4 PC4 �������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);		

	//�������룬��������������
		GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5;   			//PC5 ���� 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(GPIOC, &GPIO_InitStructure);		
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4 ;   
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);		
	GPIO_SetBits(GPIOA, GPIO_Pin_4);   	
	

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //SPI����Ϊ˫��˫��ȫ˫��
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//SPI����
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//���ͽ���8λ֡�ṹ
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//ʱ�����յ�
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	//���ݲ����ڵ�1��ʱ����
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS�ź����������
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;		//���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ16
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//���ݴ����MSBλ��ʼ
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRCֵ����Ķ���ʽ
	SPI_Init(SPI1, &SPI_InitStructure);  //����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ���
	SPI_Cmd(SPI1, ENABLE); //ʹ��SPI����
	
	SPI1_ReadWriteByte(0xff);//��������		 

	SPI1_NRF24L01_CE=0; 			//ʹ��24L01
	SPI1_NRF24L01_CSN=1;			//SPIƬѡȡ��  
	 		 	 
}



u8 SPI1_NRF24L01_Check(void)
{
	u8 buf[5]={0XA5,0XA5,0XA5,0XA5,0XA5};
	u8 i;
	//������һ�䣬���spi�ٶȹ��죬 �޷����У�顣��  
//	SPI1_SetSpeed(SPI_BaudRatePrescaler_8); //spi�ٶ�Ϊ9Mhz��24L01�����SPIʱ��Ϊ10Mhz��   	 
	SPI1_NRF24L01_Write_Buf(WRITE_REG_NRF+TX_ADDR,buf,5);//д��5���ֽڵĵ�ַ.	
	SPI1_NRF24L01_Read_Buf(TX_ADDR,buf,5); //����д��ĵ�ַ  
	for(i=0;i<5;i++)if(buf[i]!=0XA5)break;	 							   
	if(i!=5)return 1;//���24L01����	
	return 0;		 //��⵽24L01
}	 	 

u8 SPI1_NRF24L01_Write_Reg(u8 reg,u8 value)
{
	u8 status;	
   	SPI1_NRF24L01_CSN=0;                 //ʹ��SPI����
  	status =SPI1_ReadWriteByte(reg);//���ͼĴ����� 
  	SPI1_ReadWriteByte(value);      //д��Ĵ�����ֵ
  	SPI1_NRF24L01_CSN=1;                 //��ֹSPI����	   
  	return(status);       			//����״ֵ̬
}

u8 SPI1_NRF24L01_Read_Reg(u8 reg)
{
	u8 reg_val;	    
 	SPI1_NRF24L01_CSN = 0;          //ʹ��SPI����		
  	SPI1_ReadWriteByte(reg);   //���ͼĴ�����
  	reg_val=SPI1_ReadWriteByte(0XFF);//��ȡ�Ĵ�������
  	SPI1_NRF24L01_CSN = 1;          //��ֹSPI����		    
  	return(reg_val);           //����״ֵ̬
}	

u8 SPI1_NRF24L01_Read_Buf(u8 reg,u8 *pBuf,u8 len)
{
	u8 status,u8_ctr;	       
  	SPI1_NRF24L01_CSN = 0;           //ʹ��SPI����
  	status=SPI1_ReadWriteByte(reg);//���ͼĴ���ֵ(λ��),����ȡ״ֵ̬   	   
 	for(u8_ctr=0;u8_ctr<len;u8_ctr++)pBuf[u8_ctr]=SPI1_ReadWriteByte(0XFF);//��������
  	SPI1_NRF24L01_CSN=1;       //�ر�SPI����
  	return status;        //���ض�����״ֵ̬
}

u8 SPI1_NRF24L01_Write_Buf(u8 reg, u8 *pBuf, u8 len)
{
	u8 status,u8_ctr;	    
 	SPI1_NRF24L01_CSN = 0;          //ʹ��SPI����
  	status = SPI1_ReadWriteByte(reg);//���ͼĴ���ֵ(λ��),����ȡ״ֵ̬
  	for(u8_ctr=0; u8_ctr<len; u8_ctr++)SPI1_ReadWriteByte(*pBuf++); //д������	 
  	SPI1_NRF24L01_CSN = 1;       //�ر�SPI����
  	return status;          //���ض�����״ֵ̬
}				   

extern u8 NRF_payload_buf[16];
extern float OUT_P,OUT_I,OUT_D;	
extern float IN_P,IN_I,IN_D;   

u8 SPI1_NRF24L01_TxPacket(u8 *txbuf)
{
	u8 sta;
// 	SPI1_SetSpeed(SPI_BaudRatePrescaler_16);//spi�ٶ�Ϊ9Mhz��24L01�����SPIʱ��Ϊ10Mhz��   
	SPI1_NRF24L01_CE=0;
  	SPI1_NRF24L01_Write_Buf(WR_TX_PLOAD,txbuf,TX_PLOAD_WIDTH);//д���ݵ�TX BUF  32���ֽ�
 	SPI1_NRF24L01_CE=1;//��������	   
	while(SPI1_NRF24L01_IRQ!=0);//�ȴ��������
	sta=SPI1_NRF24L01_Read_Reg(STATUS);  //��ȡ״̬�Ĵ�����ֵ	   
	SPI1_NRF24L01_Write_Reg(WRITE_REG_NRF+STATUS,sta); //���TX_DS��MAX_RT�жϱ�־
	
	if(sta&MAX_TX)//�ﵽ����ط�����
	{
		SPI1_NRF24L01_Write_Reg(FLUSH_TX,0xff);//���TX FIFO�Ĵ��� 
		return MAX_TX; 
	}
	if(sta == 46)//0010 1110 �����ж���λ RX FIFO��
	{
		return TX_OK;
	}
	if(sta == 32)//0010 0000 �����ж���λ RX FIFO ͨ��0 
	{
		u8 rx_len = SPI1_NRF24L01_Read_Reg(0x60);
		if(rx_len<33)
		{
			// read receive payload from RX_FIFO buffer
			//0110 0001 ��ȡ���ն˷��͵Ĺ������� �������ݴ�FIFO������ɾ���� ����RXģʽ������
			SPI1_NRF24L01_Read_Buf(RD_RX_PLOAD,NRF_payload_buf,rx_len);// read receive payload from RX_FIFO buffer
			
			if(NRF_payload_buf[0] == 0xaa && NRF_payload_buf[1] == 0xaa)
			{
				OUT_P=NRF_payload_buf[2] 	/ 10.0;
				OUT_I=NRF_payload_buf[3]	/ 10.0;
				OUT_D=NRF_payload_buf[4]	/ 10.0;
				
				IN_P=NRF_payload_buf[5]	/ 10.0;
				IN_I=NRF_payload_buf[6]	/ 10.0;
				IN_D=NRF_payload_buf[7]	/ 10.0;
				
				
			}
			
		}
		else 
		{
			//1110 0010 ɾ�����ն� FIFO����  ����RXģʽ��
			SPI1_NRF24L01_Write_Reg(FLUSH_RX,0xff);//��ջ�����
		}
	}

	
	return 0xff;//����ԭ����ʧ��
}




void SPI1_NRF24l01_Mode(u8 model)
{
	SPI1_NRF24L01_CE = 0;
	SPI1_NRF24L01_Write_Buf(WRITE_REG_NRF+RX_ADDR_P0,(u8*)RX_ADDRESS,RX_ADR_WIDTH);	//дRX�ڵ��ַ 
	SPI1_NRF24L01_Write_Buf(WRITE_REG_NRF+TX_ADDR,(u8*)TX_ADDRESS,TX_ADR_WIDTH); 		//дTX�ڵ��ַ  
	SPI1_NRF24L01_Write_Reg(WRITE_REG_NRF+EN_AA,0x01); 													//EN_AA 			0000 0001 ʹ��ͨ��0���Զ�Ӧ�� 
	SPI1_NRF24L01_Write_Reg(WRITE_REG_NRF+EN_RXADDR,0x01);											//EN_RXADDR 	0000 0001 ʹ��ͨ��0�Ľ��յ�ַ 
	SPI1_NRF24L01_Write_Reg(WRITE_REG_NRF+SETUP_RETR,0x1a);											//SETUP_RETR  0001 1010 �����Զ��ط����ʱ��:500us;����Զ��ط�����:10�� 
	SPI1_NRF24L01_Write_Reg(WRITE_REG_NRF+RF_CH,40);														//����RFͨ��ΪCHANAL
	SPI1_NRF24L01_Write_Reg(WRITE_REG_NRF+RF_SETUP,0x0f); 												//����TX�������,0db����,2Mbps,���������濪��
	//NRF_Write_Reg(NRF_WRITE_REG+RF_SETUP,0x07); 												//����TX�������,0db����,1Mbps,���������濪��
/////////////////////////////////////////////////////////
	if(model==RX_MODE)				//RX
	{
		SPI1_NRF24L01_Write_Reg(WRITE_REG_NRF+RX_PW_P0,RX_PLOAD_WIDTH);								//ѡ��ͨ��0����Ч���ݿ�� 
		SPI1_NRF24L01_Write_Reg(WRITE_REG_NRF + CONFIG, 0x0f);   		 //CONFIG 0000 1111 IRQ�շ�����жϿ���,16λCRC,������
	}
	else if(model==TX_MODE)		//TX
	{
		SPI1_NRF24L01_Write_Reg(WRITE_REG_NRF+RX_PW_P0,RX_PLOAD_WIDTH);								//ѡ��ͨ��0����Ч���ݿ�� 
		SPI1_NRF24L01_Write_Reg(WRITE_REG_NRF + CONFIG, 0x0e);   		 // IRQ�շ�����жϿ���,16λCRC,������
	}
	else if(model==RX2_MODE)		//RX2
	{
		SPI1_NRF24L01_Write_Reg(FLUSH_TX,0xff);
		SPI1_NRF24L01_Write_Reg(FLUSH_RX,0xff);
		SPI1_NRF24L01_Write_Reg(WRITE_REG_NRF + CONFIG, 0x0f);   		 // IRQ�շ�����жϿ���,16λCRC,������
		
		SPI1_ReadWriteByte(0x50);
		SPI1_ReadWriteByte(0x73);
		SPI1_NRF24L01_Write_Reg(WRITE_REG_NRF+0x1c,0x01);
		SPI1_NRF24L01_Write_Reg(WRITE_REG_NRF+0x1d,0x06);
	}
	else								//TX2
	{
		SPI1_NRF24L01_Write_Reg(WRITE_REG_NRF + CONFIG, 0x0e);   		 // IRQ�շ�����жϿ���,16λCRC,������
		SPI1_NRF24L01_Write_Reg(FLUSH_TX,0xff);
		SPI1_NRF24L01_Write_Reg(FLUSH_RX,0xff);
		
		SPI1_ReadWriteByte(0x50);
		SPI1_ReadWriteByte(0x73);
		SPI1_NRF24L01_Write_Reg(WRITE_REG_NRF+0x1c,0x01);
		SPI1_NRF24L01_Write_Reg(WRITE_REG_NRF+0x1d,0x06);
	}
	SPI1_NRF24L01_CE = 1;
}












