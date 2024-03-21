/*
OpenTherm.cpp - OpenTherm Communication Library For Arduino, ESP8266
Copyright 2018, Ihor Melnyk
*/

#include "OpenTherm.h"

OpenTherm::OpenTherm(int inPin, int outPin, bool isSlave) : status(OpenThermStatus::NOT_INITIALIZED),
															inPin(inPin),
															outPin(outPin),
															isSlave(isSlave),
															response(0),
															responseStatus(OpenThermResponseStatus::NONE),
															responseTimestamp(0),
															handleInterruptCallback(NULL),
															processResponseCallback(NULL)
{
	imitFlag = false;
}

void OpenTherm::begin(void (*handleInterruptCallback)(void), void (*processResponseCallback)(unsigned long, OpenThermResponseStatus))
{
	pinMode(inPin, INPUT);
	pinMode(outPin, OUTPUT);
	if (handleInterruptCallback != NULL)
	{
		this->handleInterruptCallback = handleInterruptCallback;
		attachInterrupt(digitalPinToInterrupt(inPin), handleInterruptCallback, CHANGE);
	}
	activateBoiler();
	status = OpenThermStatus::READY;
	this->processResponseCallback = processResponseCallback;
}

void OpenTherm::begin(void (*handleInterruptCallback)(void))
{
	begin(handleInterruptCallback, NULL);
}

bool IRAM_ATTR OpenTherm::isReady()
{
	return status == OpenThermStatus::READY;
}

int IRAM_ATTR OpenTherm::readState()
{
	return digitalRead(inPin);
}

void OpenTherm::setActiveState()
{
	digitalWrite(outPin, LOW);
}

void OpenTherm::setIdleState()
{
	digitalWrite(outPin, HIGH);
}

void OpenTherm::activateBoiler()
{
	setIdleState();
	delay(1000);
}

void OpenTherm::sendBit(bool high)
{
	if (high)
		setActiveState();
	else
		setIdleState();
	delayMicroseconds(500);
	if (high)
		setIdleState();
	else
		setActiveState();
	delayMicroseconds(500);
}

bool OpenTherm::sendRequestAync(unsigned long request)
{
	// Serial.println("Request: " + String(request, HEX));
	noInterrupts();
	const bool ready = isReady();
	interrupts();

	if (!ready)
		return false;

	status = OpenThermStatus::REQUEST_SENDING;
	response = 0;
	responseStatus = OpenThermResponseStatus::NONE;
// Prevent switching to other tasks as there is a delay within sendBit
#ifdef ESP32
//	vTaskSuspendAll();
#endif
	sendBit(HIGH); // start bit
	for (int i = 31; i >= 0; i--)
	{
		sendBit(bitRead(request, i));
	}
	sendBit(HIGH); // stop bit
	setIdleState();
#ifdef ESP32
//	xTaskResumeAll();
#endif
	status = OpenThermStatus::RESPONSE_WAITING;
	responseTimestamp = micros();
	if (imitFlag)
		ImitationResponse(request);
	return true;
}

unsigned long OpenTherm::sendRequest(unsigned long request)
{
	if (!sendRequestAync(request))
		return 0;
	while (!isReady())
	{
		process();
		yield();
	}
	return response;
}

bool OpenTherm::sendResponse(unsigned long request)
{
	status = OpenThermStatus::REQUEST_SENDING;
	response = 0;
	responseStatus = OpenThermResponseStatus::NONE;
	// Prevent switching to other tasks as there is a delay within sendBit
#ifdef ESP32
//	vTaskSuspendAll();
#endif
	sendBit(HIGH); // start bit
	for (int i = 31; i >= 0; i--)
	{
		sendBit(bitRead(request, i));
	}
	sendBit(HIGH); // stop bit
	setIdleState();
#ifdef ESP32
//	xTaskResumeAll();
#endif
	status = OpenThermStatus::READY;
	return true;
}

unsigned long OpenTherm::getLastResponse()
{
	return response;
}

OpenThermResponseStatus OpenTherm::getLastResponseStatus()
{
	return responseStatus;
}

