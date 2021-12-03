#include "FSEditor.h"

#include "FileSystem.h"

#define FS_MAXLENGTH_FILEPATH 32

const char *excludeListFile = "/.exclude.files";
typedef struct ExcludeListS {
    char *item;
    ExcludeListS *next;
} ExcludeList;

static ExcludeList *excludes = NULL;

static bool matchWild(const char *pattern, const char *testee) {
    const char *nxPat = NULL, *nxTst = NULL;

    while (*testee) {
        if ((*pattern == '?') || (*pattern == *testee)) {
            pattern++;
            testee++;
            continue;
        }
        if (*pattern == '*') {
            nxPat = pattern++;
            nxTst = testee;
            continue;
        }
        if (nxPat) {
            pattern = nxPat + 1;
            testee = ++nxTst;
            continue;
        }
        return false;
    }
    while (*pattern == '*') {
        pattern++;
    }
    return (*pattern == 0);
}

static bool addExclude(const char *item) {
    size_t len = strlen(item);
    if (!len) {
        return false;
    }
    ExcludeList *e = (ExcludeList *)malloc(sizeof(ExcludeList));
    if (!e) {
        return false;
    }
    e->item = (char *)malloc(len + 1);
    if (!e->item) {
        free(e);
        return false;
    }
    memcpy(e->item, item, len + 1);
    e->next = excludes;
    excludes = e;
    return true;
}

static void loadExcludeList(fs::FS &_fs, const char *filename) {
    static char linebuf[FS_MAXLENGTH_FILEPATH];
    fs::File excludeFile = _fs.open(filename, "r");
    if (!excludeFile) {
        return;
    }
    if (excludeFile.isDirectory()) {
        excludeFile.close();
        return;
    }
    if (excludeFile.size() > 0) {
        uint8_t idx;
        bool isOverflowed = false;
        while (excludeFile.available()) {
            linebuf[0] = '\0';
            idx = 0;
            int lastChar;
            do {
                lastChar = excludeFile.read();
                if (lastChar != '\r') {
                    linebuf[idx++] = (char)lastChar;
                }
            } while ((lastChar >= 0) && (lastChar != '\n') && (idx < FS_MAXLENGTH_FILEPATH));

            if (isOverflowed) {
                isOverflowed = (lastChar != '\n');
                continue;
            }
            isOverflowed = (idx >= FS_MAXLENGTH_FILEPATH);
            linebuf[idx - 1] = '\0';
            if (!addExclude(linebuf)) {
                excludeFile.close();
                return;
            }
        }
    }
    excludeFile.close();
}

static bool isExcluded(fs::FS &_fs, const char *filename) {
    if (excludes == NULL) {
        loadExcludeList(_fs, excludeListFile);
    }
    if (strcmp(excludeListFile, filename) == 0) return true;
    ExcludeList *e = excludes;
    while (e) {
        if (matchWild(e->item, filename)) {
            return true;
        }
        e = e->next;
    }
    return false;
}

// WEB HANDLER IMPLEMENTATION

#ifdef ESP32
FSEditor::FSEditor(const fs::FS &fs, const String &username, const String &password)
#else
FSEditor::FSEditor(const String &username, const String &password, const fs::FS &fs)
#endif
    : _fs(fs), _username(username), _password(password), _authenticated(false), _startTime(0) {
}

bool FSEditor::canHandle(AsyncWebServerRequest *request) {
    if (request->url().equalsIgnoreCase("/edit")) {
        if (request->method() == HTTP_GET) {
            if (request->hasParam("list"))
                return true;
            if (request->hasParam("edit")) {
                request->_tempFile = _fs.open(request->arg("edit"), "r");
                if (!request->_tempFile) {
                    return false;
                }
                if (request->_tempFile.isDirectory()) {
                    request->_tempFile.close();
                    return false;
                }
            }
            if (request->hasParam("download")) {
                request->_tempFile = _fs.open(request->arg("download"), "r");
                if (!request->_tempFile) {
                    return false;
                }
                if (request->_tempFile.isDirectory()) {
                    request->_tempFile.close();
                    return false;
                }
            }
            request->addInterestingHeader("If-Modified-Since");
            return true;
        } else if (request->method() == HTTP_POST)
            return true;
        else if (request->method() == HTTP_DELETE)
            return true;
        else if (request->method() == HTTP_PUT)
            return true;
    }
    return false;
}

