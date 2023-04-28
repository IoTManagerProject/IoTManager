#include "StandWebServer.h"
#ifdef STANDARD_WEB_SERVER

File uploadFile;
String unsupportedFiles = String();

static const char TEXT_PLAIN[] PROGMEM = "text/plain";
static const char FS_INIT_ERROR[] PROGMEM = "FS INIT ERROR";
static const char FILE_NOT_FOUND[] PROGMEM = "FileNotFound";
//static bool fsOK;
//const char* fsName = "LittleFS";

void standWebServerInit()
{
    //  Кэшировать файлы для быстрой работы
    HTTP.serveStatic("/bundle.js", FileFS, "/", "max-age=31536000");      // кеширование на 1 год
    HTTP.serveStatic("/bundle.css", FileFS, "/", "max-age=31536000");     // кеширование на 1 год
    HTTP.serveStatic("/bundle.js.gz", FileFS, "/", "max-age=31536000");   // кеширование на 1 год
    HTTP.serveStatic("/bundle.css.gz", FileFS, "/", "max-age=31536000");  // кеширование на 1 год
    HTTP.serveStatic("/favicon.png", FileFS, "/", "max-age=31536000");    // кеширование на 1 год

    // HTTP.on("/devicelist.json", HTTP_GET, []() {
    //     HTTP.send(200, "application/json", devListHeapJson);
    // });
    HTTP.on("/settings.h.json", HTTP_GET, []() {
        HTTP.send(200, "application/json", settingsFlashJson);
    });
    // HTTP.on("/settings.f.json", HTTP_GET, []() {
    //     HTTP.send(200, "application/json", readFile(F("settings.json"), 20000));
    // });
    // HTTP.on("/params.json", HTTP_GET, []() {
    //     String json = getParamsJson();
    //     HTTP.send(200, "application/json", json);
    // });
    // HTTP.on("/errors.json", HTTP_GET, []() {
    //     HTTP.send(200, "application/json", errorsHeapJson);
    // });
    // HTTP.on("/config.json", HTTP_GET, []() {
    //     HTTP.send(200, "application/json", readFile(F("config.json"), 20000));
    // });
    // HTTP.on("/layout.json", HTTP_GET, []() {
    //     HTTP.send(200, "application/json", readFile(F("layout.json"), 20000));
    // });
    // HTTP.on("/restart", HTTP_GET, []() {
    //     // ESP.restart();
    //     HTTP.send(200, "text/plain", "ok");
    // });


    HTTP.on("/set", HTTP_GET, []() {
        if (HTTP.hasArg(F("routerssid")) && WiFi.getMode() == WIFI_AP) {
                    jsonWriteStr(settingsFlashJson, F("routerssid"), HTTP.arg(F("routerssid")));
                    syncSettingsFlashJson();
                    HTTP.send(200, "text/plain", "ok");
                }

        if (HTTP.hasArg(F("routerpass")) && WiFi.getMode() == WIFI_AP) {
                    jsonWriteStr(settingsFlashJson, F("routerpass"), HTTP.arg(F("routerpass")));
                    syncSettingsFlashJson();
                    HTTP.send(200, "text/plain", "ok");
                }

            });

    //  Добавляем функцию Update для перезаписи прошивки по WiFi при 1М(256K FileFS) и выше
    //  httpUpdater.setup(&HTTP);

    //  Запускаем HTTP сервер
    HTTP.begin();

    // HTTP страницы для работы с FFS

    ////////////////////////////////
    // WEB SERVER INIT

    // Filesystem status
    HTTP.on("/status", HTTP_GET, handleStatus);

    // List directory
    HTTP.on("/list", HTTP_GET, handleFileList);

    // Load editor
    HTTP.on("/edit", HTTP_GET, handleGetEdit);

    // Create file
    HTTP.on("/edit", HTTP_PUT, handleFileCreate);

    // Delete file
    HTTP.on("/edit", HTTP_DELETE, handleFileDelete);

    // Upload file
    // - first callback is called after the request has ended with all parsed arguments
    // - second callback handles file upload at that location
    HTTP.on("/edit", HTTP_POST, replyOK, handleFileUpload);

    // Default handler for all URIs not defined above
    // Use it to read files from filesystem
    HTTP.onNotFound(handleNotFound);
}

