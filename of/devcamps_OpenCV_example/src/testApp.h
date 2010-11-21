#ifndef _TEST_APP
#define _TEST_APP

#include "ofMain.h"

#include "ofxOpenCv.h"

#define _USE_LIVE_VIDEO		// uncomment this to use a live camera
								// otherwise, we'll use a movie file

#define _FILTER_BACKGROUND_WITH_DIFF // uncomment this to difference the background image

//#define _FULLSCREEN

#define _CAM_WIDTH 640
#define _CAM_HEIGHT 480

#define _SCREEN_WIDTH 1024
#define _SCREEN_HEIGHT 768
#define _SCREEN_OFFSET_X 1024
#define _SCREEN_OFFSET_Y 0

class testApp : public ofBaseApp{

	public:

	void setup();
	void update();
	void draw();

	void keyPressed  (int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);

#ifdef _USE_LIVE_VIDEO
	ofVideoGrabber 		vidGrabber;
	#else
	ofVideoPlayer 		vidPlayer;
#endif

	ofxCvColorImage		colorImg, hsbImg;

	ofxCvGrayscaleImage		hueImg;
	ofxCvGrayscaleImage		satImg;
	ofxCvGrayscaleImage		briImg;

	ofxCvGrayscaleImage		rImg;
	ofxCvGrayscaleImage		gImg;
	ofxCvGrayscaleImage		bImg;
	
	ofxCvGrayscaleImage		h_Img;
	ofxCvGrayscaleImage		s_Img;
	ofxCvGrayscaleImage		b_Img;
	
	ofxCvGrayscaleImage 	grayImage;
	ofxCvGrayscaleImage 	grayBg;
	ofxCvGrayscaleImage 	grayDiff;

	IplImage			*rgbComposite;
	IplImage			*resultComposite;

	ofxCvColorImage		brushImg;
	ofxCvColorImage		improvedBrushImg;


	ofxCvContourFinder 	contourFinder;

	int 				threshold;
	bool				bLearnBackground;
	bool				bClearBackground;


};

#endif
