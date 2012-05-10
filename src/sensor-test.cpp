#include <stdio.h>
#include "robot-comms.h"

int main(void) {
	initRobotComms();

	while (true) {
		printf("Requesting sensors... ");
		//requestSensorData();
		//printf("requested.\n");
		int x = 5;

		x++;

		printf("%d",x);
		//printf("Retrieving sensor data... ");
		//SensorData sd = retrieveSensorData();
		printf("retrieved.\n");

		/*bool leftBump = getLeftBump(sd);
		bool rightBump = getRightBump(sd);
		
		printf("Left: %s, right: %s\n", leftBump?"true":"false", rightBump?"true":"false");*/
	}
}
