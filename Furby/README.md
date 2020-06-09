# Furby

The Furby class provides an interface to a Furby over BLE. Create an instance of the Furby class and then use methods to connect to the Furby, send messages and bind a message receiver that is called when a message is received by the Furby. There is also a loop method that should be called regularly to keep the Furby BLE connection alive. 

This version will only control one Furby; the first one it detects.

To use this class just copy Furby.h and Furby.cpp into your Arduino sketch folder.

There are example programs that show how it is used here. 