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
	
	// Calibration Code
	bCalibrateMode = false;
	calibrationPhase = 0;
	mininumFeedbackMillis = 200; // 0.2 seconds
	feedbackBeginTime = 0;
	bCalibrationFailedForContrast = false;
	nCalibrationFailedMininumThreshold = 50; // At least a difference of 255 - 50
	calibratedThreshold = 0; // 0 means not calibrated yet
	calibrationCornerSize = 60; // corner blocks of 60 x 60 (measured agains the screen size)
	calibrationXOffset = 10; // 10 more to the center
	calibrationYOffset = 100; // 100 more to the center
	calibratedHistory = 0;
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
		
		
		// find contours which are between the size of 10 pixels and 1/2 the w*h pixels.
		// also, find holes is set to true so we will get interior contours as well....
		// and consider not more than 8 blobs (or holes)
		contourFinder.findContours(grayDiff, 10, (_CAM_WIDTH * _CAM_HEIGHT)/2, 8, true);	// find holes
		
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
	if (weConnected) {
		// See if we received some messages
		string msgRx = tcpClient.receive();
		if( msgRx.length() > 0 ) {
			if (msgRx == "clear") {
				cout << "Received an external call for clearScreen()";
				clearScreen();
			}
			if (msgRx == "capture") {
				cout << "Received an external call for saveImage()";
				saveImage();
			}
		}
		
	} else {
		//if we are not connected lets try and reconnect every 5 seconds
		deltaTime = ofGetElapsedTimeMillis() - connectTime;
		
		if ( deltaTime > 5000 ) {
			weConnected = tcpClient.setup("127.0.0.1", 8081);
			//weConnected = tcpClient.setup("10.0.1.33", 8081);
			connectTime = ofGetElapsedTimeMillis();
		}
		
	}
	
	
	
	
	// CALIBRATION MODE
	if (bCalibrateMode) {
		
		switch (calibrationPhase) {
			case 0:
				// step 0: draw black
				if ( (feedbackBeginTime + mininumFeedbackMillis) < ofGetElapsedTimeMillis() ) {
					calibrationPhase ++;
					feedbackBeginTime = ofGetElapsedTimeMillis();
					// step 1: take background reference
					bLearnBackground = true;
					// reset the warning
					bCalibrationFailedForContrast = false;
				} 
				break;
			case 1:
				// step 1: take background reference (keep drawing black)
				if ( (feedbackBeginTime + mininumFeedbackMillis) < ofGetElapsedTimeMillis() ) {
					calibrationPhase ++;
					feedbackBeginTime = ofGetElapsedTimeMillis();
					// step 2: draw white
					// and start step 3 with a maximum high threshold
					threshold = 255;
				} 
				break;
			case 2:
				// step 2: draw white
				if ( (feedbackBeginTime + mininumFeedbackMillis) < ofGetElapsedTimeMillis() ) {
					calibrationPhase ++;
					feedbackBeginTime = ofGetElapsedTimeMillis();
					// step 3: start decreasing threshold
				} 
				break;
			case 3:
				// step 3: keep decreasing the lightness threshold until we
				//         get to much white of the screen (keep drawing white)
				if ( contourFinder.nBlobs < 1 ) {
					// There are no blobs yet, keep decreasing the threshold
					threshold --;
				} else {
					// There is a blob, probably we're to far all ready
					// up the threshold with 5 nodges:
					threshold += 5;
					calibratedThreshold = threshold;
					// Report about the threshold if the beamer was too bright
					if (threshold > (255 - nCalibrationFailedMininumThreshold) ) {
						bCalibrationFailedForContrast = true;
					}
					// Proceed to the next phase
					calibrationPhase ++;
					feedbackBeginTime = ofGetElapsedTimeMillis();
					// step 4: Get a threshold to find the 4 corners
				}
				break;
			case 4:
				// step 4: draw the corners in white
				if ( (feedbackBeginTime + mininumFeedbackMillis) < ofGetElapsedTimeMillis() ) {
					calibrationPhase ++;
					feedbackBeginTime = ofGetElapsedTimeMillis();
					// step 5: start decreasing threshold
					calibratedHistory = 0;
				} 
				break;
			case 5:
				// step 5:  keep decreasing the threshold until we see
				//			the 4 blobs of the 4 corners (keep drawing the corners)
				
				// find contours which are between the size of 10 pixels and 1/2 the w*h pixels.
				// also, find holes is set to true so we will get interior contours as well....
				// and consider not more than 8 blobs (or holes)
				contourFinder.findContours(grayDiff, 60, 10000, 6, false);	// dont't look for holes
				
				// increase the history
				calibratedHistory = calibratedHistory << 1;
				if ( contourFinder.nBlobs != 4 ) {
					// There are not exactly 4 blobs yet, keep decreasing the threshold
					threshold --;
					//cout << "NOT calibratedHistory: " << calibratedHistory << "\n";
				} else {
					// Check if the last 10 times also had 4 blobs
					calibratedHistory ++;
					int perfectHistoryRecord = int(pow(double(2),20) - 1); // 30 frames
					//cout << "YES calibratedHistory: " << calibratedHistory << " perfectHistoryRecord: " << perfectHistoryRecord << "\n";
					if (calibratedHistory == perfectHistoryRecord) {
						// There are 4 for the last x frames.
						// This threshold seems fine for tracking the corners
						// Proceed to the next phase
						calibrationPhase ++;
						feedbackBeginTime = ofGetElapsedTimeMillis();
						// step 6: Align the corners
					} else {
						// Not quite yet
						threshold --;
					}
				}
				break;
			case 6:
				// step 6: draw the corners in white
				if ( (feedbackBeginTime + mininumFeedbackMillis) < ofGetElapsedTimeMillis() ) {
					//calibrationPhase ++;
					feedbackBeginTime = ofGetElapsedTimeMillis();
					// step 7: done
				} 
				break;
			default:
				// Ready with the calibration:
				threshold = calibratedThreshold;
				calibrationPhase = 0;
				bCalibrateMode = false;
				bClearBackground = true;
				break;
		}
	}
}



