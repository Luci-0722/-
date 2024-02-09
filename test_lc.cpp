#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPushButton>
#include <QFileDialog>
#include <stdio.h>
#include <string>
#include <queue>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <qaction.h>

void test()
{
    queue<Mat> videoframes;
    Mat frame;
    VideoCapture capture;

    capture.open("./data/output.mp4");


    if (!capture.isOpened())
    {
        return;
    }
    videoframes = queue<Mat>();
    while (capture.read(frame))
    {
        qDebug("add one frame");
        videoframes.push(frame.clone());

        //videoframes.push(move(frame)); //解决？

        //frame = cv::Mat(480, 640, CV_8UC3);
    }
    capture.release();
    qDebug("getFrameOk");

    while (!videoframes.empty())
    {

        Mat frame = videoframes.front();
        videoframes.pop();

        namedWindow("Display");
        //显示图像
        imshow("Display", frame);

        //暂停，等待按键结束
        waitKey(10);

        qDebug("Detect frame tid:%x", this_thread::get_id());

    }
}