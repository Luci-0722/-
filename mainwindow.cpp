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

#include "Yolo.h"
using namespace std;
using namespace cv;

extern Net_config yolo_nets;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->yolo_model = new YOLO(yolo_nets);
    this->bytetracker = new BYTETracker(25, 30);
    this->myThreadPool = new ThreadPool(5);
    connect(ui->img_video_in, &QPushButton::clicked, this, &MainWindow::on_viedoImg_button_clicked);
    connect(ui->caption_in, &QPushButton::clicked, this, &MainWindow::on_captionButton_clicked);
    connect(ui->stop, &QPushButton::clicked, this, &MainWindow::on_stopButton_clicked);
    connect(ui->display, &QPushButton::clicked, this, &MainWindow::on_DisplayButton_clicked);
    setImage(blank);
}

void MainWindow::run()
{
    ui->textBrowser->append("program is running");
    isStop = false;   
}

void MainWindow::on_viedoImg_button_clicked()
{
    QString str = QFileDialog::getOpenFileName(this, "choose picture");
    if (str.isEmpty()) return;
    string filename = str.toStdString();
    isDetect = false;
    isDisplay = false;
    //关闭被暂停线程
    cv_display.notify_all();

    //if (filename.find(".jpg") != -1)
    //{
    //    curImgPath = filename;
    //    myThreadPool->enqueue(bind(&MainWindow::detect_img, this));
    //    //std::thread detect_img_thread(bind(&MainWindow::detect_img, this));
    //    //detect_img_thread.detach();
    //}
    //else
    //{
        videoName = filename;
        myThreadPool->enqueue(bind(&MainWindow::detect_video_asyn, this, false));
        //std::thread detect_video_thread(bind(&MainWindow::detect_video_asyn, this, false));
        //detect_video_thread.detach();
    //}
    
}

void MainWindow::on_captionButton_clicked()
{
    myThreadPool->enqueue(std::bind(&MainWindow::detect_video_asyn, this, true));
    //std::thread detect_caption_thread(std::bind(&MainWindow::detect_video_asyn, this, true));
    //detect_caption_thread.detach();
}

void MainWindow::on_DisplayButton_clicked()
{
    myThreadPool->enqueue(std::bind(&MainWindow::DisplayButton, this));

}

void MainWindow::DisplayButton()
{

    //std::unique_lock<std::mutex> lk(vedioQueueLock);
    isDisplay = !isDisplay;

    qDebug("on_DisplayButton_clicked");
    ui->textBrowser->append("on_DisplayButton_clicked");
    if (isDisplay)
    {
        qDebug("notify_display");
        cv_display.notify_all();
    }

}


void MainWindow::on_stopButton_clicked()
{
    
    std::thread stopButton_thread(std::bind(&MainWindow::stopButton, this));
    stopButton_thread.detach();
}


void MainWindow::stopButton()
{
    qDebug("on_stopButton_clicked");
    ui->textBrowser->append("on_stopButton_clicked");

    isDetect = false; //停止检测
    isDisplay = false; //暂停播放

    cv_display.notify_all();
    waitKey(500);
    setImage(blank);
}



void MainWindow::detect_img()
{
    if (!isStop)
    {
        Mat frame = imread(curImgPath);
        vector<BoxInfo> results;
        yolo_model->detect(frame, results);
        cv::resize(frame, frame, cv::Size(640, 640));
        QImage srcQImage = QImage((uchar*)(frame.data), frame.cols, frame.rows, QImage::Format_RGB888).rgbSwapped();
        setImage(srcQImage);
        ui->personNumLCD->display(yolo_model->getPersonNum());
    }
}

void MainWindow::detect_video_asyn(bool isCaption)
{
    STrack::_count = 0;
    isDetect = false;
    isDisplay = true;
    cv_display.notify_all();
    waitKey(500);
    isDetect = true;
    isDisplay = true;
    getFrameOk = false;
    delete this->bytetracker;
    this->bytetracker = new BYTETracker(50, 40);
  
    myThreadPool->enqueue(std::bind(&MainWindow::getVideoFrame, this, isCaption));
    Mat frame;
    ui->textBrowser->append("begin detect");
    time_t last_time = time(nullptr);
    int cnt = 0;
    while (!isStop && isDetect)
    {
        unique_lock<mutex> mtx(isDisplayLock);
        while (!isDisplay && isDetect)
        {
            qDebug("detect_video_asyn::wait to Display");
            cv_display.wait_for(mtx, chrono::seconds(20));    
        }
        if (!isDetect) break;
        unique_lock<mutex> mtx2(vedioQueueLock);
        if (videoframes.empty() && getFrameOk) break;
        if (videoframes.empty()) continue;
        //get frame
        frame = videoframes.front();
        videoframes.pop();
        mtx2.unlock();

        //do detect
        vector<BoxInfo> results;
        yolo_model->detect(frame, results);
        //results.clear();
        //test_bytetrack(frame, results);
        cv::resize(frame, frame, cv::Size(640, 640));
        QImage srcQImage = QImage((uchar*)(frame.data), frame.cols, frame.rows, QImage::Format_RGB888).rgbSwapped();
        setImage(srcQImage);
        ++cnt;
        int cur_time = time(nullptr);
        if (cur_time - last_time == 1) {
            qDebug("1s: %d frame", cnt);
            cnt = 0;
            last_time = time(NULL);
        }
        //数字显示
        int personNum = yolo_model->getPersonNum();
        personNum = STrack::_count;
        ui->personNumLCD->display(personNum); //改数字
        
    }
    qDebug("end video detect");
    ui->textBrowser->append("end video detect");

}

