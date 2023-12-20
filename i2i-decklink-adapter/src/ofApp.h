#pragma once

#include "ofMain.h"
#include "VideoInputForwarder.h"
#include "ZmqVideoReceiver.h"
#include "Ring.h"
#include "DecodedFrame.h"
#include "RateTimer.h"
#include "ofxOscReceiver.h"

//#define DECKLINK_OUTPUT

class ofApp : public ofBaseApp{

public:
	void setup();
	void update();
	void draw();
	void exit();
	void keyPressed(int key);
    void updateOsc();
    
    VideoInputForwarder videoInputForwarder;
    Ring<DecodedFrame> videoInputRing;
    
    ZmqVideoReceiver zmqVideoReceiver;
    Ring<DecodedFrame> zmqVideoRing;
    
    bool fullscreen;
    bool debug;
    int latency;
    bool shouldRender;
    int mostRecentIndex;
    
    ofTexture videoInputTexture;
    ofTexture zmqVideoTexture;
    
    ofFbo fbo;
    RateTimer outputTimer;
    
    ofxOscReceiver osc;
    RateTimer oscTimer;
    ofParameter<float> alpha, x1,y1,x2,y2;
    
    float lastInputTimestamp;
    float lastWorkerLatency;
    float lastFullLatency;
    
    float lastPrintTime;
    
#ifdef DECKLINK_OUTPUT
    ofxDeckLinkAPI::Output output;
#endif
};
