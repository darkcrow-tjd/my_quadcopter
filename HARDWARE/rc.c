
#include "RC.h"
#include "sysconfig.h"
#include "sys.h"
#include "includes.h"					//ucos ʹ��	  

u16 RC_CH[6]={0};
extern u8 armed;
extern float IN_P;
//RC_CH[0]: 1224 1984     0 760   
/*		
						 ��Ƭ��ͨ��	  ��Ƭ������		���ջ����		ң����λ��			����			��Χ							�ر�ֵ								�����Χ
	RC_CH[0]		TIM4_CH1			PB6						3				�󲦸� ����		���Ŵ�С		1890~1152				�رպ�Ϊ1018					  40~416
	RC_CH[1]		TIM4_CH2			PB7						4				�󲦸� ����							1060~1520~1890		1520								-10~10
	RC_CH[2]		TIM4_CH3			PB8						5				���� ����								��2030 ��960			1520									0~1
	RC_CH[3]		TIM4_CH4			PB9						6				���� ��ť			Pֵ��С		1008~2024					1520								

	RC_CH[4]		TIM5_CH1			PA0						1				�Ҳ��� ����		���			1000~1416~1760												-10~10
	RC_CH[5]		TIM5_CH2			PA1						2				�Ҳ��� ����		����			1200~1632~2000												-10~10
*/
u8 armed_flag=0;

void RC_anl(S_INT16_RC *rc_get)
{
	if(RC_CH[0] < 1152)
	{
		armed=0;
		rc_get->THROTTLE = 0;
		rc_get->YAW			 = 0;
		rc_get->ROLL		 = 0;
		rc_get->PITCH		 = 0;
	}
	else
	{

		rc_get->THROTTLE = (1984 - RC_CH[0]) / 2.0 ; //
		rc_get->YAW			 =   RC_CH[1];	
		
		rc_get->ROLL		 =  (RC_CH[4]-1416)/30;
		if(rc_get->ROLL > 10 )
			rc_get->ROLL= 10;
		if(rc_get->ROLL < -10 )
			rc_get->ROLL= -10;		

		rc_get->PITCH		 =   (RC_CH[5]-1632)/30;
		if(rc_get->PITCH > 10 )
			rc_get->PITCH= 10;
		if(rc_get->PITCH < -10 )
			rc_get->PITCH= -10;			
		
		
		rc_get->AUX1		 =  RC_CH[2];  //���Ͽ���
		rc_get->AUX2		 =  armed*500;			
		
		rc_get->AUX3		 =  RC_CH[3];				//������ť
		IN_P = ((float)RC_CH[3] - 1000) / 100.0;
		
//		rc_get->AUX4		 =  armed*500;	
//		rc_get->AUX5		 =  armed*500;	
//		rc_get->AUX6		 =  armed*500;			


		
		if(armed_flag==1 && RC_CH[2] > 1500)
		{
				armed=1;
			armed_flag=0;
		}
		
		if(RC_CH[2] < 1500)
		{
			armed=0;
		}
		
		if(armed == 0 && RC_CH[2] < 1500)
		{
			armed_flag=1;
		}	
				
	
	
	}

	
	
	
}