void IRAM_ATTR OpenTherm::handleInterrupt()
{
	if (isReady())
	{
		if (isSlave && readState() == HIGH)
		{
			status = OpenThermStatus::RESPONSE_WAITING;
		}
		else
		{
			return;
		}
	}

	unsigned long newTs = micros();
	if (status == OpenThermStatus::RESPONSE_WAITING)
	{
		if (readState() == HIGH)
		{
			status = OpenThermStatus::RESPONSE_START_BIT;
			responseTimestamp = newTs;
		}
		else
		{
			// Error start bit / Ошибка стартового бита
			status = OpenThermStatus::RESPONSE_INVALID;
			responseTimestamp = newTs;
		}
	}
	else if (status == OpenThermStatus::RESPONSE_START_BIT)
	{
		if ((newTs - responseTimestamp < 750) && readState() == LOW)
		{
			status = OpenThermStatus::RESPONSE_RECEIVING;
			responseTimestamp = newTs;
			responseBitIndex = 0;
		}
		else
		{
			// Error Start_bit LOW 750mks / Ошибка стартового бита по тылу (нет LOW через 750мкс)
			status = OpenThermStatus::RESPONSE_INVALID;
			responseTimestamp = newTs;
		}
	}
	else if (status == OpenThermStatus::RESPONSE_RECEIVING)
	{
		// unsigned long bitDuration = newTs - responseTimestamp;
		//  В новой спецификации стоповый бит не обязателен. Если не дождались, всё равно попробуем разобрать
		if ((newTs - responseTimestamp) > 750 && (newTs - responseTimestamp) < 1300)
		{
			if (responseBitIndex < 32)
			{
				response = (response << 1) | !readState();
				responseTimestamp = newTs;
				responseBitIndex++;
			}
			else
			{ // stop bit
				status = OpenThermStatus::RESPONSE_READY;
				responseTimestamp = newTs;
			}
		}
	}
}

void OpenTherm::process()
{
	noInterrupts();
	OpenThermStatus st = status;
	unsigned long ts = responseTimestamp;
	interrupts();

	if (st == OpenThermStatus::READY)
		return;
	unsigned long newTs = micros();
	if (st != OpenThermStatus::NOT_INITIALIZED && st != OpenThermStatus::DELAY && (newTs - ts) > 1000000)
	{
		status = OpenThermStatus::READY;
		responseStatus = OpenThermResponseStatus::TIMEOUT;
		if (processResponseCallback != NULL)
		{
			processResponseCallback(response, responseStatus);
		}
	}
	else if (st == OpenThermStatus::RESPONSE_INVALID)
	{
		status = OpenThermStatus::DELAY;
		responseStatus = OpenThermResponseStatus::INVALID;
		if (processResponseCallback != NULL)
		{
			processResponseCallback(response, responseStatus);
		}
	}
	else if (st == OpenThermStatus::RESPONSE_READY)
	{
		status = OpenThermStatus::DELAY;
		responseStatus = (isSlave ? isValidRequest(response) : isValidResponse(response)) ? OpenThermResponseStatus::SUCCESS : OpenThermResponseStatus::INVALID;
		// Error msgType (READ_ACK | WRITE_ACK) is Header
		if (processResponseCallback != NULL)
		{
			processResponseCallback(response, responseStatus);
		}
	}
	else if (st == OpenThermStatus::DELAY)
	{
		if ((newTs - ts) > 100000)
		{
			status = OpenThermStatus::READY;
		}
	}
}

bool OpenTherm::parity(unsigned long frame) // odd parity
{
	byte p = 0;
	while (frame > 0)
	{
		if (frame & 1)
			p++;
		frame = frame >> 1;
	}
	return (p & 1);
}

OpenThermMessageType OpenTherm::getMessageType(unsigned long message)
{
	OpenThermMessageType msg_type = static_cast<OpenThermMessageType>((message >> 28) & 7);
	return msg_type;
}

OpenThermMessageID OpenTherm::getDataID(unsigned long frame)
{
	return (OpenThermMessageID)((frame >> 16) & 0xFF);
}

unsigned long OpenTherm::buildRequest(OpenThermMessageType type, OpenThermMessageID id, unsigned int data)
{
	unsigned long request = data;
	if (type == OpenThermMessageType::WRITE_DATA)
	{
		request |= 1ul << 28;
	}
	request |= ((unsigned long)id) << 16;
	if (parity(request))
		request |= (1ul << 31);
	return request;
}
unsigned long OpenTherm::buildRequestID(OpenThermMessageType type, unsigned int id, unsigned int data)
{
	unsigned long request = data;
	if (type == OpenThermMessageType::WRITE_DATA)
	{
		request |= 1ul << 28;
	}
	request |= ((unsigned long)id) << 16;
	if (parity(request))
		request |= (1ul << 31);
	return request;
}

