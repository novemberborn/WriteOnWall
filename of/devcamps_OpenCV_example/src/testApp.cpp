#include "testApp.h"


//--------------------------------------------------------------
void testApp::setup(){


	#ifdef _USE_LIVE_VIDEO
        vidGrabber.setVerbose(true);
        vidGrabber.initGrabber(320,240);
	#else
        vidPlayer.loadMovie("fingers.mov");
        vidPlayer.play();
	#endif

    colorImg.allocate(320,240);
	hsbImg.allocate(320,240);
	
	hueImg.allocate(320,240);
	satImg.allocate(320,240);
	briImg.allocate(320,240);
	
	rImg.allocate(320,240);
	gImg.allocate(320,240);
	bImg.allocate(320,240);
	
	h_Img.allocate(320,240);
	s_Img.allocate(320,240);
	b_Img.allocate(320,240);

	grayImage.allocate(320,240);
	grayBg.allocate(320,240);
	grayDiff.allocate(320,240);
	
	rgbComposite = cvCreateImage(cvSize(320, 240), IPL_DEPTH_8U, 3);
	resultComposite = cvCreateImage(cvSize(320, 240), IPL_DEPTH_8U, 3);
	brushImg.allocate(320,240);

	bLearnBakground = true;
	threshold = 80;
}

//--------------------------------------------------------------
void testApp::update(){
	ofBackground(100,100,100);

    bool bNewFrame = false;

	#ifdef _USE_LIVE_VIDEO
       vidGrabber.grabFrame();
	   bNewFrame = vidGrabber.isFrameNew();
    #else
        vidPlayer.idleMovie();
        bNewFrame = vidPlayer.isFrameNew();
	#endif

	if (bNewFrame){

		#ifdef _USE_LIVE_VIDEO
            colorImg.setFromPixels(vidGrabber.getPixels(), 320,240);
	    #else
            colorImg.setFromPixels(vidPlayer.getPixels(), 320,240);
        #endif
		
		hsbImg = colorImg;
		hsbImg.convertRgbToHsv();
		hsbImg.convertToGrayscalePlanarImages(hueImg, satImg, briImg);
		

        grayImage = briImg;
		if (bLearnBakground == true){
			grayBg = grayImage;		// the = sign copys the pixels from grayImage into grayBg (operator overloading)
			bLearnBakground = false;
		}

		// take the abs value of the difference between background and incoming and then threshold:
		grayDiff.absDiff(grayBg, grayImage);
		grayDiff.threshold(threshold);
		
		//
		
		colorImg.convertToGrayscalePlanarImages(rImg, gImg, bImg);
		//cv
		cvCopy(colorImg.getCvImage(), rgbComposite, grayDiff.getCvImage());
		//cvMerge(rImg.getCvImage(), gImg.getCvImage(), bImg.getCvImage(), grayDiff.getCvImage(), rgbComposite);
		brushImg = rgbComposite;
		
		/*
		 WERKT NIET!
		 COMPILED WEL MAAR CRASHT ZONDER CXERROR.CPP
		 Xcode could not locate source file: cxerror.cpp (line: 359)
		 
		 
		// to hsb
		brushImg.convertRgbToHsv();
		brushImg.convertToGrayscalePlanarImages(h_Img, s_Img, b_Img);
		
		b_Img.brightnessContrast(0.8, 0.5);
		cvMerge(h_Img.getCvImage(), s_Img.getCvImage(), b_Img.getCvImage(), NULL, resultComposite);
		cvCvtColor(resultComposite, resultComposite, CV_HSV2RGB);
		
		brushImg = resultComposite;
		
		
		 */
		
		
		
		/*
		// find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
		// also, find holes is set to true so we will get interior contours as well....
		contourFinder.findContours(grayDiff, 20, (340*240)/3, 10, true);	// find holes
		*/
	}

}

//--------------------------------------------------------------
void testApp::draw(){

	// draw the incoming, the grayscale, the bg and the thresholded difference
	ofSetColor(0xffffff);
	
//#ifdef _FULLSCREEN


//#else

	colorImg.draw(0,0);
	
	hueImg.draw(640, 0);
	satImg.draw(640, 240);
	briImg.draw(640, 480);
	
	grayImage.draw(320,0);
	grayBg.draw(0,240);
	grayDiff.draw(320,240);
	
	brushImg.draw(320,480);
	brushImg.draw(1024, 0, 1024, 768);
	
//#endif
	

	/*
	 // then draw the contours:
	 
	ofFill();
	ofSetColor(0x333333);
	ofRect(320,480,320,240);
	ofSetColor(0xffffff);

	// we could draw the whole contour finder
	//contourFinder.draw(360,540);

	// or, instead we can draw each blob individually,
	// this is how to get access to them:
    for (int i = 0; i < contourFinder.nBlobs; i++){
        contourFinder.blobs[i].draw(320,480);
    }
	 
	 */
	
	// finally, a report:

	ofSetColor(0xffffff);
	char reportStr[1024];
	//sprintf(reportStr, "bg subtraction and blob detection\npress ' ' to capture bg\nthreshold %i (press: +/-)\nnum blobs found %i, fps: %f", threshold, contourFinder.nBlobs, ofGetFrameRate());
	ofDrawBitmapString(reportStr, 20, 600);

}


//--------------------------------------------------------------
void testApp::keyPressed  (int key){

	switch (key){
		case ' ':
			bLearnBakground = true;
			break;
		case '-':
			threshold ++;
			if (threshold > 255) threshold = 255;
			break;
		case '_':
			threshold ++;
			if (threshold > 255) threshold = 255;
			break;
		case '=':
			threshold --;
			if (threshold < 0) threshold = 0;
			break;
		case '+':
			threshold --;
			if (threshold < 0) threshold = 0;
			break;
		case 's':
			vidGrabber.videoSettings();
			break;
	}
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

