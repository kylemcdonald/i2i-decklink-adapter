#include "ofApp.h"
#include "ofMain.h"

int main() {
	ofGLFWWindowSettings settings;
	settings.setSize(960, 540);
	settings.setGLVersion(4, 0);
	settings.windowMode = OF_WINDOW;
	auto window = ofCreateWindow(settings);
	ofRunApp(window, make_shared<ofApp>());
	ofRunMainLoop();
}
