#pragma once

#include "ofMain.h"

class DecodedFrame {
public:
    DecodedFrame() : timestamp(0), index(0) {
    }
    
    float timestamp;
    uint64_t index;
    ofPixels pix;
};
