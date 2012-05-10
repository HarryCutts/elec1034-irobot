// robot-comms.h

#include "type.h"

// Module Methods //

void initRobotComms(void);

void disposeRobotComms(void);

// Motor Output //

/** Sets the motor speeds to the given values. 
 * @param left	The speed of the left motor, in mm/s. 
 * @param right	The speed of the right motor, in mm/s. */
void setMotorSpeeds(s16 right, s16 left);

/** Instructs the robot to follow a course around a circle.
 * @param velocity	The speed of the robot, in mm/s. 
 * @param radius	The radius of the circle in mms. */
void setRobotCourse(s16 velocity, s16 radius);

// Sensor Input //

typedef u8 SensorData;

/** Starts an asynchronous request for sensor data to the robot. */
void requestSensorData(void);

/** Retrieve the requested sensor data from the robot. The last robot comms call
 * must have been requestSensorData for the call to work.
 * 
 * This function will block until the sensor data is received, and will be
 * quicker if the time since requestSensorData is longer.
 * @return A SensorData value containing the sensor readings. */
SensorData retrieveSensorData(void);

bool getCastorWheeldrop(SensorData d);

bool getLeftWheeldrop(SensorData d);

bool getRightWheeldrop(SensorData d);

bool getLeftBump(SensorData d);

bool getRightBump(SensorData d);
