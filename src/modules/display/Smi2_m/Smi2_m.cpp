
#include "Global.h"
#include "classes/IoTItem.h"

#include "Modbus_master_for_Smi2_m.h"

// Общая сумма доступной памяти на ведущем устройстве, чтобы хранить данные
#define TOTAL_NO_OF_REGISTERS 4

// This is the easiest way to create new packets
// Add as many as you want. TOTAL_NO_OF_PACKETS
// is automatically updated.
enum {
    PACKET1,
    TOTAL_NO_OF_PACKETS  // leave this last entry
};

// Масив пакетов модбус
Packet packets[TOTAL_NO_OF_PACKETS];

// Массив хранения содержимого принятых и передающихся регистров
/*unsigned*/ int regs[TOTAL_NO_OF_REGISTERS];

class Smi2_m : public IoTItem {
   private:
    Smi_display* smi;
    unsigned int _pin;  // номер порта, управляющий передачей по RS-485
    long int _baud;     // скорость обмена, бит/с
    int _rx, _tx;       // номера ножек мк, к которым подключен UART
    String _show;
    int i = 10;

   public:
   public:
    Smi2_m(String parameters) : IoTItem(parameters) {
        smi = new Smi_display();

        _pin = jsonReadInt(parameters, "pin");
        _baud = jsonReadLInt(parameters, "baud");
        _rx = jsonReadInt(parameters, "rx");
        _tx = jsonReadInt(parameters, "tx");
        // Настраиваем пакеты
        //  Шестой параметр - это индекс ячейки в массиве, размещенном в памяти ведущего устройства, в которую будет
        //  помещен результат или из которой будут браться данные для передачи в подчиненное устройство. В нашем коде - это массив reg
        //  Пакет,SLAVE адрес,функция модбус,адрес регистра,количесво запрашиваемых регистров,локальный адрес регистра.
        //  Пакет,SLAVE адрес,функция модбус,адрес регистра,данные,локальный адрес регистра.
        smi->modbus_construct(&packets[PACKET1], 1, PRESET_MULTIPLE_REGISTERS, 4200, 1, 0);
        smi->modbus_configure(&Serial, _baud, SERIAL_8N1, _rx, _tx, _pin, packets, TOTAL_NO_OF_PACKETS, regs);

        jsonRead(parameters, "id2show", _show);
    }

    void doByInterval() {
        if (smi) {
            smi->modbus_update();

            float tmpStr = getItemValue(_show).toFloat();
            regs[0] = tmpStr * 10;
        }
    }

    ~Smi2_m(){};
};

void* getAPI_Smi2_m(String subtype, String param) {
    if (subtype == F("Smi2_m")) {
        return new Smi2_m(param);
    } else {
        return nullptr;
    }
}
