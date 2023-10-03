#include "Global.h"
#include "classes/IoTItem.h"


extern IoTGpio IoTgpio;

class Buzzer : public IoTItem
{
private:
    int _pin;
    int _freq;
    int _duration;
    int _beatLevel = 4;
    int _originalTempo = 120;   // bpm
    float _tempoCorrection = 1; // less than 1 - is slower
    float _pauseBetween = 0;
    int _transpose = 0;
    int _cycle = 0;
    int _indication = 1;

    std::vector<int> tonesList;     // playMode 3
    std::vector<String> melodyList; // playMode 4

    int playMode = 0;

    bool firstTone = true;
    int thisTone = 0;
    int startPlayPrevTone = 0;
    int pauseBetweenTones = 0;
    int toneFrequency = 0;
    int toneDuration = 0;

    bool firstNote = true;
    int thisNote = 0;
    bool pause = false;
    int A4 = 440; // Ля 4-ой октавы
    int freqToPlay = 440;
    int prevFreqToPlay = 440;
    float melodyFloat = 1;
    int startPlayPrevNote = 0;
    int pauseBetweenNotes = 0;
    float intervalC0 = 69;     //
    float prevIntervalC0 = 69; //

    float natureInt[13] = {1, 1.0416666666667, 1.125, 1.2, 1.25, 1.3333333333, 1.3888888889, 1.5, 1.6, 1.6666666667, 1.8, 1.875, 2}; // New Nature
    String notes[12] = {"C", "CS", "D", "DS", "E", "F", "FS", "G", "GS", "A", "AS", "B"};
    float minorSecond = 1.0416666666667;
    float risingMinorSecond = 1.066667;
    float majorSecond = 1.125;
    float decreasingMajorSecond = 1.1111111;

public:
    Buzzer(String parameters) : IoTItem(parameters)
    {
        _interval = _interval / 1000; // корректируем величину интервала int, теперь он в миллисекундах

        jsonRead(parameters, "pin", _pin);
        jsonRead(parameters, "freq", _freq);
        jsonRead(parameters, "duration", _duration);
        jsonRead(parameters, "beatLevel", _beatLevel);
        jsonRead(parameters, "tempo", _originalTempo);
        jsonRead(parameters, "tempoCorrection", _tempoCorrection);
        jsonRead(parameters, "pauseBetween", _pauseBetween);
        jsonRead(parameters, "transpose", _transpose);
        jsonRead(parameters, "cycle", _cycle);
        jsonRead(parameters, "indication", _indication);

        IoTgpio.pinMode(_pin, OUTPUT);
    }

