#include "Global.h"
#include "classes/IoTBench.h"
#include <ArduinoJson.h>
// #include <map>

class BenchmarkLoad : public IoTBench
{
private:
    bool _log = false;
    uint32_t _loadP = 1; // период подсчета загруженности процессора

    uint32_t startLoad = 0; // время начало цикла loop
    uint32_t loadPrev = 0;  // время предыдущего подсчета benchmark
    uint32_t loadSum = 0;   // время выполнния всех циклов loop за период  _loadP
    float load = 0;           // загруженность процессора в процентах за период _loadP (loadSum / 1000) / _loadP * 100
    uint32_t count = 0;     // количестов циклов loop в сек в среднем за период _loadP

public:
    BenchmarkLoad(String parameters) : IoTBench(parameters)
    {
        // jsonRead(parameters, "log", _log);
        // jsonRead(parameters, "int", _loadP); // в минутах
        _loadP = _interval ; //* 1000
      //  SerialPrint("i", "Benchmark",
      //              "_interval: " + String(_interval) + " _loadP: " + String(_loadP));
        if (_loadP < 10000)
            _loadP = 10000;
    }

    void doByInterval()
    {
        printBenchmarkLoad();
    }

    void loop()
    {
        count++; 
        IoTItem::loop();
    }

    void preLoadFunction()
    {
        startLoad = micros(); // время начала выполнения одного цикла
    }
    void postLoadFunction()
    {
        loadSum += (micros() - startLoad); // высчитываем время выполнения одного цикла (после нагрузки) и прибавляем к сумме за вреям контроля _loadP
    }

    void printBenchmarkLoad()
    {
        load = (loadSum / 10ul) / _loadP; // (loadSum / 1000) / _loadP * 100

        SerialPrint("i", "Benchmark",
                    "CPU load time: " + String(loadSum) + "us, in RealTime: " + String((micros() - loadPrev)) + "us");
        SerialPrint("i", "Benchmark",
                    "CPU load in " + String(_loadP) + "ms :" + String((load)) + "%" +
                        " loop/sec: " + String(count / (_loadP / 1000)));
        loadPrev = micros(); //+= _loadP;
        loadSum = 0;
        count = 0;
    }

    IoTBench *getBenchmarkLoad()
    {
        return this;
    }
    ~BenchmarkLoad(){
        //   clearBenchConfig();
    };
};

class BenchmarkTask : public IoTBench
{
private:
    uint32_t _loadP = 1;
    bool _log = false;

public:
    BenchmarkTask(String parameters) : IoTBench(parameters)
    {
        //  jsonRead(parameters, "log", _log);
        // jsonRead(parameters, "int", _loadP); // в минутах
        _loadP = _interval;// * 1000;
        if (_loadP < 10000)
            _loadP = 10000;
    }

    void doByInterval()
    {
        printBenchmarkTask();
    }

    void preTaskFunction(const String &id)
    {
        if (banchItems.find(id) != banchItems.end())
        {
            banchItems[id]->loopTime = micros(); // micros();
        }
        else
        {
            banchItems[id] = new ItemBench;
            banchItems[id]->loopTime = micros(); // micros();
        }
    }
    void postTaskFunction(const String &id)
    {
        if (banchItems.find(id) != banchItems.end())
        {
            banchItems[id]->loopTime = micros() - banchItems[id]->loopTime;
            banchItems[id]->sumloopTime += banchItems[id]->loopTime;
            if (banchItems[id]->loopTime > banchItems[id]->loopTimeMax_glob)
                banchItems[id]->loopTimeMax_glob = banchItems[id]->loopTime;
            if (banchItems[id]->loopTime > banchItems[id]->loopTimeMax_p)
                banchItems[id]->loopTimeMax_p = banchItems[id]->loopTime;
        }
    }

    void printBenchmarkTask()
    {
        for (auto it = banchItems.begin(); it != banchItems.end(); it++)
        {
            SerialPrint(
                "i", "Benchmark",
                " load (" + String((float)(it->second)->sumloopTime / 10ul / _loadP) + "%) " +
                    " max: per (" + String((it->second)->loopTimeMax_p) + "us)" +
                    " glob (" + String((it->second)->loopTimeMax_glob) + "us) - " + it->first);
            (it->second)->sumloopTime = 0;
            (it->second)->loopTimeMax_p = 0;
        }
    }

    void clearBenchConfig()
    {
        for (auto it = banchItems.begin(); it != banchItems.end(); it++)
        {
            delete it->second;
        }
        banchItems.clear();
    }
    IoTBench *getBenchmarkTask()
    {
        return this;
    }
    ~BenchmarkTask()
    {
        clearBenchConfig();
    };
};

void *getAPI_Benchmark(String subtype, String param)
{
    if (subtype == F("loadBench"))
    {
        return new BenchmarkTask(param);
    }
    else if (subtype == F("taskBench"))
    {
        return new BenchmarkLoad(param);
    }
    else
    {
        return nullptr;
    }
}
