#include <stdio.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#include "robot-comms.h"
#include "vision.h"

// In ms
#define TICK_INTERVAL			0
#define BUMP_REVERSE_TIME		200
#define BUMP_SPIN_TIME			1500

// In radians
#define STRAIGHT_AHEAD_THRESHOLD	0.2

// All of these constants are in mm
#define	STOPPING_THRESHOLD		470	///< The distance at which the ball is 'caught'.
#define INITIAL_SEARCH_RADIUS	100	///< Initial radius at which the robot spirals
#define SEARCH_RADIUS_INC		20	///< Amount by which the search radius increases per second

// All of these constants are in mm/s
#define NAVIGATING_SPEED		200
#define SEARCHING_SPEED			NAVIGATING_SPEED
#define BUMP_REVERSE_SPEED		-200
#define BUMP_SPIN_SPEED			200

// Tick numbers
#define MAX_TICKS_WITHOUT_BALL	10
#define MAX_TICKS_BALL_CAUGHT	2

typedef enum {
	NAVIGATING,		///< The robot can see the ball and is driving towards it.
	SEARCHING,		///< The robot cannot see the ball and is looking for it.
	QUIT_REQUESTED	///< Quit signal received. Stop and exit on next loop iteration.
} StrategyState;

static StrategyState state;
static s16 searchRadius;	///< Current radius at which the robot is turning

/** The number of NAVIGATING ticks during which the robot has not seen the ball.
 * When this reaches a MAX_TICKS_WITHOUT_BALL, the ball is counted as lost.*/
static s16 numTicksWithoutBall;

/** Similar to numTicksWithoutBall, but for catching the ball instead. */
static s16 numTicksBallCaught;

/** The time at which the last SEARCHING tick occurred. Used when calculating radius
 * increments. */
static time_t lastSearchTick;

static void delay(int ms) {
	struct timespec t;
	t.tv_sec = ms/1000;
	t.tv_nsec = (long)(ms%1000 * 1000000);
	nanosleep(&t, NULL);
}

/** Changes to NAVIGATE mode and initialises the correct variables. */
static void navigate() {
	state = NAVIGATING;
	setMotorSpeeds(0, 0);
	numTicksWithoutBall = 0;
	numTicksBallCaught = 0;
}

/** Changes to SEARCHING mode and initialises the correct variables. */
static void search() {
	state = SEARCHING;
	bool dir = rand() % 2;
	searchRadius = dir ? INITIAL_SEARCH_RADIUS : -INITIAL_SEARCH_RADIUS ;
	setRobotCourse(SEARCHING_SPEED, searchRadius);
	lastSearchTick = time(NULL);
}

/** Reads the sensor data from the given SensorData value and responds
 * accordingly. */
static void handleSensorData(SensorData sd) {
	bool bumped = getLeftBump(sd) || getRightBump(sd);

	if (bumped) {
		printf("Bumped.\n");
		setMotorSpeeds(-BUMP_REVERSE_SPEED, -BUMP_REVERSE_SPEED);
		delay(BUMP_REVERSE_TIME);

		// Spin in a random direction
		bool dir = rand() % 2;
		if (dir) {
			setMotorSpeeds(-BUMP_SPIN_SPEED, BUMP_SPIN_SPEED);
		} else {
			setMotorSpeeds(BUMP_SPIN_SPEED, -BUMP_SPIN_SPEED);
		}
		delay(BUMP_SPIN_TIME);
		search();
	}
}

/** Instructs the program to quit after stopping. */
static void sigproc(int signal) {
	printf("Exit requested.\n");
	state = QUIT_REQUESTED;
}

/** @return true if the ball is to be considered straight ahead. */
static bool isInCentre(double rads) {
	return (rads > -STRAIGHT_AHEAD_THRESHOLD) && (rads < STRAIGHT_AHEAD_THRESHOLD);
}

int main(void) {
	signal(SIGINT, &sigproc);

	initVision();
	initRobotComms();

	srand(time(NULL));

	delay(700);	// Let the webcam 'warm up'

	// Set the initial state
	if (ballFound(see())) {
		printf("Ball in sight. Going straight into navigate mode.\n");
		navigate();
	} else {
		printf("Ball not in sight. Starting to search...\n");
		search();
	}
	// Begin main loop
	while (true) {
		requestSensorData();
		BallInfo* bi = see();
		SensorData sd = retrieveSensorData();
		handleSensorData(sd);

		switch (state) {
			case SEARCHING:
				if (ballFound(bi) && isInCentre(getXRadians(bi))) {
					printf("Ball found. Changing to Navigate mode.\n");
					navigate();
				} else {
					printf("Searching... (%d mm radius)\n", searchRadius);

					// Increment the search radius according to how long it was
					// since the last tick
					time_t now = time(NULL);
					int radiusChange = SEARCH_RADIUS_INC * difftime(now,
							lastSearchTick);
					if (searchRadius > 0) {
						searchRadius += radiusChange;
					} else {
						searchRadius -= radiusChange;
					}
					lastSearchTick = now;

					setRobotCourse(SEARCHING_SPEED, searchRadius);
				}
				break;

			case NAVIGATING:
				if (ballFound(bi)) {
					numTicksWithoutBall = 0;
					printf("Navigating... ");
					if (getBallDistance(bi) < STOPPING_THRESHOLD) {
						numTicksBallCaught++;
						if (numTicksBallCaught >= MAX_TICKS_BALL_CAUGHT) {
							printf("Ball 'caught'.\n");
							setMotorSpeeds(0, 0);
						} else {
							printf("Ball within stopping distance. numTicksBallCaught = %d\n",
								numTicksBallCaught);
						}

					} else {
						numTicksBallCaught = 0;
						double theta = getXRadians(bi);
						if (isInCentre(theta)) {
							printf("Ball straight ahead.\n");
							setMotorSpeeds(NAVIGATING_SPEED, NAVIGATING_SPEED);
						} else {
							// Move along an arc to the ball
							double radius = -getBallDistance(bi) * sin(M_PI / 2 - theta)
									/ sin(2 * theta);
							printf("(theta = %f, radius = %f)\n", theta, radius);
							setRobotCourse(NAVIGATING_SPEED, (int)radius);
						}
					}

				} else {
					numTicksWithoutBall++;
					if (numTicksWithoutBall > MAX_TICKS_WITHOUT_BALL) {
						printf("Ball lost. Changing to Search mode.\n");
						search();
					} else {
						printf("Cannot see ball. numTicksWithoutBall = %d\n",
							numTicksWithoutBall);
					}
				}
				break;

			case QUIT_REQUESTED:
				// Safely stop the robot and quit.
				printf("Stopping.\n");
				setMotorSpeeds(0, 0);
				disposeVision();
				return EXIT_SUCCESS;
				break;
		}
		free(bi);
		delay(TICK_INTERVAL);
	}
}
