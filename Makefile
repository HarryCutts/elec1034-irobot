all:
	g++ `pkg-config --cflags --libs opencv` src/vision.cpp