////////////////////////////////
// Utils to return HTTP codes, and determine content-type

void replyOK()
{
    HTTP.send(200, FPSTR(TEXT_PLAIN), "");
}

void replyOKWithMsg(String msg)
{
    HTTP.send(200, FPSTR(TEXT_PLAIN), msg);
}

void replyNotFound(String msg)
{
    HTTP.send(404, FPSTR(TEXT_PLAIN), msg);
}

void replyBadRequest(String msg)
{
//    DBG_OUTPUT_PORT.println(msg);
    HTTP.send(400, FPSTR(TEXT_PLAIN), msg + "\r\n");
}

void replyServerError(String msg)
{
//    DBG_OUTPUT_PORT.println(msg);
    HTTP.send(500, FPSTR(TEXT_PLAIN), msg + "\r\n");
}

/*
   Return the FS type, status and size info
*/
void handleStatus()
{
//    DBG_OUTPUT_PORT.println("handleStatus");
    String json;
    json.reserve(128);

    json = "{\"type\":\"";
    json += FS_NAME;
    json += "\", \"isOk\":";

 #ifdef ESP8266
    FSInfo fs_info;

        FileFS.info(fs_info);
        json += F("\"true\", \"totalBytes\":\"");
        json += fs_info.totalBytes;
        json += F("\", \"usedBytes\":\"");
        json += fs_info.usedBytes;
        json += "\"";
#endif     
#ifdef ESP32
        json += F("\"true\", \"totalBytes\":\"");
        json += String(FileFS.totalBytes());
        json += F("\", \"usedBytes\":\"");
        json += String(FileFS.usedBytes());
        json += "\"";
#endif

    json += F(",\"unsupportedFiles\":\"");
    json += unsupportedFiles;
    json += "\"}";

    HTTP.send(200, "application/json", json);
}

#ifdef ESP32
String getContentType(String filename) {
  if (HTTP.hasArg("download")) {
    return "application/octet-stream";
  } else if (filename.endsWith(".htm")) {
    return "text/html";
  } else if (filename.endsWith(".html")) {
    return "text/html";
  } else if (filename.endsWith(".css")) {
    return "text/css";
  } else if (filename.endsWith(".js")) {
    return "application/javascript";
  } else if (filename.endsWith(".png")) {
    return "image/png";
  } else if (filename.endsWith(".gif")) {
    return "image/gif";
  } else if (filename.endsWith(".jpg")) {
    return "image/jpeg";
  } else if (filename.endsWith(".ico")) {
    return "image/x-icon";
  } else if (filename.endsWith(".xml")) {
    return "text/xml";
  } else if (filename.endsWith(".pdf")) {
    return "application/x-pdf";
  } else if (filename.endsWith(".zip")) {
    return "application/x-zip";
  } else if (filename.endsWith(".gz")) {
    return "application/x-gzip";
  }
  return "text/plain";
}
#endif

/*
   Read the given file from the filesystem and stream it back to the client
*/
bool handleFileRead(String path)
{
//    DBG_OUTPUT_PORT.println(String("handleFileRead: ") + path);
    if (path.endsWith("/"))
    {
        path += "index.html";
    }

    String contentType;
    if (HTTP.hasArg("download"))
    {
        contentType = F("application/octet-stream");
    }
    else
    {
#ifdef ESP32        
        contentType = getContentType(path);
#endif
#ifdef ESP8266
        contentType = mime::getContentType(path);
#endif        
    }

    if (!FileFS.exists(path))
    {
        // File not found, try gzip version
        path = path + ".gz";
    }
    if (FileFS.exists(path))
    {
        File file = FileFS.open(path, "r");
        if (HTTP.streamFile(file, contentType) != file.size())
        {
 //           DBG_OUTPUT_PORT.println("Sent less data than expected!");
        }
        file.close();
        return true;
    }

    return false;
}

