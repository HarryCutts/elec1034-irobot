all:
	g++ -c -Iinclude -o obj/libvision.a src/vision.cpp
	g++ -Wl -o bin/vision obj/libvision.a -L/usr/lib -lcv -lhighgui
