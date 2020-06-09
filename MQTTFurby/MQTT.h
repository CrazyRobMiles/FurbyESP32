#pragma once

#include <Arduino.h>
#include <DNSServer.h>
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>


#define MAX_MQTT_TOPIC_LENGTH 100

class MQTT
{
public:
    
    MQTT();
    
    bool connect(char* device, char* host, char* username, char* password, char* getTopic, char * publishTopic, bool secure, int timeOutInSeconds);

    void bindToIncomingMQTTMessages(void (*receiveMessage)(unsigned char* buffer, unsigned int length));

    bool sendMessageToMQTT(unsigned char* buffer, unsigned int length);


    void loop();

    bool connected();

    enum mqttConnectStates { turnedOn, mqttConnected, mqttDisconnected, turnedOff };

private:

    mqttConnectStates mqttState;

    Client* espClient = NULL;

    PubSubClient* mqttPubSubClient = NULL;

    char mqttReceiveTopic[MAX_MQTT_TOPIC_LENGTH];

    char mqttPublishTopic[MAX_MQTT_TOPIC_LENGTH];

    static void mqttCallback(char* topic, byte* payload, unsigned int length);

    static void (*boundMessageReceiver)(unsigned char* buffer, unsigned int length);

    static void sendMessageFromMQTT(unsigned char* buffer, unsigned int length);

};