void TIM_RC_Init(void)
{
	GPIO_InitTypeDef 					GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef   TIM_TimeBaseStructure;
	TIM_ICInitTypeDef 			  TIM5_ICInitStructure;	
	TIM_ICInitTypeDef 			  TIM4_ICInitStructure;
	NVIC_InitTypeDef 					NVIC_InitStructure;


	//����NVIC��TIM4�ж�
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;//ͨ������ΪTIM4
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;//��ռ0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;//��Ӧ3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//���ж�ͨ��
	NVIC_Init(&NVIC_InitStructure);//д������
	
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;//ͨ������ΪTIM4
	NVIC_Init(&NVIC_InitStructure);//д������
	
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4 | RCC_APB1Periph_TIM5, ENABLE);//ʹ��TIM4ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA,ENABLE);//GPIOB ʱ�ӿ���


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;
	//GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//��������
	GPIO_Init(GPIOB, &GPIO_InitStructure);//GPIOB 6 7 8 9 ������
	GPIO_ResetBits(GPIOB,GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9);
	
		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
	//GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA,GPIO_Pin_0 | GPIO_Pin_1);
	

	TIM_TimeBaseStructure.TIM_Period = 20000-1;
  TIM_TimeBaseStructure.TIM_Prescaler = (72 - 1);//ʱ��Ԥ��Ƶ�� 72M/72(1M/20000) = 50HZ
 	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;//����ʱ�ӷ�Ƶϵ��������Ƶ
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//���ϼ���ģʽ(0->?)
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);//����TIM4
	
	//��ʼ��TIM4���벶�����
	TIM4_ICInitStructure.TIM_Channel = TIM_Channel_1; //CC1S=01 	ѡ������� IC1ӳ�䵽TI1��
  TIM4_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//�����ز���
  TIM4_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //ӳ�䵽TI1��
  TIM4_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //���������Ƶ,����Ƶ 
  TIM4_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 ���������˲��� ���˲�
  TIM_ICInit(TIM4, &TIM4_ICInitStructure);
	
	TIM4_ICInitStructure.TIM_Channel = TIM_Channel_2; //CC1S=01 	ѡ������� IC1ӳ�䵽TI1��
  TIM4_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//�����ز���
  TIM4_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //ӳ�䵽TI1��
  TIM4_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //���������Ƶ,����Ƶ 
  TIM4_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 ���������˲��� ���˲�
  TIM_ICInit(TIM4, &TIM4_ICInitStructure);
	
	TIM4_ICInitStructure.TIM_Channel = TIM_Channel_3; //CC1S=01 	ѡ������� IC1ӳ�䵽TI1��
  TIM4_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//�����ز���
  TIM4_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //ӳ�䵽TI1��
  TIM4_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //���������Ƶ,����Ƶ 
  TIM4_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 ���������˲��� ���˲�
  TIM_ICInit(TIM4, &TIM4_ICInitStructure);
	
	TIM4_ICInitStructure.TIM_Channel = TIM_Channel_4; //CC1S=01 	ѡ������� IC1ӳ�䵽TI1��
  TIM4_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//�����ز���
  TIM4_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //ӳ�䵽TI1��
  TIM4_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //���������Ƶ,����Ƶ 
  TIM4_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 ���������˲��� ���˲�
  TIM_ICInit(TIM4, &TIM4_ICInitStructure);
	
//////////////////////////////
	TIM_TimeBaseStructure.TIM_Period = 20000-1;
  TIM_TimeBaseStructure.TIM_Prescaler = (72 - 1);//ʱ��Ԥ��Ƶ�� 72M/72(1M/20000) = 50HZ
 	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;//����ʱ�ӷ�Ƶϵ��������Ƶ
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//���ϼ���ģʽ(0->?)
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);//����TIM
	
	//��ʼ��TIM4���벶�����
	TIM5_ICInitStructure.TIM_Channel = TIM_Channel_1; //CC1S=01 	ѡ������� IC1ӳ�䵽TI1��
  TIM5_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//�����ز���
  TIM5_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //ӳ�䵽TI1��
  TIM5_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //���������Ƶ,����Ƶ 
  TIM5_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 ���������˲��� ���˲�
  TIM_ICInit(TIM5, &TIM5_ICInitStructure);
	
	TIM5_ICInitStructure.TIM_Channel = TIM_Channel_2; //CC1S=01 	ѡ������� IC1ӳ�䵽TI1��
  TIM5_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//�����ز���
  TIM5_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //ӳ�䵽TI1��
  TIM5_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //���������Ƶ,����Ƶ 
  TIM5_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 ���������˲��� ���˲�
  TIM_ICInit(TIM5, &TIM5_ICInitStructure);	
	
	
	
	TIM_ITConfig(TIM4,TIM_IT_CC1,ENABLE);//����CC1IE�����ж�	 
	TIM_ITConfig(TIM4,TIM_IT_CC2,ENABLE);//����CC2IE�����ж�	
	TIM_ITConfig(TIM4,TIM_IT_CC3,ENABLE);//����CC3IE�����ж�	
	TIM_ITConfig(TIM4,TIM_IT_CC4,ENABLE);//����CC4IE�����ж�
	
	TIM_ITConfig(TIM5,TIM_IT_CC1,ENABLE);//����CC1IE�����ж�	 
	TIM_ITConfig(TIM5,TIM_IT_CC2,ENABLE);//����CC2IE�����ж�		
	
	
	TIM_Cmd(TIM4,ENABLE );//ʹ�ܶ�ʱ��4
	
	TIM_Cmd(TIM5,ENABLE );//ʹ�ܶ�ʱ��5
}

