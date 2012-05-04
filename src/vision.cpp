#include "vision.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <stdio.h>
#include <math.h>

#ifdef VISION_DEBUG

int main() {
	initVision();

	while (true) {
		BallInfo* bi = see();
		
		if (bi->found)
			printf("X rads: %f, Distance: %f\n", getXRadians(bi),
												 getBallDistance(bi));
	}
}

#endif

struct BallInfo_s {
	bool found;
	double xRads;
	double dist;
	IplImage* image;
};

static bool visionInitialised = false;
static CvCapture* camera;

static int GREEN_MIN = 128;
static int GREEN_MUL = 4;
static int REDBLUE_MUL = 3;
static int AREA_MIN = 4000;
static int DIST_PIX_RATIO = 41489;
static int DIAMETER = 56; /**< Diameter of ball in mm. >*/
static int IMAGE_WIDTH = 800;

void initVision(void) {
	camera = cvCreateCameraCapture(0);
	assert(camera != NULL);
	visionInitialised = true;
}

void disposeVision() {
	cvReleaseCapture(&camera);
	camera = NULL;
	visionInitialised = true;
}

static double calculateDistance(int diamPixels) {
	return DIST_PIX_RATIO / (float) diamPixels;
}

/**
 * @returns Radians from normal based upon pixels from centre of image as a 
 * signed int.
 */
static double pixelsToRads(int pixels, int diamPixels) {
	int relativePixels = pixels - (IMAGE_WIDTH / 2);
	double length = fabs( relativePixels * (DIAMETER / (double)diamPixels) );

	if (relativePixels >= 0)
		return 0.677 * asin(length/calculateDistance(diamPixels));
	else
		return -0.677 * asin(length/calculateDistance(diamPixels));
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
			if (((GREEN_MUL * (int)*green) > (REDBLUE_MUL * ((int)*red + (int)* blue))) 
					& (*green > GREEN_MIN)) {
				*green = 255;
				area = area +1;
				xMoment = xMoment + x;
				yMoment = yMoment + y; 

				// Increment the run and set diameter if necessary
				run++;
				if (run > diameter)	diameter = run;
			} else {
				*green = 0;
				run = 0;
			}
			*red = 0;
			*blue = 0;
			pixelData = pixelData + c; 
		}
	}
	if (area > AREA_MIN) {
		// ball found
		int xPos = xMoment/area;
		int yPos = yMoment/area;
		// Draw a line on the image showing the x coordinate of the ball
		unsigned char* blue_pixel =  (unsigned char *)(image->imageData) + c*xPos;
		for (int y = 0; y < h; y++) {
			* blue_pixel = 255;
			blue_pixel = blue_pixel + c*w;
		}
		
		BallInfo* bi = (BallInfo*)malloc(sizeof(BallInfo));
		bi->found = true;
		bi->xRads = pixelsToRads(xPos, diameter);
		bi->dist = calculateDistance(diameter);
		bi->image = image;

		printf("X: %d, Y: %d, Diam: %d, X rads: %f, Dist: %f\n", xPos, yPos, diameter, bi->xRads, bi->dist);
		
		return bi;
	} else {
		BallInfo* bi = (BallInfo*)malloc(sizeof(BallInfo));
		bi->found = false;
		bi->image = image;
		return bi;
	}
}

// Accessor methods //

bool ballFound(BallInfo* i) {
	return i->found;
}

double getXRadians(BallInfo* i) {
	return i->xRads;
}

double getBallDistance(BallInfo* i) {
	return i->dist;
}

IplImage* getDebugImage(BallInfo* i) {
	return i->image;
}