/*
   As some FS (e.g. LittleFS) delete the parent folder when the last child has been removed,
   return the path of the closest parent still existing
*/
String lastExistingParent(String path)
{
    while (!path.isEmpty() && !FileFS.exists(path))
    {
        if (path.lastIndexOf('/') > 0)
        {
            path = path.substring(0, path.lastIndexOf('/'));
        }
        else
        {
            path = String(); // No slash => the top folder does not exist
        }
    }
//    DBG_OUTPUT_PORT.println(String("Last existing parent: ") + path);
    return path;
}

/*
   Handle a file upload request
*/
void handleFileUpload()
{
    if (HTTP.uri() != "/edit")
    {
        return;
    }
    HTTPUpload &upload = HTTP.upload();
    if (upload.status == UPLOAD_FILE_START)
    {
        String filename = upload.filename;
        // Make sure paths always start with "/"
        if (!filename.startsWith("/"))
        {
            filename = "/" + filename;
        }
//        DBG_OUTPUT_PORT.println(String("handleFileUpload Name: ") + filename);
        uploadFile = FileFS.open(filename, "w");
        if (!uploadFile)
        {
            return replyServerError(F("CREATE FAILED"));
        }
//        DBG_OUTPUT_PORT.println(String("Upload: START, filename: ") + filename);
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        if (uploadFile)
        {
            size_t bytesWritten = uploadFile.write(upload.buf, upload.currentSize);
            if (bytesWritten != upload.currentSize)
            {
                return replyServerError(F("WRITE FAILED"));
            }
        }
//        DBG_OUTPUT_PORT.println(String("Upload: WRITE, Bytes: ") + upload.currentSize);
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (uploadFile)
        {
            uploadFile.close();
        }
//        DBG_OUTPUT_PORT.println(String("Upload: END, Size: ") + upload.totalSize);
    }
}


#ifdef ESP8266
void deleteRecursive(String path)
{
    File file = FileFS.open(path, "r");
    bool isDir = file.isDirectory();
    file.close();

    // If it's a plain file, delete it
    if (!isDir)
    {
        FileFS.remove(path);
        return;
    }
    Dir dir = FileFS.openDir(path);
    while (dir.next())
    {
        deleteRecursive(path + '/' + dir.fileName());
    }    

    // Then delete the folder itself
    FileFS.rmdir(path);
}
#endif

#ifdef ESP32
struct treename{
  uint8_t type; 
  char    *name;
};


void deleteRecursive( String path ){
    fs::File dir = FileFS.open( path );
    
    if(!dir.isDirectory()){
       Serial.printf("%s is a file\n", path);
       dir.close();
       Serial.printf( "result of removing file %s: %d\n", path, FileFS.remove( path ) );
       return;
    }
    
    Serial.printf("%s is a directory\n", path);
     
    fs::File entry, nextentry;
       
    while ( entry = dir.openNextFile() ){          
      
if ( entry.isDirectory()  ){
         deleteRecursive( entry.path() );        
      } else{ 
         String tmpname = path+"/"+strdup( entry.name() ); // buffer file name
         entry.close();
         Serial.printf( "result of removing file %s: %d\n", tmpname, FileFS.remove( tmpname ) );
      }     
      
    }

    dir.close(); 
    Serial.printf( "result of removing directory %s: %d\n", path, FileFS.rmdir( path ) );  

}
#endif
/*
   Handle a file deletion request
   Operation      | req.responseText
   ---------------+--------------------------------------------------------------
   Delete file    | parent of deleted file, or remaining ancestor
   Delete folder  | parent of deleted folder, or remaining ancestor
*/
void handleFileDelete()
{
    String path = HTTP.arg(0);
    if (path.isEmpty() || path == "/")
    {
        return replyBadRequest("BAD PATH");
    }

//    DBG_OUTPUT_PORT.println(String("handleFileDelete: ") + path);
    if (!FileFS.exists(path))
    {
        return replyNotFound(FPSTR(FILE_NOT_FOUND));
    }
    deleteRecursive(path);



    replyOKWithMsg(lastExistingParent(path));
}