u8  TIM5CH1_CAPTURE_STA=0;	//ͨ��1���벶��״̬		  �ø���λ�������־������λ���������  				
u16	TIM5CH1_CAPTURE_UPVAL;	//ͨ��1���벶��ֵ
u16	TIM5CH1_CAPTURE_DOWNVAL;//ͨ��1���벶��ֵ
       
u8  TIM5CH2_CAPTURE_STA=0;	//ͨ��2���벶��״̬		    				
u16	TIM5CH2_CAPTURE_UPVAL;	//ͨ��2���벶��ֵ
u16	TIM5CH2_CAPTURE_DOWNVAL;//ͨ��2���벶��ֵ


void TIM5_IRQHandler(void)
{
	////////////////////////////////////
	OSIntEnter();		//�����ж�
	
	if(TIM_GetITStatus(TIM5, TIM_IT_CC1) == SET)//����1���������¼�
	{
		TIM_ClearITPendingBit(TIM5, TIM_IT_CC1);//�������1��־λ
		
		if(TIM5CH1_CAPTURE_STA == 0)//����������
		{
			TIM5CH1_CAPTURE_UPVAL = TIM_GetCapture1(TIM5);//��ȡ�����ص�����
			
			TIM5CH1_CAPTURE_STA = 1;		//����Բ�����������
			TIM_OC1PolarityConfig(TIM5,TIM_ICPolarity_Falling);//����Ϊ�½��ز���
		}
		else                        //�����½��� (�Ѿ�����һ�������ĸߵ�ƽ���壡)
		{
			TIM5CH1_CAPTURE_DOWNVAL = TIM_GetCapture1(TIM5);//��ȡ�½��ص�����
			
			//�ж��Ƿ񳬳����������,����ߵ�ƽ����ʱ��us
			if(TIM5CH1_CAPTURE_DOWNVAL<TIM5CH1_CAPTURE_UPVAL)
			{
				RC_CH[4] = 20000 - TIM5CH1_CAPTURE_UPVAL + TIM5CH1_CAPTURE_DOWNVAL;
			}
			else
			{
				RC_CH[4] = TIM5CH1_CAPTURE_DOWNVAL- TIM5CH1_CAPTURE_UPVAL;
			}
			
			TIM5CH1_CAPTURE_STA = 0;
			TIM_OC1PolarityConfig(TIM5,TIM_ICPolarity_Rising); //����Ϊ�����ز���
		}
	}	
	
	
	///////////////////////////////////////////////////////
	if(TIM_GetITStatus(TIM5, TIM_IT_CC2) == SET)//����2���������¼�
	{
		TIM_ClearITPendingBit(TIM5, TIM_IT_CC2);//�������2��־λ
		
		if(TIM5CH2_CAPTURE_STA == 0)//����������
		{
			TIM5CH2_CAPTURE_UPVAL = TIM_GetCapture2(TIM5);//��ȡ�����ص�����
			
			TIM5CH2_CAPTURE_STA = 1;		//����Բ�����������
			TIM_OC2PolarityConfig(TIM5,TIM_ICPolarity_Falling);//����Ϊ�½��ز���
		}
		else                        //�����½��� (�Ѿ�����һ�������ĸߵ�ƽ���壡)
		{
			TIM5CH2_CAPTURE_DOWNVAL = TIM_GetCapture2(TIM5);//��ȡ�½��ص�����
			
			//�ж��Ƿ񳬳����������,����ߵ�ƽ����ʱ��us
			if(TIM5CH2_CAPTURE_DOWNVAL<TIM5CH2_CAPTURE_UPVAL)
			{
				RC_CH[5] = 20000 - TIM5CH2_CAPTURE_UPVAL + TIM5CH2_CAPTURE_DOWNVAL;
			}
			else
			{
				RC_CH[5] = TIM5CH2_CAPTURE_DOWNVAL- TIM5CH2_CAPTURE_UPVAL;
			}
			TIM5CH2_CAPTURE_STA = 0;
			TIM_OC2PolarityConfig(TIM5,TIM_ICPolarity_Rising); //����Ϊ�����ز���
		}
	}
	
	   
	OSIntExit();        //���������л����ж�
}


