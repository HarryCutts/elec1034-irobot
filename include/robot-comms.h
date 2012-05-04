// robot-comms.h

#include "type.h"

/** Sets the motor speeds to the given values. 
 * @param left	The speed of the left motor, in mm/s. 
 * @param right	The speed of the right motor, in mm/s. */
void setMotorSpeeds(s16 right, s16 left);

/** Instructs the robot to follow a course around a circle.
 * @param velocity	The speed of the robot, in mm/s. 
 * @param radius	The radius of the circle in mms. */
void setRobotCourse(s16 velocity, s16 radius);

void initRobotComms(void);

void disposeRobotComms(void);
