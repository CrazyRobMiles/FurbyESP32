#include "Furby.h"
#include "WiFiConnection.h"
#include "WeatherClient.h"

Furby* myFurby;
WiFiConnection* wifiConnection;
WeatherClient* weatherClient;
bool wantWeather = false;
bool wantTemp = false;
bool wantShutdown = false;

void messageDump(unsigned char* buffer, unsigned int length)
{
	Serial.print("Message from Furby: ");
	for (int i = 0; i < length; i++) {
		Serial.print(buffer[i], HEX);
		Serial.print(" ");
	}
	Serial.println();
}

void sayWeather()
{
	if (weatherClient->update(5))
	{

		unsigned char message[] = { 0x13, 0x00, 34, 1, 0, 2 };
		int len = sizeof(message) / sizeof(unsigned char);
		myFurby->sendMessageToFurby(message, len);

		delay(4000);

		Serial.println("Got weather!");

		char weatherDescription[WEATHER_DESC_LENGTH];

		weatherClient->getWeatherDescription(weatherDescription);

		Serial.printf("Description: %s\n", weatherDescription);

		bool playedSound = false;

		if (strcasecmp(weatherDescription, "clouds") == 0)
		{
			unsigned char message[] = { 0x13, 0x00, 1, 2, 1, 0 };
			int len = sizeof(message) / sizeof(unsigned char);
			myFurby->sendMessageToFurby(message, len);
			playedSound = true;
		}

		if (strcasecmp(weatherDescription, "drizzle") == 0)
		{
			// cloudy
			unsigned char message[] = { 0x13, 0x00, 2, 0, 1, 3 };
			int len = sizeof(message) / sizeof(unsigned char);
			myFurby->sendMessageToFurby(message, len);
			playedSound = true;
		}

		if (strcasecmp(weatherDescription, "rain") == 0)
		{
			// got rain
			unsigned char message[] = { 0x13, 0x00, 7, 1, 0, 0 };
			int len = sizeof(message) / sizeof(unsigned char);
			myFurby->sendMessageToFurby(message, len);
			playedSound = true;
		}
		if (!playedSound)
		{
			unsigned char message[] = { 0x13, 0x00, 2, 1, 0, 6 };
			int len = sizeof(message) / sizeof(unsigned char);
			myFurby->sendMessageToFurby(message, len);
		}
	}
}

void sayTemp()
{
	if (weatherClient->update(5))
	{

		unsigned char message[] = { 0x13, 0x00, 34, 1, 0, 3 };
		int len = sizeof(message) / sizeof(unsigned char);
		myFurby->sendMessageToFurby(message, len);

		delay(5000);

		Serial.println("Got temperature!");

		float temp = weatherClient->getTempCentigrade();

		if (temp < 10)
		{
			Serial.println("Cold");
			unsigned char message[] = { 0x13, 0x00, 56, 1, 3, 0 };
			int len = sizeof(message) / sizeof(unsigned char);
			myFurby->sendMessageToFurby(message, len);
		}
		else
		{
			if (temp < 16)
			{
				Serial.println("Mild");
				unsigned char message[] = { 0x13, 0x00, 57, 2, 0, 2 };
				int len = sizeof(message) / sizeof(unsigned char);
				myFurby->sendMessageToFurby(message, len);
			}
			else
			{
				Serial.println("Warm");
				unsigned char message[] = { 0x13, 0x00, 56, 3, 9, 0 };
				int len = sizeof(message) / sizeof(unsigned char);
				myFurby->sendMessageToFurby(message, len);
			}
		}
	}
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

void switchDecode(unsigned char* buffer, unsigned int length)
{
	messageDump(buffer, length);

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
			wantWeather = true;
			return;
		}

		if (buffer[2] & 0x80) {
			//antenna back
			wantTemp = true;
			return;
		}

		if (buffer[2] & 0x10) {
			// pulled tail
			wantShutdown = true;
			return;
		}
	}
}

void setup() {

	Serial.begin(115200);

	Serial.println(
		"Furby Weather Station\n\n"
		"Make sure that your Furby is awake before running the program\n"
		"Once it is connected you can get the weather by moving the antenna.\n"
		"Pull the antenna forwards for the weather and push it back for the\n"
		"temperature. Pull Furby's tail to disconnect Furby from the ESP32\n\n");

	myFurby = new Furby();

	while (!myFurby->connect())
	{
		Serial.print('.');
		delay(500);
	}

	Serial.println("Got a Furby!");

	myFurby->bindToIncomingFurbyMessages(switchDecode);

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

	delay(500);
	weatherClient = new WeatherClient();

    while (!weatherClient->connect("Hull", "uk", "your app key", 5))
	{
		Serial.print(".");
		delay(500);
	}

	Serial.println("Got Weather service");

	// Turn light green
	unsigned char greenMessage[] = { 0x14, 0x00, 0xFF, 0x00 };
	myFurby->sendMessageToFurby(greenMessage, 4);

	delay(500);

	// Say connected
	unsigned char connectedMessage[] = { 0x13, 0x00, 34, 0x00, 0x00, 0x00 };
	myFurby->sendMessageToFurby(connectedMessage, 6);

}

void loop() {

	myFurby->loop();

	if (wantWeather)
	{
		sayWeather();
		wantWeather = false;
	}

	if (wantTemp)
	{
		sayTemp();
		wantTemp = false;
	}

	if (wantShutdown)
	{
		shutdown();
		wantShutdown = false;
	}

	delay(100);
}
