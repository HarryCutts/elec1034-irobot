all:
	#g++ -c -Iinclude -o obj/robot-comms.o src/robot-comms.cpp
	#g++ -Wl -o bin/robot-comms obj/robot-comms.o -L/usr/lib
	g++ -c -Iinclude -o obj/vision.o src/vision.cpp
	g++ -Wl -o bin/vision obj/vision.o -L/usr/lib -lcv -lhighgui
