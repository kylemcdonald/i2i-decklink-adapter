#include "ofApp.h"

void ofApp::setup() {
    latency = 8;
    int bufferSize = latency * 2;
    
    videoInputForwarder.startThread();
    zmqVideoReceiver.startThread();
    
    videoInputRing.setup(bufferSize);
    zmqVideoRing.setup(bufferSize);
    
    fbo.allocate(1920, 1080);

    oscReceiver.setup("127.0.0.1", 7777);
    alpha.set("alpha", 0.0f, 0.0f, 1.0f);
    p0.set("p0", glm::vec2(100, 100), glm::vec2(0), glm::vec2(1920, 1080));
    p1.set("p1", glm::vec2(400, 400), glm::vec2(0), glm::vec2(1920, 1080));
    composite.load("shader/passThru.vert", "shader/composite.frag");

#ifdef DECKLINK_OUTPUT
    output.setup();
    output.start(bmdModeHD1080i5994);
#endif
}

void ofApp::update() {
//    latency = int(ofMap(mouseX, 0, ofGetWidth(), 1, 10));
    
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

    updateOSC();
   
}

void ofApp::draw() {
    
    fbo.begin();
    {
        composite.begin();
        composite.setUniform1f("alpha", alpha);
        composite.setUniform2f("p0", glm::min(p0.get(), p1.get()));
        composite.setUniform2f("p1", glm::max(p0.get(), p1.get()));
        composite.setUniform2f("resolution", ofGetWidth(), ofGetHeight());
        composite.setUniformTexture("effect", zmqVideoTexture, 1);
        videoInputTexture.draw(0, 0, fbo.getWidth(), fbo.getHeight());
        composite.end();
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

void ofApp::updateOSC() {
    if (oscReceiver.hasWaitingMessages()) {
        ofxOscMessage msg;
        oscReceiver.getNextMessage(msg);
        std::string address = msg.getAddress();
        if (address == "/main/alpha") {
            alpha.set(msg.getArgAsFloat(0));
        }
        else if (address == "/main/p0") {
            float x = msg.getArgAsFloat(0);
            float y = msg.getArgAsFloat(1);
            p0.set(glm::vec2(x, y));
        }
        else if (address == "/main/p1") {
            float x = msg.getArgAsFloat(0);
            float y = msg.getArgAsFloat(1);
            p1.set(glm::vec2(x, y));
        }
    }
}
