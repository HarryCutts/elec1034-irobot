#include <stdio.h>
#include <math.h>
#include "robot-comms.h"
#include "vision.h"

// In ms
#define TICK_INTERVAL			500

// All of these constants are in mm or mm/s
#define	STOPPING_THRESHOLD		200
#define NAVIGATING_SPEED		200
#define SEARCHING_SPEED			NAVIGATING_SPEED
#define INITIAL_SEARCH_RADIUS	100
#define SEARCH_RADIUS_INC		50

typedef enum {
	NAVIGATING,
	SEARCHING,
	FINISHED
} StrategyState;

static StrategyState state;
static s16 searchRadius;

static void delay(int ms) {
	struct timespec t;
	t.tv_sec = ms/1000;
	t.tv_nsec = (long)(ms%1000 * 1000000);
	nanosleep(&t, NULL);
}

static void navigate() {
	state = NAVIGATING;
}

static void search() {
	state = SEARCHING;
	searchRadius = INITIAL_SEARCH_RADIUS;
	setRobotCourse(SEARCHING_SPEED, searchRadius);
}

static void finish() {
	state = FINISHED;
}

static void handleSensorData(SensorData sd) {
	bool bumped = getLeftBump(sd) || getRightBump(sd);

	if (bumped) {
		printf("Bumped. Reversing, spinning and changing to search.\n");
		setMotorSpeeds(-500, -500);
		delay(60);
		setMotorSpeeds(-500, 500);
		delay(10);
		search();
	}
}

int main(void) {
	initVision();
	initRobotComms();
	search();
	while (true) {
		printf("Requesting sensor data...\n");
		requestSensorData();
		printf("Calling see()...\n");
		BallInfo* bi = see();
		SensorData sd = retrieveSensorData();
		printf("Retrieved sensor data.\n");
		handleSensorData(sd);

		switch (state) {
			case SEARCHING:
				if (ballFound(bi)) {
					printf("Ball found. Changing to Navigate mode.\n");
					navigate();
				} else {
					printf("Searching... (%d mm radius)\n", searchRadius);
					searchRadius += SEARCH_RADIUS_INC;
					setRobotCourse(SEARCHING_SPEED, searchRadius);
				}
				break;

			case NAVIGATING:
				if (ballFound(bi)) {
					printf("Navigating... ");
					if (getBallDistance(bi) < STOPPING_THRESHOLD) {
						printf("Ball within stopping threshold.\n");
						setMotorSpeeds(0, 0);
						finish();
					} else {
						double theta = getXRadians(bi);
						double radius = -getBallDistance(bi) * sin(M_PI - theta)
								/ sin(2 * theta);
						printf("(theta = %f, radius = %f)\n", theta, radius);
						setRobotCourse(NAVIGATING_SPEED, (int)radius);
					}
				} else {
					printf("Ball lost. Changing to Search mode.\n");
					search();
				}
				break;
			case FINISHED:
				printf("<insert victory dance here>\n");
				return EXIT_SUCCESS;
				break;
		}
		free(bi);
		delay(TICK_INTERVAL);
	}
}
