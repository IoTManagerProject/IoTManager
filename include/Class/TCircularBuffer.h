#pragma once

#include <Arduino.h>

template <typename T, size_t BUFFER_SIZE>

class TCircularBuffer {
   public:
    TCircularBuffer() : _head{0}, _tail{0}, _full{false} {}

    ~TCircularBuffer() {}

    void reset() {
        _head = _tail = _full = 0;
    }

    bool empty() const {
        return _head == _tail && !_full;
    }

    bool full() const {
        return _full;
    }

    /**
     * Get the number of elements currently stored in the circular_buffer.
     */
    size_t size() const {
        size_t res = 0;
        if (!_full) {
            if (_head < _tail)
                res = BUFFER_SIZE + _head - _tail;
            else
                res = _head - _tail;
        } else {
            res = BUFFER_SIZE;
        }
        return res;
    }

    /** Push the transaction to the buffer. This overwrites the buffer if it's full.
     *  Загрузить данные в буфер
     * @param data item to be pushed to the buffer.
     */
    void push(const T &item) {
        if (_full) {
            _tail++;
            if (_tail == BUFFER_SIZE)
                _tail = 0;
        }
        _pool[_head++] = item;
        if (_head == BUFFER_SIZE)
            _head = 0;
        if (_head == _tail)
            _full = true;
    }

    /** Pop from the buffer.
     *  Забрать данные из буфера
     * @param data item to store the data to be popped from the buffer.
     * @return True if data popped.
     */
    bool pop(T &item) {
        bool res = false;
        if (!empty()) {
            item = _pool[_tail++];
            if (_tail == BUFFER_SIZE) _tail = 0;
            _full = false;
            res = true;
        }
        return res;
    }

    bool pop_back(T &item) {
        bool res = false;
        if (!empty()) {
            item = _pool[--_head];
            _full = false;
            res = true;
        }
        return res;
    }

    /** Peek into circular buffer without popping.
     *
     * @param data item to be peeked from the buffer.
     * @return True if the buffer is not empty and data contains a transaction, false otherwise.
     */
    bool peek(T &item) const {
        bool res = false;
        if (!empty()) {
            item = _pool[_tail];

            res = true;
        }
        return res;
    }

   private:
    T _pool[BUFFER_SIZE];
    size_t _head;
    size_t _tail;
    bool _full;
};

// extern TCircularBuffer<char *, 1024> *myTCircularBuffer;
// extern TCircularBuffer<char *, 20480> *myWsBuffer;
