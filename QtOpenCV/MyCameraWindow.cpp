#include <stdio.h>
#include "MyCameraWindow.h"

MyCameraWindow::MyCameraWindow(CvCapture *cam, QWidget *parent) : QWidget(parent) {
    camera = cam;
    QVBoxLayout *layout = new QVBoxLayout;
    cvwidget = new QOpenCVWidget(this);
    layout->addWidget(cvwidget);
    setLayout(layout);
    resize(500, 400);

    startTimer(100);  // 0.1-second timer
 }

#define RED_MIN 128
#define RED_MUL 4
#define BLUEGREEN_MUL 3
#define AREA_MIN 4000

void MyCameraWindow::timerEvent(QTimerEvent*) {
    IplImage *image=cvQueryFrame(camera);
    unsigned char * pixel_data = (unsigned char *)(image->imageData);
    int w = image->width;
    int h = image->height;
    int c = image->nChannels;
    int area = 0;
    int moment = 0;
    int y = 0;
    while (y<h) {
      int x = 0;
      while (x < w) {
        unsigned char * blue  = pixel_data;
        unsigned char * green = pixel_data + 1;
        unsigned char * red   = pixel_data + 2;
        if (((RED_MUL*(int)*red) > (BLUEGREEN_MUL*((int)*blue + (int)* green))) 
            & (*red > RED_MIN)) {
          *red = 255;
          area = area +1;
          moment = moment + x; 
        } else {
          *red = 0;
        }
        *blue = 0;
        *green = 0;
        pixel_data = pixel_data + c; 
        x = x + 1;
      }
      y = y + 1;
    }
    if (area > AREA_MIN) {
      int xpos = moment/area;
      int y = 0;
      unsigned char * blue_pixel =  (unsigned char *)(image->imageData) + c*xpos;
      while (y < h) {
        * blue_pixel = 255;
        blue_pixel = blue_pixel + c*w;
        y = y+1;
      }
      printf("X position %d\n",xpos);
    }
    cvwidget->putImage(image);
}

