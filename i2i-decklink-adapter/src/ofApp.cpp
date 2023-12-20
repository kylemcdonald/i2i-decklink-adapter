#include "ofApp.h"

void ofApp::setup() {
    debug = false;
    fullscreen = false;
    
    lastWorkerLatency = 0;
    lastFullLatency = 0;
    lastInputTimestamp = 0;
    lastPrintTime = 0;
    
    alpha = 1;
    x1 = 0;
    y1 = 0;
    x2 = 1;
    y2 = 1;
    
    latency = 4;
    int bufferSize = latency * 2;
    
    videoInputForwarder.startThread();
    zmqVideoReceiver.startThread();
    
    videoInputRing.setup(bufferSize);
    zmqVideoRing.setup(bufferSize);
    
    fbo.allocate(1920, 1080);
    
    osc.setup("0.0.0.0", 7777);
    alpha.set("alpha", 0, 0, 1);
    
#ifdef DECKLINK_OUTPUT
    output.setup();
    output.start(bmdModeHD1080i5994);
#endif
}

void ofApp::update() {
    shouldRender = false;
    
    DecodedFrame videoInputFrame;
    if (videoInputForwarder.channel.tryReceive(videoInputFrame)) {
        videoInputRing.add(videoInputFrame);
        shouldRender = true;
        mostRecentIndex = videoInputFrame.index;
        lastInputTimestamp = videoInputFrame.timestamp;
    }
    
    DecodedFrame zmqVideoFrame;
    if (zmqVideoReceiver.channel.tryReceive(zmqVideoFrame)) {
        zmqVideoRing.add(zmqVideoFrame);
        float videoTimestamp = videoInputRing.get(zmqVideoFrame.index).timestamp;
        lastWorkerLatency = ofGetElapsedTimef() - videoTimestamp;
    }
    
    if (shouldRender && !zmqVideoRing.empty()) {
        int delayedIndex = mostRecentIndex - latency;
        videoInputTexture.loadData(videoInputRing.get(delayedIndex).pix);
        zmqVideoTexture.loadData(zmqVideoRing.get(delayedIndex).pix);
        lastFullLatency = lastInputTimestamp - zmqVideoRing.get(delayedIndex).timestamp;
    }
    
    updateOsc();
}

void ofApp::draw() {
    fbo.begin();
    if (videoInputTexture.isAllocated()) {
        videoInputTexture.draw(0,0,1920,1080);
    }
    if (zmqVideoTexture.isAllocated()) {
        ofPushStyle();
        ofSetColor(255, 255, 255, 255 * alpha);
        ofRectangle ref(x1, y1, x2-x1, y2-y1);
        ofRectangle imgRect(0, 0, zmqVideoTexture.getWidth(), zmqVideoTexture.getHeight());
        ofRectangle scrRect(0, 0, fbo.getWidth(), fbo.getHeight());
        zmqVideoTexture.drawSubsection(scrRect.map(ref), imgRect.map(ref));
        ofPopStyle();
    }
    fbo.end();
    
    fbo.draw(0, 0, ofGetWidth(), ofGetHeight());
    
    if (shouldRender) {
        outputTimer.tick();
#ifdef DECKLINK_OUTPUT
        output.publishTexture(fbo.getTexture());
#endif
    }

    using TimerInfo = std::pair<std::string, float>;
    std::vector<TimerInfo> timers = {
        {"frames latency", latency},
        {"fps app", ofGetFrameRate()},
        {"fps forwarder loop", videoInputForwarder.loopTimer.getFrameRate()},
        {"fps forwarder send", videoInputForwarder.sendTimer.getFrameRate()},
        {"fps receiver loop", zmqVideoReceiver.loopTimer.getFrameRate()},
        {"fps receiver receive", zmqVideoReceiver.receiveTimer.getFrameRate()},
        {"fps output", outputTimer.getFrameRate()},
        {"ms osc", oscTimer.getPeriod()},
        {"ms worker delay", 1000*lastWorkerLatency},
        {"ms full delay", 1000*lastFullLatency},
    };
    
    if (ofGetElapsedTimef() - lastPrintTime > 1) {
        cout << endl;
        for (const auto& [name, rate] : timers) {
            cout << ofToString(int(round(rate))) << " " << name << endl;
        }
        lastPrintTime = ofGetElapsedTimef();
    }
    
    if (debug) {
        int y = 20;
        for (const auto& [name, rate] : timers) {
            ofDrawBitmapStringHighlight(ofToString(int(round(rate))) + " " + name, 10, y);
            y += 20;
        }
    }
}

void ofApp::exit() {
}


void ofApp::keyPressed(int key) {
    if (key == 'f' || key == 'F') {
        fullscreen = !fullscreen;
        if (fullscreen) {
            ofSetWindowPosition(2000, 0);
        } else {
            ofSetWindowPosition(-2000, 0);
        }
        ofSetFullscreen(fullscreen);
    }
    if (key == 'd' || key == 'D') {
        debug = !debug;
    }
    if (key == 'a' || key == 'A') {
        if (alpha < 0.5) {
            alpha = 1;
        } else {
            alpha = 0;
        }
    }
    if (key == '-' || key == '_') {
        latency--;
    }
    if (key == '=' || key == '+') {
        latency++;
    }
}

void ofApp::updateOsc() {
    while (osc.hasWaitingMessages()) {
        oscTimer.tick();
        ofxOscMessage msg;
        osc.getNextMessage(msg);
        std::string address = msg.getAddress();
        if (address == "/main/alpha") {
            alpha.set(msg.getArgAsFloat(0));
        }
        else if (address == "/main/p0") {
            x1.set(msg.getArgAsFloat(0));
            y1.set(msg.getArgAsFloat(1));
        }
        else if (address == "/main/p1") {
            x2.set(msg.getArgAsFloat(0));
            y2.set(msg.getArgAsFloat(1));
        }
    }
}
