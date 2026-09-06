#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "Led.h"
#include "Key.h"
#include "usart.h"
#include "esp8266.h"
#include "mqtt.h"

#define WIFI_SSID          "@Tenda_B93620"
#define WIFI_PWD           "88888888"

#define MQTT_BROKER_HOST   "broker.emqx.io"
#define MQTT_BROKER_PORT   1883
#define MQTT_CLIENT_ID     "STM32F407_ESP8266_User01"
#define MQTT_TOPIC_CTRL    "stm32/control"
#define MQTT_TOPIC_STATUS  "stm32/status"

volatile uint32_t g_ticks_ms = 0;
uint8_t g_mqtt_buf[MQTT_MAX_PACKET_LEN];
static uint8_t g_is_online = 0;

void delay_ms(uint32_t ms)
{
    uint32_t start = g_ticks_ms;
    while ((uint32_t)(g_ticks_ms - start) < ms)
    {
    }
}

static void SysTick_Init(void)
{
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / 1000);
}

static uint8_t Publish_Status(const char *event_tag)
{
    char status_json[128];
    uint8_t led0_state = (GPIO_ReadOutputDataBit(GPIOF, GPIO_Pin_9) == Bit_RESET) ? 1 : 0;
    uint8_t led1_state = (GPIO_ReadOutputDataBit(GPIOF, GPIO_Pin_10) == Bit_RESET) ? 1 : 0;
    uint16_t len;
    uint8_t ok;

    snprintf(status_json, sizeof(status_json), 
             "{\"dev\":\"stm32f407\",\"led0\":%d,\"led1\":%d,\"event\":\"%s\",\"time_ms\":%u}",
             led0_state, led1_state, event_tag ? event_tag : "idle", g_ticks_ms);

    len = MQTT_PackPublish(g_mqtt_buf, MQTT_TOPIC_STATUS, status_json, 0, 0);
    ok = ESP8266_SendMQTTPacket(g_mqtt_buf, len);
    if (ok)
    {
        printf("[MQTT TX] Published status: %s\r\n", status_json);
    }
    else
    {
        printf("[MQTT TX ERROR] Failed to publish status!\r\n");
    }
    return ok;
}

static uint8_t MQTT_ConnectAndSubscribe(void)
{
    uint16_t pkt_len;
    printf("[MQTT] Connecting TCP to %s:%d...\r\n", MQTT_BROKER_HOST, MQTT_BROKER_PORT);
    if (!ESP8266_ConnectTCP(MQTT_BROKER_HOST, MQTT_BROKER_PORT))
    {
        return 0;
    }

    printf("[MQTT] Sending CONNECT (Client ID: %s)...\r\n", MQTT_CLIENT_ID);
    pkt_len = MQTT_PackConnect(g_mqtt_buf, MQTT_CLIENT_ID, 60, NULL, NULL);
    if (!ESP8266_SendMQTTPacket(g_mqtt_buf, pkt_len))
    {
        printf("[MQTT ERROR] CONNECT packet send failed!\r\n");
        return 0;
    }
    delay_ms(200);

    printf("[MQTT] Subscribing to topic '%s'...\r\n", MQTT_TOPIC_CTRL);
    pkt_len = MQTT_PackSubscribe(g_mqtt_buf, 1, MQTT_TOPIC_CTRL, 0);
    if (!ESP8266_SendMQTTPacket(g_mqtt_buf, pkt_len))
    {
        printf("[MQTT ERROR] SUBSCRIBE packet send failed!\r\n");
        return 0;
    }
    delay_ms(200);

    printf("[MQTT] Setup Complete! Subscribed to '%s'\r\n", MQTT_TOPIC_CTRL);
    g_is_online = 1;
    GPIO_ResetBits(GPIOF, GPIO_Pin_9); // 点亮 LED0 表示在线
    Publish_Status("connected");
    return 1;
}

