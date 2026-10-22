#include "AsyncWebServer.h"
#include "StandWebServer.h"  // общая регистрация/обработчики HTTP: standWebServerInit в ASYNC-сборке тоже используется
#include "WsServer.h"        // общий обработчик текстовых WS-команд (handleWsTextMessage)
#ifdef ASYNC_WEB_SERVER

// ---- отладка ASYNC WS через COM-порт (включать временно при диагностике) -------------
#define ASYNC_WS_DEBUG 0
#if ASYNC_WS_DEBUG
#define DEBUG_WS_PRINT(msg) Serial.println(String(millis()) + " [WSD] " + String(msg))
#else
#define DEBUG_WS_PRINT(msg)
#endif

// =====================================================================================
// Объект HTTP: интерфейс синхронного веб-сервера (см. WebServerCompat.h), поэтому
// обработчики в StandWebServer.cpp/UpgradeFirm.cpp/EspCam.cpp общие для обеих реализаций
// =====================================================================================
AsyncWebServerCompat HTTP;

void AsyncWebServerCompat::attach(AsyncWebServerRequest* request) {
    _request = request;
    if (_answeredRequest != request) _answeredRequest = nullptr;  // пришёл новый запрос
}

void AsyncWebServerCompat::detach() {
    _request = nullptr;
}

void AsyncWebServerCompat::detachRequest() {
    _request = nullptr;
    _answeredRequest = nullptr;
}

// имитация последовательности UPLOAD_FILE_START/UPLOAD_FILE_WRITE/UPLOAD_FILE_END синхронного сервера
void AsyncWebServerCompat::uploadBegin(const String& filename, size_t contentLength) {
    _upload.status = UPLOAD_FILE_START;
    _upload.filename = filename;
    _upload.name = "";
    _upload.type = "";
    _upload.contentLength = contentLength;
    _upload.totalSize = contentLength;
    _upload.currentSize = 0;
    _upload.buf = nullptr;
}

void AsyncWebServerCompat::uploadData(uint8_t* data, size_t len) {
    _upload.status = UPLOAD_FILE_WRITE;
    _upload.currentSize = len;
    _upload.buf = data;
}

void AsyncWebServerCompat::uploadEnd() {
    _upload.status = UPLOAD_FILE_END;
    _upload.currentSize = 0;
    _upload.buf = nullptr;
}

// ---- регистрация обработчиков (обёртки привязывают запрос к объекту HTTP) -------------

AsyncStaticWebHandler& AsyncWebServerCompat::serveStatic(const char* uri, fs::FS& fs, const char* path, const char* cache_control) {
    return server.serveStatic(uri, fs, path, cache_control);
}

void AsyncWebServerCompat::on(const char* uri, WebRequestMethodComposite method, void (*handler)()) {
    server.on(uri, method, [handler](AsyncWebServerRequest* request) {
        HTTP.attach(request);
        handler();
        HTTP.detachRequest();
    });
}

void AsyncWebServerCompat::on(const char* uri, WebRequestMethodComposite method, void (*handler)(), void (*uploadHandler)()) {
    server.on(
        uri, method,
        [handler](AsyncWebServerRequest* request) {
            HTTP.attach(request);
            // обработчик ответа (например replyOK) не отвечает второй раз, если ответ уже дан
            // обработчиком загрузки файла (как в /update или при ошибке в /edit)
            if (!HTTP.isAnswerSent(request)) handler();
            HTTP.detachRequest();
        },
        [uploadHandler](AsyncWebServerRequest* request, const String& filename, size_t index, uint8_t* data, size_t len, bool final) {
            // эмулируем последовательность синхронного сервера: START -> WRITE -> END
            HTTP.attach(request);
            if (index == 0) {
                HTTP.uploadBegin(filename, request->contentLength());
                uploadHandler();
            }
            if (len) {
                HTTP.uploadData(data, len);
                uploadHandler();
            }
            if (final) {
                HTTP.uploadEnd();
                uploadHandler();
            }
            HTTP.detach();
        });
}

void AsyncWebServerCompat::onNotFound(void (*handler)()) {
    server.onNotFound([handler](AsyncWebServerRequest* request) {
        HTTP.attach(request);
        handler();
        HTTP.detachRequest();
    });
}

void AsyncWebServerCompat::begin() {
    server.begin();
}
// ---- параметры и заголовки текущего запроса -------------------------------------------

bool AsyncWebServerCompat::hasArg(const char* name) {
    return _request ? _request->hasArg(name) : false;
}

