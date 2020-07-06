#pragma once

#include <Arduino.h>

#include "BLEDevice.h"

#define SCAN_TIME_SECS 10

#define MILLIS_BETWEEN_FURBY_TICK 500

#define MILLIS_BETWEEN_FURBY_RETRY 5000

 class Furby :BLEAdvertisedDeviceCallbacks, BLEClientCallbacks
{
public:

	Furby();

	bool connect();

	bool disconnect();

	static bool loop();

	void bindToFurbyStatusMessages(void (*receiveStatus)(const char* statusString));
	void unbindFromFurbyStatusMessages();

	void bindToIncomingFurbyMessages(void (*receiveMessage)(unsigned char* buffer, unsigned int length));
	void unbindFromIncomingFurbyMessages();

	bool sendMessageToFurby(unsigned char * buffer, unsigned int length);

	bool connected();

	enum furbyConnectStates { turnedOn,  furbyConnected, furbyDisconnected, turnedOff };

private:

	static Furby * theFurby;


	// Overridden methods from BLEClientCallbacks
	void onConnect(BLEClient* pclient);

	void onDisconnect(BLEClient* pclient);

	// Overridden methonds from BLEAdvertisedDeviceCallbacks
	void onResult(BLEAdvertisedDevice advertisedDevice);

	void connectedToDevice(BLEAdvertisedDevice advertisedDevice);

	static void notifyGPWCallback(
		BLERemoteCharacteristic* pBLERemoteCharacteristic,
		uint8_t* pData,
		size_t length,
		bool isNotify);

	static void notifyGPLCallback(
		BLERemoteCharacteristic* pBLERemoteCharacteristic,
		uint8_t* pData,
		size_t length,
		bool isNotify);

	void sendMessageFromFurby(unsigned char* buffer, unsigned int length);

	void (*boundStatusReceiver)(const char* message) = NULL;
	void (*boundMessageReceiver)(unsigned char* buffer, unsigned int length) = NULL;

	BLEAdvertisedDevice* furbyDevice;

	furbyConnectStates furbyState;

	BLEClient* furbyClient;

	BLEUUID FurbyServiceUUID;
	BLEUUID charGPW;
	BLEUUID charGPL;

	BLERemoteCharacteristic* pGPWRemoteCharacteristic;
	BLERemoteCharacteristic* pGPLRemoteCharacteristic;

	unsigned long millisAtLastUpdate;

	void sendStatus(const char* message);
	void sendHexStatus(char * buff, int length);
	void sendHexString(std::string str);

 };


