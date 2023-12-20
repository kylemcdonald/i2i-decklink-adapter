#pragma once

#include "ofMain.h"

class DecodedFrame {
public:
    DecodedFrame() : timestamp(0), index(0) {
        // this is necessary to avoid warnings when the pixelformat is switched later
        pix.allocate(1, 1, OF_IMAGE_COLOR);
    }
    
    float timestamp;
    uint64_t index;
    ofPixels pix;
};