bool AsyncWebServerCompat::hasArg(const String& name) {
    return hasArg(name.c_str());
}

bool AsyncWebServerCompat::hasArg(const __FlashStringHelper* name) {
    return hasArg(String(name).c_str());
}

String AsyncWebServerCompat::arg(const char* name) {
    if (!_request) return "";
    const AsyncWebParameter* param = _request->getParam(String(name));
    return param ? param->value() : String("");
}

String AsyncWebServerCompat::arg(const String& name) {
    return arg(name.c_str());
}

String AsyncWebServerCompat::arg(const __FlashStringHelper* name) {
    return arg(String(name));
}

String AsyncWebServerCompat::arg(size_t number) {
    if (!_request) return "";
    const AsyncWebParameter* param = _request->getParam(number);
    return param ? param->value() : String("");
}

String AsyncWebServerCompat::argName(size_t number) {
    if (!_request) return "";
    const AsyncWebParameter* param = _request->getParam(number);
    return param ? param->name() : String("");
}

size_t AsyncWebServerCompat::args() {
    return _request ? _request->params() : 0;
}

String AsyncWebServerCompat::uri() {
    return _request ? _request->url() : String("");
}

WebRequestMethodComposite AsyncWebServerCompat::method() {
    return _request ? _request->method() : HTTP_ANY;
}

bool AsyncWebServerCompat::hasHeader(const char* name) {
    return _request ? _request->hasHeader(String(name)) : false;
}

String AsyncWebServerCompat::header(const char* name) {
    return _request ? _request->header(name) : String("");
}

// декодирование %XX и '+' в URI (аналог urlDecode синхронных серверов)
String AsyncWebServerCompat::urlDecode(const String& text) {
    char temp[] = "0x00";
    unsigned int len = text.length();
    String decoded;
    decoded.reserve(len);
    for (unsigned int i = 0; i < len; i++) {
        char encodedChar = text.charAt(i);
        if ((encodedChar == '%') && (i + 2 < len)) {
            temp[2] = text.charAt(++i);
            temp[3] = text.charAt(++i);
            decoded.concat((char)strtol(temp, NULL, 16));
        } else if (encodedChar == '+') {
            decoded.concat(' ');
        } else {
            decoded.concat(encodedChar);
        }
    }
    return decoded;
}
// ---- ответ клиенту -------------------------------------------------------------------

void AsyncWebServerCompat::send(int code) {
    if (!_request || isAnswerSent(_request)) return;
    _answeredRequest = _request;
    if (_hasPendingHeader) {
        AsyncWebServerResponse* response = _request->beginResponse(code);
        response->addHeader(_pendingHeaderName, _pendingHeaderValue);
        _hasPendingHeader = false;
        _request->send(response);
    } else {
        _request->send(code);
    }
}

void AsyncWebServerCompat::send(int code, const char* content_type, const String& content) {
    if (!_request || isAnswerSent(_request)) return;
    _answeredRequest = _request;
    if (_hasPendingHeader) {
        AsyncWebServerResponse* response = _request->beginResponse(code, String(content_type), content);
        response->addHeader(_pendingHeaderName, _pendingHeaderValue);
        _hasPendingHeader = false;
        _request->send(response);
    } else {
        _request->send(code, String(content_type), content);
    }
}

void AsyncWebServerCompat::send(int code, const __FlashStringHelper* content_type, const String& content) {
    if (!_request || isAnswerSent(_request)) return;
    _answeredRequest = _request;
    String type(content_type);
    if (_hasPendingHeader) {
        AsyncWebServerResponse* response = _request->beginResponse(code, type, content);
        response->addHeader(_pendingHeaderName, _pendingHeaderValue);
        _hasPendingHeader = false;
        _request->send(response);
    } else {
        _request->send(code, type, content);
    }
}

void AsyncWebServerCompat::send_P(int code, const char* content_type, const char* content, size_t len) {
    if (!_request || isAnswerSent(_request)) return;
    _answeredRequest = _request;
#if defined(ESP8266) || defined(LIBRETINY)
    // на ESP8266 содержимое может лежать в PROGMEM — нужен вариант библиотеки для flash;
    // на LibreTiny используется форк esphome/ESPAsyncWebServer-esphome, где перегрузки
    // send(code, contentType, const uint8_t*, len) нет (добавлена только в ESP32Async 3.11+),
    // поэтому здесь тоже идём через beginResponse_P
    _request->send(_request->beginResponse_P(code, String(content_type), (const uint8_t*)content, len));
#else
    // на ESP32 PROGMEM не отличается от обычной памяти — используем современный API
    // (beginResponse_P в ESPAsyncWebServer помечен как deprecated)
    _request->send(code, String(content_type), (const uint8_t*)content, len);
#endif
}

