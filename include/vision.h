// vision.h

struct BallInfo_s;

/** Contains data on the position of the ball relative to the robot. */
typedef BallInfo_s BallInfo;

/** @return A BallInfo struct containing information on the current position of the ball relative
 * to the webcam. */
BallInfo see();

/** @return The horizontal angle between the ball and the normal from the plane of the webcam. */
double getXRadians(BallInfo i);
/** @return The vertical angle between the ball and the normal from the plane of the webcam. */
double getYRadians(BallInfo i);

/** @return The distance between the ball and the webcam, in metres. */
double getBallDistance(BallInfo i);
