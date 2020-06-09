# FurbyESP32
A suite of classes that you can use to connect a Furby toy to an ESP32 and do interesting things with it:

- **Furby** creates and maintains a Bluetooth BLE connection to a Furby Connect toy.
- **WeatherClient** creates and maintains a connection to OpenWeatherMap
- **WiFiClient** creates and maintains a connection to WiFi
- **MQTT** creates and maintains a conneciton to an MQTT broker

All the classes are used in the same way. They have a connect function to open a connection to the particular service. The MQTT and Furby objects also provide a loop function that can be used to keep their respective connections alive.

## Example programs

- **WeatherFurby** reads the weather from OpenWeatherMap and uses it to generate a response from Furby. 
- **MQTTFurby** connects a Furby to an MQTT broker. 
- **M5StickCWeatherFurby** reads the weather from OpenWeatherMap and uses it to generate a response from Furby. It also shows status messages on the M5StickC display.

## ESP32 Program Memory

These programs use both Bluetooth BLE and WiFi on the ESP32 device. This makes them too large to run in smaller ESP32 devices (those with only 2M of EEPROM). It also means that you need to use the Tools Menu in the Arduino IDE to select the "Large" programs models which disables Over The Air updates and makes it possible to run larger programs.  

## Furby Commands

Many thanks to Florian Euchner whose detailed description of how Furby uses Bluetooth BLE made it possible to create the connection. 
You can find his detailed description of the Furby Bluetooth commands and what they do [here](https://github.com/Jeija/bluefluff ).

Have Fun

Rob Miles - June 2020