//��ʱ��4ͨ�����벶������
u8  TIM4CH1_CAPTURE_STA=0;	//ͨ��1���벶��״̬		  �ø���λ�������־������λ���������  				
u16	TIM4CH1_CAPTURE_UPVAL;	//ͨ��1���벶��ֵ
u16	TIM4CH1_CAPTURE_DOWNVAL;//ͨ��1���벶��ֵ

u8  TIM4CH2_CAPTURE_STA=0;	//ͨ��2���벶��״̬		    				
u16	TIM4CH2_CAPTURE_UPVAL;	//ͨ��2���벶��ֵ
u16	TIM4CH2_CAPTURE_DOWNVAL;//ͨ��2���벶��ֵ

u8  TIM4CH3_CAPTURE_STA=0;	//ͨ��3���벶��״̬		    				
u16	TIM4CH3_CAPTURE_UPVAL;	//ͨ��3���벶��ֵ
u16	TIM4CH3_CAPTURE_DOWNVAL;//ͨ��3���벶��ֵ

u8  TIM4CH4_CAPTURE_STA=0;	//ͨ��4���벶��״̬		    				
u16	TIM4CH4_CAPTURE_UPVAL;	//ͨ��4���벶��ֵ
u16	TIM4CH4_CAPTURE_DOWNVAL;//ͨ��4���벶��ֵ

void TIM4_IRQHandler(void)
{
/////////
	 	 OSIntEnter();		//�����ж�
	
	
	if(TIM_GetITStatus(TIM4, TIM_IT_CC1) == SET)//����1���������¼�
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);//�������1��־λ
		
		if(TIM4CH1_CAPTURE_STA == 0)//����������
		{
			TIM4CH1_CAPTURE_UPVAL = TIM_GetCapture1(TIM4);//��ȡ�����ص�����
			
			TIM4CH1_CAPTURE_STA = 1;		//����Բ�����������
			TIM_OC1PolarityConfig(TIM4,TIM_ICPolarity_Falling);//����Ϊ�½��ز���
		}
		else                        //�����½��� (�Ѿ�����һ�������ĸߵ�ƽ���壡)
		{
			TIM4CH1_CAPTURE_DOWNVAL = TIM_GetCapture1(TIM4);//��ȡ�½��ص�����
			
			//�ж��Ƿ񳬳����������,����ߵ�ƽ����ʱ��us
			if(TIM4CH1_CAPTURE_DOWNVAL<TIM4CH1_CAPTURE_UPVAL)
			{
				RC_CH[0] = 20000 - TIM4CH1_CAPTURE_UPVAL + TIM4CH1_CAPTURE_DOWNVAL;
			}
			else
			{
				RC_CH[0] = TIM4CH1_CAPTURE_DOWNVAL- TIM4CH1_CAPTURE_UPVAL;
			}
			
			TIM4CH1_CAPTURE_STA = 0;
			TIM_OC1PolarityConfig(TIM4,TIM_ICPolarity_Rising); //����Ϊ�����ز���
		}
	}

