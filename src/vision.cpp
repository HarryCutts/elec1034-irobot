#include "../include/vision.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <stdio.h>
#include <math.h>

#define NULL 0

struct BallInfo_s {
	double xRads;
	//double yRads;
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
double pixelsToRads(int pixels, int diamPixels) {
	return asin((double)(DIAMETER * pixels) / 
				(diamPixels * diamPixels * DIST_PIX_RATIO));
}

double calculateDistance(int diamPixels) {
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
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			// TODO: Calculate diameter
			unsigned char* blue  = pixelData;
			unsigned char* green = pixelData + 1;
			unsigned char* red   = pixelData + 2;
			if (((RED_MUL * (int)*red) > (BLUEGREEN_MUL * ((int)*blue + (int)* green))) 
					& (*red > RED_MIN)) {
				*red = 255;
				area = area +1;
				xMoment = xMoment + x;
				yMoment = yMoment + y; 
			} else {
				*red = 0;
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
		// TODO: define static functions
		bi->xRads = pixelsToRads(xPos, diameter);
		//bi->yRads = pixelsToRads(yPos);
		bi->dist = calculateDistance(diameter);
		return bi;
	} else {
		return NULL;
	}
	cvwidget->putImage(image);
}
