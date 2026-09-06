#include "main.h"

#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern volatile uint32_t g_uptime_ms;

#define FW_VERSION "ai-console-0.1.0"
#define COMMAND_BUFFER_SIZE 96U

/* 原理图中 PF9/PF10 为板载 LED；LED 阳极接 3.3V，因此 MCU 输出低电平时点亮。 */
#define LED_GPIO GPIOF
#define LED_PIN (GPIO_Pin_9 | GPIO_Pin_10)
#define LED_ACTIVE_LEVEL Bit_RESET

static uint8_t g_command_buffer[COMMAND_BUFFER_SIZE];
static uint32_t g_command_length;
static bool g_discard_until_newline;
static bool g_led_is_on;
static const char *g_reset_reason;

static void usart1_send_byte(uint8_t value)
{
  while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
  {
  }
  USART_SendData(USART1, value);
}

static void usart1_send_text(const char *text)
{
  while (*text != '\0')
  {
    usart1_send_byte((uint8_t)*text++);
  }
}

static void usart1_send_json(const char *json)
{
  usart1_send_text(json);
  usart1_send_text("\r\n");
}

static const char *read_reset_reason(void)
{
  if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
  {
    return "power_on";
  }
  if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
  {
    return "external_reset";
  }
  if (RCC_GetFlagStatus(RCC_FLAG_SFTRST) != RESET)
  {
    return "software_reset";
  }
  if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)
  {
    return "independent_watchdog";
  }
  if (RCC_GetFlagStatus(RCC_FLAG_WWDGRST) != RESET)
  {
    return "window_watchdog";
  }
  return "unknown";
}

static void led_set(bool on)
{
  /* LED 的低有效特性集中在此处，避免命令层混淆物理电平与用户语义。 */
  if (on)
  {
    GPIO_ResetBits(LED_GPIO, LED_PIN);
  }
  else
  {
    GPIO_SetBits(LED_GPIO, LED_PIN);
  }
  g_led_is_on = on;
}

static void hardware_init(void)
{
  GPIO_InitTypeDef gpio;
  USART_InitTypeDef usart;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOF, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

  GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

  gpio.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
  gpio.GPIO_Mode = GPIO_Mode_AF;
  gpio.GPIO_Speed = GPIO_Speed_50MHz;
  gpio.GPIO_OType = GPIO_OType_PP;
  gpio.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOA, &gpio);

  USART_StructInit(&usart);
  usart.USART_BaudRate = 115200;
  usart.USART_WordLength = USART_WordLength_8b;
  usart.USART_StopBits = USART_StopBits_1;
  usart.USART_Parity = USART_Parity_No;
  usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_Init(USART1, &usart);
  USART_Cmd(USART1, ENABLE);

  gpio.GPIO_Pin = LED_PIN;
  gpio.GPIO_Mode = GPIO_Mode_OUT;
  gpio.GPIO_Speed = GPIO_Speed_50MHz;
  gpio.GPIO_OType = GPIO_OType_PP;
  gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(LED_GPIO, &gpio);
  led_set(false);
}

static void command_handle(const char *command)
{
  char response[160];

  if (strcmp(command, "help") == 0)
  {
    usart1_send_json("{\"ok\":true,\"commands\":[\"help\",\"info\",\"status\",\"ping\",\"led get\",\"led set on\",\"led set off\"]}");
  }
  else if (strcmp(command, "info") == 0)
  {
    usart1_send_json("{\"ok\":true,\"fw\":\"" FW_VERSION "\",\"chip\":\"STM32F407ZGT6\",\"transport\":\"USART1@115200-8N1\"}");
  }
  else if (strcmp(command, "status") == 0)
  {
    (void)snprintf(response, sizeof(response), "{\"ok\":true,\"uptime_ms\":%lu,\"reset\":\"%s\",\"led\":%s}", (unsigned long)g_uptime_ms, g_reset_reason, g_led_is_on ? "true" : "false");
    usart1_send_json(response);
  }
  else if (strcmp(command, "ping") == 0)
  {
    usart1_send_json("{\"ok\":true,\"reply\":\"pong\"}");
  }
  else if (strcmp(command, "led get") == 0)
  {
    usart1_send_json(g_led_is_on ? "{\"ok\":true,\"led\":true}" : "{\"ok\":true,\"led\":false}");
  }
  else if (strcmp(command, "led set on") == 0)
  {
    led_set(true);
    usart1_send_json("{\"ok\":true,\"led\":true}");
  }
  else if (strcmp(command, "led set off") == 0)
  {
    led_set(false);
    usart1_send_json("{\"ok\":true,\"led\":false}");
  }
  else
  {
    /* 只接受精确白名单命令；未知输入不会触发 GPIO、Flash 或复位等副作用。 */
    usart1_send_json("{\"ok\":false,\"error\":\"unknown_command\"}");
  }
}

static void command_receive_byte(uint8_t value)
{
  if (value == '\r')
  {
    return;
  }

  if (value == '\n')
  {
    if (g_discard_until_newline)
    {
      g_discard_until_newline = false;
      g_command_length = 0U;
      usart1_send_json("{\"ok\":false,\"error\":\"line_too_long\"}");
      return;
    }

    g_command_buffer[g_command_length] = '\0';
    if (g_command_length != 0U)
    {
      command_handle((const char *)g_command_buffer);
    }
    g_command_length = 0U;
    return;
  }

  if (g_discard_until_newline)
  {
    return;
  }

  if (g_command_length >= (COMMAND_BUFFER_SIZE - 1U))
  {
    /* 丢弃超长行至换行符，防止越界后把残余字节误解析为新命令。 */
    g_discard_until_newline = true;
    return;
  }

  g_command_buffer[g_command_length++] = value;
}

int main(void)
{
  g_reset_reason = read_reset_reason();
  RCC_ClearFlag();
  hardware_init();

  /* SysTick 只服务运行时间统计，命令处理保持在主循环中以限定中断工作量。 */
  if (SysTick_Config(SystemCoreClock / 1000U) != 0U)
  {
    usart1_send_json("{\"ok\":false,\"error\":\"systick_init\"}");
  }

  usart1_send_json("{\"ok\":true,\"event\":\"ready\",\"fw\":\"" FW_VERSION "\"}");

  while (1)
  {
    if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET)
    {
      command_receive_byte((uint8_t)USART_ReceiveData(USART1));
    }
  }
}
