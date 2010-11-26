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
	threshold = 130;
	
	// Quad Warping
	screenSourcePoints[0].set(0.0, 0.0);
	screenSourcePoints[1].set(_CAM_WIDTH, 0.0);
	screenSourcePoints[2].set(0.0, _CAM_HEIGHT);
	screenSourcePoints[3].set(_CAM_WIDTH, _CAM_HEIGHT);

	// test values
	// TODO get these from an XML file
	
	// Default values:
	quadAx = -14;
	quadAy = 21;
	quadBx = 645;
	quadBy = 21;
	quadCx = 7;
	quadCy = 504;
	quadDx = 629;
	quadDy = 506;
	
	selectedQuadPoint = 0;
	selectQuadx = NULL;
	selectQuady = NULL;
	
	screenDestPoints[0].set(quadAx, quadAy);
	screenDestPoints[1].set(quadBx, quadBy);
	screenDestPoints[2].set(quadCx, quadCy);
	screenDestPoints[3].set(quadDx, quadDy);
	
	// Idle State Checking
	bPreIdleState = false;
	bIdleState = false;
	

	//are we connected to the server - if this fails we
	//will check every few seconds to see if the server exists
	weConnected = tcpClient.setup("127.0.0.1", 11999);
	
	connectTime = 0;
	deltaTime = 0;
	
	tcpClient.setVerbose(true);
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
		
		// Set the pointer to the quad ints
		
		switch (selectedQuadPoint) {
			case 1:
				selectQuadx = &quadAx;
				selectQuady = &quadAy;
				break;
			case 2:
				selectQuadx = &quadBx;
				selectQuady = &quadBy;
				break;
			case 3:
				selectQuadx = &quadCx;
				selectQuady = &quadCy;
				break;
			case 4:
				selectQuadx = &quadDx;
				selectQuady = &quadDy;
				break;
			default:
				selectQuadx = NULL;
				selectQuady = NULL;
				break;
		}
		
		screenDestPoints[0].set(quadAx, quadAy);
		screenDestPoints[1].set(quadBx, quadBy);
		screenDestPoints[2].set(quadCx, quadCy);
		screenDestPoints[3].set(quadDx, quadDy);
		
		
		// find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
		// also, find holes is set to true so we will get interior contours as well....
		contourFinder.findContours(grayDiff, 20, (_CAM_WIDTH * _CAM_HEIGHT)/3, 10, true);	// find holes
		
		// Idle State Checking
		
		if ((bPreIdleState || bIdleState) && (contourFinder.nBlobs > 1)) {
			bPreIdleState = false;
			bIdleState = false;
			sendIdleMessage(false);
		}
		
		if (bPreIdleState) {
			int millisSinceBeginIdle = ofGetSystemTime() - millisBeginIdle;
			if (millisSinceBeginIdle > _IDLE_KICKIN_TRESHOLD_MILLIS) {
				bIdleState = true;
				bPreIdleState = false;
				sendIdleMessage(true);
			}
			
		}
		
		if ((!bPreIdleState && !bIdleState) && (contourFinder.nBlobs == 0)) {
			bPreIdleState = true;
			millisBeginIdle = ofGetSystemTime();
		}

	}
	
	// TCP SOCKET
	if (!weConnected) {
		//if we are not connected lets try and reconnect every 5 seconds
		deltaTime = ofGetElapsedTimeMillis() - connectTime;
		
		if ( deltaTime > 5000 ) {
			weConnected = tcpClient.setup("127.0.0.1", 8081);
			//weConnected = tcpClient.setup("10.0.1.33", 8081);
			connectTime = ofGetElapsedTimeMillis();
		}
		
	}
	

}


