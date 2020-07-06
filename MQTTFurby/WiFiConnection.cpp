#include "WiFiConnection.h"
#include "Furby.h"


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

        // keep the Furby awake while we wait for the 
        // WiFi connection

        Furby::loop();

        delay(100);
    }

    return true;
}




