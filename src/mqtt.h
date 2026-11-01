/**
 * @file mqtt.h
 * @brief An MQTT 3.1.1 client (lwIP's apps/mqtt) for BASIC's MQTT
 * statements, on Wi-Fi builds: connects to MQTT_BROKER from CONFIG.TXT (or
 * the one MQTT CONNECT names), keeps the connection up, and queues the
 * messages that arrive on the topics subscribed to.
 *
 * @author Ken Segler
 * @copyright Copyright (c) 2026 Ken Segler
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <stdbool.h>

#define MQTT_QUEUE   8     ///< messages kept until the program takes them
#define MQTT_PAYLOAD 255   ///< the longest payload kept (a BASIC string's; longer ones are cut)
#define MQTT_TOPIC   128   ///< the longest topic

/// At power-up: with MQTT = ON in CONFIG.TXT, connects once Wi-Fi is up.
void mqtt_start(void);

/// Every second from Wi-Fi's worker (lwIP's context): connects again after
/// the link or the broker went away, and subscribes again.
/// @param link_up Wi-Fi is up
void mqtt_tick(bool link_up);

/// MQTT CONNECT: connects (to `broker`, or CONFIG.TXT's if NULL) and waits up
/// to 10 seconds. NULL if it worked, else why not (it keeps trying).
/// @param broker the host name or address, or NULL
/// @param port its port (0: CONFIG.TXT's, else 1883)
/// @return NULL, or the reason
const char *mqtt_connect(const char *broker, int port);

/// MQTT DISCONNECT (and no reconnecting).
void mqtt_disconnect_now(void);

/// MQTT PUBLISH. NULL if it went, else why not.
/// @param topic the topic
/// @param payload the bytes
/// @param len their count
/// @param qos 0 or 1 (2 is sent as 1)
/// @param retain the broker keeps it for later subscribers
/// @return NULL, or the reason
const char *mqtt_publish_now(const char *topic, const void *payload, int len, int qos, bool retain);

/// MQTT SUBSCRIBE / UNSUBSCRIBE (kept, and subscribed again after a
/// reconnect). NULL if it went, else why not.
/// @param topic the topic (+ and # wildcards)
/// @param qos 0 or 1
/// @param sub subscribe (else unsubscribe)
/// @return NULL, or the reason
const char *mqtt_subscribe_now(const char *topic, int qos, bool sub);

/// MQTT: the broker, the connection, the subscriptions and the queue.
void mqtt_status(void);

/// MQTT(n): 0 connected (-1/0), 1 messages waiting, 2 messages dropped (the
/// queue was full).
/// @param n which
/// @return it
int mqtt_value(int n);

/// MQTT$(0): takes the next message, returning its topic ("" if none) and
/// making its payload MQTT$(1)'s.
/// @param len set to the length
/// @return the topic
const char *mqtt_take(int *len);

/// MQTT$(1): the payload of the message MQTT$(0) last took.
/// @param len set to its length
/// @return it
const char *mqtt_payload(int *len);
