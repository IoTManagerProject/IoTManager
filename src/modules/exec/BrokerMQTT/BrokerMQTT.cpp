#include "Global.h"
#include "classes/IoTItem.h"
#include <Arduino.h>
#include <PicoMQTT.h>

namespace _Broker
{
#define DEF_PORT 1883

    class myPicoMQTT : public PicoMQTT::Server
    {
    private:
        bool _debug;

    public:
        myPicoMQTT(int port) : PicoMQTT::Server(port)
        {
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
    };
    // MqttBroker broker(1883);
    myPicoMQTT *picoMqtt = nullptr;

    myPicoMQTT *instanceBroker(int port)
    {
        if (!picoMqtt)
        {
            picoMqtt = new myPicoMQTT(port);
            // ot->begin();
        }
        return picoMqtt;
    }

    TaskHandle_t brokerTask;
    // void Task2code( void * pvParameters );

    void tBrokerMQTT(void *pvParameters)
    {
        TickType_t xLastWakeTime = xTaskGetTickCount();
        Serial.print("Task PicoMQTT running on core ");
        Serial.println(xPortGetCoreID());
        for (;;)
        {
            instanceBroker(DEF_PORT)->loop();
            // picoMqtt.loop();
            // vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(5));
        }
    }

    class BrokerMQTT : public IoTItem
    {
    private:
        unsigned long ts = 0;
        int _port = 0;
        bool _debug;

    public:
        BrokerMQTT(String parameters) : IoTItem(parameters)
        {
            SerialPrint("i", F("BrokerMQTT"), " START...   ");
            jsonRead(parameters, "port", _port);
            jsonRead(parameters, "debug", _debug);
        }

        void doByInterval()
        {
            static bool flagOne = false;
            if (!flagOne)
            {
                if (!_port)
                    _port = DEF_PORT;
                instanceBroker(_port)->begin();
                instanceBroker(_port)->setDebug(_debug);
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
            // delete picoMqtt;
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
