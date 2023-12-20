#pragma once

#include "ofMain.h"
#include "VideoInputForwarder.h"
#include "ZmqVideoReceiver.h"
#include "Ring.h"
#include "DecodedFrame.h"
#include "RateTimer.h"

#define DECKLINK_OUTPUT

class ofApp : public ofBaseApp{

public:
	void setup();
	void update();
	void draw();
	void exit();
	void keyPressed(int key);
    
    VideoInputForwarder videoInputForwarder;
    Ring<DecodedFrame> videoInputRing;
    
    ZmqVideoReceiver zmqVideoReceiver;
    Ring<DecodedFrame> zmqVideoRing;
    
    int latency;
    bool shouldRender;
    int mostRecentIndex;
    
    ofTexture videoInputTexture;
    ofTexture zmqVideoTexture;
    
    ofFbo fbo;
    RateTimer outputTimer;
    
#ifdef DECKLINK_OUTPUT
    ofxDeckLinkAPI::Output output;
#endif
};
