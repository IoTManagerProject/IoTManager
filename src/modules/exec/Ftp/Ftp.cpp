#include "Global.h"
#include "classes/IoTItem.h"

#include <SimpleFTPServer.h>

#define DEFAULT_STORAGE_TYPE_ESP32 STORAGE_LITTLEFS

class FTPModule : public IoTItem
{
private:
    String login;
    String pass;
    FtpServer ftpSrv; // set #define FTP_DEBUG in ESP8266FtpServer.h to see ftp verbose on serial
public:
    FTPModule(String parameters) : IoTItem(parameters)
    {
        jsonRead(parameters, F("login"), login);
        jsonRead(parameters, F("pass"), pass);
        ftpSrv.setCallback(FTPModule::_callback);
        ftpSrv.setTransferCallback(FTPModule::_transferCallback);
        ftpSrv.begin(login.c_str(), pass.c_str(), "Welcome IoTManager FTP server"); // username, password for ftp.   (default 21, 50009 for PASV)
        SerialPrint("I", "FtpServer " + (String)_id, "begin");
    }

    void loop()
    {
        ftpSrv.handleFTP();
    }

    static void _callback(FtpOperation ftpOperation, unsigned int freeSpace, unsigned int totalSpace)
    {
        switch (ftpOperation)
        {
        case FTP_CONNECT:
            SerialPrint("i", "FTP", F("Connected!"));

            break;
        case FTP_DISCONNECT:
            SerialPrint("i", "FTP", F("Disconnected!"));
            break;
        case FTP_FREE_SPACE_CHANGE:
            SerialPrint("i", "FTP", "Free space change, free "  + (String)freeSpace + " of " + (String)totalSpace);
            break;
        default:
            break;
        }
    }

    static void _transferCallback(FtpTransferOperation ftpOperation, const char *name, unsigned int transferredSize)
    {
        switch (ftpOperation)
        {
        case FTP_UPLOAD_START:
            SerialPrint("i","FTP", F("Upload start!"));
            break;
        case FTP_UPLOAD:
            SerialPrint("i","FTP", "Upload of file " + (String)name + " byte " + (String)transferredSize);
            break;
        case FTP_TRANSFER_STOP:
            SerialPrint("i","FTP", F("Finish transfer!"));
            break;
        case FTP_TRANSFER_ERROR:
            SerialPrint("E","FTP", F("Transfer error!"));
            break;
        default:
            break;
        }

        /* FTP_UPLOAD_START = 0,
         * FTP_UPLOAD = 1,
         *
         * FTP_DOWNLOAD_START = 2,
         * FTP_DOWNLOAD = 3,
         *
         * FTP_TRANSFER_STOP = 4,
         * FTP_DOWNLOAD_STOP = 4,
         * FTP_UPLOAD_STOP = 4,
         *
         * FTP_TRANSFER_ERROR = 5,
         * FTP_DOWNLOAD_ERROR = 5,
         * FTP_UPLOAD_ERROR = 5
         */
    }

    ~FTPModule(){};
};

void *getAPI_FTPModule(String subtype, String param)
{
    if (subtype == F("ftp"))
    {
        return new FTPModule(param);
    }
    //}

    return nullptr;
}