void AsyncWebServerCompat::sendHeader(const char* name, const char* value) {
    _pendingHeaderName = name;
    _pendingHeaderValue = value;
    _hasPendingHeader = true;
}

// Ответ читает файл не сразу, а позже — из задачи AsyncTCP (_fillBuffer у AsyncFileResponse).
// File — это shared_ptr на общий FileImpl: close() вызывающего кода (handleFileRead) закрыл бы
// файл до отправки тела, и клиент получил бы пустой ответ при корректном Content-Length.
// Поэтому открываем библиотеке собственный дескриптор по пути файла — он закроется в
// деструкторе AsyncFileResponse, когда ответ полностью отправлен.
size_t AsyncWebServerCompat::streamFile(File& file, const String& content_type) {
    if (!_request || isAnswerSent(_request)) return 0;
    size_t size = file.size();
    _answeredRequest = _request;
#ifdef ESP32
    String filePath = file.path();      // полный путь открытого файла
#else
    String filePath = file.fullName();  // ESP8266: name() возвращает только имя без каталога
#endif
    File own = FileFS.open(filePath, "r");
    File& content = own ? own : file;  // не открылся (нет свободных дескрипторов) — деградация до старого поведения
    // path ответа = URI запроса: при открытом "*.gz" и URI без ".gz" библиотека сама добавит
    // Content-Encoding: gzip — как streamFile синхронных серверов
    AsyncWebServerResponse* response = _request->beginResponse(content, _request->url(), content_type.c_str());
    if (_hasPendingHeader) {
        // заголовок, ожидающий ответа (sendHeader перед streamFile), прикладываем к файловому ответу
        response->addHeader(_pendingHeaderName, _pendingHeaderValue);
        _hasPendingHeader = false;
    }
    _request->send(response);
    return size;
}

// mime-тип по расширению (аналог mime::getContentType ESP8266 и getContentType
// ESP32-ветки в StandWebServer.cpp — таблица расширений веб-данных data_svelte)
String AsyncWebServerCompat::getContentType(const String& path) {
    if (path.endsWith(".html") || path.endsWith(".htm")) return "text/html";
    if (path.endsWith(".css")) return "text/css";
    if (path.endsWith(".js")) return "application/javascript";
    if (path.endsWith(".json")) return "application/json";
    if (path.endsWith(".png")) return "image/png";
    if (path.endsWith(".gif")) return "image/gif";
    if (path.endsWith(".jpg") || path.endsWith(".jpeg")) return "image/jpeg";
    if (path.endsWith(".ico")) return "image/x-icon";
    if (path.endsWith(".svg")) return "image/svg+xml";
    if (path.endsWith(".xml")) return "text/xml";
    if (path.endsWith(".pdf")) return "application/x-pdf";
    if (path.endsWith(".zip")) return "application/x-zip";
    if (path.endsWith(".gz")) return "application/x-gzip";
    if (path.endsWith(".woff")) return "font/woff";
    if (path.endsWith(".woff2")) return "font/woff2";
    return "text/plain";
}

// ESP8266-вариант handleFileList() пользуется chunked-ответом: в асинхронном сервере
// фрагменты накапливаются и отдаются одним ответом (для списка файлов это допустимо)
bool AsyncWebServerCompat::chunkedResponseModeStart(int code, const char* content_type) {
    _chunked = "";
    _chunkedType = content_type;
    _chunkedCode = code;
    return true;
}

void AsyncWebServerCompat::sendContent(const String& content) {
    _chunked += content;
}

void AsyncWebServerCompat::chunkedResponseFinalize() {
    send(_chunkedCode, _chunkedType.c_str(), _chunked);
}

