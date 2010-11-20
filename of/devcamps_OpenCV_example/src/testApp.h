#ifndef _TEST_APP
#define _TEST_APP

#include "ofMain.h"

#include "ofxOpenCv.h"

#define _USE_LIVE_VIDEO		// uncomment this to use a live camera
								// otherwise, we'll use a movie file

//#define _FULLSCREEN

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
	
		IplImage		*rgbComposite;
	IplImage		*resultComposite;
		ofxCvColorImage		brushImg;
	

        //ofxCvContourFinder 	contourFinder;

		int 				threshold;
		bool				bLearnBakground;


};

#endif