int main(void)
{
    uint32_t last_ping_tick = 0;
    uint32_t reconnect_tick = 0;
    uint8_t key = 0;

    SysTick_Init();
    LED_Init();
    Key_Init();
    USART1_Init(115200);
    ESP8266_Init(115200);

    delay_ms(1000);

    printf("\r\n========================================\r\n");
    printf("  STM32F407 + ESP8266 MQTT IoT System   \r\n");
    printf("========================================\r\n");

    // 1. 初始化 WiFi 连接
    if (ESP8266_ConnectWiFi(WIFI_SSID, WIFI_PWD))
    {
        // 2. 连接 MQTT Broker 并订阅控制主题
        MQTT_ConnectAndSubscribe();
    }

    last_ping_tick = g_ticks_ms;
    reconnect_tick = g_ticks_ms;

    while (1)
    {
        // 1. 按键扫描 (支持 KEY_UP / KEY0 / KEY1 / KEY2)
        key = Key_Scan();
        if (key != KEY_NONE)
        {
            if (key == KEY_UP_PRES)
            {
                GPIO_ToggleBits(GPIOF, GPIO_Pin_10);
                printf("\r\n[Key] KEY_UP (PA0) Pressed! Toggled LED1.\r\n");
                Publish_Status("KEY_UP_PRESSED");
            }
            else if (key == KEY0_PRES)
            {
                GPIO_ToggleBits(GPIOF, GPIO_Pin_9);
                printf("\r\n[Key] KEY0 (PE4) Pressed! Toggled LED0.\r\n");
                Publish_Status("KEY0_PRESSED");
            }
            else if (key == KEY1_PRES)
            {
                GPIO_ToggleBits(GPIOF, GPIO_Pin_10);
                printf("\r\n[Key] KEY1 (PE3) Pressed! Toggled LED1.\r\n");
                Publish_Status("KEY1_PRESSED");
            }
            else if (key == KEY2_PRES)
            {
                GPIO_ToggleBits(GPIOF, GPIO_Pin_9 | GPIO_Pin_10);
                printf("\r\n[Key] KEY2 (PE2) Pressed! Toggled ALL LEDs.\r\n");
                Publish_Status("KEY2_PRESSED");
            }
        }

        // 2. 解析接收到的云端数据
        if (g_esp_rx_len > 0)
        {
            char *ipd_ptr = strstr((char*)g_esp_rx_buf, "+IPD,");
            if (ipd_ptr != NULL)
            {
                char *colon_ptr = strchr(ipd_ptr, ':');
                if (colon_ptr != NULL)
                {
                    uint8_t *mqtt_data = (uint8_t*)(colon_ptr + 1);
                    uint16_t data_len = g_esp_rx_len - (mqtt_data - g_esp_rx_buf);
                    MQTT_Msg_t msg;
                    uint8_t pkt_type = MQTT_ParseRxPacket(mqtt_data, data_len, &msg);

                    if (pkt_type == MQTT_PKT_PUBLISH)
                    {
                        printf("\r\n[MQTT RX] Topic: %s | Payload: %s\r\n", msg.topic, msg.payload);

                        if (strstr(msg.payload, "LED0_ON") != NULL)
                        {
                            GPIO_ResetBits(GPIOF, GPIO_Pin_9);
                            printf("[Action] LED0 turned ON\r\n");
                        }
                        else if (strstr(msg.payload, "LED0_OFF") != NULL)
                        {
                            GPIO_SetBits(GPIOF, GPIO_Pin_9);
                            printf("[Action] LED0 turned OFF\r\n");
                        }
                        else if (strstr(msg.payload, "LED0_TOGGLE") != NULL)
                        {
                            GPIO_ToggleBits(GPIOF, GPIO_Pin_9);
                            printf("[Action] LED0 TOGGLED\r\n");
                        }

                        if (strstr(msg.payload, "LED1_ON") != NULL)
                        {
                            GPIO_ResetBits(GPIOF, GPIO_Pin_10);
                            printf("[Action] LED1 turned ON\r\n");
                        }
                        else if (strstr(msg.payload, "LED1_OFF") != NULL)
                        {
                            GPIO_SetBits(GPIOF, GPIO_Pin_10);
                            printf("[Action] LED1 turned OFF\r\n");
                        }
                        else if (strstr(msg.payload, "LED1_TOGGLE") != NULL)
                        {
                            GPIO_ToggleBits(GPIOF, GPIO_Pin_10);
                            printf("[Action] LED1 TOGGLED\r\n");
                        }

                        if (strstr(msg.payload, "ALL_ON") != NULL)
                        {
                            GPIO_ResetBits(GPIOF, GPIO_Pin_9 | GPIO_Pin_10);
                            printf("[Action] ALL LEDs turned ON\r\n");
                        }
                        else if (strstr(msg.payload, "ALL_OFF") != NULL)
                        {
                            GPIO_SetBits(GPIOF, GPIO_Pin_9 | GPIO_Pin_10);
                            printf("[Action] ALL LEDs turned OFF\r\n");
                        }

                        Publish_Status("cmd_ack");
                    }
                    else if (pkt_type == MQTT_PKT_PINGRESP)
                    {
                        printf("[MQTT] Heartbeat PINGRESP received.\r\n");
                    }
                }
            }
            
            // 检查是否断开连接
            if (strstr((char*)g_esp_rx_buf, "CLOSED") != NULL || strstr((char*)g_esp_rx_buf, "WIFI DISCONNECT") != NULL)
            {
                printf("\r\n[MQTT WARN] Connection closed by broker! Will reconnect...\r\n");
                g_is_online = 0;
                GPIO_SetBits(GPIOF, GPIO_Pin_9); // 熄灭 LED0 表示离线
            }

            ESP8266_ClearRxBuf();
        }

        // 3. 心跳保活：在线时每 25 秒发送一次 PINGREQ
        if (g_is_online && (uint32_t)(g_ticks_ms - last_ping_tick) >= 25000)
        {
            uint16_t len;
            last_ping_tick = g_ticks_ms;
            len = MQTT_PackPingReq(g_mqtt_buf);
            if (!ESP8266_SendMQTTPacket(g_mqtt_buf, len))
            {
                printf("[MQTT WARN] PINGREQ failed! Setting offline.\r\n");
                g_is_online = 0;
                GPIO_SetBits(GPIOF, GPIO_Pin_9);
            }
            else
            {
                printf("[MQTT] Sent PINGREQ keepalive\r\n");
            }
        }

        // 4. 断线自动重连机制：若离线，每 5 秒尝试重连一次
        if (!g_is_online && (uint32_t)(g_ticks_ms - reconnect_tick) >= 5000)
        {
            reconnect_tick = g_ticks_ms;
            printf("[MQTT] Reconnecting to broker...\r\n");
            MQTT_ConnectAndSubscribe();
        }

        delay_ms(10);
    }
}
