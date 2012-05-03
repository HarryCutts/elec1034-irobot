#include <stdio.h>
#include <QCoreApplication>
#include "MyCameraWindow.h"
#include "vision.h"

MyCameraWindow::MyCameraWindow(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout;
    cvwidget = new QOpenCVWidget(this);
    layout->addWidget(cvwidget);
    setLayout(layout);
    resize(500, 400);

	bool lol;
	QStringList cmdline_args = QCoreApplication::arguments();
	
	if (cmdline_args.count() >= 2)
    	initVision(cmdline_args.at(1).toInt(&lol, 10));
    else
		initVision(41489);

	startTimer(100);  // 0.1-second timer
}

#define RED_MIN 128
#define RED_MUL 4
#define BLUEGREEN_MUL 3
#define AREA_MIN 4000

void MyCameraWindow::timerEvent(QTimerEvent*) {
    BallInfo* bi = see();
    assert(bi);
    IplImage* image = getDebugImage(bi);
    cvwidget->putImage(image);
    if (ballFound(bi)) {
//        fprintf(stderr, "Ball found.\t%f x radians,\t%f metres away.\n", getXRadians(bi), getBallDistance(bi));
    }
}

