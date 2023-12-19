#pragma once

#include "ofThread.h"
#include "ofxTurboJpeg.h"
#include "ofxZmq.h"
#include "EncodedMessage.h"
#include "DecodedFrame.h"
#include "RateTimer.h"

class ZmqVideoReceiver : public ofThread {
public:
    RateTimer loopTimer;
    RateTimer receiveTimer;
    ofThreadChannel<DecodedFrame> channel;
    
    ~ZmqVideoReceiver() {
        waitForThread();
    }
    
    void threadedFunction() {
        ofLog() << "starting ZmqVideoReceiver";
        ofxZmqSubscriber sub;
        sub.connect("tcp://0.0.0.0:5558");
        sub.setReceiveHighWaterMark(1);
        
        ofxMessagePack::Unpacker unpacker;
        
        ofxTurboJpeg turbo;
        
        while(isThreadRunning()){
            loopTimer.tick();
            if (!sub.hasWaitingMessage(1)) {
                continue;
            }
            receiveTimer.tick();
            ofBuffer msgBuffer;
            sub.getNextMessage(msgBuffer);
            unpacker.setBuffer(msgBuffer);
            EncodedMessage msg;
            unpacker >> msg;
            
            DecodedFrame frame;
            frame.timestamp = msg.timestamp;
            frame.index = msg.index;
            ofBuffer jpgBuffer((const char*) msg.jpg.data(), msg.jpg.size());
            turbo.load(frame.pix, jpgBuffer);
            
            channel.send(frame);
        }
    }
};