void testApp::sendIdleMessage(bool isIdle) {
	if (isIdle) {
		cout << "WE SHOULD SEND A IDLE MESSAGE\n";
		if ( weConnected ) {
			tcpClient.send("idle");
		}
	} else {
		cout << "CANCEL THE IDLE MESSAGE\n";
		if ( weConnected ) {
			tcpClient.send("active");
		}
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
	
	int colorImgOffsetX = 320;
	int colorImgOffsetY = 240;
	
	cameraImg.draw(0, 0, w_preview, h_preview);
	colorImg.draw(colorImgOffsetX, colorImgOffsetY, w_preview, h_preview);
	
	hueImg.draw(320, 0, w_preview_small, h_preview_small);
	satImg.draw(480, 0, w_preview_small, h_preview_small);
	briImg.draw(640, 0, w_preview_small, h_preview_small);
	grayImage.draw(800,0, w_preview_small, h_preview_small);
	
	grayBg.draw(320,120, w_preview_small, h_preview_small);
	
	grayDiff.draw(0, 240, w_preview, h_preview);
	
	brushImg.draw(640, 480, w_preview, h_preview);
	
	// Draw the Quad Warp Points
	int anchorSize = 8;
	int anchorOffset = 4;
	float scaleAspect = w_preview /(_CAM_WIDTH * 1.0);
	// A
	if (selectedQuadPoint == 1) {
		ofFill();
		ofSetColor(0xffffff);
	} else {
		ofNoFill();
		ofSetColor(0xaaaaaa);
	}
	// B
	ofRect((colorImgOffsetX - anchorOffset) + (quadAx * scaleAspect),
		   (colorImgOffsetY - anchorOffset) + (quadAy * scaleAspect), anchorSize, anchorSize);
	if (selectedQuadPoint == 2) {
		ofFill();
		ofSetColor(0xffffff);
	} else {
		ofNoFill();
		ofSetColor(0xaaaaaa);
	}
	// C
	ofRect((colorImgOffsetX - anchorOffset) + (quadBx * scaleAspect),
		   (colorImgOffsetY - anchorOffset) + (quadBy * scaleAspect), anchorSize, anchorSize);
	if (selectedQuadPoint == 3) {
		ofFill();
		ofSetColor(0xffffff);
	} else {
		ofNoFill();
		ofSetColor(0xaaaaaa);
	}
	// D
	ofRect((colorImgOffsetX - anchorOffset) + (quadCx * scaleAspect),
		   (colorImgOffsetY - anchorOffset) + (quadCy * scaleAspect), anchorSize, anchorSize);
	if (selectedQuadPoint == 4) {
		ofFill();
		ofSetColor(0xffffff);
	} else {
		ofNoFill();
		ofSetColor(0xaaaaaa);
	}
	ofRect((colorImgOffsetX - anchorOffset) + (quadDx * scaleAspect),
		   (colorImgOffsetY - anchorOffset) + (quadDy * scaleAspect), anchorSize, anchorSize);
	ofSetColor(0xffffff);
	
	
	//
	// Adjust the projected image size to fit the multiple(?) beamer setup
	int adjustedScreenOffsetY = _SCREEN_OFFSET_Y;
	int adjustedScreenHeight = _SCREEN_HEIGHT;
	// check if the aspect ratio of the projection is wider than that of the camera
	float aspect_cam = _CAM_HEIGHT / (_CAM_WIDTH * 1.0);
	float aspect_out = _SCREEN_HEIGHT / (_SCREEN_WIDTH * 1.0);
	if ( aspect_cam > aspect_out ) {
		adjustedScreenHeight = _SCREEN_WIDTH * aspect_cam;
		adjustedScreenOffsetY = _SCREEN_OFFSET_Y - ((adjustedScreenHeight - _SCREEN_HEIGHT) / 2);
	}

	int offsetx = _SCREEN_OFFSET_X;
	int screenw = _SCREEN_WIDTH;
	/*
	cout << "aspect_cam" << aspect_cam << "aspect_out" << aspect_out << "\n";
	brushImg.draw(1024, -192, 1536, 1152);
	cout << "brushImg.draw(1024, -192, 1536, 1152);\n";
	cout << offsetx << " " << adjustedScreenOffsetY << " " << screenw << " " << adjustedScreenHeight << "\n";
	*/
	brushImg.draw(offsetx, adjustedScreenOffsetY, screenw, adjustedScreenHeight);
	
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
	sprintf(reportStr, "bg subtraction and blob detection\n\
press 'b' to capture bg\n\
press 's' to control the video settings\n\
press ' ' to clear the image\n\
press ENTER to save the image\n\
threshold %i (press: +/-) (shift +/-:  +10/-10)\n\
press '[' / ']' to select a quad point\n\
use ARROW keys to move the selected point around\n\
quad A: (%i, %i), quad B: (%i, %i)\n\
quad C: (%i, %i), quad D: (%i, %i)\n\
num blobs found %i, fps: %f", threshold, quadAx, quadAy, quadBx, quadBy,
			quadCx, quadCy, quadDx, quadDy,
			contourFinder.nBlobs, ofGetFrameRate());
	ofDrawBitmapString(reportStr, 20, 500);

}


string createName() {
	stringstream s1;
	s1 << _SAVE_LOCATION << "wallimage" << ofGetSystemTime() << ".png";
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
#ifdef _MIRROR_OUTPUT_IMAGE
			brushImg.mirror(false, true);
#endif
			outputImage.setFromPixels(brushImg.getPixels(), brushImg.getWidth(), brushImg.getHeight(), OF_IMAGE_COLOR, true);
			outputImage.saveImage(createName());
#ifdef _MIRROR_OUTPUT_IMAGE
			brushImg.mirror(false, true);
#endif
			break;
		// Use the s key to select the source
		case 's':
			vidGrabber.videoSettings();
			break;
		// Adjust the quad warps
		// '['  and ']' for selecting the A, B, C or D (or none)
		// Arrow keys for moving the quad corner around
		case '[':
			selectedQuadPoint --;
			if (selectedQuadPoint < 0) selectedQuadPoint = 4;
			break;
		case ']':
			selectedQuadPoint ++;
			if (selectedQuadPoint > 4) selectedQuadPoint = 0;
			break;
		// LEFT
		case 356:
			if ( selectQuadx != NULL) {
				(* selectQuadx) --;
			}
			break;
		// RIGHT
		case 358:
			if ( selectQuadx != NULL) {
				(* selectQuadx) ++;
			}
			break;
		// UP
		case 357:
			if ( selectQuady != NULL) {
				(* selectQuady) --;
			}
			break;
		// DOWN
		case 359:
			if ( selectQuady != NULL) {
				(* selectQuady) ++;
			}
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
		default:
			cout << "unhandled key: " << key << "\n";
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

