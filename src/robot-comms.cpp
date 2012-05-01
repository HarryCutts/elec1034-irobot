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
#define DELAY 10 

<<<<<<< HEAD
/**
 * File descriptors.
 */
static s32 ser;
static s32 pipefd[2];
=======
//holds the file descriptors for the pipe
static int pipeToRobot[2];

s32 ser;
>>>>>>> 6a7eee9f256b703d2fdda66f5defde4f7caf6d13

static void delay(s32 ms){
	struct timespec t;
	t.tv_sec = ms / 1000;
	t.tv_nsec = (long) (ms % 1000) * 1000000;
	nanosleep(&t, NULL);
}

<<<<<<< HEAD


/**
 * Child program.
 */
void serialRun() {
	u8 nextByte;
	while(read(pipefd[0],&nextByte,1)>0){
		assert (write(ser, &nextByte, 1) == 1);
		delay(DELAY);
	}
	close(pipefd[0]);
	return;
}

void closeSerial(){
	close(pipefd[1]);
	wait();
}


=======
int initRobotComms(){
	pid_t process;

	//create a pipe so the parent can send information to the child
	assert(pipe(pipeToRobot) == 0);
	//create the child process to deal with the serial port
	process = fork();
	//see if we are the parent or child
	if (current_process == (pid_t) 0){
		//if we are the child, close the sending end
		close(pipeToRobot[1]);
		//initialise the serial port
		initSerialPort();
		//transfer information between the serial port and the pipe
		serialPortController(pipeToRobot[0]);
		//once the parent closes the pipe, return to the main program, indicating that this child process should exit
		return 0;
	} else {
		close(pipeToRobot[0]);
		//return 1 to indicate that we are the parent
		return 1;
	}	
}

>>>>>>> 6a7eee9f256b703d2fdda66f5defde4f7caf6d13
void initSerialPort(){
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

<<<<<<< HEAD
s32 initRobotComms(){
	pid_t cpid;

	// Pipe successful?
	assert(pipe(pipefd) == 0);
	
	assert((cpid = fork()) != -1);

	if (cpid == 0) {
		close(pipefd[1]); // Close write end.
		initSerialPort();
	} else {
		close(pipefd[0]); // Close read end.
	}
=======
void serialPortController(int pipeEnd){
	FILE *streamToRobot;
	streamToRobot = fdopen(pipeEnd, "r");
	while ((c=fgetc(streamToRobot)) != EOF) {
		//write the characters to the serial port
		assert(write(ser, c, 1) != 1);
		//wait in between each character so the robot doesn't get confused
		delay(10);
	}
	fclose(streamToRobot);
	return;
}

void delay(s32 ms){
	struct timespec t;
	t.tv_sec = ms / 1000;
	t.tv_nsec = (long) (ms % 1000) * 1000000;
	nanosleep(&t, NULL);
}
>>>>>>> 6a7eee9f256b703d2fdda66f5defde4f7caf6d13

	return cpid;
}

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
	write(pipefd[1],&param1High,1);
	write(pipefd[1],&param1Low,1);
	write(pipefd[1],&param2High,1);
	write(pipefd[1],&param2Low,1);
	printf("written commands");
}

void setMotorSpeeds(s16 right, s16 left){
	sendMotorCommand(DRIVE_DIRECT,right,left);	
}


void setRobotCourse(s16 velocity, s16 radius){
	sendMotorCommand(DRIVE,velocity,radius);
}


s32 main(){
	if (initRobotComms() == 0) {
		serialRun();
		printf("Child about to die\n");
		return 0;
	} else {
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
		closeSerial();
		return 0;
	}



	/*initRobotComms();
	
	setMotorSpeeds(250, 250);
	delay(1000);
	setMotorSpeeds(250, -250);
	delay(1000);
	setRobotCourse(250, 100);
	delay(1000);
	setMotorSpeeds(0, 0);

	return 0;*/
}