    void loop()
    {

        switch (playMode)
        {
        case 0: // for stop
                // noTone(_pin);
            break;

        case 1:
            // for doByIntervals
            IoTItem::loop();
            break;

        case 2:
            // for tone
            break;
        case 3:
            // for  tones() ...................................
            if (((millis() - startPlayPrevTone) > pauseBetweenTones))
            {
                if (firstTone)
                {
                    firstTone = false;
                    if (_indication)
                    {
                        value.valD = 1;
                        regEvent((String)(int)value.valD, "Buzzer", false, false);
                    }
                }
                else
                {
                    thisTone = thisTone + 2;
                }

                if (thisTone >= tonesList.size()) // если сиграли последнюю ноту
                {
                    if (_cycle)
                    {
                        thisTone = 0;
                    }
                    else
                    {
                        playMode = 0;
                        if (_indication)
                        {
                            value.valD = 0;
                            regEvent((String)(int)value.valD, "Buzzer", false, false);
                        }
                        break;
                    }
                }

                toneFrequency = tonesList[thisTone];
                toneDuration = tonesList[thisTone + 1];
                //  noTone(_pin); // останавливаем, на всякий случай воспроизведение предыдущей ноты
                tone(_pin, toneFrequency, toneDuration);
                startPlayPrevTone = millis();
                pauseBetweenTones = toneDuration * (1 + _pauseBetween);
            }
            break;

        case 4:
            // FOR melody ............................................
            if (((millis() - startPlayPrevNote) > pauseBetweenNotes))
            {
                if (firstNote)
                {
                    if (_indication)
                    {
                        value.valD = 1;
                        regEvent((String)(int)value.valD, "Buzzer", false, false);
                    }
                }
                else
                {
                    thisNote++;
                }

                if (thisNote >= melodyList.size()) // если сыграли последнюю ноту
                {
                    if (_cycle)
                    {
                        thisNote = 0;
                    }
                    else
                    {
                        playMode = 0;
                        if (_indication)
                        {
                            value.valD = 0;
                            regEvent((String)(int)value.valD, "Buzzer", false, false);
                        }
                        break;
                    }
                }
                String melodyString = melodyList[thisNote];
                String fore, aft;
                for (int i = 0; i < melodyString.length(); i++)
                {
                    if (melodyString.charAt(i) == '.')
                    {
                        fore = melodyString.substring(0, i); // получили строку до точки
                        aft = melodyString.substring(i + 1); // получили строку после точки
                        break;
                    }
                }
                char last = fore.charAt(fore.length() - 1); // получили номер октавы
                int octaveN = (int)(last - '0');
                String note = fore.substring(0, fore.length() - 1); // получили ноту
                int noteindex;
                pause = true;
                for (int j = 0; j < 12; j++)
                {
                    if (note == notes[j]) // если находим ноту, значит не пауза
                    {
                        noteindex = j;
                        pause = false;
                        break;
                    }
                }
                float noteDurationFloat = aft.toFloat(); // в миллидолях такта
                int noteDuration = int(noteDurationFloat / (float(_originalTempo) / (_beatLevel * 60)) / _tempoCorrection); // в миллисекундах
                if (!pause) // находим абсолютный интервал у ноты, паузу пропускаем
                {
                    // номер октавы на нот в октаве                  номер ноты в октаве в целой части и длительность в дробной
                    intervalC0 = (octaveN + 1) * 12 + noteindex + _transpose;
                    pause = false;
                    int relIntervalC0 = int(intervalC0) - int(prevIntervalC0); // находим интервал от предыдущей ноты
                    float intervalA4 = 69 - int(intervalC0);
                    int monoFreqToPlay = A4 / pow(2.0, (intervalA4 / 12)); // расчет частоты по монохроматической гамме
                    if (firstNote) //  для первой ноты не делаем перерасчет, играем как есть
                    {
                        freqToPlay = monoFreqToPlay;
                    }
                    else
                    {
                        if (relIntervalC0 == 0) // играем туже ноту
                        {
                            freqToPlay = prevFreqToPlay;
                        }
                        // далее расчет частоты новой ноты в природном интервале от предыдущей
                        if (relIntervalC0 > 0)
                        {
                            freqToPlay = prevFreqToPlay * (int(abs(relIntervalC0 / 12)) + natureInt[abs(relIntervalC0 - int(relIntervalC0 / 12) * 12)]);
                        }
                        if (relIntervalC0 < 0)
                        {
                            freqToPlay = prevFreqToPlay / (int(abs(relIntervalC0 / 12)) + natureInt[abs(relIntervalC0 - int(relIntervalC0 / 12) * 12)]);
                        }
                        // это приводит к рассогласованию нот и убеганию в конце произведения
                        if (freqToPlay < monoFreqToPlay) // делаем корректировку убегания с помощью выбора малых или больших секунд
                        {
                            natureInt[1] = risingMinorSecond;
                            natureInt[2] = majorSecond;
                        }
                        else
                        {
                            natureInt[2] = decreasingMajorSecond;
                            natureInt[1] = minorSecond;
                        }
                    }
                }
                else // если пауза
                {
                    intervalC0 = prevIntervalC0;
                    freqToPlay = 0;
                    pause = false;
                }
               // noTone(_pin); // останавливаем, на всякий случай воспроизведение предыдущей ноты
                if (freqToPlay < 31)
                    freqToPlay = 0;
                // воспроизведём ноту
                tone(_pin, freqToPlay, noteDuration);
                startPlayPrevNote = millis();
                if (freqToPlay) // нулевые значения частоты (паузы) не сохраняем
                {
                    prevFreqToPlay = freqToPlay; // if it was not 0
                }
                prevIntervalC0 = intervalC0;
                /*
                  Чтобы отделить ноты друг от друга, добавим небольшую паузу между ними
                  около 0.3 от длины ноты звучат неплохо
                */
                pauseBetweenNotes = noteDuration * (1 + _pauseBetween);
                if (firstNote)
                    firstNote = false;
            }
            break;

        default:
            break;
        }
    }