unsigned long OpenTherm::buildResponse(OpenThermMessageType type, OpenThermMessageID id, unsigned int data)
{
	unsigned long response = data;
	response |= ((unsigned long)type) << 28;
	response |= ((unsigned long)id) << 16;
	if (parity(response))
		response |= (1ul << 31);
	return response;
}

bool OpenTherm::isValidResponse(unsigned long response)
{
	if (parity(response))
		return false;
	byte msgType = (response << 1) >> 29;
	return msgType == READ_ACK || msgType == WRITE_ACK;
}

bool OpenTherm::isValidRequest(unsigned long request)
{
	if (parity(request))
		return false;
	byte msgType = (request << 1) >> 29;
	return msgType == READ_DATA || msgType == WRITE_DATA;
}

void OpenTherm::end()
{
	if (this->handleInterruptCallback != NULL)
	{
		detachInterrupt(digitalPinToInterrupt(inPin));
	}
}

const char *OpenTherm::statusToString(OpenThermResponseStatus status)
{
	switch (status)
	{
	case NONE:
		return "NONE";
	case SUCCESS:
		return "SUCCESS";
	case INVALID:
		return "INVALID";
	case TIMEOUT:
		return "TIMEOUT";
	default:
		return "UNKNOWN";
	}
}

const char *OpenTherm::messageTypeToString(OpenThermMessageType message_type)
{
	switch (message_type)
	{
	case READ_DATA:
		return "READ_DATA";
	case WRITE_DATA:
		return "WRITE_DATA";
	case INVALID_DATA:
		return "INVALID_DATA";
	case RESERVED:
		return "RESERVED";
	case READ_ACK:
		return "READ_ACK";
	case WRITE_ACK:
		return "WRITE_ACK";
	case DATA_INVALID:
		return "DATA_INVALID";
	case UNKNOWN_DATA_ID:
		return "UNKNOWN_DATA_ID";
	default:
		return "UNKNOWN";
	}
}

// building requests

unsigned long OpenTherm::buildSetBoilerStatusRequest(bool enableCentralHeating, bool enableHotWater, bool enableCooling, bool enableOutsideTemperatureCompensation, bool enableCentralHeating2, bool enableSummerMode, bool dhwBlock)
{
	unsigned int data = enableCentralHeating | (enableHotWater << 1) | (enableCooling << 2) | (enableOutsideTemperatureCompensation << 3) | (enableCentralHeating2 << 4) | (enableSummerMode << 5) | (dhwBlock << 6);
	data <<= 8;
	return buildRequest(OpenThermMessageType::READ_DATA, OpenThermMessageID::Status, data);
}

unsigned long OpenTherm::buildSetBoilerTemperatureRequest(float temperature)
{
	unsigned int data = temperatureToData(temperature);
	return buildRequest(OpenThermMessageType::WRITE_DATA, OpenThermMessageID::TSet, data);
}

unsigned long OpenTherm::buildGetBoilerTemperatureRequest()
{
	return buildRequest(OpenThermMessageType::READ_DATA, OpenThermMessageID::Tboiler, 0);
}

// parsing responses
bool OpenTherm::isFault(unsigned long response)
{
	return response & 0x1;
}

bool OpenTherm::isCentralHeatingActive(unsigned long response)
{
	return response & 0x2;
}

bool OpenTherm::isHotWaterActive(unsigned long response)
{
	return response & 0x4;
}

bool OpenTherm::isFlameOn(unsigned long response)
{
	return response & 0x8;
}

bool OpenTherm::isCoolingActive(unsigned long response)
{
	return response & 0x10;
}

bool OpenTherm::isDiagnostic(unsigned long response)
{
	return response & 0x40;
}

uint16_t OpenTherm::getUInt(const unsigned long response) const
{
	const uint16_t u88 = response & 0xffff;
	return u88;
}

float OpenTherm::getFloat(const unsigned long response) const
{
	const uint16_t u88 = getUInt(response);
	const float f = (u88 & 0x8000) ? -(0x10000L - u88) / 256.0f : u88 / 256.0f;
	return f;
}

