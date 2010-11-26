#ifndef _TEST_APP
#define _TEST_APP

#include "ofMain.h"

#include "ofxOpenCv.h"
#include "ofxVectorMath.h"
#include "ofxNetwork.h"

#define RECONNECT_TIME 400


#define _USE_LIVE_VIDEO		// uncomment this to use a live camera
								// otherwise, we'll use a movie file

#define _FILTER_BACKGROUND_WITH_DIFF // uncomment this to difference the background image

#define _MIRROR_OUTPUT_IMAGE // uncomment when you're not using rear projection

#define _SAVE_LOCATION "/Users/dirk/Desktop/myData/"

//#define _FULLSCREEN

#define _CAM_WIDTH 640
#define _CAM_HEIGHT 480

#define _SCREEN_WIDTH 1536
//#define _SCREEN_WIDTH 1024
#define _SCREEN_HEIGHT 768
#define _SCREEN_OFFSET_X 1024
#define _SCREEN_OFFSET_Y 0
#define _WINDOW_X 140
#define _WINDOW_Y 60


#define _IDLE_KICKIN_TRESHOLD_MILLIS 60000

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

	ofxCvColorImage		cameraImg;
	ofxCvColorImage		colorImg;
	ofxCvColorImage		hsbImg;

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

	ofImage outputImage;
	
	ofxCvContourFinder 	contourFinder;

	int 				threshold;
	bool				bLearnBackground;
	bool				bClearBackground;
	
	ofxPoint2f screenSourcePoints[4];
	ofxPoint2f screenDestPoints[4];
	
	int quadAx;
	int quadAy;
	int quadBx;
	int quadBy;
	int quadCx;
	int quadCy;
	int quadDx;
	int quadDy;
	
	int selectedQuadPoint;
	int *selectQuadx;
	int *selectQuady;
	
	bool bIdleState;
	bool bPreIdleState;
	int millisBeginIdle;
	
	
	ofxTCPClient tcpClient;
	bool weConnected;
	int connectTime;
	int deltaTime;



	
	
private:
	void sendIdleMessage(bool isIdle);

};

#endif
