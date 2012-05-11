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
#define DRIVE (u8)137
#define DRIVE_DIRECT (u8)145
#define GET_SENSOR (u8)142
#define SENSOR_BUMP_DROP (u8)7
#define START_OI (u8)128
#define SAFE_MODE (u8)131
#define FULL_MODE (u8)132
#define ROBOT_START_MODE SAFE_MODE
#define DELAY 10
/*ensures that bytes are not sent too frequently. Specifically, reads from robot
sensors must not occur more often than every 15ms (but this requires 2 bytes
sent, i.e. 2*10ms > 15ms, so delay=10 is okay) */

// Sensor Constants //
#define SENSOR_BUMP_RIGHT	0b00001
#define SENSOR_BUMP_LEFT	0b00010
#define SENSOR_DROP_RIGHT	0b00100
#define SENSOR_DROP_LEFT	0b01000
#define SENSOR_DROP_CASTOR	0b10000

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
	setMotorSpeeds(250, 250);
	delay(1000);
	setMotorSpeeds(250, -250);
	delay(1000);
	setRobotCourse(250, 100);
	delay(1000);
	setMotorSpeeds(0, 0);
	disposeRobotComms();
	return 0;
}

#endif

/**
 * File descriptors.
 */
static s32 ser;
static s32 pipeToRobotfd[2];
static s32 pipeFromRobotfd[2];

/**
 * Called after a fork to
 * -determine whether the process is child or parent
 * -close the relevant ends of the pipe
 * -tell the caller whether the process is a child (returns zero) or parent (returns non-zero)
 */
void initRobotComms(void){
	pid_t cpid;

	// Pipe successful?
	assert(pipe(pipeToRobotfd) == 0);
	assert(pipe(pipeFromRobotfd) == 0);

	assert((cpid = fork()) != -1);

	if (cpid == 0) {
		//we are the child
		close(pipeToRobotfd[1]); // Close write end of pipe we read from.
		close(pipeFromRobotfd[0]); // Close read end of pipe we write to.
		serialRun();
		exit(EXIT_SUCCESS);
	} else {
		//we are the parent
		close(pipeToRobotfd[0]); // Close read end of pipe we write to.
		close(pipeFromRobotfd[1]); // Close write end of pipe we read from.
		//put the robot into the right mode
		s16 resetRobot =START_OI,robotMode=ROBOT_START_MODE;
		write(pipeToRobotfd[1],&resetRobot,1);
		write(pipeToRobotfd[1],&robotMode,1);
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

//requests a "bump and wheel drops" packet from the robot
void requestSensorData(void){
	u8 command = GET_SENSOR;
	u8 packetNo = SENSOR_BUMP_DROP;
	write(pipeToRobotfd[1],&command,1);
	write(pipeToRobotfd[1],&packetNo,1);
}

//retrieves the sensor packet from the child; blocking
SensorData retrieveSensorData(void){
	SensorData sensorData = 0;
	assert(read(pipeFromRobotfd[0],&sensorData,1)>0);
	return sensorData;
}

bool getCastorWheeldrop(SensorData d) {
	return (d & SENSOR_DROP_CASTOR) > 0;
}

bool getLeftWheeldrop(SensorData d) {
	return (d & SENSOR_DROP_LEFT) > 0;
}

bool getRightWheeldrop(SensorData d) {
	return (d & SENSOR_DROP_RIGHT) > 0;
}

bool getLeftBump(SensorData d) {
	return (d & SENSOR_BUMP_LEFT) > 0;
}

bool getRightBump(SensorData d) {
	return (d & SENSOR_BUMP_RIGHT) > 0;
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

	write(pipeToRobotfd[1],&command,1);
	write(pipeToRobotfd[1],&param1High,1);
	write(pipeToRobotfd[1],&param1Low,1);
	write(pipeToRobotfd[1],&param2High,1);
	write(pipeToRobotfd[1],&param2Low,1);
}

void disposeRobotComms(void){
	close(pipeToRobotfd[1]);
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
//sends a byte on the serial port
static void sendByte(u8 byte) {
	assert (write(ser, &byte, 1) == 1);
	assert (tcdrain(ser) != -1);
	delay(DELAY);
}
static void serialRun(void) {
	//set up the serial port
	initSerialPort();
	//when this is set to zero, the pipe is closed	
	u8 pipe_to_parent_open=1;
	//byte from the parent to the robot
	u8 byteToRobot;
	//byte from the robot to the parent
	u8 byteFromRobot;
	//this is 0 normally, gets set to 1 with a GET_SENSOR and gets set to 2 with a subsequent SENSOR_BUMP_DROP; when it is 2, we know we have to read from the robot.
	u8 readyToReadFromRobot = 0;
	while ((read(pipeToRobotfd[0],&byteToRobot,1)>0)){
		//read a byte from the pipe; it will return 0 at eof so we know we can quit
		if (byteToRobot == GET_SENSOR) {
			readyToReadFromRobot = 1;
		}
		if ((readyToReadFromRobot == 1)&&(byteToRobot == SENSOR_BUMP_DROP)) {
			readyToReadFromRobot = 2;
		}
		sendByte(byteToRobot);
		//see if there is a byte from the robot
		if (readyToReadFromRobot == 2) {
			assert(read(ser,&byteFromRobot,1)>0);
			write(pipeFromRobotfd[1],&byteFromRobot,1);
			readyToReadFromRobot = 0;
		}
	}


	close(pipeToRobotfd[0]);
	close(ser);
	return;
}
