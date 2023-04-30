#include "Global.h"
#include "classes/IoTItem.h"

#include "esp_camera.h"
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems

void handleGetPic();

// ===================
// Select camera model
// ===================
//#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
//#define CAMERA_MODEL_ESP32S3_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
//#define CAMERA_MODEL_M5STACK_UNITCAM // No PSRAM
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM
//#define CAMERA_MODEL_XIAO_ESP32S3 // Has PSRAM
// ** Espressif Internal Boards **
//#define CAMERA_MODEL_ESP32_CAM_BOARD
//#define CAMERA_MODEL_ESP32S2_CAM_BOARD
//#define CAMERA_MODEL_ESP32S3_CAM_LCD


#define LED_LEDC_CHANNEL 2 //Using different ledc channel/timer than camera
#define CONFIG_LED_MAX_INTENSITY 255

#include "camera_pins.h"


IoTItem* globalItem = nullptr;


class EspCam : public IoTItem {
   private:
    bool _useLed, _ticker, _webTicker;

   public:
    bool isUsedLed() { return _useLed; }
    bool isWebTicker() { return _webTicker; }

    EspCam(String parameters): IoTItem(parameters) {
        WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

        jsonRead(parameters, "useLed", _useLed);    // используем = 1 или нет = 0 подсветку (вспышку)
        if (_useLed) {
            ledcSetup(LED_LEDC_CHANNEL, 5000, 8);
            ledcAttachPin(LED_GPIO_NUM, LED_LEDC_CHANNEL);
        }

        jsonRead(parameters, "ticker", _ticker);    // тикать = 1 - сообщаем всем, что сделали снимок и он готов
        jsonRead(parameters, "webTicker", _webTicker);    // сообщать всем, что через веб попросили отдать картинку с камеры
        globalItem = this;   // выносим адрес переменной экземпляра для доступа к данным из обработчика событий веб

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
        //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
        config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
        config.fb_location = CAMERA_FB_IN_PSRAM;
        config.jpeg_quality = 12;
        config.fb_count = 1;
        
        // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
        //                      for larger pre-allocated frame buffer.

        if(psramFound()){
            config.jpeg_quality = 10;
            config.fb_count = 2;
            config.grab_mode = CAMERA_GRAB_LATEST;
        } else {
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
        if (err != ESP_OK) {
            Serial.printf("Camera init failed with error 0x%x", err);
            return;
        }

        sensor_t * s = esp_camera_sensor_get();
        // initial sensors are flipped vertically and colors are a bit saturated
        if (s->id.PID == OV3660_PID) {
            s->set_vflip(s, 1); // flip it back
            s->set_brightness(s, 1); // up the brightness just a bit
            s->set_saturation(s, -2); // lower the saturation
        }

        #if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
        s->set_vflip(s, 1);
        s->set_hmirror(s, 1);
        #endif

        #if defined(CAMERA_MODEL_ESP32S3_EYE)
        s->set_vflip(s, 1);
        #endif

        HTTP.on("/getpic", HTTP_GET, handleGetPic);
    }

    void save_picture() {
        // if (_useLed) digitalWrite(4, HIGH); //Turn on the flash

        // // Take Picture with Camera
        // fb = esp_camera_fb_get();  
        // if(!fb || fb->len >= PICBUF_SIZE) {
        //     if (fb) {
        //         Serial.printf("Camera capture failed size=%d\n", fb->len);
        //         esp_camera_fb_return(fb);
        //     } else Serial.printf("Camera capture failed\n");
        //     return;
        // }
        
        // // if (value.extBinInfoSize < fb->len) {
        // //     if (value.extBinInfo) free(value.extBinInfo);
        // //     value.extBinInfo = (uint8_t*)malloc(sizeof(uint8_t) * fb->len);
        // // }
        // memcpy(value.extBinInfo, fb->buf, fb->len);
        // value.extBinInfoSize = fb->len;

        // Serial.printf("try send pic by size=%d", fb->len);
         
        // if (_useLed) digitalWrite(4, LOW);
        // if (_ticker) regEvent("shot", "EspCam");
        // esp_camera_fb_return(fb);
    }

    void doByInterval() {
       //save_picture();
    }

    IoTValue execute(String command, std::vector<IoTValue> &param) {
        if (command == "save") { 
            save_picture();
        } else if (command == "ledOn" && param.size() == 1) {         
            ledcSetup(LED_LEDC_CHANNEL, 5000, 8);
            ledcAttachPin(LED_GPIO_NUM, LED_LEDC_CHANNEL);
            ledcWrite(LED_LEDC_CHANNEL, param[0].valD);
        } else if (command == "ledOff") { 
            ledcWrite(LED_LEDC_CHANNEL, 0);
        }

        return {};  
    }

    ~EspCam() {
        //free(value.extBinInfo);
        globalItem = nullptr;
    };
};

void handleGetPic() {
    if (!globalItem) return;

    if (((EspCam*)globalItem)->isUsedLed()) ledcWrite(LED_LEDC_CHANNEL, CONFIG_LED_MAX_INTENSITY); //Turn on the flash

    camera_fb_t* fb = NULL;
    fb = esp_camera_fb_get();
    if (!fb) {
        HTTP.send(200, "text/json", F("Item EspCam not prepared yet or camera hasn't taken a picture yet"));
        return;
    }
    
    HTTP.send_P(200, "image/jpeg", (char *)fb->buf, fb->len);

    if (((EspCam*)globalItem)->isUsedLed()) ledcWrite(LED_LEDC_CHANNEL, 0);
    if (((EspCam*)globalItem)->isWebTicker()) globalItem->regEvent("webTakesPhoto", "EspCam");
    esp_camera_fb_return(fb);
}


void* getAPI_EspCam(String subtype, String param) {
    if (subtype == F("EspCam")) {
        return new EspCam(param);
    } else {
        return nullptr;
    }
}
