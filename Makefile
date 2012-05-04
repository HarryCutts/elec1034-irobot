all:
	g++ -c -Iinclude -o obj/libvision.a		src/vision.cpp
	g++ -c -Iinclude -o obj/librobot-comms.a	src/robot-comms.cpp
	g++ -c -Iinclude -o obj/control.a		src/control.cpp
	g++ -Wl -o bin/control obj/control.a obj/libvision.a obj/librobot-comms.a -L/usr/lib -lcv -lhighgui

vision:
	g++ -c -Iinclude -o obj/libvision.a src/vision.cpp
	g++ -Wl -o bin/vision obj/libvision.a -L/usr/lib -lcv -lhighgui

vision-with-main:
	g++ -c -Iinclude -o obj/libvision.a -D VISION_DEBUG  src/vision.cpp
	g++ -Wl -o bin/vision obj/libvision.a -L/usr/lib -lcv -lhighgui


robot-comms:
	g++ -Iinclude -o bin/robot-comms -D ROBOT_COMMS_DEBUG src/robot-comms.cpp
