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

//call this to request bump and wheel drop data from the robot. getSensorData MUST be the next motor function called
void setSensorData(void);
/*retrieve the requested sensor data from the robot. setSensorData must have been called first, with no other motor
functions in between. This function is blocking but will take less time the longer ago that setSensorData was called -
therefore, I suggest doing any processing in between these functions.
Data is a byte, with bits 4:0 containing caster wheeldrop, left wheeldrop, right wheeldrop, bump left, bump right, respectively*/
u8 getSensorData(void);

void disposeRobotComms(void);
