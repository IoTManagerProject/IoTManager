#include "Global.h"
#include "classes/IoTItem.h"
#include <Arduino.h>
#include <PicoMQTT.h>

namespace _Broker
{
#define DEF_PORT 1883

    // MqttBroker broker(1883);

    class myPicoMQTT : public PicoMQTT::Server
    {
    private:
        bool _debug;
        String _user;
        String _pass;

    public:
        myPicoMQTT(int port) : PicoMQTT::Server(port)
        {
        }

        void setAuth(String user, String pass)
        {
            _user = user;
            _pass = pass;
        }

        void setDebug(bool debug)
        {
            _debug = debug;
        }

    protected:
        void on_connected(const char *client_id)
        {
            if (_debug)
            {
                Serial.print("[BrokerMQTT], Client connected: ");
                Serial.println(client_id);
            }
        }
        void on_disconnected(const char *client_id)
        {
            if (_debug)

            {
                // SerialPrint("i", "BrokerMQTT", "Client disconnected: " + client_id);
                Serial.print("[BrokerMQTT], Client disconnected: ");
                Serial.println(client_id);
            }
        }
        void on_subscribe(const char *client_id, const char *topic)
        {
            if (_debug)

            {
                // SerialPrint("i", "BrokerMQTT", "Client " + client_id + ", subscribe: " + topic);
                Serial.print("[BrokerMQTT], Client: ");
                Serial.print(client_id);
                Serial.print(", subscribe: ");
                Serial.println(topic);
            }
        }
        void on_unsubscribe(const char *client_id, const char *topic)
        {
            if (_debug)

            {
                // SerialPrint("i", "BrokerMQTT", "Client " + client_id + ", unsubscribe: " + topic);
                Serial.print("[BrokerMQTT], Client: ");
                Serial.print(client_id);
                Serial.print(", unsubscribe: ");
                Serial.println(topic);
            }
        }
        PicoMQTT::ConnectReturnCode auth(const char *client_id, const char *username, const char *password)
        {
            if (String(client_id).length() < 3)
            {
                return PicoMQTT::CRC_IDENTIFIER_REJECTED;
            }
            if (!username && !password)
            {
                return PicoMQTT::CRC_NOT_AUTHORIZED;
            }
            if (String(username) == _user && String(password) == _pass)
            {
                return PicoMQTT::CRC_ACCEPTED;
            }
            Serial.print("[BrokerMQTT], Client: ");
            Serial.print(client_id);
            Serial.print(", NOT Authorized: ");
            Serial.print(username);
            Serial.print(" != ");
            Serial.print(_user);
            Serial.print(" ,pass: ");
            Serial.print(password);
            Serial.print(" != ");
            Serial.println(_pass);
            return PicoMQTT::CRC_BAD_USERNAME_OR_PASSWORD;
        }
    };

    myPicoMQTT *picoMqtt = nullptr;
    PicoMQTT::Client *clientMqtt = nullptr;

    TaskHandle_t brokerTask;
    // void Task2code( void * pvParameters );

    void tBrokerMQTT(void *pvParameters)
    {
        TickType_t xLastWakeTime = xTaskGetTickCount();
        Serial.print("Task PicoMQTT running on core ");
        Serial.println(xPortGetCoreID());
        for (;;)
        {
            if (clientMqtt)
                clientMqtt->loop();
            if (picoMqtt)
                picoMqtt->loop();
            // picoMqtt.loop();
            // vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(5));
        }
    }

    class BrokerMQTT : public IoTItem
    {
    private:
        unsigned long ts = 0;
        int _port = 0;
        String _user;
        String _pass;
        bool _debug;
        bool _brige;
        String _server;
        String _srvUser;
        String _srvPass;
        int _srvPort;

    public:
        BrokerMQTT(String parameters) : IoTItem(parameters)
        {
            SerialPrint("i", F("BrokerMQTT"), " START...   ");
            jsonRead(parameters, "port", _port);
            jsonRead(parameters, "user", _user);
            jsonRead(parameters, "pass", _pass);
            jsonRead(parameters, "debug", _debug);
            jsonRead(parameters, "brige", _brige);
            jsonRead(parameters, "server", _server);
            jsonRead(parameters, "srvUser", _srvUser);
            jsonRead(parameters, "srvPass", _srvPass);
            jsonRead(parameters, "srvPort", _srvPort);

            if (_brige)
            {
                clientMqtt = new PicoMQTT::Client(_server.c_str(), _srvPort, nullptr, _srvUser.c_str(), _srvPass.c_str());
                if (_debug)
                {
                    SerialPrint("i", F("BrigeMQTT"), "Bridge mode : ON");
                    SerialPrint("i", F("BrigeMQTT"), "Bridge server: " + _server);
                    SerialPrint("i", F("BrigeMQTT"), "Bridge port: " + String(_srvPort));
                    SerialPrint("i", F("BrigeMQTT"), "Bridge user: " + _srvUser);
                    SerialPrint("i", F("BrigeMQTT"), "Bridge pass: " + _srvPass);
                }
            }
        }

        void doByInterval()
        {
            static bool flagOne = false;
            if (!flagOne)
            {
                if (!_port)
                    _port = DEF_PORT;
                picoMqtt = new myPicoMQTT(_port);
                picoMqtt->begin();
                picoMqtt->setDebug(_debug);
                picoMqtt->setAuth(_user, _pass);
                if (_brige && picoMqtt && clientMqtt)
                {
                    picoMqtt->subscribe("#", [](const char *topic, const char *message)
                                        { clientMqtt->publish(topic, message);
                                         SerialPrint("i", F("BrigeMQTT"), "client publish, topic: " + String(topic) + " msg: " + String(message) ); });
                }
                // picoMqtt.begin();
                xTaskCreatePinnedToCore(
                    tBrokerMQTT,  // Функция задачи.
                    "BrokerMQTT", // Имя задачи.
                    10000,        // Размер стека
                    NULL,         // Параметры задачи
                    0,            // Приоритет
                    &brokerTask,  // Дескриптор задачи для отслеживания
                    0);
                flagOne = true;
            }
        }

        // Основной цикл программы
        void loop()
        {
            IoTItem::loop();
        }

        ~BrokerMQTT()
        {
            vTaskDelete(brokerTask);
            delete picoMqtt;
            delete clientMqtt;
        }
    };
}

void *getAPI_BrokerMQTT(String subtype, String param)
{
    if (subtype == F("BrokerMQTT"))
    {
        return new _Broker::BrokerMQTT(param);
    }
    else
    {
        return nullptr;
    }
}
