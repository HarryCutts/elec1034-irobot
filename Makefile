all:
	g++ -c -Iinclude -o obj/robot-comms.o src/robot-comms.cpp
	g++ -Wl -o bin/robot-comms obj/robot-comms.o -L/usr/lib
