#include "ofMain.h"
#include "testApp.h"
#include "ofAppGlutWindow.h"

//========================================================================
int main( ){

    ofAppGlutWindow window;
	int totalScreenWidth = 1024 + _SCREEN_WIDTH;
	ofSetupOpenGL(&window, totalScreenWidth ,768, OF_WINDOW);			// <-------- setup the GL context
	
	// Set the position of the window:
	window.setWindowPosition(_WINDOW_X, _WINDOW_Y);

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp( new testApp());

}
