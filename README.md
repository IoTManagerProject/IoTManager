# esp32-esp8266_iot-manager_modules_firmware

It is based on esp8266 and esp32 automation system. Each module has a web interface for its initial configuration. The modules are managed using the "iot manager" application. This application is released for both android and ios platform. The application connects to a MQTT broker, you can add several brokers and switch between them. All modules also connect to same broker. All esp8266 esp32 are combined and appear in the application as widgets.

There are two configuration options in the web interface of the modules:

1. Select a ready-made preset. 

The following presets are available:

- On off relay   
- On off relay according to the schedule specified in the application   
- On off the relay after a period of time specified in the application   
- On off relay groups on different devices with one button in the application   
- Light switch module   
- PWM controlled by the slider in the application   
- Reading and logging analog input into graph with scaling function   
- Reading and logging in the graph of the following sensors:   

DHT22, DHT33, DHT44, AM2302, RHT03
DS18B20
JSN-SR04T, HC-SR04, HY-SRF05
BME280 BMP280 and other i2c sensors

-ds18b20 termostat controlled from application with graph (you can use any sensor for termostat any other supported).

2. Configure with special scripts. A simple programming language was invented which can very flexibly configure the module.

People who do not know how to program can use ready-made presets (option 1), and people who want to play with the system can use scripts (option 2).

Scenarios:

The web interface has the ability to configure Scenarios. An event occurs on one esp, and a reaction to this event can setup to occurs on another.

Logging of sensors data in this project made with out any server. All data for graf storring in esp flash. You can look any time your sensor history for 2 - 3 days or week in mobile app in graf.  And for this option needed only esp.


