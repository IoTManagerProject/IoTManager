#include "Sensors/DallasSensor.h"

#include "Utils/TimeUtils.h"

DallasSensor::DallasSensor(const char* name, OneWireAddress addr) : _wire(onewire.get()),
                                                                    _addr{addr},
                                                                    _bitResolution{9},
                                                                    _requestConversationTime{0} {
    strncpy(_name, name, sizeof(_name));
}

const char* DallasSensor::name() {
    return _name;
}

OneWireAddress* DallasSensor::addr() {
    return &_addr;
}

bool DallasSensor::update() {
    if (!isConnected()) {
        return false;
    }

    switch (_state) {
        case DALLAS_IDLE:
            if (requestTemperature()) {
                _state = DALLAS_CONVERSATION;
                _requestConversationTime = millis();
                _hasValue = false;
            }
            break;
        case DALLAS_CONVERSATION:
            if (millis_since(_requestConversationTime) >= getWaitTimeForConversion()) {
                if (isConversionComplete()) {
                    _value = getTempC();
                    _hasValue = true;
                    _state = DALLAS_IDLE;
                }
            };
            break;
        default:
            break;
    };
    return _hasValue;
}

bool DallasSensor::isConversionComplete() {
    return _wire->read_bit();
}

bool DallasSensor::isConnected() {
    return readRawData() && isValid();
}

bool DallasSensor::isValid() {
    return !isZeros(_rawData) && (_wire->crc8(_rawData, 8) == _rawData[CRC_DATA_BYTE]);
}

bool DallasSensor::isZeros(uint8_t* data, const size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (*(data + i) != 0) {
            return false;
        }
    }
    return true;
}

// convert from raw to Celsius
float DallasSensor::rawToCelsius(int16_t raw) {
    if (raw <= DEVICE_DISCONNECTED_RAW) {
        return DEVICE_DISCONNECTED_C;
    }
    // C = RAW/128
    return (float)raw * 0.0078125;
}

bool DallasSensor::readRawData() {
    if (!_wire->reset()) {
        return false;
    };

    _wire->select(_addr.get());
    _wire->write(READSCRATCH);

    for (uint8_t i = 0; i < 9; i++) {
        _rawData[i] = _wire->read();
    }

    return (_wire->reset() == 1);
}

float DallasSensor::rawToFahrenheit(int16_t raw) {
    if (raw <= DEVICE_DISCONNECTED_RAW)
        return DEVICE_DISCONNECTED_F;
    // C = RAW/128
    // F = (C*1.8)+32 = (RAW/128*1.8)+32 = (RAW*0.0140625)+32
    return ((float)raw * 0.0140625) + 32;
}

int16_t DallasSensor::calculateTemperature() {
    int16_t fpTemperature = (((int16_t)_rawData[TEMP_MSB]) << 11) | (((int16_t)_rawData[TEMP_LSB]) << 3);

    if ((_addr.at(0) == DS18S20MODEL) && (_rawData[COUNT_PER_C] != 0)) {
        fpTemperature = ((fpTemperature & 0xfff0) << 3) - 32 + (((_rawData[COUNT_PER_C] - _rawData[COUNT_REMAIN]) << 7) / _rawData[COUNT_PER_C]);
    }

    return fpTemperature;
}

int16_t DallasSensor::getTemp() {
    if (isConnected()) {
        return calculateTemperature();
    } else {
        return DEVICE_DISCONNECTED_RAW;
    }
}

float DallasSensor::getTempC() {
    return rawToCelsius(getTemp());
}

float DallasSensor::getTempF() {
    return rawToFahrenheit(getTemp());
}

void DallasSensor::setAddr(OneWireAddress addr) {
    _addr = addr;
}

bool DallasSensor::requestTemperature() {
    if (!_bitResolution) {
        return false;
    }

    _wire->reset();
    _wire->select(_addr.get());
    _wire->write(STARTCONVO, _parasite);

    return true;
}

uint16_t DallasSensor::getWaitTimeForConversion() {
    switch (_bitResolution) {
        case 9:
            return 94;
        case 10:
            return 188;
        case 11:
            return 375;
        default:
            return 750;
    }
}