// =====================================================================================
// Инициализация асинхронного веб-сервера
// =====================================================================================
void asyncWebServerInit() {
    // Регистрация маршрутов, статики и onNotFound — общий код (StandWebServer.cpp):
    // /set, /status, /list, /edit (GET/PUT/DELETE/POST), /localota, /localota_handler,
    // /update (OTA), CORS; статика /build и /favicon.ico; отдача файлов из FS в том числе *.gz
    standWebServerInit();

    // [DEPRECATED] Базовая HTTP-аутентификация. В STANDARD_WEB_SERVER аутентификации нет,
    // чтобы варианты не отличались, по умолчанию выключена и здесь. Если нужно включить —
    // в standWebServerInit() заменить статику "/" на
    //   server.serveStatic("/", FileFS, "/").setDefaultFile("index.html")
    //         .setAuthentication(jsonReadStr(settingsFlashJson, "weblogin").c_str(),
    //                            jsonReadStr(settingsFlashJson, "webpass").c_str());
    SerialPrint("i", F("WEB"), F("Async WebServer Init"));
}
#endif


#ifdef ASYNC_WEB_SOCKETS

// =====================================================================================
// Веб-сокеты: протокол полностью как у standWebSocket (STANDARD_WEB_SOCKETS)
// =====================================================================================
AsyncWebServer wsAsyncServer(81);    // порт как у standWebSocket
AsyncWebSocket ws("/");              // веб-интерфейс подключается к ws://<ip>:81 (без пути)
AsyncEventSource events("/events");  // [DEPRECATED] SSE прошивкой не используется, оставлено для совместимости

// слоты клиентов: веб-интерфейс и общий код обращаются к клиентам по номерам (как в WebSocketsServer)
static uint32_t asyncWsSlots[WEBSOCKETS_CLIENT_MAX] = {0};
static String asyncWsTextBuffer;  // накопитель сообщения (данные могут прийти частями)

// «застрявшие» клиенты: окно TCP не освобождается (клиент не отдаёт ACK — потерял WiFi).
// При дропе фрейма фиксируем время начала; asyncWebSocketsLoop() закрывает таких клиентов
// через ASYNC_WEB_SOCKETS_STUCK_CLOSE_MS, а до этого момента им не тратится бюджет отправки.
static uint32_t asyncWsStuckSince[WEBSOCKETS_CLIENT_MAX] = {0};

// время последнего приёма данных от клиента (по слотам). Входящий трафик (в том числе /pi|)
// — признак живого клиента: даже при забитом окне TCP его нельзя закрывать как «застрявшего»
static uint32_t asyncWsLastRx[WEBSOCKETS_CLIENT_MAX] = {0};

// очередь принятых сообщений: веб-интерфейс шлёт несколько команд в одном TCP-пакете
// (/devlist| и /| приходят за одну миллисекунду), одиночная ячейка теряла все сообщения,
// кроме последнего. Колбэки AsyncTCP на ESP8266 выполняются в контексте loop(), поэтому
// доступа из двух задач нет; при переносе на ESP32 потребуются критические секции.
#define ASYNC_WS_QUEUE_SIZE 8
static struct {
    String msg;
    uint8_t slot;
} asyncWsQueue[ASYNC_WS_QUEUE_SIZE];
static uint8_t asyncWsQueueCount = 0;
static uint8_t asyncWsQueueHead = 0;  // позиция для записи следующего сообщения

uint32_t asyncWebSocketSlotToId(uint8_t slot) {
    if (slot >= WEBSOCKETS_CLIENT_MAX) return 0;
    return asyncWsSlots[slot];
}

int8_t asyncWebSocketSlotById(uint32_t id) {
    if (!id) return -1;
    for (uint8_t i = 0; i < WEBSOCKETS_CLIENT_MAX; i++) {
        if (asyncWsSlots[i] == id) return i;
    }
    return -1;
}

int8_t asyncWebSocketSlotAdd(uint32_t id) {
    for (uint8_t i = 0; i < WEBSOCKETS_CLIENT_MAX; i++) {
        if (asyncWsSlots[i] == 0) {
            asyncWsSlots[i] = id;
            asyncWsLastRx[i] = millis();
            return i;
        }
    }
    return -1;
}

void asyncWebSocketSlotRemove(uint32_t id) {
    int8_t slot = asyncWebSocketSlotById(id);
    if (slot >= 0) {
        asyncWsSlots[slot] = 0;
        asyncWsStuckSince[slot] = 0;
        asyncWsLastRx[slot] = 0;
    }
}

void asyncWebSocketSlotReset() {
    for (uint8_t i = 0; i < WEBSOCKETS_CLIENT_MAX; i++) {
        asyncWsSlots[i] = 0;
        asyncWsStuckSince[i] = 0;
        asyncWsLastRx[i] = 0;
    }
}