#ifdef ESP8266
void FSEditor::getDirList(const String &path, String &output) {
    auto dir = _fs.openDir(path.c_str());
    while (dir.next()) {
        String fname = dir.fileName();
        if (!path.endsWith("/") && !fname.startsWith("/")) {
            fname = "/" + fname;
        }
        fname = path + fname;
        if (isExcluded(_fs, fname.c_str())) {
            continue;
        }
        if (dir.isDirectory()) {
            getDirList(fname, output);
            continue;
        }
        if (output != "[") output += ',';
        char buf[128];
        sprintf(buf, "{\"type\":\"file\",\"name\":\"%s\",\"size\":%d}", fname.c_str(), dir.fileSize());
        output += buf;
    }
}
#else
void FSEditor::getDirList(const String &path, String &output) {
    File dir = _fs.open(path.c_str(), FILE_READ);
    dir.rewindDirectory();
    File file;
    while (file = dir.openNextFile()) {
        String fname = file.name();
        //if (!path.endsWith("/") && !fname.startsWith("/")) {
        //    fname = "/" + fname;
        //}
        //fname = path + fname;
        //if (isExcluded(_fs, fname.c_str())) {
        //    continue;
        //}
        //if (dir.isDirectory()) {
        //    getDirList(fname, output);
        //    continue;
        //}
        if (output != "[") output += ',';
        char buf[128];
        sprintf(buf, "{\"type\":\"file\",\"name\":\"%s\",\"size\":%d}", fname.c_str(), dir.size());
        output += buf;
    }
}
#endif

void FSEditor::handleRequest(AsyncWebServerRequest *request) {
    if (_username.length() && _password.length() && !request->authenticate(_username.c_str(), _password.c_str()))
        return request->requestAuthentication();

    if (request->method() == HTTP_GET) {
        if (request->hasParam("list")) {
            if (request->hasParam("list")) {
                String path = request->getParam("list")->value();
                String output = "[";
                getDirList(path, output);
                output += "]";
                request->send(200, "application/json", output);
                output = String();
            }
        } else if (request->hasParam("edit") || request->hasParam("download")) {
#ifdef ESP8266
            request->send(request->_tempFile, request->_tempFile.fullName(), String(), request->hasParam("download"));
#else
            request->send(request->_tempFile, request->_tempFile.name(), String(), request->hasParam("download"));
#endif
        } else {
            const char *buildTime = __DATE__ " " __TIME__ " GMT";
            if (request->header("If-Modified-Since").equals(buildTime)) {
                request->send(304);
            } else {
                AsyncWebServerResponse *response = request->beginResponse(FileFS, "/edit.htm", "text/html");
                // response->addHeader("Content-Encoding", "gzip");
                response->addHeader("Last-Modified", buildTime);
                request->send(response);
            }
        }
    } else if (request->method() == HTTP_DELETE) {
        if (request->hasParam("path", true)) {
            if (!(_fs.remove(request->getParam("path", true)->value()))) {
#ifdef ESP32
                _fs.rmdir(request->getParam("path", true)->value());  // try rmdir for littlefs
#endif
            }

            request->send(200, "", "DELETE: " + request->getParam("path", true)->value());
        } else
            request->send(404);
    } else if (request->method() == HTTP_POST) {
        if (request->hasParam("data", true, true) && _fs.exists(request->getParam("data", true, true)->value()))
            request->send(200, "", "UPLOADED: " + request->getParam("data", true, true)->value());

        else if (request->hasParam("rawname", true) && request->hasParam("raw0", true)) {
            String rawnam = request->getParam("rawname", true)->value();

            if (_fs.exists(rawnam)) _fs.remove(rawnam);  // delete it to allow a mode

            int k = 0;
            uint16_t i = 0;
            fs::File f = _fs.open(rawnam, "a");

            while (request->hasParam("raw" + String(k), true)) {  //raw0 .. raw1
                if (f) {
                    i += f.print(request->getParam("raw" + String(k), true)->value());
                }
                k++;
            }
            f.close();
            request->send(200, "", "IPADWRITE: " + rawnam + ":" + String(i));

        } else {
            request->send(500);
        }

    } else if (request->method() == HTTP_PUT) {
        if (request->hasParam("path", true)) {
            String filename = request->getParam("path", true)->value();
            if (_fs.exists(filename)) {
                request->send(200);
            } else {
/*******************************************************/
#ifdef ESP32
                if (strchr(filename.c_str(), '/')) {
                    // For file creation, silently make subdirs as needed.  If any fail,
                    // it will be caught by the real file open later on
                    char *pathStr = strdup(filename.c_str());
                    if (pathStr) {
                        // Make dirs up to the final fnamepart
                        char *ptr = strchr(pathStr, '/');
                        while (ptr) {
                            *ptr = 0;
                            _fs.mkdir(pathStr);
                            *ptr = '/';
                            ptr = strchr(ptr + 1, '/');
                        }
                    }
                    free(pathStr);
                }
#endif
                /*******************************************************/
                fs::File f = _fs.open(filename, "w");
                if (f) {
                    f.write((uint8_t)0x00);
                    f.close();
                    request->send(200, "", "CREATE: " + filename);
                } else {
                    request->send(500);
                }
            }
        } else
            request->send(400);
    }
}

void FSEditor::handleUpload(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!index) {
        if (!_username.length() || request->authenticate(_username.c_str(), _password.c_str())) {
            _authenticated = true;
            request->_tempFile = _fs.open(filename, "w");
            _startTime = millis();
        }
    }
    if (_authenticated && request->_tempFile) {
        if (len) {
            request->_tempFile.write(data, len);
        }
        if (final) {
            request->_tempFile.close();
        }
    }
}
