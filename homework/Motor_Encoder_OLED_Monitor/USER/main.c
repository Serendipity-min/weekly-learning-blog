#include "stm32f4xx.h"
#include "systick.h"
#include "led.h"
#include "key.h"
#include "pwm.h"
#include "oled.h"
#include "encoder.h"
#include "adc.h"
#include "timer.h"
#include <stdio.h>

/**
 * @brief  主函数: 电机控制系统与 OLED 综合状态监视器
 */
int main(void)
{
    uint8_t key = KEY_NONE;
    MotorDir_t dir = MOTOR_DIR_STOP; /* 当前方向 */
    uint16_t speed = 0;              /* 当前设定速度 (0 ~ 100%) */
    uint8_t refresh_cnt = 0;         /* OLED 刷新计数 */
    char str_buf[20];                /* 字符缓存 (每行最多16字符) */

    /* 1. 初始化系统滴答延时 */
    SysTick_Init();

    /* 2. 初始化按键与 LED 指示灯 */
    LED_Init();
    Key_Init();

    /* 3. 初始化电机 PWM 驱动 (TIM3 10kHz PWM) */
    Motor_Init();

    /* 4. 初始化硬件编码器接口 (TIM4: PB6=A相, PB7=B相) */
    Encoder_Init();

    /* 5. 初始化 100ms 测速高精度硬件定时器 (TIM6) */
    Tim6_Init();

    /* 6. 初始化电池电压采集 ADC1 (同时支持 PA5 和 PA6 并禁用板载 Flash) */
    Battery_ADC_Init();

    /* 7. 初始化 0.96 寸 OLED 显示屏 (I2C: PB8, PB9) */
    OLED_Init();
    OLED_Clear();

    /* 第 0 行 (Page 0): 标题栏 */
    OLED_ShowString(0, 0, "== MOTOR MON ==");

    while (1)
    {
        /* 扫描按键 */
        key = Key_Scan();
        if (key != KEY_NONE)
        {
            switch (key)
            {
                /* KEY_UP (PA0): 加速 */
                case KEY_UP_PRES:
                    if (dir == MOTOR_DIR_STOP)
                    {
                        dir = MOTOR_DIR_FORWARD;
                        speed = 30;
                    }
                    else
                    {
                        if (speed + 10 <= 100)
                            speed += 10;
                        else
                            speed = 100;
                    }
                    Motor_SetState(dir, speed);
                    break;

                /* KEY0 (PE4): 减速 */
                case KEY0_PRES:
                    if (speed >= 10)
                        speed -= 10;
                    else
                        speed = 0;

                    if (speed == 0)
                    {
                        dir = MOTOR_DIR_STOP;
                        Motor_Stop();
                    }
                    else
                    {
                        Motor_SetState(dir, speed);
                    }
                    break;

                /* KEY1 (PE3): 正转 / 启停切换 */
                case KEY1_PRES:
                    if (dir == MOTOR_DIR_FORWARD)
                    {
                        dir = MOTOR_DIR_STOP;
                        Motor_Stop();
                    }
                    else
                    {
                        dir = MOTOR_DIR_FORWARD;
                        if (speed == 0)
                            speed = 50;
                        Motor_SetState(dir, speed);
                    }
                    break;

                /* KEY2 (PE2): 反转 / 启停切换 */
                case KEY2_PRES:
                    if (dir == MOTOR_DIR_REVERSE)
                    {
                        dir = MOTOR_DIR_STOP;
                        Motor_Stop();
                    }
                    else
                    {
                        dir = MOTOR_DIR_REVERSE;
                        if (speed == 0)
                            speed = 50;
                        Motor_SetState(dir, speed);
                    }
                    break;

                default:
                    break;
            }

            /* 更新板载状态指示灯 */
            if (dir != MOTOR_DIR_STOP && speed > 0)
            {
                LED0_Set(1);
                LED1_Set(dir == MOTOR_DIR_REVERSE ? 1 : 0);
            }
            else
            {
                LED0_Set(0);
                LED1_Set(0);
            }
        }

        /* 周期刷新 OLED 实时信息 (约每 100ms 刷新一次) */
        refresh_cnt++;
        if (refresh_cnt >= 10)
        {
            refresh_cnt = 0;

            /* 第 2 行 (Page 2): 运转方向与设定占空比 */
            if (dir == MOTOR_DIR_STOP || speed == 0)
                sprintf(str_buf, "DIR:STOP  SPD:  0%%");
            else if (dir == MOTOR_DIR_FORWARD)
                sprintf(str_buf, "DIR:FWD   SPD:%3d%%", speed);
            else
                sprintf(str_buf, "DIR:REV   SPD:%3d%%", speed);
            OLED_ShowString(0, 2, str_buf);

            /* 第 4 行 (Page 4): 电机输出轴实测转速 (RPM) 与 100ms 脉冲计数 (C) */
            int16_t raw_cnt = 0;
            int16_t rpm = Encoder_GetSpeedRPM(&raw_cnt);
            if (raw_cnt < 0) raw_cnt = -raw_cnt;
            sprintf(str_buf, "RPM:%4d C:%-5d", rpm, raw_cnt);
            OLED_ShowString(0, 4, str_buf);

            /* 第 6 行 (Page 6): 动力电池电压与 RAW 原始采样码值 (清晰无歧义) */
            uint16_t raw_val = 0;
            uint8_t active_pin = 0;
            float v_batt = Battery_GetVoltage(&raw_val, &active_pin);
            sprintf(str_buf, "V:%4.1fV RAW:%-4d", v_batt, raw_val);
            OLED_ShowString(0, 6, str_buf);
        }

        Delay_ms(10);
    }
}
