#pragma once

#include <Logs/LogReader.h>
#include <Base/Writer.h>
#include <Logs/LogWriter.h>
#include <Logs/LogEntry.h>
#include <Logs/LogMetadata.h>
#include <Logs/LogBuffer.h>

class LoggerTask {
   public:
    LoggerTask(size_t id, const char* name, unsigned long period, size_t limit);
    ~LoggerTask();

    void publish(LogEntryHandler h);
    void update();
    void clear();

    LogMetadata* getMetadata();

    const String getName() const;
    const String getMetaFile() const;

   public:
    LogMetadata* metadata();
    const String asJson();

   private:
    void publishFile();

   private:
    LogMetadata _meta;
    LogBuffer _buffer;
    Value* _value;
    LogReader* _reader;
    LogWriter* _writer;

    size_t _id;
    char* _name;
    unsigned long _logInterval;
    size_t _limit;
    unsigned long _lastUpdated;
};