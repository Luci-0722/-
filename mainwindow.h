#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <opencv2\opencv.hpp>
#include <QTimer>
#include <qimage.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <mutex>
#include <queue>
#include <QCloseEvent>
#include <condition_variable>
#include <qpainter.h>

#include "BYTETracker.h"
#include "threadPool.h"
#include "Yolo.h"
using namespace cv;
using namespace std;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent);
    ~MainWindow();
	void setImage(const QImage& image) {
		m_image = image;
		update();
	}
	void run();
protected:

	void closeEvent(QCloseEvent* event) override {
		// 在关闭窗口时将 bool 变量置为 0
		isStop = true;
		event->accept();
	}

	void cvShow(Mat& frame) const
	{
		namedWindow("Display");
		//显示图像
		imshow("Display", frame);

		//暂停，等待按键结束
		waitKey(10);
	}

	void paintEvent(QPaintEvent* event) override {
		QPainter painter(this);
		painter.drawImage(0, 0, m_image);
	}

private:
    Ui::MainWindow *ui;
	QImage m_image;

	string curImgPath;
	string videoName;

	//bool isDetect = false;
	//bool isDisplay = false;
	//bool isStop = false;
	//bool getFrameOk = false;
	atomic<bool> isDetect = false;
	atomic<bool> isDisplay = false;
	atomic<bool> isStop = false;
	atomic<bool> getFrameOk = false;
	YOLO* yolo_model;
	BYTETracker* bytetracker;
	queue<Mat> videoframes;
	mutex vedioQueueLock;
	mutex isDisplayLock;
	condition_variable cv;
	condition_variable cv_display;

	ThreadPool* myThreadPool;

	QImage blank = QImage(640, 640, QImage::Format_RGB32);
	void detect_img();

	void detect_video_asyn(bool isCaption);

	void detect_video_syn(bool isCaption);

	void getVideoFrame(bool isCaption);

	void test_bytetrack(cv::Mat& frame, std::vector<BoxInfo>& results);

	void DisplayButton();

	void stopButton();

	
private slots:
	void on_viedoImg_button_clicked();
	void on_captionButton_clicked();
	void on_stopButton_clicked();
	void on_DisplayButton_clicked();

};
#endif // MAINWINDOW_H
