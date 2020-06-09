#include "Furby.h"
#include "MQTT.h"
#include "WiFiConnection.h"

WiFiConnection* wifiConnection;
Furby* myFurby;
MQTT* mqtt;

bool wantShutdown = false;

void messageDump(char* title, unsigned char* buffer, unsigned int length)
{
    Serial.print(title);
    for (int i = 0; i < length; i++) {
        Serial.print(buffer[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

void shutdown()
{
    Serial.println("Shutting down");

    unsigned char message[] = { 0x13, 0x00, 55, 2, 11, 0 };
    int len = sizeof(message) / sizeof(unsigned char);
    myFurby->sendMessageToFurby(message, len);
    delay(2000);
    myFurby->disconnect();
}

void MQTTMessageReceiver(unsigned char* buffer, unsigned int length)
{
    messageDump("MQTT message received: ", buffer, length);
    myFurby->sendMessageToFurby(buffer, length);
}

void switchDecode(unsigned char* buffer, unsigned int length)
{

    if (length == 8)
    {
        if (buffer[0] != 0x21)
        {
            return;
        }

        if (buffer[1] & 0x02) {
            // Antenna left
            return;
        }

        if (buffer[1] & 0x01) {
            // Antenna right
            return;
        }

        if (buffer[2] & 0x40) {
            // Antenna front
            return;
        }

        if (buffer[2] & 0x80) {
            //antenna back
            return;
        }

        if (buffer[2] & 0x10) {
            // pulled tail
            wantShutdown = true;
            return;
        }
    }
}


void FurbyMessageReceiver(unsigned char* buffer, unsigned int length)
{
    messageDump("Furby message received: ", buffer, length);
    switchDecode(buffer, length);
    mqtt->sendMessageToMQTT(buffer, length);
}

void setup() {

    Serial.begin(115200);


    Serial.println(
        "Furby MQTT Client\n\n"
        "Make sure that your Furby is awake before running the program\n"
        "Once it is connected it will act on any MQTT messages received \n"
        "and send messages if one of the Furby sensors is changed.\n"
        "Pull Furby's tail to disconnect Furby from the ESP32\n\n");

    myFurby = new Furby();

    Serial.println("Connecting to Furby");

    while (!myFurby->connect())
    {
        Serial.print('.');
        delay(500);
    }

    Serial.println("Got a Furby!");

    // Turn light red
    unsigned char redMessage[] = { 0x14, 0xFF, 0x00, 0x00 };
    myFurby->sendMessageToFurby(redMessage, 4);

    delay(500);

    Serial.println("Connecting to WiFi");

    wifiConnection = new WiFiConnection();

  while (!wifiConnection->connect("your WiFi SSID", "your WiFi password", 10))
    {
        Serial.print(".");
        delay(500);
    }

    Serial.println("Got WiFi");

    // Turn light blue
    unsigned char blueMessage[] = { 0x14, 0x00, 0x00, 0xFF };
    myFurby->sendMessageToFurby(blueMessage, 4);

    Serial.println("Connecting MQTT");

    mqtt = new MQTT();

    while (!mqtt->connect(
        "Furby",  // device name
        "your mqtt host", // mqtt host 
        "your mqtt username",       // username
        "your mqtt password",       // password
        "furbyIn",                  // topic subscribed to
        "furbyOut",                 // topic published to
        false,                      // true for secure
        10)) {                      // timeout in secs
        Serial.println(".");
    }

    mqtt->bindToIncomingMQTTMessages(MQTTMessageReceiver);
    myFurby->bindToIncomingFurbyMessages(FurbyMessageReceiver);

    Serial.println("MQTT connected");

    // Turn light green
    unsigned char greenMessage[] = { 0x14, 0x00, 0xFF, 0x00 };
    myFurby->sendMessageToFurby(greenMessage, 4);

    delay(500);

    // Say connected
    unsigned char connectedMessage[] = { 0x13, 0x00, 34, 0x00, 0x00, 0x00 };
    myFurby->sendMessageToFurby(connectedMessage, 6);
}

void loop() {
    mqtt->loop();
    myFurby->loop();
    if (wantShutdown)
    {
        shutdown();
        wantShutdown = false;
    }
    delay(100);
}
