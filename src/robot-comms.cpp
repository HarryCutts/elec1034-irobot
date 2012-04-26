#include "../include/robot-comms.h"
#include "../include/type.h"

#define DEVICE "/dev/ttyUSB0"
#define DRIVE (s16)137
#define DRIVE_DIRECT (s16)145
#define DELAY 10

//holds the file descriptors for the pipe
static int pipeToRobot[2];

s32 ser;

s32 main(){
	initRobotComms();
	return 0;
}

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

void initSerialPort(){
	//open the serial port
	ser = open(DEVICE, 0_RDWR);
	assert (ser != -1);

	//tty_init - without errors
	struct terminos termsettings;
	assert (tcgetattr(ser, &termsettings) != -1);
	assert (cfsetispeed(&termsettings, B57600) != -1);
	assert (cfsetospeed(&termsettings, B57600) != -1);
	cfmakeraw(&termsettings);
	assert (tcsetattr(ser, TCSANOW, &termsettings) != -1);
}

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


static void sendMotorCommand(u8 command, s16 param1, s16 param2){
	#define PACKET_SIZE 5 
	
	u8 commandBytes[PACKET_SIZE];
	u8 param1High,param1Low,param2High,param2leftLow;
	//remove the high 8 bits of each parameter and cast to a u8
	param1Low=(u8)(param1 & (0x00FF));
	param2Low=(u8)(param2 & (0x00FF));
	//shift the high 8 bits into the low 8 bits of each parameter and cast to a u8
	param1High=(u8)(param1 >> 8);
	param2High=(u8)(param2 >> 8);

	commandBytes[0]=command;
	commandBytes[1]=param1High;
	commandBytes[2]=param1Low;
	commandBytes[3]=param2High;
	commandBytes[4]=param2Low;

	assert (write(ser, commandBytes, PACKET_SIZE) != PACKET_SIZE);
}

void setMotorSpeed(s16 right, s16 left){
	sendMotorCommand(DRIVE_DIRECT,right,left);
}


void setRobotCourse(s16 velocity, s16 radius){
	sendMotorCommand(DRIVE,velocity,radius);
}