// пометка клиента «застрявшим» (слот по id клиента); первую метку запоминаем как время
static void asyncWsMarkStuck(AsyncWebSocketClient* client) {
    if (!client) return;
    int8_t slot = asyncWebSocketSlotById(client->id());
    if ((slot >= 0) && !asyncWsStuckSince[slot]) asyncWsStuckSince[slot] = millis();
}

static void asyncWsClearStuck(int8_t slot) {
    if (slot >= 0) asyncWsStuckSince[slot] = 0;
}

// лог дропа с rate-limit: не чаще одного сообщения в секунду на слот (при потоковой
// отправке «застрявшему» клиенту дропов было бы много)
static void asyncWsDebugDrop(AsyncWebSocketClient* client, uint8_t opcode, size_t len, const char* reason) {
    static uint32_t asyncWsLastDropLog[WEBSOCKETS_CLIENT_MAX] = {0};
    int8_t slot = client ? asyncWebSocketSlotById(client->id()) : -1;
    if (slot < 0) return;
    if ((millis() - asyncWsLastDropLog[slot]) < 1000) return;
    asyncWsLastDropLog[slot] = millis();
    DEBUG_WS_PRINT("frame DROP: " + String(reason) + ", slot=" + String(slot) + " op=" + String(opcode) + " len=" + String(len));
}

// ---- отправка фреймов ----------------------------------------------------------------
// ESPAsyncWebServer не умеет отправлять одно сообщение несколькими фреймами (у него нет
// аналога sendBIN(..., fin, continuation) из WebSocketsServer), а веб-интерфейс собирает
// файлы по offset'ам из заголовка первого фрейма. Поэтому фреймы формируются вручную
// по RFC 6455 (фреймы сервера не маскируются): данные больших файлов уходят фрагментами,
// а не собираются целиком в оперативной памяти — как и в STANDARD_WEB_SOCKETS.
static bool asyncWsWriteFrame(AsyncWebSocketClient* client, uint8_t opcode, bool fin, const uint8_t* data, size_t len) {
    if (!client || (client->status() != WS_CONNECTED)) {
        DEBUG_WS_PRINT("frame DROP: client null or not connected, op=" + String(opcode) + " len=" + String(len));
        return false;
    }
    AsyncClient* tcp = client->client();
    if (!tcp) {
        DEBUG_WS_PRINT("frame DROP: no tcp, op=" + String(opcode) + " len=" + String(len));
        return false;
    }
    int8_t slot = asyncWebSocketSlotById(client->id());

    // клиент уже признан «застрявшим»: окно TCP не освободилось с прошлой попытки,
    // бюджет не тратим — дропаем сразу; соединение закроет asyncWebSocketsLoop()
    if ((slot >= 0) && asyncWsStuckSince[slot]) {
        asyncWsDebugDrop(client, opcode, len, "stuck client");
        return false;
    }

    // [DEPRECATED] Экспериментальный «быстрый дроп» маленьких бинарных фреймов при занятом
    // TCP-окне убран: под него попадали и мелкие файлы одним фреймом (scenario.txt ~157 Б),
    // из-за чего сценарии переставали отображаться. Маленькие фреймы ждут окно в пределах
    // ASYNC_WEB_SOCKETS_SEND_BUDGET_MS, а живого, но медленного клиента больше не закрывает
    // автозакрытие «застрявших» (см. asyncWsLastRx / asyncWsClearStuck).

    // [DEPRECATED] Раньше здесь синхронно ждали освобождения окна TCP до
    // ASYNC_WEB_SOCKETS_SEND_TIMEOUT (3000 мс): при потере WiFi у клиента loop()
    // замирал на секунды, вместе с ним — сценарии и таймеры. Теперь — короткий
    // бюджет на весь фрейм: статусы (~70 Б) уходят почти всегда мгновенно, а при
    // реальном затыке фрейм дропается (статусы волатильны) и клиент помечается
    // «застрявшим». Для потоковой отправки файлов и крупных JSON (фреймы >=
    // ASYNC_WEB_SOCKETS_SEND_BUDGET_FILES_BYTES) бюджет шире, чтобы здоровый
    // клиент успел подтвердить приём.
    uint32_t budget = (len >= ASYNC_WEB_SOCKETS_SEND_BUDGET_FILES_BYTES)
                          ? ASYNC_WEB_SOCKETS_SEND_BUDGET_FILES_MS
                          : ASYNC_WEB_SOCKETS_SEND_BUDGET_MS;
    uint32_t start = millis();

    // фаза 1: ждём готовности TCP (окно/буфер отправки). Если клиент «мёртв» —
    // окно не освобождается, по истечении бюджета фрейм дропаем и метим клиента
    while (!tcp->canSend()) {
        if ((millis() - start) > budget) {
            asyncWsMarkStuck(client);
            asyncWsDebugDrop(client, opcode, len, "canSend");
            return false;
        }
        delay(1);
        yield();
        if (!client->client() || (client->status() != WS_CONNECTED)) return false;
    }

    uint8_t head[10];
    size_t headLen = 2;
    head[0] = (uint8_t)((fin ? 0x80 : 0x00) | (opcode & 0x0F));
    if (len < 126) {
        head[1] = (uint8_t)len;
    } else if (len < 65536) {
        head[1] = 126;
        head[2] = (uint8_t)(len >> 8);
        head[3] = (uint8_t)(len & 0xFF);
        headLen = 4;
    } else {
        head[1] = 127;
        for (uint8_t i = 0; i < 8; i++) head[2 + i] = (uint8_t)((len >> (8 * (7 - i))) & 0xFF);
        headLen = 10;
    }

    // фаза 2: пишем по частям (окно TCP может быть меньше фрейма — крупные JSON шлются
    // одним фреймом), но суммарно на фрейм выделен тот же бюджет: если до его исчерпания
    // не успели — фрейм считается потерянным, а клиент — «застрявшим» (поток фреймов
    // для него всё равно уже сломан; такого клиента закроет asyncWebSocketsLoop)
    size_t sentHead = 0;
    size_t sentData = 0;
    while ((sentHead < headLen) || (sentData < len)) {
        if (!client->client() || (client->status() != WS_CONNECTED)) return false;
        bool progress = false;
        size_t room = tcp->space();
        if (room) {
            if (sentHead < headLen) {
                size_t part = room < (headLen - sentHead) ? room : (headLen - sentHead);
                size_t sent = tcp->add((const char*)head + sentHead, part, ASYNC_WRITE_FLAG_COPY);
                sentHead += sent;
                room -= sent;
                progress = progress || (sent > 0);
            }
            if ((sentHead == headLen) && room && (sentData < len)) {
                size_t part = room < (len - sentData) ? room : (len - sentData);
                size_t sent = tcp->add((const char*)data + sentData, part, ASYNC_WRITE_FLAG_COPY);
                sentData += sent;
                progress = progress || (sent > 0);
            }
            if (progress) tcp->send();
        }
        if ((sentHead == headLen) && (sentData == len)) break;  // фрейм ушёл целиком
        if (!progress) {
            if ((millis() - start) > budget) {
                asyncWsMarkStuck(client);
                asyncWsDebugDrop(client, opcode, len, "space");
                return false;
            }
            delay(1);
            yield();
        }
    }
    asyncWsClearStuck(slot);
    DEBUG_WS_PRINT("frame out: op=" + String(opcode) + " fin=" + String(fin) + " len=" + String(len));
    return true;
}
#endif



