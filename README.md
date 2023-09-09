# IoTManager

This is a smart home based on esp8266 and esp32 microcontrollers. These microcontrollers gained their popularity due to their low cost. Each such microcontroller is able to connect to your home wifi router. They can be purchased at any robotics store or on aliexpress, there are also ready-made devices based on them. This microcontroller has a certain number of pins on which digital signals are generated. Various peripheral devices can be connected to it: sensors, relays, stepper motors, servo drives, etc.

Our firmware allows you to receive data from all these devices and manage them. The iot manager app available for ios and android is used to display the data. In order to connect devices and the application, a special mqtt server is needed, in other words, an mqtt broker. All devices are first connected to a wifi router, and then to this mqtt broker, the application is also connected to it. As a result, through the application you can manage devices from anywhere in the world, monitor sensor readings, build graphs and much more. Broker mqtt can be used in the cloud, such as wqtt.ru, or your own, raised, for example, on a single-board computer raspberry pi. There is also a second way to manage devices, it works even when you do not have the Internet - control through a web browser. All your devices will be available on one page. Both methods, through the application or through the web, work simultaneously with full mutual synchronization.

To achieve your goal, you only need three things:

1. Buy an esp microcontroller
2. Download the app
3. Get a cloud broker

If remote control and the application are not needed, then the last step can be omitted.

The logic of each device is configured using scripts. They are needed in order to teach the device to carry out your invented algorithms. You can assign any reaction to any action. The temperature has risen - the device will turn off the heater. Humidity has fallen and the level in the tank is more than 10% - the device will start watering, if not, it will send you a telegram notification that there is not enough water. These are just a few examples. Scenarios are created by you, and their flexibility will allow you to fulfill your every desire.
