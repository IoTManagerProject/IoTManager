#pragma once

#include "Base/CharBuffer.h"

class EditLine : public CharBuffer {
   public:
    EditLine(size_t size) : CharBuffer(size){};

    char &operator[](size_t i) { return _pool[i]; }

    char operator[](size_t i) const { return _pool[i]; }

    EditLine &operator=(const EditLine &src) {
        delete[] _pool;
        _pool = new char[src._capacity];
        memcpy(_pool, src._pool, src._capacity);
        _read = src._read;
        _write = src._write;
        return *this;
    }

    void del() {
        size_t i;
        for (i = _write; i < _capacity; ++i)
            _pool[i] = _pool[i + 1];
        _pool[i] = '\x00';
    }

    bool backspace() {
        bool res = false;
        if (prev()) {
            del();
            res = true;
        }
        return res;
    }

    bool next() {
        bool res = false;
        if (_write < _capacity - 1) {
            _write++;
            res = true;
        }
        return res;
    }

    bool prev() {
        bool res = false;
        if (_write > 0) {
            _write--;
            res = true;
        }
        return res;
    }

    size_t home() {
        size_t res = _write;
        _write = 0;
        return res;
    }

    size_t end() {
        size_t n;
        for (n = 0; n < _capacity - 1; ++n)
            if (_pool[n] == '\x00') break;
        return n;
    }
};
