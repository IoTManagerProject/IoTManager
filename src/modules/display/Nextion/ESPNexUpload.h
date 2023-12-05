/**
 * @file NexUpload.h
 * The definition of class NexUpload.
 *
 *
 * 1 - BugFix when display baudrate is diffrent from initial ESP baudrate
 * 2 - Improved debug information
 * 3 - Make delay commands dependent on the baudrate
 * @author Machiel Mastenbroek (machiel.mastenbroek@gmail.com)
 * @date   2019/11/04
 * @version 0.5.5
 *
 * Stability improvement, Nextion display doesn’t freeze after the seconds 4096 trance of firmware bytes.
 * Now the firmware upload process is stabled without the need of a hard Display power off-on intervention.
 * Undocumented features (not mentioned in nextion-hmi-upload-protocol-v1-1 specification) are added.
 * This implementation is based in on a reverse engineering with a UART logic analyser between
 * the Nextion editor v0.58 and a NX4024T032_011R Display.
 *
 * @author Machiel Mastenbroek (machiel.mastenbroek@gmail.com)
 * @date   2019/10/24
 * @version 0.5.0
 *
 * Modified to work with ESP32, HardwareSerial and removed SPIFFS dependency
 * @author Onno Dirkzwager (onno.dirkzwager@gmail.com)
 * @date   2018/12/26
 * @version 0.3.0
 *
 * Modified to work with ESP8266 and SoftwareSerial
 * @author Ville Vilpas (psoden@gmail.com)
 * @date   2018/2/3
 * @version 0.2.0
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

#ifndef __ESPNEXUPLOAD_H__
#define __ESPNEXUPLOAD_H__
#include <functional>
#include <Arduino.h>
#include <StreamString.h>

#ifdef ESP8266
#include <SoftwareSerial.h>
#else
#include <HardwareSerial.h>
#include <SoftwareSerial.h>
#endif

/**
 * @addtogroup CoreAPI
 * @{
 */

// callback template definition
typedef std::function<void(void)> THandlerFunction;

/**
 *
 * Provides the API for nextion to upload the ftf file.
 */
class ESPNexUpload
{
public: /* methods */
    // callback template definition
    typedef std::function<void(void)> THandlerFunction;

    /**
     * Constructor.
     *
     * @param uint32_t upload_baudrate - set upload baudrate.
     */
    ESPNexUpload(uint32_t upload_baudrate, int line, int rx, int tx);

    /**
     * destructor.
     *
     */
    ~ESPNexUpload() {}

    /**
     * Connect to Nextion over serial
     *
     * @return true or false.
     */
    bool connect();

    /**
     * prepare upload. Set file size & Connect to Nextion over serial
     *
     * @return true if success, false for failure.
     */
    bool prepareUpload(uint32_t file_size);

    /**
     * set Update Progress Callback. (What to do during update progress)
     *
     * @return none
     */
    void setUpdateProgressCallback(THandlerFunction value);

    /**
     * start update tft file to nextion.
     *
     * @param const uint8_t *file_buf
     * @param size_t buf_size
     * @return true if success, false for failure.
     */
    bool upload(const uint8_t *file_buf, size_t buf_size);

    /**
     * start update tft file to nextion.
     *
     * @param Stream &myFile
     * @return true if success, false for failure.
     */
    bool upload(Stream &myFile);

    /**
     * Send reset command to Nextion over serial
     *
     * @return none.
     */
    void softReset(void);

    /**
     * Send reset, end serial, reset _sent_packets & update status message
     *
     * @return none.
     */
    void end(void);

public: /* data */
    String statusMessage = "";

private: /* methods */
    /*
     * get communicate baudrate.
     *
     * @return communicate baudrate.
     *
     */
    uint16_t _getBaudrate(void);

    /*
     * search communicate baudrate.
     *
     * @param baudrate - communicate baudrate.
     *
     * @return true if success, false for failure.
     */
    bool _searchBaudrate(uint32_t baudrate);

    /*
     * set download baudrate.
     *
     * @param baudrate - set download baudrate.
     *
     * @return true if success, false for failure.
     */
    bool _setPrepareForFirmwareUpdate(uint32_t upload_baudrate);

    /*
     * set Nextion running mode.
     *
     * Undocumented feature of the Nextion protocol.
     * It's used by the 'upload to Nextion device' feature of the Nextion Editor V0.58
     *
     * The nextion display doesn't send any response
     *
     */
    void _setRunningMode(void);

    /*
     * Test UART nextion connection availability
     *
     * @param input  - echo string,
     *
     * @return true when the 'echo string' that is send is equal to the received string
     *
     * This test is used by the 'upload to Nextion device' feature of the Nextion Editor V0.58
     *
     */
    bool _echoTest(String input);

    /*
     * This function get the sleep and dim value from the Nextion display.
     *
     * If sleep = 1 meaning: sleep is enabled
     *	            action : sleep will be disabled
     * If dim = 0,  meaning: the display backlight is turned off
     *              action : dim will be set to 100 (percent)
     *
     */
    bool _handlingSleepAndDim(void);

    /*
     * This function (debug) print the Nextion response to a human readable string
     *
     * @param esp_request  - true:  request  message from esp     to nextion
     *                       false: response message from nextion to esp
     *
     * @param input - string to print
     *
     */
    void _printSerialData(bool esp_request, String input);

    /*
     * This function print a prefix debug line
     *
     * @param line: optional debug/ info line
     */
    void _printInfoLine(String line = "");

    /*
     * Send command to Nextion.
     *
     * @param cmd       - the string of command.
     * @param tail      - end the string with tripple 0xFF byte
     * @param null_head - start the string with a single 0x00 byte
     *
     * @return none.
     */
    void sendCommand(const char *cmd, bool tail = true, bool null_head = false);

    /*
     * Receive string data.
     *
     * @param buffer - save string data.
     * @param timeout - set timeout time.
     * @param recv_flag - if recv_flag is true,will braak when receive 0x05.
     *
     * @return the length of string buffer.
     *
     */
    uint16_t recvRetString(String &string, uint32_t timeout = 500, bool recv_flag = false);

    /*
     *
     * This function calculates the transmission time, the transmission time
     * is based on the length of the message and the baudrate.
     *
     * @param message - only used to determine the length of the message
     *
     * @return time in us length of string buffer.
     *
     */
    uint32_t calculateTransmissionTimeMs(String message);

    void nexSerialBegin(uint32_t upload_baudrate, int line, int rx, int tx);

private:                        /* data */
    uint32_t _baudrate;         /* nextion serail baudrate */
    uint32_t _undownloadByte;   /* undownload byte of tft file */
    uint32_t _upload_baudrate;  /* upload baudrate */
    uint16_t _sent_packets = 0; /* upload baudrate */
    uint8_t _rx;
    uint8_t _tx;
    uint8_t _line;
    THandlerFunction _updateProgressCallback;

#ifdef ESP8266
        SoftwareSerial* nexSerial;
#else
        Stream* nexSerial;
#endif
};
/**
 * @}
 */

#endif /* #ifndef __ESPNEXUPLOAD_H__ */