#include "stm32f4xx.h"
#include "systick.h"
#include "led.h"
#include "key.h"
#include "pwm.h"

/**
 * @brief  主函数: 4按键控制电机正反转与加减速
 */
int main(void)
{
    uint8_t key = KEY_NONE;
    MotorDir_t dir = MOTOR_DIR_STOP; /* 当前运行方向 */
    uint16_t speed = 0;              /* 当前速度百分比 (0 ~ 100%) */

    /* 1. 初始化系统滴答定时器 (提供精准毫秒延时) */
    SysTick_Init();

    /* 2. 初始化板载指示灯与 4 个控制按键 */
    LED_Init();
    Key_Init();

    /* 3. 初始化 TB6612 电机驱动 (配置 TIM3 硬件 PWM 与方向 GPIO) */
    Motor_Init();

    while (1)
    {
        /* 扫描按键事件 */
        key = Key_Scan();

        if (key != KEY_NONE)
        {
            switch (key)
            {
                /* KEY_UP (PA0): 电机加速 (每次递增 10%，最高 100%) */
                case KEY_UP_PRES:
                    if (dir == MOTOR_DIR_STOP)
                    {
                        /* 若当前处于停转状态，按下加速键默认以正转 30% 启动 */
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

                /* KEY0 (PE4): 电机减速 (每次递减 10%，减到 0% 自动刹车停机) */
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

                /* KEY1 (PE3): 正转运行 / 启停切换 */
                case KEY1_PRES:
                    if (dir == MOTOR_DIR_FORWARD)
                    {
                        /* 若当前正在正转，再次按下则刹车停止 */
                        dir = MOTOR_DIR_STOP;
                        Motor_Stop();
                    }
                    else
                    {
                        /* 切换为正转模式；若初速为 0 则赋予 50% 启动速度 */
                        dir = MOTOR_DIR_FORWARD;
                        if (speed == 0)
                            speed = 50;
                        Motor_SetState(dir, speed);
                    }
                    break;

                /* KEY2 (PE2): 反转运行 / 启停切换 */
                case KEY2_PRES:
                    if (dir == MOTOR_DIR_REVERSE)
                    {
                        /* 若当前正在反转，再次按下则刹车停止 */
                        dir = MOTOR_DIR_STOP;
                        Motor_Stop();
                    }
                    else
                    {
                        /* 切换为反转模式；若初速为 0 则赋予 50% 启动速度 */
                        dir = MOTOR_DIR_REVERSE;
                        if (speed == 0)
                            speed = 50;
                        Motor_SetState(dir, speed);
                    }
                    break;

                default:
                    break;
            }

            /* 更新板载 LED 指示灯状态:
             * - LED0 (PF9): 电机运行指示灯 (运转中点亮，停止时熄灭)
             * - LED1 (PF10): 电机反转指示灯 (反转时点亮，正转/停机时熄灭)
             */
            if (dir != MOTOR_DIR_STOP && speed > 0)
            {
                LED0_Set(1);
                if (dir == MOTOR_DIR_REVERSE)
                    LED1_Set(1);
                else
                    LED1_Set(0);
            }
            else
            {
                LED0_Set(0);
                LED1_Set(0);
            }
        }

        /* 轮询周期延时 10ms */
        Delay_ms(10);
    }
}
