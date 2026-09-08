#include "oled.h"
#include "oledfont.h"
#include "systick.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

#define OLED_ADDR        0x78  /* OLED I2C 写入地址 */
#define OLED_CMD         0x00  /* 写命令标志 */
#define OLED_DATA        0x40  /* 写数据标志 */

#define OLED_SCL_HIGH()  GPIO_SetBits(GPIOB, GPIO_Pin_8)
#define OLED_SCL_LOW()   GPIO_ResetBits(GPIOB, GPIO_Pin_8)
#define OLED_SDA_HIGH()  GPIO_SetBits(GPIOB, GPIO_Pin_9)
#define OLED_SDA_LOW()   GPIO_ResetBits(GPIOB, GPIO_Pin_9)

static void I2C_Delay(void)
{
    for (volatile int i = 0; i < 25; i++);
}

static void I2C_Start(void)
{
    OLED_SDA_HIGH();
    OLED_SCL_HIGH();
    I2C_Delay();
    OLED_SDA_LOW();
    I2C_Delay();
    OLED_SCL_LOW();
    I2C_Delay();
}

static void I2C_Stop(void)
{
    OLED_SDA_LOW();
    OLED_SCL_HIGH();
    I2C_Delay();
    OLED_SDA_HIGH();
    I2C_Delay();
}

static void I2C_WriteByte(uint8_t byte)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        if (byte & 0x80)
            OLED_SDA_HIGH();
        else
            OLED_SDA_LOW();
        I2C_Delay();
        OLED_SCL_HIGH();
        I2C_Delay();
        OLED_SCL_LOW();
        I2C_Delay();
        byte <<= 1;
    }
    /* ACK 应答时钟脉冲 */
    OLED_SDA_HIGH();
    I2C_Delay();
    OLED_SCL_HIGH();
    I2C_Delay();
    OLED_SCL_LOW();
    I2C_Delay();
}

static void OLED_WriteByte(uint8_t dat, uint8_t cmd)
{
    I2C_Start();
    I2C_WriteByte(OLED_ADDR);
    if (cmd == OLED_CMD)
        I2C_WriteByte(0x00);
    else
        I2C_WriteByte(0x40);
    I2C_WriteByte(dat);
    I2C_Stop();
}

void OLED_Set_Pos(uint8_t x, uint8_t y)
{
    OLED_WriteByte(0xB0 + y, OLED_CMD);                 /* 设置页地址 (0~7) */
    OLED_WriteByte(((x & 0xF0) >> 4) | 0x10, OLED_CMD); /* 设置列高4位 */
    OLED_WriteByte((x & 0x0F), OLED_CMD);               /* 设置列低4位 */
}

void OLED_Clear(void)
{
    for (uint8_t y = 0; y < 8; y++)
    {
        OLED_Set_Pos(0, y);
        for (uint8_t x = 0; x < 128; x++)
        {
            OLED_WriteByte(0x00, OLED_DATA);
        }
    }
}

void OLED_ShowChar(uint8_t x, uint8_t y, char chr)
{
    uint8_t c = chr - ' ';
    if (x > 120) { x = 0; y += 2; }
    
    /* 写字符上半部 (8字节) */
    OLED_Set_Pos(x, y);
    for (uint8_t i = 0; i < 8; i++)
    {
        OLED_WriteByte(F8X16[c * 16 + i], OLED_DATA);
    }
    /* 写字符下半部 (8字节) */
    OLED_Set_Pos(x, y + 1);
    for (uint8_t i = 0; i < 8; i++)
    {
        OLED_WriteByte(F8X16[c * 16 + i + 8], OLED_DATA);
    }
}

void OLED_ShowString(uint8_t x, uint8_t y, const char *str)
{
    while (*str)
    {
        OLED_ShowChar(x, y, *str++);
        x += 8;
        if (x > 120)
        {
            x = 0;
            y += 2;
        }
    }
}

void OLED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    /* 配置 PB8 (SCL) 与 PB9 (SDA) 为开漏输出模式 (配合板载 4.7K 上拉) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    OLED_SCL_HIGH();
    OLED_SDA_HIGH();
    Delay_ms(100);

    /* SSD1306 官方推荐初始化序列 */
    OLED_WriteByte(0xAE, OLED_CMD); /* 关闭显示 */
    OLED_WriteByte(0x00, OLED_CMD); /* 设置列低地址 */
    OLED_WriteByte(0x10, OLED_CMD); /* 设置列高地址 */
    OLED_WriteByte(0x40, OLED_CMD); /* 设置起始行地址 */
    OLED_WriteByte(0x81, OLED_CMD); /* 对比度控制 */
    OLED_WriteByte(0xCF, OLED_CMD); /* 对比度数值 */
    OLED_WriteByte(0xA1, OLED_CMD); /* 段重映射 0xA1 */
    OLED_WriteByte(0xC8, OLED_CMD); /* 行扫描方向 0xC8 */
    OLED_WriteByte(0xA6, OLED_CMD); /* 正常显示 (不反相) */
    OLED_WriteByte(0xA8, OLED_CMD); /* 设置多路复用比 */
    OLED_WriteByte(0x3F, OLED_CMD); /* 1/64 duty */
    OLED_WriteByte(0xD3, OLED_CMD); /* 设置显示偏移 */
    OLED_WriteByte(0x00, OLED_CMD); /* 偏移 0 */
    OLED_WriteByte(0xD5, OLED_CMD); /* 设置时钟分频 */
    OLED_WriteByte(0x80, OLED_CMD);
    OLED_WriteByte(0xD9, OLED_CMD); /* 设置预充电周期 */
    OLED_WriteByte(0xF1, OLED_CMD);
    OLED_WriteByte(0xDA, OLED_CMD); /* COM 引脚配置 */
    OLED_WriteByte(0x12, OLED_CMD);
    OLED_WriteByte(0xDB, OLED_CMD); /* VCOMH 阈值 */
    OLED_WriteByte(0x40, OLED_CMD);
    OLED_WriteByte(0x20, OLED_CMD); /* 页寻址模式 */
    OLED_WriteByte(0x02, OLED_CMD);
    OLED_WriteByte(0x8D, OLED_CMD); /* 电荷泵使能 */
    OLED_WriteByte(0x14, OLED_CMD); /* 开启电荷泵 (0x14) */
    OLED_WriteByte(0xAF, OLED_CMD); /* 开启显示面板 */

    OLED_Clear();
}
