#pragma once

#include "ofThread.h"
#include "ofxTurboJpeg.h"
#include "ofxZmq.h"
#include "ofxDeckLink.h"

#include "EncodedMessage.h"
#include "DecodedFrame.h"
#include "RateTimer.h"
#include "ColorConversion.h"

//#define WEBCAM_INPUT

class VideoInput {
private:
#ifdef WEBCAM_INPUT
    ofVideoGrabber video;
#else
    ofxDeckLinkAPI::Input video;
    ColorConversion color;
    ofPixels pix_rgb;
    uint64_t frame_index;
#endif
    
public:
    void setup() {
#ifdef WEBCAM_INPUT
        video.setup(1920, 1080, false);
#else
        video.setup(0, false);
        video.start(bmdModeHD1080i5994);
        color.setup();
        frame_index = 0;
#endif
    }
    
    void update() {
        video.update();
    }
    
    bool isFrameNew() {
        bool newFrame = video.isFrameNew();
        if (newFrame) {
            video.markFrameOld();
            return true;
        }
        return false;
    }
    
    ofPixels& getPixels() {
#ifdef WEBCAM_INPUT
        return video.getPixels();
#else
        ofPixels& pix_in = video.getPixels();
        int width = pix_in.getWidth();
        int height = pix_in.getHeight();
        pix_rgb.allocate(width, height, OF_IMAGE_COLOR);
        int n = width * height;
        color.cby0cry1_to_rgb(pix_in.getData(), pix_rgb.getData(), n);
        return pix_rgb;
#endif
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
        pub.bind("tcp://0.0.0.0:5554");
        pub.setSendHighWaterMark(1);
        
        while(isThreadRunning()){
            loopTimer.tick();
            video.update();
            if (video.isFrameNew()) {
                uint64_t time = ofGetElapsedTimef();
                
                ofPixels& pix = video.getPixels();
                
                // low-quality NN interpolation
                pix.resize(960, 540);
                pix.setImageType(OF_IMAGE_COLOR);
                
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
