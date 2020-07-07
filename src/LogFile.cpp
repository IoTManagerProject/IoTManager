#include "LogFile.h"
#include "Utils/FileUtils.h"
LogFile::LogFile(const char* name) {
    strcpy(_name, name);
    init();
};

void LogFile::init() {
    char filename[32];
    strcpy(filename, "log_");
    strcat(filename, _name);
    strcat(filename, ".dat");

    if (LittleFS.exists(filename)) {
        auto file = LittleFS.open(filename, FILE_READ);
        file.read((uint8_t*)&_meta, sizeof(_meta));
        file.close();
    }
}

void LogFile::open() {
    char filename[32];
    strcpy(filename, "log_");
    strcat(filename, _name);
    strcat(filename, ".txt");
    _file = LittleFS.open(filename, "a");
}

void LogFile::write(LogEntry* entry) {
    if (_file) {
        size_t bytes = entry->printTo(_file);
        _meta.add(bytes, entry->time());
    }
}

void LogFile::clear() {
    char filename[32];
    strcpy(filename, "log_");
    strcat(filename, _name);
    strcat(filename, ".txt");
    removeFile(filename);
}

void LogFile::close() {
    _file.flush();
    _file.close();

    char filename[32];
    strcpy(filename, "log_");
    strcat(filename, _name);
    strcat(filename, ".dat");

    auto file = LittleFS.open(filename, FILE_WRITE);
    file.write((uint8_t*)&_meta, sizeof(_meta));
    file.close();
}

LogMetadata* LogFile::metadata() {
    return &_meta;
}