#pragma once

#include <Arduino.h>

class CharBuffer : Print {
   public:
    CharBuffer(size_t size) : _capacity(size < 2 ? 2 : size), _write(0), _read(0) {
        _pool = new char[_capacity + 1];
        memset(_pool, 0, _capacity + 1);
    }

    CharBuffer(const CharBuffer &src) {
        _capacity = src._capacity;
        _write = src._write;
        memcpy(_pool, src._pool, src._write);
    }

    CharBuffer(const char *str) : CharBuffer(strlen(str) + 1) {
        write((const uint8_t *)str, strlen(str));
    }

    ~CharBuffer() {
        delete _pool;
    }

    void clear() {
        memset(_pool, 0, _capacity);
        _write = 0;
        _read = 0;
    }

    size_t size() const { return _capacity; }

    size_t free() const { return _capacity - _write - 2; }

    size_t available() const { return _write; }

    const char *c_str() {
        if (_pool[_write] != '\x00')
            _pool[_write] = '\x00';
        return _pool;
    }

    size_t write(char ch) {
        return write((uint8_t)ch);
    };

    size_t write(const uint8_t ch) {
        size_t n = 0;
        if (_write < (_capacity - 2)) {
            _pool[_write++] = ch;
            n = 1;
        }
        return n;
    }

    size_t write(const uint8_t *ptr, const size_t size) {
        size_t n = 0;
        while (n < size) {
            uint8_t ch = ptr[n++];
            if (!write(ch))
                break;
        }
        return n;
    }

   protected:
    char *_pool;
    size_t _capacity;
    size_t _write;
    size_t _read;
};
