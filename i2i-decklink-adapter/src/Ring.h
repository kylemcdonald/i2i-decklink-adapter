#pragma once

#include <iostream>
#include <vector>
#include <stdexcept>

template <class T>
class Ring {
private:
    std::vector<T> buffer;
    int maxBufferSize = 0;
    int currentSize = 0;
    int startIndex = 0;
    
public:
    void setup(int maxBufferSize) {
        this->maxBufferSize = maxBufferSize;
        buffer.resize(maxBufferSize);
    }

    void add(const T& frame) {
        int effectiveIndex = frame.index % maxBufferSize;
        if (currentSize == maxBufferSize && frame.index >= startIndex) {
            startIndex++;
        }
        buffer[effectiveIndex] = frame;
        if (currentSize < maxBufferSize) {
            currentSize++;
        }
    }
    
    bool empty() {
        return currentSize == 0;
    }
    
    bool full() {
        return currentSize == maxBufferSize;
    }

    const T& get(int index) {
        if (currentSize == 0) {
            throw std::runtime_error("Ring buffer is empty");
        }
        if (index < startIndex) {
            index = startIndex;
        } else if (index >= startIndex + currentSize) {
            index = startIndex + currentSize - 1;
        }
        return buffer[index % maxBufferSize];
    }
};