unsigned int OpenTherm::temperatureToData(float temperature)
{
	if (temperature < 0)
		temperature = 0;
	if (temperature > 100)
		temperature = 100;
	unsigned int data = (unsigned int)(temperature * 256);
	return data;
}

// basic requests

unsigned long OpenTherm::setBoilerStatus(bool enableCentralHeating, bool enableHotWater, bool enableCooling, bool enableOutsideTemperatureCompensation, bool enableCentralHeating2, bool enableSummerMode, bool dhwBlock)
{
	return sendRequest(buildSetBoilerStatusRequest(enableCentralHeating, enableHotWater, enableCooling, enableOutsideTemperatureCompensation, enableCentralHeating2, enableSummerMode, dhwBlock));
}

bool OpenTherm::setBoilerTemperature(float temperature)
{
	unsigned long response = sendRequest(buildSetBoilerTemperatureRequest(temperature));
	return isValidResponse(response);
}

float OpenTherm::getBoilerTemperature()
{
	unsigned long response = sendRequest(buildGetBoilerTemperatureRequest());
	return isValidResponse(response) ? getFloat(response) : 0;
}

float OpenTherm::getReturnTemperature()
{
	unsigned long response = sendRequest(buildRequest(OpenThermRequestType::READ, OpenThermMessageID::Tret, 0));
	return isValidResponse(response) ? getFloat(response) : 0;
}

bool OpenTherm::setDHWSetpoint(float temperature)
{
	unsigned int data = temperatureToData(temperature);
	unsigned long response = sendRequest(buildRequest(OpenThermMessageType::WRITE_DATA, OpenThermMessageID::TdhwSet, data));
	return isValidResponse(response);
}

float OpenTherm::getDHWTemperature()
{
	unsigned long response = sendRequest(buildRequest(OpenThermMessageType::READ_DATA, OpenThermMessageID::Tdhw, 0));
	return isValidResponse(response) ? getFloat(response) : 0;
}

float OpenTherm::getModulation()
{
	unsigned long response = sendRequest(buildRequest(OpenThermRequestType::READ, OpenThermMessageID::RelModLevel, 0));
	return isValidResponse(response) ? getFloat(response) : 0;
}

float OpenTherm::getPressure()
{
	unsigned long response = sendRequest(buildRequest(OpenThermRequestType::READ, OpenThermMessageID::CHPressure, 0));
	return isValidResponse(response) ? getFloat(response) : 0;
}