#ifdef ASYNC_WEB_SOCKETS
// ---- отправка данных клиентам (вызывается из WsServer.cpp) ----------------------------

// отправка сообщения (одним фреймом) в выбранный слот
void asyncWebSocketSendBin(uint8_t slot, uint8_t* data, size_t size, bool fin, bool continuation) {
    uint32_t id = asyncWebSocketSlotToId(slot);
    AsyncWebSocketClient* client = ws.client(id);
    if (!client) {
        DEBUG_WS_PRINT("sendBin DROP: no client for slot=" + String(slot) + " id=" + String(id));
        return;
    }
    asyncWsWriteFrame(client, continuation ? WS_CONTINUATION : WS_BINARY, fin, data, size);
}

// отправка сообщения всем подключённым клиентам
void asyncWebSocketSendBinAll(uint8_t* data, size_t size, bool fin, bool continuation) {
    for (uint8_t i = 0; i < WEBSOCKETS_CLIENT_MAX; i++) {
        uint32_t id = asyncWsSlots[i];
        if (!id) continue;
        AsyncWebSocketClient* client = ws.client(id);
        if (!client) continue;
        asyncWsWriteFrame(client, continuation ? WS_CONTINUATION : WS_BINARY, fin, data, size);
    }
}

void asyncWebSocketSendText(uint8_t slot, const String& msg) {
    AsyncWebSocketClient* client = ws.client(asyncWebSocketSlotToId(slot));
    if (!client) return;
    asyncWsWriteFrame(client, WS_TEXT, true, (const uint8_t*)msg.c_str(), msg.length());
}

