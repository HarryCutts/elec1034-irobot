#include "../include/robot-comms.h"
#include "../include/type.h"

#define DEVICE "/dev/ttyUSB0"
#define DRIVE (s16)137
#define DRIVE_DIRECT (s16)145
#define DELAY 10

s32 ser;

s32 main(){
	initRobotComms();
	return 0;
}

void initRobotComms(){
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