unsigned char OpenTherm::getFault()
{
	return ((sendRequest(buildRequest(OpenThermRequestType::READ, OpenThermMessageID::ASFflags, 0)) >> 8) & 0xff);
}
int8_t flame_timer = 0;
void OpenTherm::ImitationResponse(unsigned long request)
{

	//	unsigned long response;
	unsigned int data = getUInt(request);
	OpenThermMessageType msgType;
	byte ID;
	OpenThermMessageID id = getDataID(request);
	uint8_t flags;

	switch (id)
	{
	case OpenThermMessageID::Status:
		// Статус котла получен
		msgType = OpenThermMessageType::READ_ACK;
		static int8_t flame = 0;
		flame_timer++;
		if (flame_timer > 10)
			flame = 1;
		if (flame_timer > 20)
		{
			flame_timer = 0;
			flame = 0;
		}
		static int8_t fault = 0;
		// fault = 1 - fault;
		data = (bool)fault | (true << 1) | (true << 2) | ((bool)flame << 3) | (false << 4);
		break;
	case OpenThermMessageID::SConfigSMemberIDcode:
		msgType = OpenThermMessageType::READ_ACK;
		break;
	case OpenThermMessageID::SlaveVersion:
		msgType = OpenThermMessageType::READ_ACK;
		break;
	case OpenThermMessageID::MasterVersion:
		msgType = OpenThermMessageType::WRITE_ACK;
		break;
	case OpenThermMessageID::RelModLevel:
		static float RelModLevel = 10;
		// RelModLevel = RelModLevel > 100 ? 10 : RelModLevel + 1;
		if (flame_timer < 11)
		{
			RelModLevel = 0;
		}
		else
		{
			RelModLevel = RelModLevel == 0 ? 10 : RelModLevel + 1;
		}
		// data = RelModLevel;
		data = temperatureToData(RelModLevel);
		msgType = OpenThermMessageType::READ_ACK;
		break;
	case OpenThermMessageID::Tboiler:
		// Получили температуру котла
		static float Tboiler = 40;
		Tboiler = Tboiler > 60 ? 40 : Tboiler + 1;
		data = temperatureToData(Tboiler);
		msgType = OpenThermMessageType::READ_ACK;
		break;
	case OpenThermMessageID::Tdhw:
		// Получили температуру ГВС
		static float Tdhw = 60;
		Tdhw = Tdhw > 80 ? 60 : Tdhw + 1;
		data = temperatureToData(Tdhw);
		msgType = OpenThermMessageType::READ_ACK;
		break;
	case OpenThermMessageID::Toutside:
		// Получили внешнюю температуру
		static float Toutside = -10;
		Toutside = Toutside > 10 ? -10 : Toutside + 1;
		data = temperatureToData(Toutside);
		msgType = OpenThermMessageType::READ_ACK;
		break;
	case OpenThermMessageID::ASFflags:
		msgType = OpenThermMessageType::READ_ACK;
		break;

	case OpenThermMessageID::TdhwSetUBTdhwSetLB:
		msgType = OpenThermMessageType::READ_ACK;
		break;
	case OpenThermMessageID::MaxTSetUBMaxTSetLB:
		msgType = OpenThermMessageType::READ_ACK;
		break;

	case OpenThermMessageID::OEMDiagnosticCode:
		msgType = OpenThermMessageType::READ_ACK;
		break;

	case OpenThermMessageID::OpenThermVersionSlave:
		msgType = OpenThermMessageType::READ_ACK;
		break;

	case OpenThermMessageID::CHPressure:
		msgType = OpenThermMessageType::READ_ACK;
		break;

		break;
	case OpenThermMessageID::DHWFlowRate:
		msgType = OpenThermMessageType::READ_ACK;
		break;
	case OpenThermMessageID::DayTime:
		if (getMessageType(request) == OpenThermMessageType::READ_DATA)
			msgType = OpenThermMessageType::READ_ACK;
		else
			msgType = OpenThermMessageType::WRITE_ACK;
		break;
	case OpenThermMessageID::Date:
		if (getMessageType(request) == OpenThermMessageType::READ_DATA)
			msgType = OpenThermMessageType::READ_ACK;
		else
			msgType = OpenThermMessageType::WRITE_ACK;
		break;
	case OpenThermMessageID::Year:
		if (getMessageType(request) == OpenThermMessageType::READ_DATA)
			msgType = OpenThermMessageType::READ_ACK;
		else
			msgType = OpenThermMessageType::WRITE_ACK;
		break;

	case OpenThermMessageID::Tret:
		//
		msgType = OpenThermMessageType::READ_ACK;
		break;
	case OpenThermMessageID::Tstorage:
		//
		msgType = OpenThermMessageType::READ_ACK;
		break;
	case OpenThermMessageID::Tcollector:
		//
		msgType = OpenThermMessageType::READ_ACK;
		break;
	case OpenThermMessageID::TflowCH2:
		//
		msgType = OpenThermMessageType::READ_ACK;

		break;
	case OpenThermMessageID::Tdhw2:
		//
		msgType = OpenThermMessageType::READ_ACK;

		break;
	case OpenThermMessageID::Texhaust:
		//
		msgType = OpenThermMessageType::READ_ACK;
		break;
	case OpenThermMessageID::TheatExchanger:
		//
		msgType = OpenThermMessageType::READ_ACK;
		break;
	case OpenThermMessageID::BoilerFanSpeed:
		//
		msgType = OpenThermMessageType::READ_ACK;
		break;
	case OpenThermMessageID::ElectricBurnerFlame:
		//
		msgType = OpenThermMessageType::READ_ACK;
		break;
	case OpenThermMessageID::BurnerStarts:
		//
		if (getMessageType(request) == OpenThermMessageType::READ_DATA)
			msgType = OpenThermMessageType::READ_ACK;
		else
			msgType = OpenThermMessageType::WRITE_ACK;
		break;
	case OpenThermMessageID::CHPumpStarts:
		//
		if (getMessageType(request) == OpenThermMessageType::READ_DATA)
			msgType = OpenThermMessageType::READ_ACK;
		else
			msgType = OpenThermMessageType::WRITE_ACK;
		break;
	case OpenThermMessageID::DHWPumpValveStarts:
		//
		if (getMessageType(request) == OpenThermMessageType::READ_DATA)
			msgType = OpenThermMessageType::READ_ACK;
		else
			msgType = OpenThermMessageType::WRITE_ACK;
		break;
	case OpenThermMessageID::DHWBurnerStarts:
		//
		if (getMessageType(request) == OpenThermMessageType::READ_DATA)
			msgType = OpenThermMessageType::READ_ACK;
		else
			msgType = OpenThermMessageType::WRITE_ACK;
		break;
	case OpenThermMessageID::BurnerOperationHours:
		//
		if (getMessageType(request) == OpenThermMessageType::READ_DATA)
			msgType = OpenThermMessageType::READ_ACK;
		else
			msgType = OpenThermMessageType::WRITE_ACK;
		break;
	case OpenThermMessageID::CHPumpOperationHours:
		//
		if (getMessageType(request) == OpenThermMessageType::READ_DATA)
			msgType = OpenThermMessageType::READ_ACK;
		else
			msgType = OpenThermMessageType::WRITE_ACK;
		break;
	case OpenThermMessageID::DHWPumpValveOperationHours:
		//
		if (getMessageType(request) == OpenThermMessageType::READ_DATA)
			msgType = OpenThermMessageType::READ_ACK;
		else
			msgType = OpenThermMessageType::WRITE_ACK;
		break;
	case OpenThermMessageID::DHWBurnerOperationHours:
		//
		if (getMessageType(request) == OpenThermMessageType::READ_DATA)
			msgType = OpenThermMessageType::READ_ACK;
		else
			msgType = OpenThermMessageType::WRITE_ACK;
		break;
	case OpenThermMessageID::RBPflags:
		//
		// Pre-Defined Remote Boiler Parameters
		//
		msgType = OpenThermMessageType::READ_ACK;
		break;
	case OpenThermMessageID::TdhwSet:
		//
		if (getMessageType(request) == OpenThermMessageType::READ_DATA)
			msgType = OpenThermMessageType::READ_ACK;
		else
			msgType = OpenThermMessageType::WRITE_ACK;
		break;
	case OpenThermMessageID::TSet:
		//
		if (getMessageType(request) == OpenThermMessageType::READ_DATA)
			msgType = OpenThermMessageType::READ_ACK;
		else
			msgType = OpenThermMessageType::WRITE_ACK;
		break;
	case OpenThermMessageID::MaxTSet:
		//
		if (getMessageType(request) == OpenThermMessageType::READ_DATA)
			msgType = OpenThermMessageType::READ_ACK;
		else
			msgType = OpenThermMessageType::WRITE_ACK;
		break;
	case OpenThermMessageID::Hcratio:
		//
		if (getMessageType(request) == OpenThermMessageType::READ_DATA)
			msgType = OpenThermMessageType::READ_ACK;
		else
			msgType = OpenThermMessageType::WRITE_ACK;
		break;
	case OpenThermMessageID::TSP:
		//
		// Transparent Slave Parameters
		//
		msgType = OpenThermMessageType::READ_ACK;
		break;
	case OpenThermMessageID::FHBsize:
		//
		// Fault History Data
		//
		msgType = OpenThermMessageType::READ_ACK;
		break;
	case OpenThermMessageID::MaxCapacityMinModLevel:
		//
		// Boiler Sequencer Control
		//
		msgType = OpenThermMessageType::READ_ACK;
		break;
	case OpenThermMessageID::TrOverride:
		//
		// Remote override room setpoint
		//
		msgType = OpenThermMessageType::READ_ACK;
		break;
	case OpenThermMessageID::RemoteOverrideFunction:
		msgType = OpenThermMessageType::READ_ACK;
		break;

	default:
		msgType = OpenThermMessageType::UNKNOWN_DATA_ID;
		break;
	}
	response = buildResponse(msgType, id, data);
	status = OpenThermStatus::RESPONSE_READY;
	responseStatus = OpenThermResponseStatus::SUCCESS;
	/*
	if (processResponseCallback != NULL)
	{
		processResponseCallback(response, OpenThermResponseStatus::SUCCESS);
	}
	*/
}