#pragma once
#include "ofFileUtils.h"
#include "ofxMessagePack.h"

class EncodedMessage {
public:
    EncodedMessage() : timestamp(0), index(0), jpg() {
    }

    MSGPACK_DEFINE(timestamp, index, jpg)

    float timestamp;
    uint64_t index;
    std::vector<uint8_t> jpg;
};
