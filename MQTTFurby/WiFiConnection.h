#pragma once

#include <Arduino.h>
#include <WiFi.h>

class WiFiConnection
{
public:
	WiFiConnection();

	bool connect(char* ssid, char* password, int timeOutInSeconds);

};

