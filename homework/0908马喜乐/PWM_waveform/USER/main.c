#include "stm32f4xx.h"
#include "systick.h"
#include "led.h"
#include "key.h"
#include "beep.h"
#include "oled.h"
#include <stdio.h>

/**
 * @brief  主函数：选取 TIM13 硬件定时器产生 PWM 驱动蜂鸣器工作
 * @note   硬件与定时器配置说明：
 *         - 输出引脚: PF8 (板载 BEEP 信号，驱动 S8050 NPN 三极管)
 *         - 定时器通道: TIM13_CH1 (AF9, GPIO_AF_TIM13)
 *         - 定时器时钟: APB1 定时器时钟 84MHz (SYSCLK 168MHz, APB1 DIV4, 定时器 x2)
 *         - 预分频器 PSC = 84 - 1 = 83 (计数基准频率 1MHz, 即每 1us 计数 1 次)
 *         - 占空比: 50% 方波对称输出
 *         - 静音逻辑: CCR1 = 0，引脚输出低电平，三极管截止
 */

/* 预设测试频率表 (Hz) */
static const uint32_t s_preset_freqs[] = {
    1000,   /* 1.0 kHz: 低音提示 */
    1500,   /* 1.5 kHz: 柔和提示 */
    2000,   /* 2.0 kHz: 默认共振频率 (声音最清脆响亮) */
    2700,   /* 2.7 kHz: 经典警报高频 */
    4000    /* 4.0 kHz: 尖锐提示音 */
};
#define PRESET_FREQ_COUNT (sizeof(s_preset_freqs) / sizeof(s_preset_freqs[0]))

/* 音阶序列：Do Re Mi Fa Sol La Si High-Do */
static const uint16_t s_melody_notes[] = {
    BEEP_NOTE_C4, BEEP_NOTE_D4, BEEP_NOTE_E4, BEEP_NOTE_F4,
    BEEP_NOTE_G4, BEEP_NOTE_A4, BEEP_NOTE_B4, BEEP_NOTE_C5
};
#define MELODY_LENGTH (sizeof(s_melody_notes) / sizeof(s_melody_notes[0]))

int main(void)
{
    uint8_t key = KEY_NONE;
    uint8_t freq_idx = 2;               /* 默认选择 2000Hz (第3档) */
    uint32_t current_freq = 2000;
    uint8_t beep_on = 0;                /* 初始关闭 */
    uint16_t heartbeat_cnt = 0;         /* LED0 心跳计数器 */
    uint8_t led_state = 0;
    uint8_t i;
    char str_buf[24];

    /* 1. 初始化系统滴答延时 */
    SysTick_Init();

    /* 2. 初始化 LED 与按键 */
    LED_Init();
    Key_Init();

    /* 3. 初始化蜂鸣器硬件 (TIM13_CH1 / PF8) */
    BEEP_Init();

    /* 4. 初始化 OLED 屏幕 */
    OLED_Init();
    OLED_Clear();

    /* 5. 播放开机双音提示音（确认蜂鸣器硬件正常发声） */
    BEEP_PlayTone(1200, 80);
    Delay_ms(50);
    BEEP_PlayTone(2000, 120);

    /* 6. OLED 静态与初始信息显示 */
    OLED_ShowString(0, 0, "== BEEP PWM ==");
    OLED_ShowString(0, 2, "Pin : PF8(TIM13)");

    sprintf(str_buf, "Freq: %4d Hz", (int)current_freq);
    OLED_ShowString(0, 4, str_buf);

    OLED_ShowString(0, 6, "State: OFF (50%)");

    while (1)
    {
        /* 按键扫描 */
        key = Key_Scan();
        if (key != KEY_NONE)
        {
            switch (key)
            {
                /* KEY_UP: 循环切换预设 PWM 频率 (1000Hz ~ 4000Hz) */
                case KEY_UP_PRES:
                    freq_idx = (freq_idx + 1) % PRESET_FREQ_COUNT;
                    current_freq = s_preset_freqs[freq_idx];
                    if (beep_on)
                    {
                        BEEP_On(current_freq);
                    }
                    break;

                /* KEY0: 蜂鸣器开关切换 (ON / OFF) */
                case KEY0_PRES:
                    beep_on = !beep_on;
                    if (beep_on)
                    {
                        BEEP_On(current_freq);
                    }
                    else
                    {
                        BEEP_Off();
                    }
                    break;

                /* KEY1: 演示播放经典八音阶 (Do Re Mi Fa Sol La Si Do) */
                case KEY1_PRES:
                    OLED_ShowString(0, 6, "State: PLAYING  ");
                    for (i = 0; i < MELODY_LENGTH; i++)
                    {
                        sprintf(str_buf, "Freq: %4d Hz", (int)s_melody_notes[i]);
                        OLED_ShowString(0, 4, str_buf);
                        BEEP_PlayTone(s_melody_notes[i], 160);
                        Delay_ms(30);
                    }
                    /* 恢复原设定频率与状态 */
                    if (beep_on)
                    {
                        BEEP_On(current_freq);
                    }
                    else
                    {
                        BEEP_Off();
                    }
                    break;

                default:
                    break;
            }

            /* 刷新 OLED 状态行 */
            sprintf(str_buf, "Freq: %4d Hz", (int)current_freq);
            OLED_ShowString(0, 4, str_buf);

            if (beep_on)
            {
                OLED_ShowString(0, 6, "State: ON  (50%)");
            }
            else
            {
                OLED_ShowString(0, 6, "State: OFF (50%)");
            }
        }

        /* 运行心跳灯：LED0 每 500ms 翻转一次 */
        heartbeat_cnt++;
        if (heartbeat_cnt >= 50)
        {
            heartbeat_cnt = 0;
            led_state = !led_state;
            LED0_Set(led_state);
        }

        Delay_ms(10);
    }
}
