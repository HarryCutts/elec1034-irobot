all:
	g++ -c -Iinclude -o obj/libvision.a		src/vision.cpp
	g++ -c -Iinclude -o obj/librobot-comms.a	src/robot-comms.cpp
	g++ -Wl -o bin/control obj/libvision.a obj/librobot-comms.a -L/usr/lib -lcv -lhighgui

vision:
	g++ -c -Iinclude -o obj/libvision.a src/vision.cpp
	g++ -Wl -o bin/vision obj/libvision.a -L/usr/lib -lcv -lhighgui

robot-comms:
	g++ -Iinclude -o bin/robot-comms src/robot-comms.cpp
