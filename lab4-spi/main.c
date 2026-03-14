#include "stm32f10x.h"

// ==========================================
// 1. �?NH NGHIA CH�N ST7735 (SOFTWARE SPI)
// ==========================================
#define TFT_PORT GPIOA
#define TFT_A0   GPIO_Pin_2  // Ch�n Data/Command (DC)
#define TFT_RST  GPIO_Pin_3  // Ch�n RESET
#define TFT_CS   GPIO_Pin_4  // Ch�n Chip Select
#define TFT_SCK  GPIO_Pin_5  // Ch�n Clock (SCL)
#define TFT_SDA  GPIO_Pin_7  // Ch�n Data (MOSI)

// C�c macro di?u khi?n ch�n
#define CS_L()   GPIO_ResetBits(TFT_PORT, TFT_CS)
#define CS_H()   GPIO_SetBits(TFT_PORT, TFT_CS)
#define A0_CMD() GPIO_ResetBits(TFT_PORT, TFT_A0)
#define A0_DAT() GPIO_SetBits(TFT_PORT, TFT_A0)
#define RST_L()  GPIO_ResetBits(TFT_PORT, TFT_RST)
#define RST_H()  GPIO_SetBits(TFT_PORT, TFT_RST)

#define SCK_L()  GPIO_ResetBits(TFT_PORT, TFT_SCK)
#define SCK_H()  GPIO_SetBits(TFT_PORT, TFT_SCK)
#define SDA_L()  GPIO_ResetBits(TFT_PORT, TFT_SDA)
#define SDA_H()  GPIO_SetBits(TFT_PORT, TFT_SDA)

// M� m�u co b?n (RGB565)
#define RGB_BLACK   0x0000
#define RGB_BLUE    0x001F
#define RGB_RED     0xF800
#define RGB_GREEN   0x07E0
#define RGB_CYAN    0x07FF
#define RGB_BROWN   0xA145
#define RGB_MAGENTA 0xF81F // Tim hong
#define RGB_PINK    0xFC90
#define RGB_YELLOW  0xFFE0
#define RGB_WHITE   0xFFFF
#define RGB_ORANGE  0xF940

// ==========================================
// 2. H�M DELAY (D�NG TIMER 2)
// ==========================================
void TIM2_Init(void){
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_TimeBaseInitTypeDef TM;
    TM.TIM_ClockDivision = TIM_CKD_DIV1;
    TM.TIM_CounterMode = TIM_CounterMode_Up;
    TM.TIM_Period = 0xFFFF;
    TM.TIM_Prescaler = 71;
    TIM_TimeBaseInit(TIM2, &TM);
    TIM_Cmd(TIM2, ENABLE);
}

void delay_us(uint16_t us){
    TIM_SetCounter(TIM2, 0);
    while(TIM_GetCounter(TIM2) < us);
}

void delay_ms(volatile uint16_t ms){
    while(ms--){ delay_us(1000); }
}

// ==========================================
// 3. KH?I T?O GPIO CHO M�N H�NH
// ==========================================
void ST7735_GPIO_Init(void){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // C?u h�nh t?t c? 5 ch�n th�nh Output Push-Pull
    GPIO_InitStructure.GPIO_Pin = TFT_CS | TFT_A0 | TFT_RST | TFT_SCK | TFT_SDA;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(TFT_PORT, &GPIO_InitStructure);
    
    // �?t m?c m?c d?nh ban d?u
    CS_H();
    SCK_H();
}

// ==========================================
// 4. C�C H�M GIAO TI?P G?I D? LI?U
// ==========================================
void SPI_WriteByte(uint8_t data){
    for(uint8_t i = 0; i < 8; i++){
        SCK_L(); // K�o Clock xu?ng
        
        if(data & 0x80) { SDA_H(); } 
        else            { SDA_L(); }
        
        SCK_H(); // K�o Clock l�n ch?t bit
        data <<= 1; 
    }
}

void ST7735_WriteCmd(uint8_t cmd){
    CS_L();      
    A0_CMD();    // B�o l� L?nh (Command)
    SPI_WriteByte(cmd);
    CS_H();      
}

void ST7735_WriteData(uint8_t data){
    CS_L();
    A0_DAT();    // B�o l� D? li?u (Data)
    SPI_WriteByte(data);
    CS_H();
}

void ST7735_Init(void){
    ST7735_GPIO_Init();

    // Reset c?ng m�n h�nh
    RST_L();
    delay_ms(50);
    RST_H();
    delay_ms(50);

    ST7735_WriteCmd(0x11); // Tho�t Sleep Mode
    delay_ms(120);

    ST7735_WriteCmd(0x3A); // C?u h�nh m�u
    ST7735_WriteData(0x05); // 16-bit color (RGB565)

    ST7735_WriteCmd(0x36); // Xoay m�n h�nh ngang
    ST7735_WriteData(0xA0); 

    ST7735_WriteCmd(0x29); // B?t hi?n th?
    delay_ms(50);
}

void ST7735_SetWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1){
    ST7735_WriteCmd(0x2A); 
    ST7735_WriteData(0x00); ST7735_WriteData(x0);
    ST7735_WriteData(0x00); ST7735_WriteData(x1);

    ST7735_WriteCmd(0x2B); 
    ST7735_WriteData(0x00); ST7735_WriteData(y0);
    ST7735_WriteData(0x00); ST7735_WriteData(y1);

    ST7735_WriteCmd(0x2C); 
}

void ST7735_FillScreen(uint16_t color){
    uint8_t hi = color >> 8, lo = color & 0xFF;
    ST7735_SetWindow(0, 0, 159, 127); // K�ch thu?c m�n 160x128
    
    CS_L();
    A0_DAT();
    for(uint32_t i = 0; i < 160 * 128; i++){
        SPI_WriteByte(hi);
        SPI_WriteByte(lo);
    }
    CS_H();
}

// ==========================================
// 5. CHUONG TR�NH CH�NH
// ==========================================
int main(void){
    TIM2_Init();
    ST7735_Init();
    
    while(1){
        ST7735_FillScreen(RGB_BLACK);
				delay_ms(1000);
			ST7735_FillScreen(RGB_BLUE);
				delay_ms(1000);
			ST7735_FillScreen(RGB_RED);
				delay_ms(1000);
			ST7735_FillScreen(RGB_GREEN);
				delay_ms(1000);
			ST7735_FillScreen(RGB_CYAN);
				delay_ms(1000);
			ST7735_FillScreen(RGB_BROWN);
				delay_ms(1000);
			ST7735_FillScreen(RGB_MAGENTA);
				delay_ms(1000);
			ST7735_FillScreen(RGB_PINK);
				delay_ms(1000);
			ST7735_FillScreen(RGB_YELLOW);
				delay_ms(1000);
			ST7735_FillScreen(RGB_WHITE);
				delay_ms(1000);
			ST7735_FillScreen(RGB_ORANGE);
				delay_ms(1000);
    }
}