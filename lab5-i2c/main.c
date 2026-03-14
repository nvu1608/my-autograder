#include "stm32f10x.h"

#define I2C_Port        I2C1
#define I2C_GPIO        GPIOB
#define I2C_SCL_Pin     GPIO_Pin_6
#define I2C_SDA_Pin     GPIO_Pin_7

#define LCD_ADDR        0x27

#define LCD_BACKLIGHT   0x08
#define ENABLE_BIT      0x04
#define RS_CMD          0x00
#define RS_DATA         0x01

#define LCD_FUNCTIONSET 0x20
#define LCD_DISPLAYCTRL 0x08
#define LCD_CLEARDISP   0x01
#define LCD_SETDDRAM    0x80

#define LCD_DISPLAYON   0x04
#define LCD_4BITMODE    0x00
#define LCD_2LINE       0x08

static uint8_t blv = LCD_BACKLIGHT;

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
    while(ms--){
        delay_us(1000);
    }
}

void I2C_Config(void){
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef gp;
    I2C_InitTypeDef i2c;

    gp.GPIO_Pin = I2C_SCL_Pin | I2C_SDA_Pin;
    gp.GPIO_Mode = GPIO_Mode_AF_OD;
    gp.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(I2C_GPIO, &gp);

    I2C_DeInit(I2C_Port);
    i2c.I2C_ClockSpeed = 100000;
    i2c.I2C_Mode = I2C_Mode_I2C;
    i2c.I2C_DutyCycle = I2C_DutyCycle_2;
    i2c.I2C_OwnAddress1 = 0x00;
    i2c.I2C_Ack = I2C_Ack_Enable;
    i2c.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C_Port, &i2c);

    I2C_Cmd(I2C_Port, ENABLE);
}

void I2C_Start(uint8_t address, uint8_t direction){
    I2C_GenerateSTART(I2C_Port, ENABLE);
    while(!I2C_CheckEvent(I2C_Port, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(I2C_Port, address, direction);
    
    if(direction == I2C_Direction_Transmitter)
        while(!I2C_CheckEvent(I2C_Port, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    else
        while(!I2C_CheckEvent(I2C_Port, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
}

void I2C_Stop(void){
    I2C_GenerateSTOP(I2C_Port, ENABLE);
}

void I2C_Write(uint8_t data){
    I2C_SendData(I2C_Port, data);
    while(!I2C_CheckEvent(I2C_Port, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}

static void EX_Write(uint8_t data){
    I2C_Start(LCD_ADDR << 1, I2C_Direction_Transmitter);
    I2C_Write(data | blv);
    I2C_Stop();
}

static void PL_EN(uint8_t data){
    EX_Write(data | ENABLE_BIT);
    delay_ms(1);
    EX_Write(data & ~ENABLE_BIT);
    delay_ms(1);
}

static void WRITE_4BIT(uint8_t val){
    EX_Write(val);
    PL_EN(val);
}

static void Send(uint8_t val, uint8_t mode){
    uint8_t upper = val & 0xF0;
    uint8_t lower = (val << 4) & 0xF0;
    WRITE_4BIT(upper | mode);
    WRITE_4BIT(lower | mode);
}

void LCD_SendCmd(uint8_t cmd){
    Send(cmd, RS_CMD);
}

void LCD_SendData(uint8_t data){
    Send(data, RS_DATA);
}

void LCD_Init(void){
    delay_ms(50);
    WRITE_4BIT(0x30); delay_ms(5);
    WRITE_4BIT(0x30); delay_ms(5);
    WRITE_4BIT(0x30); delay_ms(5);
    WRITE_4BIT(0x20); delay_ms(5);
    
    LCD_SendCmd(LCD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINE);
    LCD_SendCmd(LCD_DISPLAYCTRL | LCD_DISPLAYON);
    LCD_SendCmd(LCD_CLEARDISP);
    delay_ms(2);
}

void LCD_SetCursor(uint8_t col, uint8_t row){
    static const uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};
    if(row > 3) row = 3;
    LCD_SendCmd(LCD_SETDDRAM | (col + row_offsets[row]));
}

void LCD_SendString(const char *str){
    while(*str){
        LCD_SendData(*str++);
    }
}

int main(void){
    TIM2_Init();
    I2C_Config();
    LCD_Init();

    LCD_SetCursor(0, 0);
    LCD_SendString("HELLO STM32");

    LCD_SetCursor(0, 1);
    LCD_SendString("Cre: NN Chien");

    while(1);
}