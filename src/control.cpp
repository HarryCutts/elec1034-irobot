#include <stdio.h>
#include <math.h>
#include <time.h>
#include "robot-comms.h"
#include "vision.h"

// In ms
#define TICK_INTERVAL			0

// In radians
#define STRAIGHT_AHEAD_THRESHOLD	0.2

// All of these constants are in mm or mm/s
#define	STOPPING_THRESHOLD		500
#define NAVIGATING_SPEED		200
#define SEARCHING_SPEED			NAVIGATING_SPEED
#define INITIAL_SEARCH_RADIUS	100
#define SEARCH_RADIUS_INC		50	/*The amount to increment the search radius
									  by per second. */

typedef enum {
	NAVIGATING,
	SEARCHING,
	FINISHED
} StrategyState;

static StrategyState state;
static s16 searchRadius;
static s16 numTicksWithoutBall, numTicksBallCaught;
static time_t lastSearchTick;

static void delay(int ms) {
	struct timespec t;
	t.tv_sec = ms/1000;
	t.tv_nsec = (long)(ms%1000 * 1000000);
	nanosleep(&t, NULL);
}

static void navigate() {
	state = NAVIGATING;
	setMotorSpeeds(0, 0);
	numTicksWithoutBall = 0;
	numTicksBallCaught = 0;
}

static void search() {
	state = SEARCHING;
	searchRadius = INITIAL_SEARCH_RADIUS;
	setRobotCourse(SEARCHING_SPEED, searchRadius);
	lastSearchTick = time(NULL);
}

static void finish() {
	state = FINISHED;
}

int main(void) {
	initVision();
	initRobotComms();
	delay(700);	// Let the webcam 'warm up'
	if (ballFound(see())) {
		printf("Ball in sight. Going straight into navigate mode.\n");
		navigate();
	} else {
		printf("Ball not in sight. Starting to search...\n");
		search();
	}
	while (true) {
		BallInfo* bi = see();
		switch (state) {
			case SEARCHING:
				if (ballFound(bi) && getXRadians(bi) > -STRAIGHT_AHEAD_THRESHOLD) {
					printf("Ball found. Changing to Navigate mode.\n");
					navigate();
				} else {
					printf("Searching... (%d mm radius)\n", searchRadius);
					time_t now = time(NULL);
					searchRadius += SEARCH_RADIUS_INC *
							difftime(now, lastSearchTick);
					lastSearchTick = now;
					setRobotCourse(SEARCHING_SPEED, searchRadius);
					//setMotorSpeeds(-50, 50);
				}
				break;

			case NAVIGATING:
				if (ballFound(bi)) {
					numTicksWithoutBall = 0;
					printf("Navigating... ");
					if (getBallDistance(bi) < STOPPING_THRESHOLD) {
						numTicksBallCaught++;
						if (numTicksBallCaught >= 2) {
							printf("Ball 'caught'.\n");
							setMotorSpeeds(0, 0);
							finish();
						} else {
							printf("Ball within stopping distance. numTicksBallCaught = %d\n",
								numTicksBallCaught);
						}
					} else {
						numTicksBallCaught = 0;
						double theta = getXRadians(bi);
						if (theta < STRAIGHT_AHEAD_THRESHOLD) {
							printf("Ball straight ahead.\n");
							setMotorSpeeds(NAVIGATING_SPEED, NAVIGATING_SPEED);
						} else {
							double radius = getBallDistance(bi) * sin(M_PI / 2 - theta)
									/ sin(2 * theta);
							printf("(theta = %f, radius = %f)\n", theta, radius);
							setRobotCourse(NAVIGATING_SPEED, (int)radius);
						}
					}
				} else {
					numTicksWithoutBall++;
					if (numTicksWithoutBall > 10) {
						printf("Ball lost. Changing to Search mode.\n");
						search();
					} else {
						printf("Cannot see ball. numTicksWithoutBall = %d\n",
							numTicksWithoutBall);
					}
				}
				break;
			case FINISHED:
				printf("<insert victory dance here>\n");
				return EXIT_SUCCESS;
				break;
		}
		delay(TICK_INTERVAL);
	}
}
