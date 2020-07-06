#include "Furby.h"


/**
 * A Furby remote control manager for ESP32.
 *
 * Built using the information at: https://github.com/Jeija/bluefluff
 * which is awesome.
 *
 * based on the BLE Client by chegewara that is part of the ESP32 BLE examples
 * Rob Miles 2020
 *
 */


// Overridden methods from BLEClientCallbacks

void Furby::onConnect(BLEClient* client) {
    sendStatus("onConnect");
}

void Furby::onDisconnect(BLEClient* client) {
    sendStatus("onDisconnect");
    furbyState = furbyDisconnected;
}


void Furby::onResult(BLEAdvertisedDevice advertisedDevice) {
    sendStatus("BLE Advertised Device found: ");
    sendStatus(advertisedDevice.toString().c_str());

    // Found a device. Is it a Furby?
    if (advertisedDevice.haveServiceUUID() && 
        advertisedDevice.isAdvertisingService(FurbyServiceUUID)) {
        // found a Furby!
        BLEDevice::getScan()->stop();
        furbyDevice = new BLEAdvertisedDevice(advertisedDevice);
    }
}


// Need a static member to refer to this Furby because
// the callback methods need to be static

Furby* Furby::theFurby = NULL;

void Furby::notifyGPWCallback(
    BLERemoteCharacteristic* characteristic,
    uint8_t* pData,
    size_t length,
    bool isNotify) {
    theFurby->sendStatus("Notify callback for GPWR ");
    theFurby->sendHexStatus((char*)pData, length);
}

void Furby::notifyGPLCallback(
    BLERemoteCharacteristic* characteristic,
    uint8_t* data,
    size_t length,
    bool isNotify) {
    theFurby->sendStatus("Notify callback for GPL ");
    theFurby->sendHexStatus((char*)data, length);
    theFurby->sendMessageFromFurby((unsigned char *) data, length);
}

Furby::Furby()
{
    theFurby = this;

    BLEDevice::init("Furby Control");

    // This is the "Fluff" service - note that you have to add the - characters to the UUIDs
    FurbyServiceUUID = BLEUUID("dab91435-b5a1-e29c-b041-bcd562613bde");

    // This is the characteristic for GeneralPlusWrite
    charGPW = BLEUUID ("dab91383-b5a1-e29c-b041-bcd562613bde");

    // This is the characteristic for GeneralPlusListen
    charGPL = BLEUUID ("dab91382-b5a1-e29c-b041-bcd562613bde");

    boundMessageReceiver = NULL;
    boundStatusReceiver = NULL;
}


void Furby::connectedToDevice(BLEAdvertisedDevice advertisedDevice)
{
    sendStatus("Connected to BLE device");
}


