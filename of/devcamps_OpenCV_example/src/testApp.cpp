#include "testApp.h"


//--------------------------------------------------------------
void testApp::setup(){


	#ifdef _USE_LIVE_VIDEO
        vidGrabber.setVerbose(true);
        vidGrabber.initGrabber(_CAM_WIDTH, _CAM_HEIGHT);
	#else
        vidPlayer.loadMovie("fingers.mov");
        vidPlayer.play();
	#endif
	
	cameraImg.allocate(_CAM_WIDTH, _CAM_HEIGHT);
    colorImg.allocate(_CAM_WIDTH, _CAM_HEIGHT);
	outputImage.allocate(_CAM_WIDTH, _CAM_HEIGHT, OF_IMAGE_COLOR);
	hsbImg.allocate(_CAM_WIDTH, _CAM_HEIGHT);
	
	hueImg.allocate(_CAM_WIDTH, _CAM_HEIGHT);
	satImg.allocate(_CAM_WIDTH, _CAM_HEIGHT);
	briImg.allocate(_CAM_WIDTH, _CAM_HEIGHT);
	
	rImg.allocate(_CAM_WIDTH, _CAM_HEIGHT);
	gImg.allocate(_CAM_WIDTH, _CAM_HEIGHT);
	bImg.allocate(_CAM_WIDTH, _CAM_HEIGHT);
	
	h_Img.allocate(_CAM_WIDTH, _CAM_HEIGHT);
	s_Img.allocate(_CAM_WIDTH, _CAM_HEIGHT);
	b_Img.allocate(_CAM_WIDTH, _CAM_HEIGHT);

	grayImage.allocate(_CAM_WIDTH, _CAM_HEIGHT);
	grayBg.allocate(_CAM_WIDTH, _CAM_HEIGHT);
	grayDiff.allocate(_CAM_WIDTH, _CAM_HEIGHT);
	
	rgbComposite = cvCreateImage(cvSize(_CAM_WIDTH, _CAM_HEIGHT), IPL_DEPTH_8U, 3);
	resultComposite = cvCreateImage(cvSize(_CAM_WIDTH, _CAM_HEIGHT), IPL_DEPTH_8U, 3);
	
	brushImg.allocate(_CAM_WIDTH, _CAM_HEIGHT);
	improvedBrushImg.allocate(_CAM_WIDTH, _CAM_HEIGHT);

	bLearnBackground = true;
	threshold = 80;
	
	// Quad Warping
	screenSourcePoints[0].set(0.0, 0.0);
	screenSourcePoints[1].set(_CAM_WIDTH, 0.0);
	screenSourcePoints[2].set(0.0, _CAM_HEIGHT);
	screenSourcePoints[3].set(_CAM_WIDTH, _CAM_HEIGHT);

	// test values
	// TODO get these from an XML file
	screenDestPoints[0].set(20.0, 20.0);
	screenDestPoints[1].set(600.0, 40.0);
	screenDestPoints[2].set(-40.0, 400.0);
	screenDestPoints[3].set(500.0, 550.0);
	
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
		cameraImg.setFromPixels(vidGrabber.getPixels(), _CAM_WIDTH, _CAM_HEIGHT);
#else
		cameraImg.setFromPixels(vidPlayer.getPixels(), _CAM_WIDTH, _CAM_HEIGHT);
#endif

		// Warp the camera image onto the projected image
		colorImg.warpIntoMe(cameraImg, screenSourcePoints, screenDestPoints);
		
		hsbImg = colorImg;
		hsbImg.convertRgbToHsv();
		hsbImg.convertToGrayscalePlanarImages(hueImg, satImg, briImg);
		

        grayImage = briImg;
		if (bLearnBackground == true){
			grayBg = grayImage;		// the = sign copys the pixels from grayImage into grayBg (operator overloading)
			bLearnBackground = false;
		}

#ifdef _FILTER_BACKGROUND_WITH_DIFF
		// take the abs value of the difference between background and incoming:
		grayDiff.absDiff(grayBg, grayImage);
#endif	
		// and then threshold:
		grayDiff.threshold(threshold);
		//
		
		colorImg.convertToGrayscalePlanarImages(rImg, gImg, bImg);
		if (bClearBackground == true) {
			cvZero(rgbComposite);
			bClearBackground = false;
		}
		cvCopy(colorImg.getCvImage(), rgbComposite, grayDiff.getCvImage());
		brushImg = rgbComposite;
		
		/*
		 WERKT NIET!
		 COMPILED WEL MAAR CRASHT ZONDER CXERROR.CPP
		 Xcode could not locate source file: cxerror.cpp (line: 359)
		 
		*/
		
		/*
		// to hsb
		brushImg.convertRgbToHsv();
		brushImg.convertToGrayscalePlanarImages(h_Img, s_Img, b_Img);
		
		// THIS IS THE LINE OF CODE IT CRASHES ON RIGHT NOW
		//b_Img.brightnessContrast(50, 50);
		
		cvMerge(h_Img.getCvImage(), s_Img.getCvImage(), b_Img.getCvImage(), NULL, resultComposite);
		cvCvtColor(resultComposite, resultComposite, CV_HSV2RGB);
		
		brushImg = resultComposite;
		
		*/
		
		
		
		
		// find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
		// also, find holes is set to true so we will get interior contours as well....
		contourFinder.findContours(grayDiff, 20, (_CAM_WIDTH * _CAM_HEIGHT)/3, 10, true);	// find holes
	}

}