    void doByInterval()
    {
        tone(_pin, _freq, _duration);
        //  regEvent(value.valD, "Buzzer");
    }

    IoTValue execute(String command, std::vector<IoTValue> &param)
    {
        // param - вектор ("массив") значений параметров переданных вместе с командой: ID.Команда("пар1", 22, 33) -> param[0].ValS = "пар1", param[1].ValD = 22
        if (command == "tone")
        {
            playMode = 2;
            tone(_pin, param[0].valD, param[1].valD);
            SerialPrint("I", "Sensor Buzzer, tone(), playMode = ", String(playMode), _id);
        }
        else if (command == "tones")
        {
            int sizeOfParam = param.size();
            tonesList.clear();
            tonesList.reserve(sizeOfParam);
            for (unsigned int i = 0; i < sizeOfParam; i++)
            {
                tonesList.push_back(param[i].valD);
            }
            playMode = 3;
            SerialPrint("I", "Sensor Buzzer, tones(), playMode = ", String(playMode), _id);
            startPlayPrevTone = 0;
            pauseBetweenTones = 0;
            firstTone = true;
            thisTone = 0;
            prevFreqToPlay = 440;
            prevIntervalC0 = 69;
        }
        else if (command == "notone")
        {
            playMode = 0;
            noTone(_pin);
            if (_indication)
            {
                value.valD = 0;
                regEvent((String)(int)value.valD, "Buzzer", false, false);
            }
            SerialPrint("I", "Sensor Buzzer, notone(), playMode = ", String(playMode), _id);
        }
        else if (command == "melodySetting")
        {
            if (param[0].isDecimal)
                _beatLevel = param[0].valD;
            if (param[1].isDecimal)
                _originalTempo = param[1].valD;
            if (param[2].isDecimal)
                _tempoCorrection = param[2].valD;
            if (param[3].isDecimal)
                _pauseBetween = param[3].valD;
            if (param[4].isDecimal)
                _transpose = param[4].valD;
            if (param[5].isDecimal)
                _cycle = param[5].valD;
            String input = "beatLevel = " + String(_beatLevel) + "  originalTempo = " + String(_originalTempo) + "  tempoCorrection = " + String(_tempoCorrection) + "  pauseBetween = " + String(_pauseBetween) + "  transpose = " + String(_transpose) + "  cycle = " + String(_cycle);
            SerialPrint("I", "Sensor Buzzer, melodySetting() exec", input, _id);
        }
        else if (command == "melody")
        {
            int sizeOfParam = param.size();
            melodyList.clear();
            melodyList.reserve(sizeOfParam);
            for (unsigned int i = 0; i < sizeOfParam; i++)
            {
                melodyList.push_back(param[i].valS);
            }
            playMode = 4;
            SerialPrint("I", "Sensor Buzzer, melody(), playMode = ", String(playMode), _id);
            thisNote = 0;
            firstNote = true;
            startPlayPrevNote = 0;
            pauseBetweenNotes = 0;
            prevFreqToPlay = 440;
            prevIntervalC0 = 69;
        }
        return {}; // команда поддерживает возвращаемое значения. Т.е. по итогу выполнения команды или общения с внешней системой, можно вернуть значение в сценарий для дальнейшей обработки
    }

    void setValue(const IoTValue &Value, bool genEvent = true)
    {
        value = Value;
        playMode = value.valD;
        regEvent((String)(int)value.valD, "Buzzer", false, genEvent);
    }
    //=======================================================================================================
    String getValue()
    {
        return (String)(int)value.valD;
    }

    ~Buzzer(){};
};

void *getAPI_Buzzer(String subtype, String param)
{
    if (subtype == F("Buzzer"))
    {
        return new Buzzer(param);
    }
    else
    {
        return nullptr;
    }
}
