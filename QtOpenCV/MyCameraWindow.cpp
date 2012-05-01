#include <stdio.h>
#include "MyCameraWindow.h"
#include "vision.h"

MyCameraWindow::MyCameraWindow(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout;
    cvwidget = new QOpenCVWidget(this);
    layout->addWidget(cvwidget);
    setLayout(layout);
    resize(500, 400);
    initVision();
    startTimer(100);  // 0.1-second timer
}

#define RED_MIN 128
#define RED_MUL 4
#define BLUEGREEN_MUL 3
#define AREA_MIN 4000

void MyCameraWindow::timerEvent(QTimerEvent*) {
    fprintf(stderr, "Calling see()...\n");
    BallInfo* bi = see();
    assert(bi);
    IplImage* image = getDebugImage(bi);
    fprintf(stderr, "Putting image on form...");
    cvwidget->putImage(image);
    fprintf(stderr, " done.\n");
}

