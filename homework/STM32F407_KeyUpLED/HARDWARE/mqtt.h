#ifndef __MQTT_H
#define __MQTT_H

#include "stm32f4xx.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define MQTT_PKT_CONNECT     0x10
#define MQTT_PKT_CONNACK     0x20
#define MQTT_PKT_PUBLISH     0x30
#define MQTT_PKT_PUBACK      0x40
#define MQTT_PKT_SUBSCRIBE   0x80
#define MQTT_PKT_SUBACK      0x90
#define MQTT_PKT_PINGREQ     0xC0
#define MQTT_PKT_PINGRESP    0xD0
#define MQTT_PKT_DISCONNECT  0xE0

#define MQTT_MAX_PACKET_LEN  512

typedef struct {
    char topic[64];
    char payload[256];
    uint16_t payload_len;
} MQTT_Msg_t;

uint16_t MQTT_PackConnect(uint8_t *buf, const char *client_id, uint16_t keep_alive, const char *user, const char *pass);
uint16_t MQTT_PackSubscribe(uint8_t *buf, uint16_t msg_id, const char *topic, uint8_t req_qos);
uint16_t MQTT_PackPublish(uint8_t *buf, const char *topic, const char *payload, uint8_t qos, uint8_t retain);
uint16_t MQTT_PackPingReq(uint8_t *buf);
uint8_t MQTT_ParseRxPacket(const uint8_t *buf, uint16_t len, MQTT_Msg_t *msg);

#endif /* __MQTT_H */
