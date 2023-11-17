/**
 * @file NexUpload.cpp
 *
 * The implementation of uploading tft file for nextion displays.
 *
 * Original version (a part of https://github.com/itead/ITEADLIB_Arduino_Nextion)
 * @author  Chen Zengpeng (email:<zengpeng.chen@itead.cc>)
 * @date    2016/3/29
 * @copyright
 * Copyright (C) 2014-2015 ITEAD Intelligent Systems Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#define DEBUG_SERIAL_ENABLE
#include "ESPNexUpload.h"

#ifdef DEBUG_SERIAL_ENABLE
#define dbSerialPrint(a) Serial.print(a)
#define dbSerialPrintHex(a) Serial.print(a, HEX)
#define dbSerialPrintln(a) Serial.println(a)
#define dbSerialBegin(a) Serial.begin(a)
#else
#define dbSerialPrint(a) \
	do                   \
	{                    \
	} while (0)
#define dbSerialPrintHex(a) \
	do                      \
	{                       \
	} while (0)
#define dbSerialPrintln(a) \
	do                     \
	{                      \
	} while (0)
#define dbSerialBegin(a) \
	do                   \
	{                    \
	} while (0)
#endif

ESPNexUpload::ESPNexUpload(uint32_t upload_baudrate, int line, int rx, int tx)
{
	_upload_baudrate = upload_baudrate;
	_rx = rx;
	_tx = tx;
	_line = line;

#if defined ESP8266
	nexSerial = new SoftwareSerial(_rx, _tx);
#else
	if (line >= 0) {
        nexSerial = new HardwareSerial(line);
 //       ((HardwareSerial*)nexSerial)->begin(_upload_baudrate, SERIAL_8N1, _rx, _tx);
    } else {
        nexSerial = new SoftwareSerial(_rx, _tx);
 //       ((SoftwareSerial*)nexSerial)->begin(_upload_baudrate);
    }
#endif

}

void ESPNexUpload::nexSerialBegin(uint32_t _speed, int _line, int _rx, int _tx)
{
#if defined ESP8266
	nexSerial->begin(_speed);
#else
	if (_line >= 0) {
        ((HardwareSerial*)nexSerial)->begin(_speed, SERIAL_8N1, _rx, _tx);
    } else {
        ((SoftwareSerial*)nexSerial)->begin(_speed);
    }
#endif	
}

bool ESPNexUpload::connect()
{
#if defined ESP8266
	yield();
#endif

	dbSerialBegin(115200);
	_printInfoLine(F("serial tests & connect"));

	if (_getBaudrate() == 0)
	{
		statusMessage = F("get baudrate error");
		_printInfoLine(statusMessage);
		return false;
	}

	_setRunningMode();

	if (!_echoTest("mystop_yesABC"))
	{
		statusMessage = F("echo test failed");
		_printInfoLine(statusMessage);
		return false;
	}

	if (!_handlingSleepAndDim())
	{
		statusMessage = F("handling sleep and dim settings failed");
		_printInfoLine(statusMessage);
		return false;
	}

	if (!_setPrepareForFirmwareUpdate(_upload_baudrate))
	{
		statusMessage = F("modifybaudrate error");
		_printInfoLine(statusMessage);
		return false;
	}

	return true;
}

bool ESPNexUpload::prepareUpload(uint32_t file_size)
{
	_undownloadByte = file_size;
	return this->connect();
}

uint16_t ESPNexUpload::_getBaudrate(void)
{

	_baudrate = 0;
	uint32_t baudrate_array[7] = {115200, 19200, 9600, 57600, 38400, 4800, 2400};
	for (uint8_t i = 0; i < 7; i++)
	{
		if (_searchBaudrate(baudrate_array[i]))
		{
			_baudrate = baudrate_array[i];
			_printInfoLine(F("baudrate determined"));
			break;
		}
		delay(1500); // wait for 1500 ms
	}
	return _baudrate;
}

bool ESPNexUpload::_searchBaudrate(uint32_t baudrate)
{

#if defined ESP8266
	yield();
#endif

	String response = String("");
	_printInfoLine();
	dbSerialPrint(F("init nextion serial interface on baudrate: "));
	dbSerialPrintln(baudrate);

	nexSerialBegin(baudrate, _line, _rx, _tx);
	_printInfoLine(F("ESP baudrate established, try to connect to display"));
	const char _nextion_FF_FF[3] = {0xFF, 0xFF, 0x00};

	this->sendCommand("DRAKJHSUYDGBNCJHGJKSHBDN");
	this->sendCommand("", true, true); // 0x00 0xFF 0xFF 0xFF

	this->recvRetString(response);
	if (response[0] != 0x1A)
	{
		_printInfoLine(F("first indication that baudrate is wrong"));
	}
	else
	{
		_printInfoLine(F("first respone from display, first indication that baudrate is correct"));
	}

	this->sendCommand("connect"); // first connect attempt

	this->recvRetString(response);
	if (response.indexOf(F("comok")) == -1)
	{
		_printInfoLine(F("display doesn't accept the first connect request"));
	}
	else
	{
		_printInfoLine(F("display accept the first connect request"));
	}

	response = String("");
	delay(110); // based on serial analyser from Nextion editor V0.58 to Nextion display NX4024T032_011R
	this->sendCommand(_nextion_FF_FF, false);

	this->sendCommand("connect"); // second attempt
	this->recvRetString(response);
	if (response.indexOf(F("comok")) == -1 && response[0] != 0x1A)
	{
		_printInfoLine(F("display doesn't accept the second connect request"));
		_printInfoLine(F("conclusion, wrong baudrate"));
		return 0;
	}
	else
	{
		_printInfoLine(F("display accept the second connect request"));
		_printInfoLine(F("conclusion, correct baudrate"));
	}

	return 1;
}

void ESPNexUpload::sendCommand(const char *cmd, bool tail, bool null_head)
{

#if defined ESP8266
	yield();
#endif

	if (null_head)
	{
		((HardwareSerial*)nexSerial)->write(0x00);
	}

	while (nexSerial->available())
	{
		nexSerial->read();
	}

	nexSerial->print(cmd);
	if (tail)
	{
		nexSerial->write(0xFF);
		nexSerial->write(0xFF);
		nexSerial->write(0xFF);
	}
	_printSerialData(true, cmd);
}

uint16_t ESPNexUpload::recvRetString(String &response, uint32_t timeout, bool recv_flag)
{

#if defined ESP8266
	yield();
#endif

	uint16_t ret = 0;
	uint8_t c = 0;
	uint8_t nr_of_FF_bytes = 0;
	long start;
	bool exit_flag = false;
	bool ff_flag = false;
	if (timeout != 500)
		_printInfoLine("timeout setting serial read: " + String(timeout));

	start = millis();

	while (millis() - start <= timeout)
	{

		while (nexSerial->available())
		{

			c = nexSerial->read();
			if (c == 0)
			{
				continue;
			}

			if (c == 0xFF)
				nr_of_FF_bytes++;
			else
			{
				nr_of_FF_bytes = 0;
				ff_flag = false;
			}

			if (nr_of_FF_bytes >= 3)
				ff_flag = true;

			response += (char)c;

			if (recv_flag)
			{
				if (response.indexOf(0x05) != -1)
				{
					exit_flag = true;
				}
			}
		}
		if (exit_flag || ff_flag)
		{
			break;
		}
	}
	_printSerialData(false, response);

	// if the exit flag and the ff flag are both not found, than there is a timeout
	// if(!exit_flag && !ff_flag)
	// _printInfoLine(F("recvRetString: timeout"));

	if (ff_flag)
		response = response.substring(0, response.length() - 3); // Remove last 3 0xFF

	ret = response.length();
	return ret;
}

bool ESPNexUpload::_setPrepareForFirmwareUpdate(uint32_t upload_baudrate)
{

#if defined ESP8266
	yield();
#endif

	String response = String("");
	String cmd = String("");

	cmd = F("00");
	this->sendCommand(cmd.c_str());
	delay(0.1);

	this->recvRetString(response, 800, true); // normal response time is 400ms

	String filesize_str = String(_undownloadByte, 10);
	String baudrate_str = String(upload_baudrate);
	cmd = "whmi-wri " + filesize_str + "," + baudrate_str + ",0";

	this->sendCommand(cmd.c_str());

	// Without flush, the whmi command will NOT transmitted by the ESP in the current baudrate
	// because switching to another baudrate (nexSerialBegin command) has an higher prio.
	// The ESP will first jump to the new 'upload_baudrate' and than process the serial 'transmit buffer'
	// The flush command forced the ESP to wait until the 'transmit buffer' is empty
	nexSerial->flush();

	nexSerialBegin(upload_baudrate, _line, _rx, _tx);
	_printInfoLine(F("changing upload baudrate..."));
	_printInfoLine(String(upload_baudrate));

	this->recvRetString(response, 800, true); // normal response time is 400ms

	// The Nextion display will, if it's ready to accept data, send a 0x05 byte.
	if (response.indexOf(0x05) != -1)
	{
		_printInfoLine(F("preparation for firmware update done"));
		return 1;
	}
	else
	{
		_printInfoLine(F("preparation for firmware update failed"));
		return 0;
	}
}

void ESPNexUpload::setUpdateProgressCallback(THandlerFunction value)
{
	_updateProgressCallback = value;
}

bool ESPNexUpload::upload(const uint8_t *file_buf, size_t buf_size)
{

#if defined ESP8266
	yield();
#endif

	uint8_t c;
	uint8_t timeout = 0;
	String string = String("");

	for (uint16_t i = 0; i < buf_size; i++)
	{

		// Users must split the .tft file contents into 4096 byte sized packets with the final partial packet size equal to the last remaining bytes (<4096 bytes).
		if (_sent_packets == 4096)
		{

			// wait for the Nextion to return its 0x05 byte confirming reception and readiness to receive the next packets
			this->recvRetString(string, 500, true);
			if (string.indexOf(0x05) != -1)
			{

				// reset sent packets counter
				_sent_packets = 0;

				// reset receive String
				string = "";
			}
			else
			{
				if (timeout >= 8)
				{
					statusMessage = F("serial connection lost");
					_printInfoLine(statusMessage);
					return false;
				}

				timeout++;
			}

			// delay current byte
			i--;
		}
		else
		{

			// read buffer
			c = file_buf[i];

			// write byte to nextion over serial
			nexSerial->write(c);

			// update sent packets counter
			_sent_packets++;
		}
	}

	return true;
}

bool ESPNexUpload::upload(Stream &myFile)
{
#if defined ESP8266
	yield();
#endif

	// create buffer for read
	uint8_t buff[2048] = {0};

	// read all data from server
	while (_undownloadByte > 0 || _undownloadByte == -1)
	{

		// get available data size
		size_t size = myFile.available();

		if (size)
		{
			// read up to 2048 byte into the buffer
			int c = myFile.readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

			// Write the buffered bytes to the nextion. If this fails, return false.
			if (!this->upload(buff, c))
			{
				return false;
			}
			else
			{
				if (_updateProgressCallback)
				{
					_updateProgressCallback();
				}
			}

			if (_undownloadByte > 0)
			{
				_undownloadByte -= c;
			}
		}
		delay(1);
	}

	return true;
}

void ESPNexUpload::softReset(void)
{
	// soft reset nextion device
	this->sendCommand("rest");
}

void ESPNexUpload::end()
{

	// wait for the nextion to finish internal processes
	delay(1600);

	// soft reset the nextion
	this->softReset();

	// end Serial connection
	((HardwareSerial*)nexSerial)->end();

	// reset sent packets counter
	_sent_packets = 0;

	statusMessage = F("upload ok");
	_printInfoLine(statusMessage + F("\r\n"));
}

void ESPNexUpload::_setRunningMode(void)
{
	String cmd = String("");
	delay(100);
	cmd = F("runmod=2");
	this->sendCommand(cmd.c_str());
	delay(60);
}

bool ESPNexUpload::_echoTest(String input)
{
	String cmd = String("");
	String response = String("");

	cmd = "print \"" + input + "\"";
	this->sendCommand(cmd.c_str());

	uint32_t duration_ms = calculateTransmissionTimeMs(cmd) * 2 + 10; // times 2  (send + receive) and 10 ms extra
	this->recvRetString(response, duration_ms);

	return (response.indexOf(input) != -1);
}

bool ESPNexUpload::_handlingSleepAndDim(void)
{
	String cmd = String("");
	String response = String("");
	bool set_sleep = false;
	bool set_dim = false;

	cmd = F("get sleep");
	this->sendCommand(cmd.c_str());

	this->recvRetString(response);

	if (response[0] != 0x71)
	{
		statusMessage = F("unknown response from 'get sleep' request");
		_printInfoLine(statusMessage);
		return false;
	}

	if (response[1] != 0x00)
	{
		_printInfoLine(F("sleep enabled"));
		set_sleep = true;
	}
	else
	{
		_printInfoLine(F("sleep disabled"));
	}

	response = String("");
	cmd = F("get dim");
	this->sendCommand(cmd.c_str());

	this->recvRetString(response);

	if (response[0] != 0x71)
	{
		statusMessage = F("unknown response from 'get dim' request");
		_printInfoLine(statusMessage);
		return false;
	}

	if (response[1] == 0x00)
	{
		_printInfoLine(F("dim is 0%, backlight from display is turned off"));
		set_dim = true;
	}
	else
	{
		_printInfoLine();
		dbSerialPrint(F("dim "));
		dbSerialPrint((uint8_t)response[1]);
		dbSerialPrintln(F("%"));
	}

	if (!_echoTest("ABC"))
	{
		statusMessage = F("echo test in 'handling sleep and dim' failed");
		_printInfoLine(statusMessage);
		return false;
	}

	if (set_sleep)
	{
		cmd = F("sleep=0");
		this->sendCommand(cmd.c_str());
		// Unfortunately the display doesn't send any respone on the wake up request (sleep=0)
		// Let the ESP wait for one second, this is based on serial analyser from Nextion editor V0.58 to Nextion display NX4024T032_011R
		// This gives the Nextion display some time to wake up
		delay(1000);
	}

	if (set_dim)
	{
		cmd = F("dim=100");
		this->sendCommand(cmd.c_str());
		delay(15);
	}

	return true;
}

void ESPNexUpload::_printSerialData(bool esp_request, String input)
{

	char c;
	if (esp_request)
		dbSerialPrint(F("ESP     request: "));
	else
		dbSerialPrint(F("Nextion respone: "));

	if (input.length() == 0)
	{
		dbSerialPrintln(F("none"));
		return;
	}

	for (int i = 0; i < input.length(); i++)
	{

		c = input[i];
		if ((uint8_t)c >= 0x20 && (uint8_t)c <= 0x7E)
			dbSerialPrint(c);
		else
		{
			dbSerialPrint(F("0x"));
			dbSerialPrintHex(c);
			dbSerialPrint(F(" "));
		}
	}
	dbSerialPrintln();
}

uint32_t ESPNexUpload::calculateTransmissionTimeMs(String message)
{
	// In general, 1 second (s) = 1000 (10^-3) millisecond (ms) or
	//             1 second (s) = 1000 000 (10^-6) microsecond (us).
	// To calculate how much microsecond one BIT of data takes with a certain baudrate you have to divide
	// the baudrate by one second.
	// For example 9600 baud = 1000 000 us / 9600 ≈ 104 us
	// The time to transmit one DATA byte (if we use default UART modulation) takes 10 bits.
	// 8 DATA bits and one START and one STOP bit makes 10 bits.
	// In this example (9600 baud) a byte will take 1041 us to send or receive.
	// Multiply this value by the length of the message (number of bytes) and the total transmit/ receive time
	// is calculated.

	uint32_t duration_one_byte_us = 10000000 / _baudrate; // 1000 000 * 10 bits / baudrate
	uint16_t nr_of_bytes = message.length() + 3;		  // 3 times 0xFF byte
	uint32_t duration_message_us = nr_of_bytes * duration_one_byte_us;
	uint32_t return_value_ms = duration_message_us / 1000;

	_printInfoLine("calculated transmission time: " + String(return_value_ms) + " ms");
	return return_value_ms;
}

void ESPNexUpload::_printInfoLine(String line)
{
	dbSerialPrint(F("Status     info: "));
	if (line.length() != 0)
		dbSerialPrintln(line);
}