/////////
	if(TIM_GetITStatus(TIM4, TIM_IT_CC2) == SET)//����2���������¼�
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_CC2);//�������2��־λ
		
		if(TIM4CH2_CAPTURE_STA == 0)//����������
		{
			TIM4CH2_CAPTURE_UPVAL = TIM_GetCapture2(TIM4);//��ȡ�����ص�����
			
			TIM4CH2_CAPTURE_STA = 1;		//����Բ�����������
			TIM_OC2PolarityConfig(TIM4,TIM_ICPolarity_Falling);//����Ϊ�½��ز���
		}
		else                        //�����½��� (�Ѿ�����һ�������ĸߵ�ƽ���壡)
		{
			TIM4CH2_CAPTURE_DOWNVAL = TIM_GetCapture2(TIM4);//��ȡ�½��ص�����
			
			//�ж��Ƿ񳬳����������,����ߵ�ƽ����ʱ��us
			if(TIM4CH2_CAPTURE_DOWNVAL<TIM4CH2_CAPTURE_UPVAL)
			{
				RC_CH[1] = 20000 - TIM4CH2_CAPTURE_UPVAL + TIM4CH2_CAPTURE_DOWNVAL;
			}
			else
			{
				RC_CH[1] = TIM4CH2_CAPTURE_DOWNVAL- TIM4CH2_CAPTURE_UPVAL;
			}
			TIM4CH2_CAPTURE_STA = 0;
			TIM_OC2PolarityConfig(TIM4,TIM_ICPolarity_Rising); //����Ϊ�����ز���
		}
	}

	
///////////
	if(TIM_GetITStatus(TIM4, TIM_IT_CC3) == SET)//����3���������¼�
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_CC3);//�������3��־λ
		
		if(TIM4CH3_CAPTURE_STA == 0)//����������
		{
			TIM4CH3_CAPTURE_UPVAL = TIM_GetCapture3(TIM4);//��ȡ�����ص�����
			
			TIM4CH3_CAPTURE_STA = 1;		//����Բ�����������
			TIM_OC3PolarityConfig(TIM4,TIM_ICPolarity_Falling);//����Ϊ�½��ز���
		}
		else                        //�����½��� (�Ѿ�����һ�������ĸߵ�ƽ���壡)
		{
			TIM4CH3_CAPTURE_DOWNVAL = TIM_GetCapture3(TIM4);//��ȡ�½��ص�����
			
			//�ж��Ƿ񳬳����������,����ߵ�ƽ����ʱ��us
			if(TIM4CH3_CAPTURE_DOWNVAL<TIM4CH3_CAPTURE_UPVAL)
			{
				RC_CH[2] = 20000 - TIM4CH3_CAPTURE_UPVAL + TIM4CH3_CAPTURE_DOWNVAL;
			}
			else
			{
				RC_CH[2] = TIM4CH3_CAPTURE_DOWNVAL- TIM4CH3_CAPTURE_UPVAL;
			}
			
			TIM4CH3_CAPTURE_STA = 0;
			TIM_OC3PolarityConfig(TIM4,TIM_ICPolarity_Rising); //����Ϊ�����ز���
			
		}
	}
	
///////////////
	if(TIM_GetITStatus(TIM4, TIM_IT_CC4) == SET)//����4���������¼�
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_CC4);//�������4��־λ
		
		if(TIM4CH4_CAPTURE_STA == 0)//����������
		{
			TIM4CH4_CAPTURE_UPVAL = TIM_GetCapture4(TIM4);//��ȡ�����ص�����
			
			TIM4CH4_CAPTURE_STA = 1;		//����Բ�����������
			TIM_OC4PolarityConfig(TIM4,TIM_ICPolarity_Falling);//����Ϊ�½��ز���
		}
		else                        //�����½��� (�Ѿ�����һ�������ĸߵ�ƽ���壡)
		{
			TIM4CH4_CAPTURE_DOWNVAL = TIM_GetCapture4(TIM4);//��ȡ�½��ص�����
			
			//�ж��Ƿ񳬳����������,����ߵ�ƽ����ʱ��us
			if(TIM4CH4_CAPTURE_DOWNVAL<TIM4CH4_CAPTURE_UPVAL)
			{
				RC_CH[3] = 20000 - TIM4CH4_CAPTURE_UPVAL + TIM4CH4_CAPTURE_DOWNVAL;
			}
			else
			{
				RC_CH[3] = TIM4CH4_CAPTURE_DOWNVAL- TIM4CH4_CAPTURE_UPVAL;
			}
			
			TIM4CH4_CAPTURE_STA = 0;
			TIM_OC4PolarityConfig(TIM4,TIM_ICPolarity_Rising); //����Ϊ�����ز���
			
		}
	}
	

	  OSIntExit();        //���������л����ж�

}

