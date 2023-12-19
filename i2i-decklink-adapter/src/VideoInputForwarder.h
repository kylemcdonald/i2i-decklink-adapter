#pragma once

#include "ofThread.h"
#include "ofxTurboJpeg.h"
#include "ofxZmq.h"
#include "EncodedMessage.h"
#include "DecodedFrame.h"
#include "RateTimer.h"
#include "ofxDeckLink.h"

class VideoInput {
private:
    ofVideoGrabber video;
//    DeckLinkInput video;
    
public:
    void setup() {
        video.setup(640, 480, false);
//        video.setup();
//        video.start(bmdModeHD1080p30);
    }
    
    void update() {
        video.update();
    }
    
    bool isFrameNew() {
        return video.isFrameNew();
    }
    
    ofPixels& getPixels() {
        return video.getPixels();
    }
    
    void close() {
        
    }
};

class VideoInputForwarder : public ofThread {
public:
    RateTimer loopTimer;
    RateTimer sendTimer;
    ofThreadChannel<DecodedFrame> channel;
    
    ~VideoInputForwarder() {
        waitForThread();
    }
    
    void threadedFunction() {
        VideoInput video;
        video.setup();
        int index = 0;
        
        ofxTurboJpeg turbo;
        
        ofxZmqPublisher pub;
        pub.bind("tcp://0.0.0.0:5555");
        pub.setSendHighWaterMark(1);
        
        while(isThreadRunning()){
            loopTimer.tick();
            video.update();
            if (video.isFrameNew()) {
                uint64_t time = ofGetElapsedTimef();
                
                ofPixels& pix = video.getPixels();
                
                DecodedFrame frame;
                frame.index = index;
                frame.timestamp = time;
                frame.pix = pix;
                channel.send(frame);
                
                ofBuffer buffer;
                turbo.save(buffer, pix);
                
                ofxMessagePack::Packer packer;
                EncodedMessage msg;
                msg.index = index;
                msg.timestamp = time;
                msg.jpg = std::vector<uint8_t>(buffer.getData(), buffer.getData() + buffer.size());
                packer << msg;
                
                pub.send(packer.getBuffer());
                sendTimer.tick();
                
                index++;
            }
            // this adds up to 1ms latency, but reduces wasted CPU cycles
            ofSleepMillis(1);
        }
        
        video.close();
    }
};
