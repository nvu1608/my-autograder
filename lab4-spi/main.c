#include "stm32f10x.h"

void delay_ms(uint32_t cnt);
void usart1_init(uint32_t baud_rate);
void usart1_send_char(char c);
void usart1_send_string(const char *str);
void spi1_init(void);
void spi1_send_byte(uint8_t data);
void spi1_send_string(const uint8_t *buf, uint16_t len);

/* SPI1 pins:
   PA4 = NSS  (CS)
   PA5 = SCK
   PA6 = MISO
   PA7 = MOSI
*/

int main(void)
{
    usart1_init(9600);
    spi1_init();

    const uint8_t msg[] = "Hello ESP32-S3";
    uint16_t len = sizeof(msg) - 1;

    while (1)
    {
        // Kéo CS xu?ng LOW ? b?t d?u truy?n
        GPIO_ResetBits(GPIOA, GPIO_Pin_4);

        spi1_send_string(msg, len);

        // Kéo CS lên HIGH ? k?t thúc truy?n
        GPIO_SetBits(GPIOA, GPIO_Pin_4);

        usart1_send_string("SPI sent: Hello ESP32-S3\n");
        delay_ms(1000);
    }
}

void spi1_init(void)
{
    // 1. C?p clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 | RCC_APB2Periph_GPIOA, ENABLE);

    // 2. C?u hình GPIO
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // PA5=SCK, PA7=MOSI ? AF Push-Pull
    GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_5 | GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // PA6=MISO ? Input Floating
    GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_6;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // PA4=NSS (CS) ? Output Push-Pull, di?u khi?n tay
    GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_4;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // M?c d?nh CS = HIGH (không ch?n slave)
    GPIO_SetBits(GPIOA, GPIO_Pin_4);

    // 3. C?u hình SPI1
    SPI_InitTypeDef SPI_InitStruct = {0};
    SPI_InitStruct.SPI_Mode              = SPI_Mode_Master;
    SPI_InitStruct.SPI_Direction         = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStruct.SPI_DataSize          = SPI_DataSize_8b;
    SPI_InitStruct.SPI_CPOL              = SPI_CPOL_Low;   // ESP32 default
    SPI_InitStruct.SPI_CPHA              = SPI_CPHA_1Edge; // ESP32 default
    SPI_InitStruct.SPI_NSS               = SPI_NSS_Soft;   // CS di?u khi?n tay
    SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4; // 72/4 = 18MHz
    SPI_InitStruct.SPI_FirstBit          = SPI_FirstBit_MSB;
    SPI_InitStruct.SPI_CRCPolynomial     = 7;
    SPI_Init(SPI1, &SPI_InitStruct);

    SPI_Cmd(SPI1, ENABLE);
}

void spi1_send_byte(uint8_t data)
{
    // Ch? TX buffer r?ng
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI1, data);
    // Ch? truy?n xong (BSY = 0)
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
}

void spi1_send_string(const uint8_t *buf, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
    {
        spi1_send_byte(buf[i]);
    }
}

void delay_ms(uint32_t cnt)
{
    volatile uint32_t i, j;
for (i = 0; i < cnt; i++)
        for (j = 0; j < 0x1995; j++);
}

void usart1_init(uint32_t baud_rate)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    USART_InitTypeDef USART_InitStruct = {0};
    USART_InitStruct.USART_BaudRate            = baud_rate;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStruct.USART_Parity              = USART_Parity_No;
    USART_InitStruct.USART_StopBits            = USART_StopBits_1;
    USART_InitStruct.USART_WordLength          = USART_WordLength_8b;
    USART_Init(USART1, &USART_InitStruct);
    USART_Cmd(USART1, ENABLE); 
}

void usart1_send_char(char c)
{
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART1->DR = (uint8_t)c;
}

void usart1_send_string(const char *str)
{
    while (*str) usart1_send_char(*str++);
}