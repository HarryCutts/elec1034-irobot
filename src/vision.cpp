#include "../include/vision.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <stdio.h>
#include <math.h>

#define NULL 0

struct BallInfo_s {
	double xRads;
	double dist;
};

static bool visionInitialised = false;
static CvCapture* camera;

void initVision() {
	camera = cvCreateCameraCapture(0);
	assert(camera != NULL);
	visionInitialised = true;
}

void disposeVision() {
	cvReleaseCapture(&camera);
	camera = NULL;
	visionInitialised = true;
}

#define RED_MIN		128
#define RED_MUL		4
#define BLUEGREEN_MUL	3
#define AREA_MIN	4000
#define DIST_PIX_RATIO 23
#define DIAMETER 56 /**< Diameter of ball in mm. >*/

/**
 * @returns Radians from normal based upon pixels from centre of image as a 
 * signed int.
 */
static double pixelsToRads(int pixels, int diamPixels) {
	return asin((double)(DIAMETER * pixels) / 
				(diamPixels * diamPixels * DIST_PIX_RATIO));
}

static double calculateDistance(int diamPixels) {
	return (double)diamPixels * DIST_PIX_RATIO;
}

BallInfo* see() {
	assert(visionInitialised);

	IplImage* image=cvQueryFrame(camera);
	unsigned char* pixelData = (unsigned char *)(image->imageData);
		// pointer to iterate through image->imageData
	int w = image->width;
	int h = image->height;
	int c = image->nChannels;	// used to iterate through pixels in pixelData

	// Find the "Centre of mass" of the red pixels
	int area = 0;
	int xMoment = 0, yMoment = 0;
	int diameter = 0;	// Used to work out diameter
	for (int y = 0; y < h; y++) {
		int run = 0;
		for (int x = 0; x < w; x++) {
			unsigned char* blue  = pixelData;
			unsigned char* green = pixelData + 1;
			unsigned char* red   = pixelData + 2;
			if (((RED_MUL * (int)*red) > (BLUEGREEN_MUL * ((int)*blue + (int)* green))) 
					& (*red > RED_MIN)) {
				*red = 255;
				area = area +1;
				xMoment = xMoment + x;
				yMoment = yMoment + y; 

				// Increment the run and set diameter if necessary
				run++;
				if (run > diameter)	diameter = run;
			} else {
				*red = 0;
				run = 0;
			}
			*blue = 0;
			*green = 0;
			pixelData = pixelData + c; 
		}
	}
	if (area > AREA_MIN) {
		// ball found
		int xPos = xMoment/area;
		int yPos = yMoment/area;
		// Draw a line on the image showing the x coordinate of the ball
		/*unsigned char* blue_pixel =  (unsigned char *)(image->imageData) + c*xpos;
		for (int y = 0; y < h; y++) {
			* blue_pixel = 255;
			blue_pixel = blue_pixel + c*w;
		}*/
		printf("Ball at (%d, %d)", xPos, yPos);
		BallInfo* bi = (BallInfo*)malloc(sizeof(BallInfo));
		bi->xRads = pixelsToRads(xPos, diameter);
		bi->dist = calculateDistance(diameter);
		return bi;
	} else {
		return NULL;
	}
	cvwidget->putImage(image);
}

// Accessor methods //

double getXRadians(BallInfo i) {
	return i->xRads;
}

double getBallDistance(BallInfo i) {
	return i->dist;
}
