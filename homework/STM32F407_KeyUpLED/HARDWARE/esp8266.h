#ifndef __ESP8266_H
#define __ESP8266_H

#include "stm32f4xx.h"
#include <stdio.h>
#include <string.h>

#define ESP8266_RX_BUF_SIZE 2048

extern uint8_t g_esp_rx_buf[ESP8266_RX_BUF_SIZE];
extern volatile uint16_t g_esp_rx_len;

void ESP8266_Init(uint32_t baudrate);
void ESP8266_ClearRxBuf(void);
void ESP8266_SendString(const char *str);
void ESP8266_SendBytes(const uint8_t *buf, uint16_t len);
uint8_t ESP8266_SendCmd(const char *cmd, const char *expected_ack, uint32_t timeout_ms);
uint8_t ESP8266_ConnectWiFi(const char *ssid, const char *pwd);
uint8_t ESP8266_ConnectTCP(const char *host, uint16_t port);
uint8_t ESP8266_SendMQTTPacket(const uint8_t *pkt, uint16_t len);

#endif /* __ESP8266_H */
