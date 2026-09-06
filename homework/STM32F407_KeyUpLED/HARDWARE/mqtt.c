#include "mqtt.h"

static uint8_t encode_remaining_length(uint8_t *buf, uint32_t length)
{
    uint8_t encoded_bytes = 0;
    uint8_t digit;
    do {
        digit = length % 128;
        length /= 128;
        if (length > 0)
            digit |= 0x80;
        buf[encoded_bytes++] = digit;
    } while (length > 0);
    return encoded_bytes;
}

uint16_t MQTT_PackConnect(uint8_t *buf, const char *client_id, uint16_t keep_alive, const char *user, const char *pass)
{
    uint8_t variable_header[10];
    uint8_t payload[256];
    uint16_t payload_len = 0;
    uint8_t flags = 0x02; // Clean Session
    uint16_t cid_len;
    uint32_t rem_len;
    uint16_t idx = 0;

    variable_header[0] = 0x00;
    variable_header[1] = 0x04;
    variable_header[2] = 'M';
    variable_header[3] = 'Q';
    variable_header[4] = 'T';
    variable_header[5] = 'T';
    variable_header[6] = 0x04; // 3.1.1

    if (user != NULL && strlen(user) > 0) flags |= 0x80;
    if (pass != NULL && strlen(pass) > 0) flags |= 0x40;
    variable_header[7] = flags;
    variable_header[8] = (keep_alive >> 8) & 0xFF;
    variable_header[9] = keep_alive & 0xFF;

    cid_len = strlen(client_id);
    payload[payload_len++] = (cid_len >> 8) & 0xFF;
    payload[payload_len++] = cid_len & 0xFF;
    memcpy(&payload[payload_len], client_id, cid_len);
    payload_len += cid_len;

    if (user != NULL && strlen(user) > 0) {
        uint16_t ulen = strlen(user);
        payload[payload_len++] = (ulen >> 8) & 0xFF;
        payload[payload_len++] = ulen & 0xFF;
        memcpy(&payload[payload_len], user, ulen);
        payload_len += ulen;
    }
    if (pass != NULL && strlen(pass) > 0) {
        uint16_t plen = strlen(pass);
        payload[payload_len++] = (plen >> 8) & 0xFF;
        payload[payload_len++] = plen & 0xFF;
        memcpy(&payload[payload_len], pass, plen);
        payload_len += plen;
    }

    rem_len = sizeof(variable_header) + payload_len;
    buf[idx++] = MQTT_PKT_CONNECT;
    idx += encode_remaining_length(&buf[idx], rem_len);
    memcpy(&buf[idx], variable_header, sizeof(variable_header));
    idx += sizeof(variable_header);
    memcpy(&buf[idx], payload, payload_len);
    idx += payload_len;

    return idx;
}

uint16_t MQTT_PackSubscribe(uint8_t *buf, uint16_t msg_id, const char *topic, uint8_t req_qos)
{
    uint8_t body[128];
    uint16_t body_len = 0;
    uint16_t tlen;
    uint16_t idx = 0;

    body[body_len++] = (msg_id >> 8) & 0xFF;
    body[body_len++] = msg_id & 0xFF;

    tlen = strlen(topic);
    body[body_len++] = (tlen >> 8) & 0xFF;
    body[body_len++] = tlen & 0xFF;
    memcpy(&body[body_len], topic, tlen);
    body_len += tlen;
    body[body_len++] = req_qos;

    buf[idx++] = MQTT_PKT_SUBSCRIBE | 0x02;
    idx += encode_remaining_length(&buf[idx], body_len);
    memcpy(&buf[idx], body, body_len);
    idx += body_len;

    return idx;
}

uint16_t MQTT_PackPublish(uint8_t *buf, const char *topic, const char *payload, uint8_t qos, uint8_t retain)
{
    uint8_t header_byte = MQTT_PKT_PUBLISH | ((qos & 0x03) << 1) | (retain & 0x01);
    uint16_t tlen = strlen(topic);
    uint16_t plen = strlen(payload);
    uint32_t rem_len = 2 + tlen + plen;
    uint16_t idx = 0;

    buf[idx++] = header_byte;
    idx += encode_remaining_length(&buf[idx], rem_len);

    buf[idx++] = (tlen >> 8) & 0xFF;
    buf[idx++] = tlen & 0xFF;
    memcpy(&buf[idx], topic, tlen);
    idx += tlen;

    memcpy(&buf[idx], payload, plen);
    idx += plen;

    return idx;
}

uint16_t MQTT_PackPingReq(uint8_t *buf)
{
    buf[0] = MQTT_PKT_PINGREQ;
    buf[1] = 0x00;
    return 2;
}

uint8_t MQTT_ParseRxPacket(const uint8_t *buf, uint16_t len, MQTT_Msg_t *msg)
{
    uint8_t pkt_type;
    if (len < 2) return 0;
    pkt_type = buf[0] & 0xF0;

    if (pkt_type == MQTT_PKT_CONNACK) {
        if (len >= 4 && buf[3] == 0x00) {
            return MQTT_PKT_CONNACK;
        }
    }
    else if (pkt_type == MQTT_PKT_SUBACK) {
        return MQTT_PKT_SUBACK;
    }
    else if (pkt_type == MQTT_PKT_PINGRESP) {
        return MQTT_PKT_PINGRESP;
    }
    else if (pkt_type == MQTT_PKT_PUBLISH) {
        uint16_t idx = 1;
        uint32_t rem_len = 0;
        uint32_t multiplier = 1;
        uint8_t digit;
        uint16_t topic_len;
        uint16_t copy_tlen;
        uint16_t payload_len;
        uint16_t copy_plen;

        do {
            if (idx >= len) return 0;
            digit = buf[idx++];
            rem_len += (digit & 0x7F) * multiplier;
            multiplier *= 128;
        } while ((digit & 0x80) != 0);

        if (idx + 2 > len) return 0;
        topic_len = (buf[idx] << 8) | buf[idx + 1];
        idx += 2;
        if (idx + topic_len > len) return 0;

        copy_tlen = (topic_len < sizeof(msg->topic) - 1) ? topic_len : (sizeof(msg->topic) - 1);
        memcpy(msg->topic, &buf[idx], copy_tlen);
        msg->topic[copy_tlen] = '\0';
        idx += topic_len;

        payload_len = len - idx;
        copy_plen = (payload_len < sizeof(msg->payload) - 1) ? payload_len : (sizeof(msg->payload) - 1);
        memcpy(msg->payload, &buf[idx], copy_plen);
        msg->payload[copy_plen] = '\0';
        msg->payload_len = copy_plen;

        return MQTT_PKT_PUBLISH;
    }

    return 0;
}
