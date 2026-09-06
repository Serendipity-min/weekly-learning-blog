#include "esp8266.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "misc.h"

extern void delay_ms(uint32_t ms);

uint8_t g_esp_rx_buf[ESP8266_RX_BUF_SIZE];
volatile uint16_t g_esp_rx_len = 0;

void ESP8266_Init(uint32_t baudrate)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART3, &USART_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART3, ENABLE);
}

void ESP8266_ClearRxBuf(void)
{
    memset((void*)g_esp_rx_buf, 0, sizeof(g_esp_rx_buf));
    g_esp_rx_len = 0;
}

void ESP8266_SendString(const char *str)
{
    while (*str)
    {
        while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
        USART_SendData(USART3, (uint8_t)*str++);
    }
}

void ESP8266_SendBytes(const uint8_t *buf, uint16_t len)
{
    uint16_t i;
    for (i = 0; i < len; i++)
    {
        while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
        USART_SendData(USART3, buf[i]);
    }
}

uint8_t ESP8266_SendCmd(const char *cmd, const char *expected_ack, uint32_t timeout_ms)
{
    uint32_t count = 0;
    ESP8266_ClearRxBuf();
    ESP8266_SendString(cmd);
    ESP8266_SendString("\r\n");

    while (count < timeout_ms)
    {
        delay_ms(10);
        count += 10;
        if (expected_ack && strstr((char*)g_esp_rx_buf, expected_ack) != NULL)
        {
            return 1;
        }
    }
    return (expected_ack == NULL) ? 1 : 0;
}

uint8_t ESP8266_ConnectWiFi(const char *ssid, const char *pwd)
{
    char cmd[128];
    printf("[WiFi] Setting Station mode...\r\n");
    ESP8266_SendCmd("AT+CWMODE=1", "OK", 500);

    snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"", ssid, pwd);
    printf("[WiFi] Connecting to '%s'...\r\n", ssid);
    
    if (ESP8266_SendCmd(cmd, "OK", 10000))
    {
        printf("[WiFi] Connected to WiFi '%s' successfully!\r\n", ssid);
        
        ESP8266_ClearRxBuf();
        ESP8266_SendString("AT+CIFSR\r\n");
        delay_ms(500);
        printf("[WiFi] IP Info:\r\n%s\r\n", g_esp_rx_buf);
        return 1;
    }

    printf("[WiFi ERROR] Failed to connect to '%s'\r\n", ssid);
    return 0;
}

uint8_t ESP8266_ConnectTCP(const char *host, uint16_t port)
{
    char cmd[128];
    ESP8266_SendCmd("AT+CIPMUX=0", "OK", 500);
    snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"TCP\",\"%s\",%d", host, port);
    printf("[MQTT] Connecting TCP to %s:%d...\r\n", host, port);

    if (ESP8266_SendCmd(cmd, "CONNECT", 5000) || strstr((char*)g_esp_rx_buf, "OK") != NULL || strstr((char*)g_esp_rx_buf, "ALREADY CONNECTED") != NULL)
    {
        printf("[MQTT] TCP Socket Connected!\r\n");
        return 1;
    }

    printf("[MQTT ERROR] TCP Connection to %s:%d failed!\r\n", host, port);
    return 0;
}

uint8_t ESP8266_SendMQTTPacket(const uint8_t *pkt, uint16_t len)
{
    char cmd[32];
    uint32_t count = 0;
    uint8_t retry = 0;

    for (retry = 0; retry < 3; retry++)
    {
        snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%d", len);
        ESP8266_ClearRxBuf();
        ESP8266_SendString(cmd);
        ESP8266_SendString("\r\n");

        count = 0;
        while (count < 1000)
        {
            delay_ms(5);
            count += 5;
            if (strstr((char*)g_esp_rx_buf, ">") != NULL)
            {
                break;
            }
            if (strstr((char*)g_esp_rx_buf, "busy") != NULL || strstr((char*)g_esp_rx_buf, "ERROR") != NULL)
            {
                break;
            }
        }

        if (strstr((char*)g_esp_rx_buf, ">") != NULL)
        {
            ESP8266_ClearRxBuf();
            ESP8266_SendBytes(pkt, len);

            count = 0;
            while (count < 2000)
            {
                delay_ms(10);
                count += 10;
                if (strstr((char*)g_esp_rx_buf, "SEND OK") != NULL)
                {
                    delay_ms(50); // Áô³ö 50ms »º³å£¬·ÀÖ¹ÏÂÒ»ÌõÖ¸Áî³åÍ»
                    return 1;
                }
                if (strstr((char*)g_esp_rx_buf, "SEND FAIL") != NULL || strstr((char*)g_esp_rx_buf, "CLOSED") != NULL)
                {
                    break;
                }
            }
        }
        delay_ms(100);
    }
    return 0;
}
