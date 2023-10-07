#include "Global.h"
#include "classes/IoTItem.h"
#include "NTP.h"

#include "esp_camera.h"
#include "soc/soc.h"          // Disable brownour problems
#include "soc/rtc_cntl_reg.h" // Disable brownour problems

#include "FS.h"     // SD Card ESP32
#include "SD_MMC.h" // SD Card ESP32

void handleGetPic();

// ===================
// Select camera model
// ===================
// #define CAMERA_MODEL_WROVER_KIT // Has PSRAM
// #define CAMERA_MODEL_ESP_EYE // Has PSRAM
// #define CAMERA_MODEL_ESP32S3_EYE // Has PSRAM
// #define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
// #define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
// #define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
// #define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
// #define CAMERA_MODEL_M5STACK_UNITCAM // No PSRAM
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
// #define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM
// #define CAMERA_MODEL_XIAO_ESP32S3 // Has PSRAM
//  ** Espressif Internal Boards **
// #define CAMERA_MODEL_ESP32_CAM_BOARD
// #define CAMERA_MODEL_ESP32S2_CAM_BOARD
// #define CAMERA_MODEL_ESP32S3_CAM_LCD

#define LED_LEDC_CHANNEL 2 // Using different ledc channel/timer than camera
#define CONFIG_LED_MAX_INTENSITY 255

#include "camera_pins.h"

 IoTItem *_camItem = nullptr;
camera_fb_t *frame = NULL;
class EspCam : public IoTItem
{
private:
    bool _useLed, _ticker, _webTicker, _initSD;

public:
    bool isUsedLed() { return _useLed; }
    bool isWebTicker() { return _webTicker; }

    EspCam(String parameters) : IoTItem(parameters)
    {

        jsonRead(parameters, "ticker", _ticker);       // тикать = 1 - сообщаем всем, что сделали снимок и он готов
        jsonRead(parameters, "webTicker", _webTicker); // сообщать всем, что через веб попросили отдать картинку с камеры
        jsonRead(parameters, "flashOn", _useLed);      // используем = 1 или нет = 0 подсветку (вспышку)
                                                       //        globalItem = this;                             // выносим адрес переменной экземпляра для доступа к данным из обработчика событий веб
        _camItem = this;
        initCam();

        HTTP.on("/getpic", HTTP_GET, handleGetPic);

        initSD();
    }

    void doByInterval()
    {
        // save_picture();
    }

    IoTValue execute(String command, std::vector<IoTValue> &param)
    {
        if (command == "save")
        {
            if (param.size() == 1)
                save_picture(param[0].valS);
            else
                save_picture();
        }
        else if (command == "flashOn")
        {
            ledcSetup(LED_LEDC_CHANNEL, 5000, 8);
            ledcAttachPin(LED_GPIO_NUM, LED_LEDC_CHANNEL);
            _useLed = true;
        }
        else if (command == "flashOff")
        {
            _useLed = false;
        }
        else if (command == "sendFoto")
        {
            sendFoto();
        }
        else if (command == "editFoto")
        {
            editFoto();
        }

        return {};
    }
    /*
    IoTItem *getCAMDriver()
    {
        return this;
    }
*/
    void initCam()
    {
        WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector
        camera_config_t config;
        config.ledc_channel = LEDC_CHANNEL_0;
        config.ledc_timer = LEDC_TIMER_0;
        config.pin_d0 = Y2_GPIO_NUM;
        config.pin_d1 = Y3_GPIO_NUM;
        config.pin_d2 = Y4_GPIO_NUM;
        config.pin_d3 = Y5_GPIO_NUM;
        config.pin_d4 = Y6_GPIO_NUM;
        config.pin_d5 = Y7_GPIO_NUM;
        config.pin_d6 = Y8_GPIO_NUM;
        config.pin_d7 = Y9_GPIO_NUM;
        config.pin_xclk = XCLK_GPIO_NUM;
        config.pin_pclk = PCLK_GPIO_NUM;
        config.pin_vsync = VSYNC_GPIO_NUM;
        config.pin_href = HREF_GPIO_NUM;
        config.pin_sscb_sda = SIOD_GPIO_NUM;
        config.pin_sscb_scl = SIOC_GPIO_NUM;
        config.pin_pwdn = PWDN_GPIO_NUM;
        config.pin_reset = RESET_GPIO_NUM;
        config.xclk_freq_hz = 20000000;
        config.frame_size = FRAMESIZE_UXGA;
        config.pixel_format = PIXFORMAT_JPEG; // for streaming
        // config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
        config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
        config.fb_location = CAMERA_FB_IN_PSRAM;
        config.jpeg_quality = 12;
        config.fb_count = 1;

        // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
        //                      for larger pre-allocated frame buffer.

        if (psramFound())
        {
            config.jpeg_quality = 10;
            config.fb_count = 2;
            config.grab_mode = CAMERA_GRAB_LATEST;
        }
        else
        {
            // Limit the frame size when PSRAM is not available
            config.frame_size = FRAMESIZE_SVGA;
            config.fb_location = CAMERA_FB_IN_DRAM;
        }

#if defined(CAMERA_MODEL_ESP_EYE)
        pinMode(13, INPUT_PULLUP);
        pinMode(14, INPUT_PULLUP);
#endif

        // camera init
        esp_err_t err = esp_camera_init(&config);
        if (err != ESP_OK)
        {
            Serial.printf("Camera init failed with error 0x%x", err);
            return;
        }

        sensor_t *s = esp_camera_sensor_get();
        // initial sensors are flipped vertically and colors are a bit saturated
        if (s->id.PID == OV3660_PID)
        {
            s->set_vflip(s, 1);       // flip it back
            s->set_brightness(s, 1);  // up the brightness just a bit
            s->set_saturation(s, -2); // lower the saturation
        }

        if (_useLed)
        {
            ledcSetup(LED_LEDC_CHANNEL, 5000, 8);
            ledcAttachPin(LED_GPIO_NUM, LED_LEDC_CHANNEL);
        }

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
        s->set_vflip(s, 1);
        s->set_hmirror(s, 1);
#endif

#if defined(CAMERA_MODEL_ESP32S3_EYE)
        s->set_vflip(s, 1);
#endif
    }

