#pragma once
#include <HTTPClient.h>
#include "ArduinoJson-v6.15.2.h"

#define LOCATION_LENGTH 200
#define COUNTRY_LENGTH 200
#define APPKEY_LENGTH 500
#define HTTP_REQUEST_LENGTH 1000
#define WEATHER_DESC_LENGTH 20


class WeatherClient
{


public:

	WeatherClient();

	bool connect(char* location, char* country, char* appKey);
	bool update(int timeOutInSeconds);
	float getTempCentigrade();
	void getWeatherDescription(char * destination);

private:

	char weatherLocation[LOCATION_LENGTH];
	char weatherCountry[COUNTRY_LENGTH];
	char weatherAppkey[APPKEY_LENGTH];
	char weatherhttpRequest[HTTP_REQUEST_LENGTH];

	char weatherDescription[WEATHER_DESC_LENGTH];
	float tempInCentigrade;

	HTTPClient http;
};

