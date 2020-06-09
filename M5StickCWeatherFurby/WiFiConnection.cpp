#include "WiFiConnection.h"


WiFiConnection::WiFiConnection()
{
}

bool WiFiConnection::connect(char* ssid, char* password, int timeOutInSeconds)
{
    unsigned long timeOutInMillis = timeOutInSeconds * 10000;
    unsigned long startTimeMillis = millis();

    WiFi.mode(WIFI_OFF);
    delay(100);
    WiFi.mode(WIFI_STA);
    delay(100);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        unsigned long elapsedTimeMillis = millis() - startTimeMillis;

        if (elapsedTimeMillis > timeOutInMillis)
        {
            WiFi.mode(WIFI_OFF);
            return false;
        }

        delay(100);
    }

    return true;
}




