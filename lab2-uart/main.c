#include "stm32f10x.h"                  // Device header


void delay_ms(uint32_t cnt);
void usart1_init(uint32_t baud_rate);
void usart1_send_char(char c);
void usart1_send_string(const char *str);

int main()
{
	usart1_init(9600);
	while (1)
	{
		usart1_send_string("Hello Raspberry Pi\n");
		delay_ms(1000);
	}
}

void delay_ms(uint32_t cnt)
{
	volatile uint32_t i, j;
	for (i = 0; i < cnt; i++)
	{
		for (j = 0; j < 0x1995; j++);
	}
}

void usart1_init(uint32_t baud_rate)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	USART_InitTypeDef USART_InitStruct = {0};
	USART_InitStruct.USART_BaudRate = baud_rate;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_Cmd(USART1, ENABLE);
	USART_Init(USART1, &USART_InitStruct);
}
void usart1_send_char(char c)
{
	while (!(USART1->SR & (1 << 7)));
	USART1->DR = (uint16_t)(c & 0xff);
}
void usart1_send_string(const char *str)
{
	while (*str)
	{
		usart1_send_char(*str++);
	}
}