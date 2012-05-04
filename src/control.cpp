#include <math.h>
#include "robot-comms.h"
#include "vision.h"

// In ms
#define TICK_INTERVAL			500

// All of these constants are in mm or mm/s
#define	STOPPING_THRESHOLD		200
#define NAVIGATING_SPEED		100
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

int main(void) {
	initVision();
	initRobotComms();
	state = SEARCHING;
	while (true) {
		BallInfo* bi = see();
		switch (state) {
			case SEARCHING:
				if (ballFound(bi)) {
					navigate();
				} else {
					searchRadius += SEARCH_RADIUS_INC;
					setRobotCourse(SEARCHING_SPEED, searchRadius);
				}
				break;

			case NAVIGATING:
				if (ballFound(bi)) {
					if (getBallDistance(bi) < STOPPING_THRESHOLD) {
						setMotorSpeeds(0, 0);
						finish();
					} else {
						double theta = getXRadians(bi);
						double radius = -getBallDistance(bi) * sin(M_PI - theta)
								/ sin(2 * theta);
						setRobotCourse(NAVIGATING_SPEED, radius);
					}
				} else {
					search();
				}
				break;
			case FINISHED:
				return EXIT_SUCCESS;
				break;
		}
		delay(TICK_INTERVAL);
	}
}
