all:
	g++ -c -Iinclude -o obj/vision.o src/vision.cpp
	g++ -Wl -o bin/vision obj/vision.o -L/usr/lib -lcv -lhighgui
