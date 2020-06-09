#include "WeatherClient.h"


WeatherClient::WeatherClient()
{
}


bool WeatherClient::connect(char* location, char* country, char* appKey, int timeOutInSeconds)
{
	snprintf(weatherLocation, LOCATION_LENGTH, "%s", location);
	snprintf(weatherCountry, COUNTRY_LENGTH, "%s", country);
	snprintf(weatherAppkey, APPKEY_LENGTH, "%s", appKey);
	return update(timeOutInSeconds);
}

bool WeatherClient::update(int timeOutInSeconds)
{
	snprintf(weatherhttpRequest, HTTP_REQUEST_LENGTH,
		"http://api.openweathermap.org/data/2.5/weather?q=%s,%s&units=metric&APPID=%s",
		weatherLocation, weatherCountry, weatherAppkey);

	//Serial.printf("Request: %s\n", weatherhttpRequest);

	http.begin(weatherhttpRequest);
	http.setTimeout(timeOutInSeconds * 1000);
	int response = http.GET();

	if (response < 0)
	{
		return false;
	}

	String payload = http.getString();

	//Serial.printf("Got payload: %s\n", payload.c_str());

	const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + 2 * JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(13) + 280;

	DynamicJsonDocument doc(capacity);

	//Serial.println("Starting deserialize");

	deserializeJson(doc, payload.c_str());

	//Serial.println("Deserialized");

	JsonObject weather_0 = doc["weather"][0];
	const char* weather_0_main = weather_0["main"]; // "Rain"
	snprintf(weatherDescription, WEATHER_DESC_LENGTH, "%s", weather_0_main);

	JsonObject main = doc["main"];
	tempInCentigrade = main["feels_like"];

	//Serial.printf("Weather: %s Temp: %f\n", weather_0_main, tempInCentigrade);

	return true;
}

float WeatherClient::getTempCentigrade()
{
	return tempInCentigrade;
}


void WeatherClient::getWeatherDescription(char* destination)
{
	snprintf(destination, WEATHER_DESC_LENGTH, "%s", weatherDescription);
}

