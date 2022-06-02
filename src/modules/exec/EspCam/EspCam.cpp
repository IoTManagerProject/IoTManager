#include "Global.h"
#include "classes/IoTItem.h"

#include "esp_camera.h"
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems


// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22


IoTItem* globalItem = nullptr;
bool webTicker = false;

void handleGetCam() {
    //Serial.printf("try send pic by size=%d", lastPhotoBSize);
    if (globalItem && globalItem->value.extBinInfoSize) {
        //Serial.printf("try send pic by size=%d", globalItem->value.extBinInfoSize);
        HTTP.send_P(200, "image/jpeg", (char*)globalItem->value.extBinInfo, globalItem->value.extBinInfoSize);
        if (webTicker) globalItem->regEvent("webAsk", "EspCam");
    } else HTTP.send(200, "text/json", "Item EspCam not prepared yet or camera hasn't taken a picture yet");
}

class EspCam : public IoTItem {
   private:
    camera_fb_t * fb = NULL;
    bool _useLed, _ticker, _webTicker;

   public:
    EspCam(String parameters): IoTItem(parameters) {
        WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

        jsonRead(parameters, "useLed", _useLed);    // используем = 1 или нет = 0 подсветку (вспышку)
        jsonRead(parameters, "ticker", _ticker);    // тикать = 1 - сообщаем всем, что сделали снимок и он готов
        jsonRead(parameters, "webTicker", _webTicker);    // сообщать всем, что через веб попросили отдать картинку с камеры
        webTicker = _webTicker;
        globalItem = this;   // выносим адрес переменной экземпляра для доступа к данным из обработчика событий веб

        pinMode(4, OUTPUT);
        digitalWrite(4, LOW);

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
        config.pixel_format = PIXFORMAT_JPEG; 

        value.extBinInfo = (uint8_t*)malloc(sizeof(uint8_t) * 35000);
        
        if(psramFound()){
            config.frame_size = FRAMESIZE_SVGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
            config.jpeg_quality = 12;
            config.fb_count = 1;
            Serial.printf("Camera psramFound\n");
        } else {
            config.frame_size = FRAMESIZE_SVGA;
            config.jpeg_quality = 12;
            config.fb_count = 1;
        }
        
        // Init Camera
        esp_err_t err = esp_camera_init(&config);
        if (err != ESP_OK) {
            Serial.printf("Camera init failed with error 0x%x\n", err);
            return;
        }
        
        HTTP.on("/getcam", HTTP_GET, handleGetCam);
    }

    void take_picture() {
        if (_useLed) digitalWrite(4, HIGH); //Turn on the flash

        // Take Picture with Camera
        fb = esp_camera_fb_get();  
        if(!fb || fb->len >= 35000) {
            Serial.println("Camera capture failed");
            return;
        }
        
        // if (value.extBinInfoSize < fb->len) {
        //     if (value.extBinInfo) free(value.extBinInfo);
        //     value.extBinInfo = (uint8_t*)malloc(sizeof(uint8_t) * fb->len);
        // }
        memcpy(value.extBinInfo, fb->buf, fb->len);
        value.extBinInfoSize = fb->len;

        Serial.printf("try send pic by size=%d", fb->len);
         
        if (_useLed) digitalWrite(4, LOW);
        if (_ticker) regEvent("shot", "EspCam");
        esp_camera_fb_return(fb);
    }

    void doByInterval() {
       take_picture();
    }

    IoTValue execute(String command, std::vector<IoTValue> &param) {
        if (command == "shot") { 
            take_picture();
        }

        return {};  
    }

    ~EspCam() {
        free(value.extBinInfo);
        //globalItem = nullptr;
    };
};

void* getAPI_EspCam(String subtype, String param) {
    if (subtype == F("EspCam")) {
        return new EspCam(param);
    } else {
        return nullptr;
    }
}
