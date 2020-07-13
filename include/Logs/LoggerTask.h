#pragma once

#include <Logs/LogReader.h>
#include <Base/Writer.h>
#include <Logs/LogEntry.h>
#include <Logs/LogMetadata.h>

#define FILE_READ "r"
#define FILE_WRITE "w"

class LoggerTask {
   public:
    LoggerTask(size_t id, const char* name, unsigned long period, size_t limit);
    ~LoggerTask();

    void publish(Writer*);
    void update();
    void clear();

    LogMetadata* getMetadata();
    const String getName() const;
    const String getMetaFile() const;
    const String getDataFile() const;

   public:
    void init();
    void open();
    void write(LogEntry* entry);
    void close();
    LogMetadata* metadata();
    const String asJson();

   private:
    void storeMeta();
    void publishFile();
    void flush();
    void post();
    void postFile();
    const String getTopic();

   private:
    std::vector<LogEntry> _buf;
    Value* _value;
    LogReader* _reader;
    LogMetadata _meta;
    File _file;

    size_t _id;
    char* _name;
    unsigned long _logInterval;
    size_t _limit;
    unsigned long _lastUpdated;
};