//----------------------------
// EXTRA FUNCTIONS 

string createName() {
	stringstream s1;
	s1 << _SAVE_LOCATION << "wallimage" << ofGetSystemTime() << ".png";
	return s1.str();
}



void testApp::saveImage() {
	// SAVE image
#ifdef _MIRROR_OUTPUT_IMAGE
	brushImg.mirror(false, true);
#endif
	outputImage.setFromPixels(brushImg.getPixels(), brushImg.getWidth(), brushImg.getHeight(), OF_IMAGE_COLOR, true);
	outputImage.saveImage(createName());
#ifdef _MIRROR_OUTPUT_IMAGE
	brushImg.mirror(false, true);
#endif
}



void testApp::clearScreen() {
	bClearBackground = true;
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

//	char calibrateStr[128];
//	if (bCalibrationFailedForContrast) {
//		sprintf(calibrateStr, "CALIBRATION FAILED: BEAMER IS TOO BRIGHT");
//	} else {
//		sprintf(calibrateStr, "press 'c' to automatically calibrate.");		
//	}
	string calibrateStr = bCalibrationFailedForContrast ? "CALIBRATION FAILED: BEAMER IS TOO BRIGHT" : "press 'c' to automatically calibrate.";
	char reportStr[1024];
	sprintf(reportStr, "bg subtraction and blob detection\n\
press 'b' to capture bg\n\
press 's' to control the video settings\n\
press ' ' to clear the image\n\
press ENTER to save the image\n\
threshold %i (press: +/-) (shift +/-:  +10/-10)\n\n\
press '[' / ']' to select a quad point\n\
use ARROW keys to move the selected point around (shift step with 10)\n\n\
quad A: (%i, %i), quad B: (%i, %i)\n\
quad C: (%i, %i), quad D: (%i, %i)\n\
num blobs found %i, fps: %f\n\n%s\n\
press 'C' to exit calibration mode.", threshold, quadAx, quadAy, quadBx, quadBy,
			quadCx, quadCy, quadDx, quadDy,
			contourFinder.nBlobs, ofGetFrameRate(), calibrateStr.c_str());
	ofDrawBitmapString(reportStr, 20, 500);


	
	// CALIBRATION MODE
	if (bCalibrateMode) {
		
		switch (calibrationPhase) {
			case 0:
				// step 0: draw black
				ofFill();
				ofSetColor(0x000000);
				ofRect(offsetx, adjustedScreenOffsetY, screenw, adjustedScreenHeight);
				break;
			case 1:
				// step 1: take background reference (keep drawing black)
				ofFill();
				ofSetColor(0x000000);
				ofRect(offsetx, adjustedScreenOffsetY, screenw, adjustedScreenHeight);
				break;
			case 2:
				// step 2: draw white
				ofFill();
				ofSetColor(0xffffff);
				ofRect(offsetx, adjustedScreenOffsetY, screenw, adjustedScreenHeight);
				break;
			case 3:
				// step 3: keep increasing lightness threshold (keep drawing white)
				ofFill();
				ofSetColor(0xffffff);
				ofRect(offsetx, adjustedScreenOffsetY, screenw, adjustedScreenHeight);
				break;
			case 4:
				// step 4: draw the corners in white
				ofFill();
				ofSetColor(0x000000);
				ofRect(offsetx, adjustedScreenOffsetY, screenw, adjustedScreenHeight);
				ofSetColor(0xffffff);
				ofRect(offsetx + calibrationXOffset, adjustedScreenOffsetY + calibrationYOffset, calibrationCornerSize, calibrationCornerSize);
				ofRect(offsetx + (screenw - (calibrationCornerSize + calibrationXOffset)), adjustedScreenOffsetY + calibrationYOffset, calibrationCornerSize, calibrationCornerSize);
				ofRect(offsetx + calibrationXOffset, adjustedScreenOffsetY + (adjustedScreenHeight - (calibrationCornerSize + calibrationYOffset)), calibrationCornerSize, calibrationCornerSize);
				ofRect(offsetx + (screenw - (calibrationCornerSize + calibrationXOffset)), adjustedScreenOffsetY + (adjustedScreenHeight - (calibrationCornerSize + calibrationYOffset)), calibrationCornerSize, calibrationCornerSize);
				break;
			case 5:
				// step 5:  keep decreasing the threshold until we see
				ofFill();
				ofSetColor(0x000000);
				ofRect(offsetx, adjustedScreenOffsetY, screenw, adjustedScreenHeight);
				ofSetColor(0xffffff);
				ofRect(offsetx + calibrationXOffset, adjustedScreenOffsetY + calibrationYOffset, calibrationCornerSize, calibrationCornerSize);
				ofRect(offsetx + (screenw - (calibrationCornerSize + calibrationXOffset)), adjustedScreenOffsetY + calibrationYOffset, calibrationCornerSize, calibrationCornerSize);
				ofRect(offsetx + calibrationXOffset, adjustedScreenOffsetY + (adjustedScreenHeight - (calibrationCornerSize + calibrationYOffset)), calibrationCornerSize, calibrationCornerSize);
				ofRect(offsetx + (screenw - (calibrationCornerSize + calibrationXOffset)), adjustedScreenOffsetY + (adjustedScreenHeight - (calibrationCornerSize + calibrationYOffset)), calibrationCornerSize, calibrationCornerSize);
				break;
			case 6:
				// step 6: Manual Calibration of Corners
				ofNoFill();
				ofSetLineWidth(5);
				ofSetColor(0xffffff);
				// A
				if (selectedQuadPoint == 1) {
					ofSetColor(0xff6600);
					ofCircle(offsetx + calibrationXOffset + (calibrationCornerSize / 2), adjustedScreenOffsetY + calibrationYOffset + (calibrationCornerSize / 2), calibrationCornerSize);
					ofSetColor(0xffffff);
				}
				ofLine(offsetx + calibrationXOffset + (calibrationCornerSize / 2), adjustedScreenOffsetY + calibrationYOffset,offsetx + calibrationXOffset + (calibrationCornerSize / 2), adjustedScreenOffsetY + calibrationYOffset + calibrationCornerSize);
				ofLine(offsetx + calibrationXOffset, adjustedScreenOffsetY + calibrationYOffset + (calibrationCornerSize / 2), offsetx + calibrationXOffset + calibrationCornerSize, adjustedScreenOffsetY + calibrationYOffset + (calibrationCornerSize / 2));
				// B
				if (selectedQuadPoint == 2) {
					ofSetColor(0xff6600);
					ofCircle(offsetx + (screenw - (calibrationCornerSize + calibrationXOffset)) + (calibrationCornerSize / 2), adjustedScreenOffsetY + calibrationYOffset + (calibrationCornerSize / 2), calibrationCornerSize);
					ofSetColor(0xffffff);
				}
				ofLine(offsetx + (screenw - (calibrationCornerSize + calibrationXOffset)) + (calibrationCornerSize / 2), adjustedScreenOffsetY + calibrationYOffset, offsetx + (screenw - (calibrationCornerSize + calibrationXOffset)) + (calibrationCornerSize / 2), adjustedScreenOffsetY + calibrationYOffset + calibrationCornerSize);
				ofLine(offsetx + (screenw - (calibrationCornerSize + calibrationXOffset)), adjustedScreenOffsetY + calibrationYOffset + (calibrationCornerSize / 2), offsetx + (screenw - (calibrationCornerSize + calibrationXOffset)) + calibrationCornerSize, adjustedScreenOffsetY + calibrationYOffset + (calibrationCornerSize / 2));
				// C
				if (selectedQuadPoint == 3) {
					ofSetColor(0xff6600);
					ofCircle(offsetx + calibrationXOffset + (calibrationCornerSize / 2), adjustedScreenOffsetY + (adjustedScreenHeight - (calibrationCornerSize + calibrationYOffset)) + (calibrationCornerSize / 2), calibrationCornerSize);
					ofSetColor(0xffffff);
				}
				ofLine(offsetx + calibrationXOffset + (calibrationCornerSize / 2), adjustedScreenOffsetY + (adjustedScreenHeight - (calibrationCornerSize + calibrationYOffset)), offsetx + calibrationXOffset + (calibrationCornerSize / 2), adjustedScreenOffsetY + (adjustedScreenHeight - (calibrationCornerSize + calibrationYOffset)) + calibrationCornerSize);
				ofLine(offsetx + calibrationXOffset, adjustedScreenOffsetY + (adjustedScreenHeight - (calibrationCornerSize + calibrationYOffset)) + (calibrationCornerSize / 2), offsetx + calibrationXOffset + calibrationCornerSize, adjustedScreenOffsetY + (adjustedScreenHeight - (calibrationCornerSize + calibrationYOffset)) + (calibrationCornerSize / 2));
				// D
				if (selectedQuadPoint == 4) {
					ofSetColor(0xff6600);
					ofCircle(offsetx + (screenw - (calibrationCornerSize + calibrationXOffset)) + (calibrationCornerSize / 2), adjustedScreenOffsetY + (adjustedScreenHeight - (calibrationCornerSize + calibrationYOffset)) + (calibrationCornerSize / 2), calibrationCornerSize);
					ofSetColor(0xffffff);
				}
				ofLine(offsetx + (screenw - (calibrationCornerSize + calibrationXOffset)) + (calibrationCornerSize / 2), adjustedScreenOffsetY + (adjustedScreenHeight - (calibrationCornerSize + calibrationYOffset)), offsetx + (screenw - (calibrationCornerSize + calibrationXOffset)) + (calibrationCornerSize / 2), adjustedScreenOffsetY + (adjustedScreenHeight - (calibrationCornerSize + calibrationYOffset)) + calibrationCornerSize);
				ofLine(offsetx + (screenw - (calibrationCornerSize + calibrationXOffset)), adjustedScreenOffsetY + (adjustedScreenHeight - (calibrationCornerSize + calibrationYOffset)) + (calibrationCornerSize / 2), offsetx + (screenw - (calibrationCornerSize + calibrationXOffset)) + calibrationCornerSize, adjustedScreenOffsetY + (adjustedScreenHeight - (calibrationCornerSize + calibrationYOffset)) + (calibrationCornerSize / 2));
				ofSetLineWidth(1);
				ofFill();
				break;
			default:
				break;
		}
	}
	
}


