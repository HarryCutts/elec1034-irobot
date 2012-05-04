#include "robot-comms.h"
#include "type.h"

#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h> 
#include <errno.h>
#include <error.h>
#include <time.h>
#include <stdio.h>
#include <assert.h>

#define DEVICE "/dev/ttyUSB0"
#define DRIVE (s16)137
#define DRIVE_DIRECT (s16)145
#define DELAY 1

static void delay(s32);
static void initSerialPort(void);
static void serialRun(void);
static void sendMotorCommand(u8 command, s16 param1, s16 param2);

#ifdef ROBOT_COMMS_DEBUG

/**
 * "Dummy" main function to test robot-comms with until vision & control are ready
 */
s32 main(void){
	initRobotComms();
	/*camera stuff*/
	printf("Parent about to begin commands\n");
	setMotorSpeeds(250, 250);
	delay(1000);
	setMotorSpeeds(250, -250);
	delay(1000);
	setRobotCourse(250, 100);
	delay(1000);
	setMotorSpeeds(0, 0);
	printf("Parent finished - killing child\n");
	disposeRobotComms();
	printf("Parent Dead\n");
	return 0;
}

#endif

/**
 * File descriptors.
 */
static s32 ser;
static s32 pipefd[2];

/**
 * Called after a fork to
 * -determine whether the process is child or parent
 * -close the relevant ends of the pipe
 * -tell the caller whether the process is a child (returns zero) or parent (returns non-zero)
 */
void initRobotComms(void){
	pid_t cpid;

	// Pipe successful?
	assert(pipe(pipefd) == 0);

	assert((cpid = fork()) != -1);

	if (cpid == 0) {
		close(pipefd[1]); // Close write end.
		serialRun();
		exit(EXIT_SUCCESS);
	} else {
		close(pipefd[0]); // Close read end.
	}
}
//Sets motor speeds to given values
void setMotorSpeeds(s16 right, s16 left){
	sendMotorCommand(DRIVE_DIRECT,right,left);	
}

//Instructs the robot to follow a course around a circle
void setRobotCourse(s16 velocity, s16 radius){
	sendMotorCommand(DRIVE,velocity,radius);
}

/**
 * Function to send a 5-byte packet over the pipe, in the style of "drive" and "drive direct" commands
 */
static void sendMotorCommand(u8 command, s16 param1, s16 param2){
	#define PACKET_SIZE 5 
	u8 param1High,param1Low,param2High,param2Low;
	//remove the high 8 bits of each parameter and cast to a u8
	param1Low=(u8)(param1 & (0x00FF));
	param2Low=(u8)(param2 & (0x00FF));
	//shift the high 8 bits into the low 8 bits of each parameter and cast to a u8
	param1High=(u8)(param1 >> 8);
	param2High=(u8)(param2 >> 8);

	write(pipefd[1],&command,1);
	printf("written command 1\n");
	write(pipefd[1],&param1High,1);
	printf("written command 2\n");
	write(pipefd[1],&param1Low,1);
	printf("written command 3\n");
	write(pipefd[1],&param2High,1);
	printf("written command 4\n");
	write(pipefd[1],&param2Low,1);
	printf("written command 5\n");
}

void disposeRobotComms(void){
	close(pipefd[1]);
	wait();
}

/**
 * Child program.
 */

static void delay(s32 ms){
	struct timespec t;
	t.tv_sec = ms / 1000;
	t.tv_nsec = (long) (ms % 1000) * 1000000;
	nanosleep(&t, NULL);
}

static void initSerialPort(void){
	//open the serial port
	ser = open(DEVICE, O_RDWR);
	assert (ser != -1);

	//tty_init - without errors
	struct termios termsettings;
	assert (tcgetattr(ser, &termsettings) != -1);
	assert (cfsetispeed(&termsettings, B57600) != -1);
	assert (cfsetospeed(&termsettings, B57600) != -1);
	cfmakeraw(&termsettings);
	assert (tcsetattr(ser, TCSANOW, &termsettings) != -1);
}

static void serialRun(void) {
	initSerialPort();
	u8 nextByte;
	while(read(pipefd[0],&nextByte,1)>0){
		assert (write(ser, &nextByte, 1) == 1);
		assert (tcdrain(ser) != -1);
		printf("Written a byte\n");
		delay(DELAY);
	}
	close(pipefd[0]);
	close(ser);
	return;
}




