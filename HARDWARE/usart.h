#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 

#include "sysconfig.h"

#define USART_REC_LEN  			200  	//�����������ֽ��� 200

	  	
extern u8  USART_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u16 USART_RX_STA;         		//����״̬���	
//����봮���жϽ��գ��벻Ҫע�����º궨��
extern void uart_init(u32 bound);

extern void usart1_send_char(u8);



#endif