//--------------------------------------------------------------
void testApp::keyPressed  (int key){

	switch (key){
		// Use the b key to learn the background
		case 'b':
			bLearnBackground = true;
			break;
		// Use the c key to automatically calibrate
		case 'c':
			bCalibrateMode = true;
			feedbackBeginTime = ofGetElapsedTimeMillis();
			bClearBackground = true;
			break;
		// Use capital (shift) C key to stop calibration mode
		case 'C':
			threshold = calibratedThreshold;
			calibrationPhase = 0;
			bCalibrateMode = false;
			bClearBackground = true;
			break;
		// Use the spacebar to clear the image
		case ' ':
			clearScreen();
			break;
		// Use the enter key to select the source
		case 13:
			// SAVE image
			saveImage();
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
				if (glutGetModifiers() == GLUT_ACTIVE_SHIFT) {
					(* selectQuadx) -= 10;
				} else {
					(* selectQuadx) --;
				}
			}
			break;
		// RIGHT
		case 358:
			if ( selectQuadx != NULL) {
				if (glutGetModifiers() == GLUT_ACTIVE_SHIFT) {
					(* selectQuadx) += 10;
				} else {
					(* selectQuadx) ++;
				}
			}
			break;
		// UP
		case 357:
			if ( selectQuady != NULL) {
				if (glutGetModifiers() == GLUT_ACTIVE_SHIFT) {
					(* selectQuady) -= 10;
				} else {
					(* selectQuady) --;
				}
			}
			break;
		// DOWN
		case 359:
			if ( selectQuady != NULL) {
				if (glutGetModifiers() == GLUT_ACTIVE_SHIFT) {
					(* selectQuady) += 10;
				} else {
					(* selectQuady) ++;
				}
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