    void initSD()
    {
        // Start Micro SD card
        _initSD = true;
        Serial.println("Starting SD Card");
        if (!SD_MMC.begin("/sdcard", true))
        {
            Serial.println("SD Card Mount Failed");
            _initSD = false;
            return;
        }

        uint8_t cardType = SD_MMC.cardType();
        if (cardType == CARD_NONE)
        {
            Serial.println("No SD Card attached");
            _initSD = false;
            return;
        }

        fs::FS &fs = SD_MMC;
        fs.mkdir("/photos");
    }

    void sendFoto()
    {
        if (tlgrmItem)
        {
            if (_useLed)
                ledcWrite(LED_LEDC_CHANNEL, CONFIG_LED_MAX_INTENSITY); // Turn on the flash
            frame = esp_camera_fb_get();
            if (!frame)
            {
                SerialPrint("E", F("Esp-Cam to Telegram"), "Fail esp-cam initialization");
                return;
            }
            if (_useLed)
                ledcWrite(LED_LEDC_CHANNEL, 0);
            (tlgrmItem)->sendFoto((byte *)frame->buf, frame->len, "photo.jpg");
            esp_camera_fb_return(frame);
            SerialPrint("i", F("Esp-Cam to Telegram"), "esp_CAM send foto");
        }
    }

    void editFoto()
    {
        if (tlgrmItem)
        {
            if (_useLed)
                ledcWrite(LED_LEDC_CHANNEL, CONFIG_LED_MAX_INTENSITY); // Turn on the flash
            frame = esp_camera_fb_get();
            if (!frame)
            {
                SerialPrint("E", F("Esp-Cam to Telegram"), "Fail esp-cam initialization");
                return;
            }
            if (_useLed)
                ledcWrite(LED_LEDC_CHANNEL, 0);

            (tlgrmItem)->editFoto((byte *)frame->buf, frame->len, "photo.jpg");
            esp_camera_fb_return(frame);
            SerialPrint("i", F("Esp-Cam to Telegram"), "esp_CAM edit foto");
        }
    }

    void save_picture(String path = "")
    {
        // Save picture to microSD card
        fs::FS &fs = SD_MMC;

        if (path == "")
        {
            path = "/photos/";
            path += getTodayDateDotFormated();
            //           path += "/";
            fs.mkdir(path.c_str());

            char buf[32];
            sprintf(buf, "%02d-%02d-%02d", _time_local.hour, _time_local.minute, _time_local.second);
            path += buf;
            path += ".jpg";
        }
        Serial.println(path);

        if (_useLed)
            ledcWrite(LED_LEDC_CHANNEL, CONFIG_LED_MAX_INTENSITY); // Turn on the flash

        // Take Picture with Camera
        frame = esp_camera_fb_get();
        if (_useLed)
            ledcWrite(LED_LEDC_CHANNEL, 0); // Turn on the flash

        if (!frame)
        {
            Serial.println("Camera capture failed");
            return;
        }

        File file = fs.open(path.c_str(), FILE_WRITE);
        if (!file)
        {
            Serial.println("Failed to open file in writing mode");
        }
        else
        {
            file.write(frame->buf, frame->len); // payload (image), payload length
            Serial.printf("Saved file to path: %s\n", path.c_str());
        }
        file.close();

        // return the frame buffer back to the driver for reuse
        esp_camera_fb_return(frame);
    }

    ~EspCam()
    {
        // free(value.extBinInfo);
        _camItem = nullptr;
    };
};

void handleGetPic()
{
    if (!_camItem)
        return;

    if (((EspCam *)_camItem)->isUsedLed())
        ledcWrite(LED_LEDC_CHANNEL, CONFIG_LED_MAX_INTENSITY); // Turn on the flash

 //   camera_fb_t *fb = NULL;
    frame = esp_camera_fb_get();

    if (((EspCam *)_camItem)->isUsedLed())
        ledcWrite(LED_LEDC_CHANNEL, 0);

    if (!frame)
    {
        HTTP.send(200, "text/json", F("Item EspCam not prepared yet or camera hasn't taken a picture yet"));
        return;
    }

    HTTP.send_P(200, "image/jpeg", (char *)frame->buf, frame->len);

    if (((EspCam *)_camItem)->isWebTicker())
        _camItem->regEvent("webTakesPhoto", "EspCam");
    esp_camera_fb_return(frame);
}

void *getAPI_EspCam(String subtype, String param)
{
    if (subtype == F("EspCam"))
    {
        return new EspCam(param);
    }
    else
    {
        return nullptr;
    }
}
