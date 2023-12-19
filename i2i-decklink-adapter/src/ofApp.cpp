#include "ofApp.h"

void ofApp::setup() {
    latency = 30;
    int bufferSize = latency * 2;
    
    videoInputForwarder.startThread();
    zmqVideoReceiver.startThread();
    
    videoInputRing.setup(bufferSize);
    zmqVideoRing.setup(bufferSize);
    
    fbo.allocate(1920, 1080);
    
//    output.setup();
//    output.start(bmdModeHD1080p30);
}

void ofApp::update() {
    shouldRender = false;
    
    DecodedFrame videoInputFrame;
    if (videoInputForwarder.channel.tryReceive(videoInputFrame)) {
        videoInputRing.add(videoInputFrame);
        shouldRender = true;
        mostRecentIndex = videoInputFrame.index;
    }
    
    DecodedFrame zmqVideoFrame;
    if (zmqVideoReceiver.channel.tryReceive(zmqVideoFrame)) {
        zmqVideoRing.add(zmqVideoFrame);
    }
    
    if (shouldRender && !zmqVideoRing.empty()) {
        int delayedIndex = mostRecentIndex - latency;
        videoInputTexture.loadData(videoInputRing.get(delayedIndex).pix);
        zmqVideoTexture.loadData(zmqVideoRing.get(delayedIndex).pix);
    }
}

void ofApp::draw() {
    
    fbo.begin();
    if (videoInputTexture.isAllocated()) {
        videoInputTexture.draw(0,0);
    }
    if (zmqVideoTexture.isAllocated()) {
        zmqVideoTexture.draw(320,0);
    }
    fbo.end();
    
    fbo.draw(0, 0);
    
    if (shouldRender) {
        outputTimer.tick();
//        output.publishTexture(fbo.getTexture());
    }
    
    using TimerInfo = std::pair<std::string, float>;
    std::vector<TimerInfo> timers = {
        {"app", ofGetFrameRate()},
        {"forwarder loop", videoInputForwarder.loopTimer.getFrameRate()},
        {"forwarder send", videoInputForwarder.sendTimer.getFrameRate()},
        {"receiver loop", zmqVideoReceiver.loopTimer.getFrameRate()},
        {"receiver receive", zmqVideoReceiver.receiveTimer.getFrameRate()},
        {"output", outputTimer.getFrameRate()}
    };
    
    int y = 20;
    for (const auto& [name, rate] : timers) {
        ofDrawBitmapStringHighlight(ofToString(int(rate)) + " fps " + name, 10, y);
        y += 20;
    }
}

void ofApp::exit() {
}


void ofApp::keyPressed(int key) {
}