/*
   Handle the creation/rename of a new file
   Operation      | req.responseText
   ---------------+--------------------------------------------------------------
   Create file    | parent of created file
   Create folder  | parent of created folder
   Rename file    | parent of source file
   Move file      | parent of source file, or remaining ancestor
   Rename folder  | parent of source folder
   Move folder    | parent of source folder, or remaining ancestor
*/
void handleFileCreate()
{
    String path = HTTP.arg("path");
    if (path.isEmpty())
    {
        return replyBadRequest(F("PATH ARG MISSING"));
    }

#ifdef USE_SPIFFS
    if (checkForUnsupportedPath(path).length() > 0)
    {
        return replyServerError(F("INVALID FILENAME"));
    }
#endif

    if (path == "/")
    {
        return replyBadRequest("BAD PATH");
    }
    if (FileFS.exists(path))
    {
        return replyBadRequest(F("PATH FILE EXISTS"));
    }

    String src = HTTP.arg("src");
    if (src.isEmpty())
    {
        // No source specified: creation
//        DBG_OUTPUT_PORT.println(String("handleFileCreate: ") + path);
        if (path.endsWith("/"))
        {
            // Create a folder
            path.remove(path.length() - 1);
            if (!FileFS.mkdir(path))
            {
                return replyServerError(F("MKDIR FAILED"));
            }
        }
        else
        {
            // Create a file
            File file = FileFS.open(path, "w");
            if (file)
            {
#ifdef ESP8266                
                file.write((const char *)0);
#endif
#ifdef ESP32
                file.write(0);
#endif                
                file.close();
            }
            else
            {
                return replyServerError(F("CREATE FAILED"));
            }
        }
        if (path.lastIndexOf('/') > -1)
        {
            path = path.substring(0, path.lastIndexOf('/'));
        }
        replyOKWithMsg(path);
    }
    else
    {
        // Source specified: rename
        if (src == "/")
        {
            return replyBadRequest("BAD SRC");
        }
        if (!FileFS.exists(src))
        {
            return replyBadRequest(F("SRC FILE NOT FOUND"));
        }

//        DBG_OUTPUT_PORT.println(String("handleFileCreate: ") + path + " from " + src);

        if (path.endsWith("/"))
        {
            path.remove(path.length() - 1);
        }
        if (src.endsWith("/"))
        {
            src.remove(src.length() - 1);
        }
        if (!FileFS.rename(src, path))
        {
            return replyServerError(F("RENAME FAILED"));
        }
        replyOKWithMsg(lastExistingParent(src));
    }
}