void asyncWebSocketDisconnect(uint8_t slot) {
    uint32_t id = asyncWebSocketSlotToId(slot);
    DEBUG_WS_PRINT("[WS] asyncWebSocketDisconnect slot=" + String(slot) + " id=" + String(id));
    if (!id) return;
    ws.close(id);
}

int asyncWebSocketCount() {
    return ws.count();
}

// отправка файла фрагментами (аналог sendBIN/broadcastBIN с fin/continuation в STANDARD_WEB_SOCKETS)
void asyncWebSocketSendFileFrames(File& file, const String& header, const String& json, int client_id, size_t frameSize) {
    char buf[32];
    sprintf(buf, "%04d", json.length() + 12);
    String data = header + "|" + String(buf) + "|" + json;

    auto frameBuf = new uint8_t[frameSize];
    if (!frameBuf) {
        SerialPrint("E", "WS", F("no memory for frame"));
        file.close();
        return;
    }

    size_t headerSize = data.length();
    size_t maxPayloadSize = frameSize - headerSize;
    int i = 0;
    while (file.available()) {
        size_t hSize = 0;
        if (i == 0) {
            data.toCharArray((char*)frameBuf, frameSize);
            hSize = headerSize;
        } else {
            maxPayloadSize = frameSize;
        }
        size_t payloadSize = file.read(&frameBuf[hSize], maxPayloadSize);
        if (!payloadSize) break;
        size_t size = hSize + payloadSize;
        bool fin = !file.available();                       // последний фрейм сообщения
        bool continuation = (i > 0);
        if (client_id == -1) {
            asyncWebSocketSendBinAll(frameBuf, size, fin, continuation);
        } else {
            asyncWebSocketSendBin((uint8_t)client_id, frameBuf, size, fin, continuation);
        }
        i++;
    }
    delete[] frameBuf;
    file.close();
}
#endif


#ifdef ASYNC_WEB_SOCKETS
// =====================================================================================
// Инициализация веб-сокетов и приём сообщений
// =====================================================================================
void asyncWebSocketsInit() {
    asyncWebSocketSlotReset();
    for (size_t i = 0; i < WEBSOCKETS_CLIENT_MAX; i++) {
        ws_clients[i] = -1;
    }

    ws.onEvent(onWsEvent);
    wsAsyncServer.addHandler(&ws);
    wsAsyncServer.begin();  // порт 81 — как standWebSocket в STANDARD_WEB_SOCKETS

    // [DEPRECATED] SSE прошивкой не используется, оставлено для совместимости
    events.onConnect([](AsyncEventSourceClient* client) { client->send("", NULL, millis(), 1000); });
    server.addHandler(&events);

    SerialPrint("i", "WS", "Async WS server initialized");
}

// Обработка принятых команд выполняется в loop(): на ESP32 колбэк onWsEvent работает
// в задаче AsyncTCP, а обработка команд (в том числе отправка файлов фреймами) блокирующая
void asyncWebSocketsLoop() {
    ws.cleanupClients(WEBSOCKETS_CLIENT_MAX);

    // автозакрытие «застрявших» клиентов: окно TCP не освобождается дольше
    // ASYNC_WEB_SOCKETS_STUCK_CLOSE_MS (клиент не отдаёт ACK — потерял WiFi).
    // Отправка такому клиенту бесполезна, а держать его — каждый раз тратить
    // бюджет и плодить дропы, поэтому закрываем принудительно.
    for (uint8_t i = 0; i < WEBSOCKETS_CLIENT_MAX; i++) {
        uint32_t stuck = asyncWsStuckSince[i];
        if (stuck && ((millis() - stuck) > ASYNC_WEB_SOCKETS_STUCK_CLOSE_MS)) {
            uint32_t id = asyncWsSlots[i];
            if (id) {
                // клиент продолжает слать данные (/pi| и др.) — он жив, просто не успевает
                // читать поток. Закрывать нельзя: снимаем метку, следующий фрейм получит
                // свежий бюджет, а при реальном обрыве WiFi входящих данных не будет
                if ((millis() - asyncWsLastRx[i]) <= ASYNC_WEB_SOCKETS_STUCK_CLOSE_MS) {
                    asyncWsStuckSince[i] = 0;
                    DEBUG_WS_PRINT("stuck client alive (rx), keep slot=" + String(i));
                    continue;
                }
                DEBUG_WS_PRINT("close stuck client: slot=" + String(i) + " id=" + String(id));
                ws.close(id);
                asyncWebSocketSlotRemove(id);  // EVT_DISCONNECT придёт позже, слот освобождаем сразу
            } else {
                asyncWsStuckSince[i] = 0;      // клиент уже отключился сам
            }
        }
    }

    if (!asyncWsQueueCount) return;

    // очередь заполняется с головы, читаем с хвоста (голова - количество)
    uint8_t idx = (asyncWsQueueHead + ASYNC_WS_QUEUE_SIZE - asyncWsQueueCount) % ASYNC_WS_QUEUE_SIZE;
    uint8_t slot = asyncWsQueue[idx].slot;
    String message = asyncWsQueue[idx].msg;
    asyncWsQueue[idx].msg = "";
    asyncWsQueueCount--;
    DEBUG_WS_PRINT("loop handles slot=" + String(slot) + " msg=" + message);

    // общий обработчик команд веб-интерфейса (см. WsServer.cpp)
    handleWsTextMessage(slot, (uint8_t*)message.c_str(), message.length());
}

void onWsEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT: {
            int8_t slot = asyncWebSocketSlotAdd(client->id());
            SerialPrint("i", "WS " + String(slot), "WS client connected");
            DEBUG_WS_PRINT("EVT_CONNECT id=" + String(client->id()) + " slot=" + String(slot));
            // как в STANDARD_WEB_SOCKETS: больше трёх клиентов не обслуживаем
            if ((slot < 0) || (slot >= 3)) {
                SerialPrint("E", "WS", "Too many clients, connection closed!!!");
                jsonWriteInt(errorsHeapJson, "wse1", 1);
                if (slot >= 0) asyncWebSocketSlotRemove(client->id());
                client->close();
            }
        } break;

        case WS_EVT_DISCONNECT: {
            SerialPrint("i", "WS", "WS client disconnected");
            DEBUG_WS_PRINT("EVT_DISCONNECT id=" + String(client->id()));
            asyncWebSocketSlotRemove(client->id());
        } break;

        case WS_EVT_ERROR: {
            SerialPrint("E", "WS", "WS error: " + String(*((uint16_t*)arg)));
        } break;

        case WS_EVT_PONG: {
            // сервер пинги не рассылает, но если клиент ответил — отмечаем активность
            int8_t slot = asyncWebSocketSlotById(client->id());
            if (slot >= 0) ws_clients[slot] = 1;
        } break;

        case WS_EVT_DATA: {
            AwsFrameInfo* info = (AwsFrameInfo*)arg;
            if (info->opcode != WS_TEXT) break;  // от веб-интерфейса приходят только текстовые команды
            DEBUG_WS_PRINT("EVT_DATA id=" + String(client->id()) + " final=" + String(info->final) + " idx=" + String((uint32_t)info->index) + "/" + String((uint32_t)info->len) + " len=" + String(len));

            // любой входящий трафик — признак живого клиента: снимаем «stuck»-метку, чтобы
            // потоковая отправка файлов не закрыла живое соединение (см. asyncWebSocketsLoop)
            {
                int8_t rxSlot = asyncWebSocketSlotById(client->id());
                if (rxSlot >= 0) {
                    asyncWsLastRx[rxSlot] = millis();
                    asyncWsClearStuck(rxSlot);
                }
            }

            if ((info->num == 0) && (info->index == 0)) asyncWsTextBuffer = "";
            for (size_t i = 0; i < len; i++) asyncWsTextBuffer += (char)data[i];
            // сообщение собрано целиком — ставим в очередь на обработку в loop()
            // (см. asyncWebSocketsLoop)
            if (info->final && ((info->index + len) >= info->len)) {
                int8_t slot = asyncWebSocketSlotById(client->id());
                if (slot >= 0) {
                    if (asyncWsQueueCount < ASYNC_WS_QUEUE_SIZE) {
                        asyncWsQueue[asyncWsQueueHead].slot = slot;
                        asyncWsQueue[asyncWsQueueHead].msg = asyncWsTextBuffer;
                        asyncWsQueueHead = (asyncWsQueueHead + 1) % ASYNC_WS_QUEUE_SIZE;
                        asyncWsQueueCount++;
                        DEBUG_WS_PRINT("queued slot=" + String(slot) + " msg=" + asyncWsTextBuffer);
                    } else {
                        DEBUG_WS_PRINT("WS queue FULL, dropped: " + asyncWsTextBuffer);
                    }
                }
                asyncWsTextBuffer = "";
            }
        } break;

        default: break;
    }
}
#endif

