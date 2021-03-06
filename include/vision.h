// vision.h

#ifndef VISION_H
#define VISION_H

#include <opencv/cv.h>

struct BallInfo_s;

/** Contains data on the position of the ball relative to the robot. */
typedef BallInfo_s BallInfo;

/** Starts the vision system. Must be called before the vision system is used. */
void initVision(void);

/** Releases resources in use by the vision system. Should be called when the vision system 
 * is no longer required. */
void disposeVision();

//void attachDebugWindow(VisionDebugWindow* w);

/** @return A pointer to a BallInfo struct containing information on the current position of the ball 
 * relative to the webcam, or NULL if the ball cannot be found. */
BallInfo* see();

/** @return true if the ball is in sight; else false. */
bool ballFound(BallInfo* i);

/** @return The horizontal angle between the ball and the normal from the plane of the webcam. */
double getXRadians(BallInfo* i);

/** @return The distance between the ball and the webcam, in metres. */
double getBallDistance(BallInfo* i);

IplImage* getDebugImage(BallInfo* i);

#endif