/*
   Return the list of files in the directory specified by the "dir" query string parameter.
   Also demonstrates the use of chunked responses.
*/
#ifdef ESP8266
void handleFileList()
{
    if (!HTTP.hasArg("dir"))
    {
        return replyBadRequest(F("DIR ARG MISSING"));
    }

    String path = HTTP.arg("dir");
    if (path != "/" && !FileFS.exists(path))
    {
        return replyBadRequest("BAD PATH");
    }

//    DBG_OUTPUT_PORT.println(String("handleFileList: ") + path);
    Dir dir = FileFS.openDir(path);
    path.clear();

    // use HTTP/1.1 Chunked response to avoid building a huge temporary string
    if (!HTTP.chunkedResponseModeStart(200, "text/json"))
    {
        HTTP.send(505, F("text/html"), F("HTTP1.1 required"));
        return;
    }

    // use the same string for every line
    String output;
    output.reserve(64);
    while (dir.next())
    {
#ifdef USE_SPIFFS
        String error = checkForUnsupportedPath(dir.fileName());
        if (error.length() > 0)
        {
//            DBG_OUTPUT_PORT.println(String("Ignoring ") + error + dir.fileName());
            continue;
        }
#endif
        if (output.length())
        {
            // send string from previous iteration
            // as an HTTP chunk
            HTTP.sendContent(output);
            output = ',';
        }
        else
        {
            output = '[';
        }

        output += "{\"type\":\"";
        if (dir.isDirectory())
        {
            output += "dir";
        }
        else
        {
            output += F("file\",\"size\":\"");
            output += dir.fileSize();
        }

        output += F("\",\"name\":\"");
        // Always return names without leading "/"
        if (dir.fileName()[0] == '/')
        {
            output += &(dir.fileName()[1]);
        }
        else
        {
            output += dir.fileName();
        }

        output += "\"}";
    }

    // send last string
    output += "]";
    HTTP.sendContent(output);
    HTTP.chunkedResponseFinalize();
}
#endif

#ifdef ESP32
void handleFileList() {
  if (!HTTP.hasArg("dir")) {
    HTTP.send(500, "text/plain", "BAD ARGS");
    return;
  }

  String path = HTTP.arg("dir");
//  DBG_OUTPUT_PORT.println("handleFileList: " + path);


  File root = FileFS.open(path);
  path = String();

  String output = "[";
  if(root.isDirectory()){
      File file = root.openNextFile();
      while(file){
          if (output != "[") {
            output += ',';
          }
          output += "{\"type\":\"";
 //         output += (file.isDirectory()) ? "dir" : "file";
        if (file.isDirectory())
        {
            output += "dir";
        }
        else
        {
            output += F("file\",\"size\":\"");
            output += file.size();
        }

          output += "\",\"name\":\"";
            output += String(file.name());
          output += "\"}";
          file = root.openNextFile();
      }
  }
  output += "]";
  HTTP.send(200, "text/json", output);
  
}
#endif


/*
   The "Not Found" handler catches all URI not explicitly declared in code
   First try to find and return the requested file from the filesystem,
   and if it fails, return a 404 page with debug information
*/
void handleNotFound()
{
#ifdef ESP8266
    String uri = ESP8266WebServer::urlDecode(HTTP.uri()); // required to read paths with blanks
#endif
#ifdef ESP32
    String uri = WebServer::urlDecode(HTTP.uri()); // required to read paths with blanks
#endif
    if (handleFileRead(uri))
    {
        return;
    }

    // Dump debug data
    String message;
    message.reserve(100);
    message = F("Error: File not found\n\nURI: ");
    message += uri;
    message += F("\nMethod: ");
    message += (HTTP.method() == HTTP_GET) ? "GET" : "POST";
    message += F("\nArguments: ");
    message += HTTP.args();
    message += '\n';
    for (uint8_t i = 0; i < HTTP.args(); i++)
    {
        message += F(" NAME:");
        message += HTTP.argName(i);
        message += F("\n VALUE:");
        message += HTTP.arg(i);
        message += '\n';
    }
    message += "path=";
    message += HTTP.arg("path");
    message += '\n';
//    DBG_OUTPUT_PORT.print(message);

    return replyNotFound(message);
}

/*
   This specific handler returns the index.htm (or a gzipped version) from the /edit folder.
   If the file is not present but the flag INCLUDE_FALLBACK_INDEX_HTM has been set, falls back to the version
   embedded in the program code.
   Otherwise, fails with a 404 page with debug information
*/
void handleGetEdit()
{
    if (handleFileRead(F("/edit.htm")))
    {
        return;
    }

#ifdef INCLUDE_FALLBACK_INDEX_HTM
    server.sendHeader(F("Content-Encoding"), "gzip");
    server.send(200, "text/html", index_htm_gz, index_htm_gz_len);
#else
    replyNotFound(FPSTR(FILE_NOT_FOUND));
#endif
}

#endif