void MainWindow::getVideoFrame(bool isCaption)
{
    Mat frame;
    VideoCapture capture;
    if (isCaption)
    {
        frame = capture.open(0);
    }
    else
    {
        frame = capture.open(videoName);
    }
    
    if (!capture.isOpened())
    {
        return;
    }
    videoframes = queue<Mat>();
    while (!isStop && isDetect && capture.read(frame))
    {
        unique_lock<mutex> mtx2(isDisplayLock);
        while (!isDisplay && isDetect)
        {

            qDebug("getVideoFrame::wait to Display");
            cv_display.wait_for(mtx2, chrono::seconds(20));
        }
        if (!isDetect)
        {
            return;
        }
        lock_guard<mutex> mtx(vedioQueueLock);
        //qDebug("add one frame");

        videoframes.push(frame.clone());
    }
    capture.release();
    getFrameOk = true;
    qDebug("getFrameOk");
}

void MainWindow::detect_video_syn(bool isCaption)
{

    isDetect = false;
    isDisplay = true;
    cv_display.notify_all();
    waitKey(500);
    isDetect = true;
    isDisplay = true;
    time_t last_time = time(nullptr);
    int cnt = 0;
    VideoCapture capture;
    Mat frame;
    if (isCaption)
    {
        frame = capture.open(0);
    }
    else
    {
        frame = capture.open(videoName);
    }

    if (!capture.isOpened())
    {
        return;
    }
    videoframes = queue<Mat>();
    while (!isStop && isDetect && capture.read(frame))
    {

        unique_lock<mutex> mtx(isDisplayLock);
        while (!isDisplay)
        {
            cv_display.wait(mtx);
        }
        qDebug("Detect frame tid:%x", this_thread::get_id());
        std::vector<BoxInfo> results;
        yolo_model->detect(frame, results);
        //test_bytetrack(frame, results);
        cv::resize(frame, frame, cv::Size(640, 640));
        QImage srcQImage = QImage((uchar*)(frame.data), frame.cols, frame.rows, QImage::Format_RGB888).rgbSwapped();
        setImage(srcQImage);
        ++cnt;
        int cur_time = time(nullptr);
        if (cur_time - last_time == 1) {
            qDebug("1s: %d frame", cnt);
            cnt = 0;
            last_time = time(NULL);
        }
        
    }
    capture.release();
    qDebug("end video detect");
    setImage(blank);
}

void MainWindow::test_bytetrack(cv::Mat& frame, std::vector<BoxInfo>& results)
{
    std::vector<BoxInfo>& objects = results;


    std::vector<STrack> output_stracks = bytetracker->update(objects);

    //for (unsigned long i = 0; i < output_stracks.size(); i++)
    //{
    //    std::vector<float> tlwh = output_stracks[i].tlwha;

    //    cv::Scalar s = this->bytetracker->get_color(output_stracks[i].track_id);
    //    cv::putText(frame, cv::format("%d", output_stracks[i].track_id), cv::Point(tlwh[0], tlwh[1] - 5),
    //        0, 0.6, cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
    //    cv::rectangle(frame, cv::Rect(tlwh[0], tlwh[1], tlwh[2], tlwh[3]), s, 2);
    //    
    //}

    for (size_t i = 0; i < output_stracks.size(); ++i)
    {
        std::vector<float> tlwh = output_stracks[i].tlwha;
        RotatedRect rectInput = RotatedRect{ Point2f(tlwh[0] + tlwh[2], tlwh[1] + tlwh[3]), Size2f(tlwh[2], tlwh[3]), output_stracks[i].angle };
        Point2f* vertices = new cv::Point2f[4];
        rectInput.points(vertices);
        cv::Scalar s = this->bytetracker->get_color(output_stracks[i].track_id);
        for (int j = 0; j < 4; j++)
        {
            line(frame, vertices[j], vertices[(j + 1) % 4], s, 2);
        }

        int xmin = (int)vertices[0].x;
        int ymin = (int)vertices[0].y - 10;
        string label = format("%.2f", 0.75);
        label = string("person") + ":" + label;
        
        //putText(frame, label, Point(xmin, ymin - 5), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 255, 0), 2);
        putText(frame, cv::format("%d", output_stracks[i].track_id), Point(xmin, ymin - 5),
            0, 0.6, s, 2, cv::LINE_AA);
    }
}

MainWindow::~MainWindow()
{
    delete myThreadPool;
    delete yolo_model;
    delete bytetracker;
    delete ui;
}
