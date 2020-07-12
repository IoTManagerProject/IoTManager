#pragma once

#include <LittleFS.h>

#define FILE_READ "r"
#define FILE_WRITE "w"

struct LogEntry : public Printable {
    unsigned long _time;
    unsigned short _value;

    LogEntry(unsigned long time, uint16_t value) : _time{time}, _value{value} {};

    unsigned long time() {
        return _time;
    }

    size_t printTo(Print& p) const override {
        size_t res = p.print(_time, DEC);
        res += p.print(' ');
        res += p.println(_value, DEC);
        return res;
    }
};

struct LogMetadata {
    size_t _entries;
    size_t _bytes;
    unsigned long _start_time;
    unsigned long _finish_time;

    LogMetadata() : _entries{0}, _bytes{0}, _start_time{0}, _finish_time{0} {};

    void add(size_t bytes, unsigned long time) {
        if (!_entries) {
            _start_time = time;
        }
        _bytes += bytes;
        _finish_time = time;
        _entries++;
    }

    void print(Print& p) const {
        p.print(F("entries: "));
        p.print(_entries);
        p.print(F(" bytes: "));
        p.print(_bytes);
        p.print(F(" start: "));
        p.print(_start_time);
        p.print(F(" finish: "));
        p.print(_finish_time, DEC);
        p.print(F(" entry_size: "));
        p.println(sizeof(LogEntry), DEC);
    }
};

class LoggerTask {
   public:
    LoggerTask(const String& name, unsigned long period, size_t limit);
    void publish();
    void update();
    void clear();

    LogMetadata* getMetadata();
    const String getName() const;
    const String getMetaFilename() const;
    const String getDataFile() const;

   private:
    void storeMeta();
    enum LoggerState { LOGGER_FLUSH,
                       LOGGER_BUFFER };

    void publishFile();
    void flush();
    void post();
    void postFile();
    const String getTopic();

   public:
    void init();
    void open();
    void write(LogEntry* entry);
    void close();
    LogMetadata* metadata();

   private:
    LoggerState _state;
    std::vector<LogEntry> _buf;
    LogMetadata _meta;
    File _file;
    char _name[32];
    unsigned long _period;
    size_t _limit;
    unsigned long _lastExecute;
};