#pragma once
// =====================================================================================
// ASYNC_WEB_SERVER: интерфейс, совместимый с синхронным веб-сервером
// -------------------------------------------------------------------------------------
// Синхронный сервер (STANDARD_WEB_SERVER) даёт объект HTTP с методами
// serveStatic/on/hasArg/arg/send/upload/streamFile и т.п.
// Асинхронный сервер (ESPAsyncWebServer) работает через колбэки, в которые передаётся
// объект запроса AsyncWebServerRequest. Чтобы один и тот же код обработчиков
// (StandWebServer.cpp, UpgradeFirm.cpp, модуль EspCam) собирался для обеих реализаций,
// здесь объявлен класс-обёртка AsyncWebServerCompat с интерфейсом синхронного сервера и
// глобальный объект HTTP (вместо объекта ядра).
//
// Текущий запрос привязывается к обёртке в AsyncWebServer.cpp (метод attach()).
// =====================================================================================

#include "Global.h"
#include <ESPAsyncWebServer.h>

#ifdef ASYNC_WEB_SERVER

// --- совместимые с ядром Arduino статусы загрузки файла -----------------------------
// в асинхронной сборке ядро не подключает WebServer.h/ESP8266WebServer.h, объявляем сами
enum HTTPUploadStatus { UPLOAD_FILE_START, UPLOAD_FILE_WRITE, UPLOAD_FILE_END, UPLOAD_FILE_ABORTED };

// Буфер текущей загрузки файла — аналог HTTPUpload синхронного сервера.
// Отличие: buf — указатель на буфер ESPAsyncWebServer (в ядре это массив uint8_t)
struct AsyncWebUpload {
    HTTPUploadStatus status = UPLOAD_FILE_START;  // статус загрузки (начало/запись/конец)
    String filename;                              // имя загружаемого файла
    String name;                                  // имя поля формы
    String type;                                  // тип содержимого
    size_t totalSize = 0;                         // полный размер файла (из Content-Length)
    size_t currentSize = 0;                       // размер текущего фрагмента
    size_t contentLength = 0;                     // длина тела запроса
    uint8_t* buf = nullptr;                       // данные текущего фрагмента
};
typedef AsyncWebUpload HTTPUpload;  // общий код обработчиков пишет: HTTPUpload& upload = HTTP.upload();

class AsyncWebServerCompat {
  private:
    AsyncWebServerRequest* _request = nullptr;          // текущий обрабатываемый запрос
    AsyncWebServerRequest* _answeredRequest = nullptr;  // запрос, на который ответ уже отправлен
    AsyncWebUpload _upload;                             // буфер текущей загрузки файла
    String _chunked;                                    // накопитель chunked-ответа (эмуляция ESP8266)
    String _chunkedType;                                // тип содержимого chunked-ответа
    int _chunkedCode = 200;                             // код chunked-ответа
    String _pendingHeaderName;                          // заголовок, ожидающий send()
    String _pendingHeaderValue;
    bool _hasPendingHeader = false;

  public:
    // ---- служебные методы (используются обёртками в AsyncWebServer.cpp) ----
    void attach(AsyncWebServerRequest* request);      // привязать текущий запрос
    void detach();                                    // отвязать запрос (обработчик загрузки)
    void detachRequest();                             // отвязать запрос и сбросить признак ответа
    bool isAnswerSent(AsyncWebServerRequest* request) { return (request != nullptr) && (request == _answeredRequest); }
    // эмуляция последовательности UPLOAD_FILE_START / WRITE / END синхронного сервера
    void uploadBegin(const String& filename, size_t contentLength);
    void uploadData(uint8_t* data, size_t len);
    void uploadEnd();

    // ---- регистрация обработчиков (интерфейс синхронного сервера) ----
    AsyncStaticWebHandler& serveStatic(const char* uri, fs::FS& fs, const char* path, const char* cache_control = nullptr);
    void on(const char* uri, WebRequestMethodComposite method, void (*handler)());
    void on(const char* uri, WebRequestMethodComposite method, void (*handler)(), void (*uploadHandler)());
    void onNotFound(void (*handler)());
    void begin();

    // ---- параметры и заголовки текущего запроса ----
    bool hasArg(const char* name);
    bool hasArg(const String& name);
    bool hasArg(const __FlashStringHelper* name);
    String arg(const char* name);
    String arg(const String& name);
    String arg(const __FlashStringHelper* name);
    String arg(size_t number);
    // HTTP.arg(0) в общем коде: без перегрузки литерал 0 неоднозначен (size_t или указатель)
    String arg(int number) { return arg((size_t)number); }
    String argName(size_t number);
    size_t args();
    String uri();
    String urlDecode(const String& text);
    String getContentType(const String& path);  // mime-тип по расширению (аналог mime::getContentType ESP8266)
    WebRequestMethodComposite method();
    bool hasHeader(const char* name);
    String header(const char* name);
    HTTPUpload& upload() { return _upload; }

    // ---- ответ клиенту ----
    void send(int code);
    void send(int code, const char* content_type, const String& content);
    // перегрузка для FPSTR/F-строк: на ESP8266 FPSTR() возвращает const __FlashStringHelper*
    void send(int code, const __FlashStringHelper* content_type, const String& content);
    void send_P(int code, const char* content_type, const char* content, size_t len);
    void sendHeader(const char* name, const char* value);
    size_t streamFile(File& file, const String& content_type);
    // эмуляция chunked-ответа ESP8266WebServer: данные накапливаются и уходят одним ответом
    bool chunkedResponseModeStart(int code, const char* content_type);
    void sendContent(const String& content);
    void chunkedResponseFinalize();
};

extern AsyncWebServerCompat HTTP;
#endif