//--------------------------------------------------------------
void testApp::draw(){

	// draw the incoming, the grayscale, the bg and the thresholded difference
	ofSetColor(0xffffff);
	
//#ifdef _FULLSCREEN


//#else

	int w_preview = 320;
	int h_preview = 240;
	int w_preview_small = 160;
	int h_preview_small = 120;
	
	cameraImg.draw(640, 480, w_preview, h_preview);
	colorImg.draw(0,0, w_preview, h_preview);
	
	hueImg.draw(320, 0, w_preview_small, h_preview_small);
	satImg.draw(480, 0, w_preview_small, h_preview_small);
	briImg.draw(640, 0, w_preview_small, h_preview_small);
	grayImage.draw(800,0, w_preview_small, h_preview_small);
	
	grayBg.draw(320,120, w_preview_small, h_preview_small);
	
	grayDiff.draw(0, 240, w_preview, h_preview);
	
	brushImg.draw(320, 240, w_preview, h_preview);
	
	brushImg.draw(_SCREEN_OFFSET_X, _SCREEN_OFFSET_Y, _SCREEN_WIDTH, _SCREEN_HEIGHT);
	
//#endif
	

	 // then draw the contours:
	 
	ofFill();
	ofSetColor(0x333333);
	ofRect(640,240, w_preview, h_preview);
	ofSetColor(0xffffff);

	// we could draw the whole contour finder
	contourFinder.draw(640,240, w_preview, h_preview);

	/*
	// or, instead we can draw each blob individually,
	// this is how to get access to them:
    for (int i = 0; i < contourFinder.nBlobs; i++){
        contourFinder.blobs[i].draw(320, 240);
    }
	*/
	
	// finally, a report:

	ofSetColor(0xffffff);
	char reportStr[1024];
	sprintf(reportStr, "bg subtraction and blob detection\npress ' ' to capture bg\nthreshold %i (press: +/-)\nnum blobs found %i, fps: %f", threshold, contourFinder.nBlobs, ofGetFrameRate());
	ofDrawBitmapString(reportStr, 20, 600);

}


string createName() {
	stringstream s1;
	s1 << "wallimage" << ofGetSystemTime() << ".png";
	return s1.str();
}

//--------------------------------------------------------------
void testApp::keyPressed  (int key){

	switch (key){
		// Use the b key to learn the background
		case 'b':
			bLearnBackground = true;
			break;
		// Use the spacebar to clear the image
		case ' ':
			bClearBackground = true;
			break;
		// Use the enter key to select the source
		case 13:
			// SAVE image
			outputImage.setFromPixels(brushImg.getPixels(), brushImg.getWidth(), brushImg.getHeight(), OF_IMAGE_COLOR, true);
			outputImage.saveImage(createName());
			break;
		// Use the s key to select the source
		case 's':
			vidGrabber.videoSettings();
			break;
		// Adjust the treshold of the contour with - / +
		// in steps of 1,
		// with SHIFT: in steps of 10
		case '-':
			threshold ++;
			if (threshold > 255) threshold = 255;
			break;
		case '_':
			threshold += 10;
			if (threshold > 255) threshold = 255;
			break;
		case '=':
			threshold --;
			if (threshold < 0) threshold = 0;
			break;
		case '+':
			threshold -= 10;
			if (threshold < 0) threshold = 0;
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