bool Furby::connect()
{
    sendStatus("Connect to Furby starting scan");

    furbyDevice = NULL;
    furbyState = furbyDisconnected;

    // Retrieve a Scanner and set the callback we want to use to be informed when we
    // have detected a new device.  Specify that we want active scanning and start the
    // scan to run for 5 seconds.
    BLEScan* scanner = BLEDevice::getScan();      // get a scanner
    scanner->setAdvertisedDeviceCallbacks(this);  // connect it to our callbacks
    scanner->setInterval(1349);                   // interval between listening
    scanner->setWindow(449);                      // time to listen for
    scanner->setActiveScan(true);                 // send scan requests to servers
    scanner->start(SCAN_TIME_SECS, false);        // begin the scan

    sendStatus("Connect to Furby scan finished");

    // the onResult method will set myDevice if it finds a matching service

    if (furbyDevice == NULL)
    {
        sendStatus("Furby not found");
        return false;
    }

    // When we get here we have found our Furby - can connect to it

    sendStatus("Connecting to the Furby service");

    sendStatus("Forming a connection to ");

    sendStatus(furbyDevice->getAddress().toString().c_str());

    furbyClient = BLEDevice::createClient();

    sendStatus(" - Created client");

    furbyClient->setClientCallbacks(this);

    // Connect to the remote BLE Server.

    furbyClient->connect(furbyDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    sendStatus(" - Connected to server");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* furbyService = furbyClient->getService(FurbyServiceUUID);

    if (furbyService == nullptr) {
        sendStatus("Failed to find our service UUID: ");
        sendStatus(FurbyServiceUUID.toString().c_str());
        furbyClient->disconnect();
        return false;
    }

    sendStatus(" - Found our service");

    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pGPWRemoteCharacteristic = furbyService->getCharacteristic(charGPW);
    if (pGPWRemoteCharacteristic == nullptr) {
        sendStatus("Failed to find GPW UUID: ");
        sendHexString(charGPW.toString());
        furbyClient->disconnect();
        return false;
    }

    sendStatus(" - Found our GPW characteristic");

    // Read the value of the characteristic.
    if (pGPWRemoteCharacteristic->canRead()) {
        std::string value = pGPWRemoteCharacteristic->readValue();
        sendStatus("The GPW characteristic value was: ");
        sendHexString(value);
    }

    if (pGPWRemoteCharacteristic->canNotify()) {
        pGPWRemoteCharacteristic->registerForNotify(this->notifyGPWCallback);
        sendStatus("Can get GPW notifications");
    }


    /// Now the GPL characteristic

    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pGPLRemoteCharacteristic = furbyService->getCharacteristic(charGPL);
    if (pGPLRemoteCharacteristic == nullptr) {
        sendStatus("Failed to find GPL UUID: ");
        sendStatus(charGPL.toString().c_str());
        furbyClient->disconnect();
        return false;
    }

    sendStatus(" - Found our GPL characteristic");

    // Read the value of the characteristic.
    if (pGPLRemoteCharacteristic->canRead()) {
        std::string value = pGPLRemoteCharacteristic->readValue();
        sendStatus("The GPL characteristic value was: ");
        sendHexString(value);
    }

    if (pGPLRemoteCharacteristic->canNotify())
    {
        pGPLRemoteCharacteristic->registerForNotify(this->notifyGPLCallback);
        sendStatus("Can get GPL notifications");
    }

    millisAtLastUpdate = millis();

    furbyState = furbyConnected;

    return true;
}


bool Furby::loop()
{

    if(theFurby==NULL)
    {
        return false;
    }

    unsigned long millisAtLoop = millis();
    unsigned long millisSinceUpdate = millisAtLoop - theFurby->millisAtLastUpdate;

    switch (theFurby->furbyState)
    {

    case turnedOn:
        break;

    case furbyConnected:
        if (millisSinceUpdate > MILLIS_BETWEEN_FURBY_TICK)
        {
            // Need to ping the Furby to keep the connection alive
            // A null command will do it

            theFurby->sendStatus("tick");

            unsigned char emptyCommand[] = { 0x00 };
            theFurby->sendMessageToFurby(emptyCommand, 1);

            theFurby->millisAtLastUpdate = millisAtLoop;
        }
        break;

    case furbyDisconnected:
        break;

    case turnedOff:
        break;

    }
}


bool Furby::disconnect()
{
    byte doDisconnect[2] = { 0x20,0x05 };
    sendMessageToFurby(doDisconnect, 2);
    delay(100);
    furbyClient->disconnect();
}

void Furby::bindToFurbyStatusMessages(void (*receiveStatus)(const char* statusString))
{
    boundStatusReceiver = receiveStatus;
}

void Furby::unbindFromFurbyStatusMessages()
{
    boundStatusReceiver = NULL;
}


void Furby::sendStatus(const char* message)
{
    if (boundStatusReceiver != NULL)
        boundStatusReceiver(message);
}

void Furby::sendHexStatus(char * buf, int len)
{
    char buffer[20];

    for (int i = 0; i < len; i++)
    {
        snprintf(buffer, 20, "%2d  %c %x %d", i, buf[i], buf[i], buf[i]);
        sendStatus(buffer);
    }
}


void Furby::sendHexString(std::string buf)
{
    char buffer[20];

    for (int i = 0; i < buf.length(); i++)
    {
        snprintf(buffer, 20, "%2d  %c %x %d", i, buf[i], buf[i], buf[i]);
        sendStatus(buffer);
    }
}

void Furby::bindToIncomingFurbyMessages(void (*receiveMessage)(unsigned char* buffer, unsigned int length))
{
    boundMessageReceiver = receiveMessage;
}

void Furby::unbindFromIncomingFurbyMessages()
{
    boundMessageReceiver = NULL;
}

bool Furby::sendMessageToFurby(unsigned char  * buffer, unsigned int length)
{
    pGPWRemoteCharacteristic->writeValue(buffer, length);
    return true;
}

void Furby::sendMessageFromFurby(unsigned char* buffer, unsigned int length)
{
    if (boundMessageReceiver != NULL)
        boundMessageReceiver(buffer, length);
}

bool Furby::connected()
{
    return furbyState == furbyConnected;
}

