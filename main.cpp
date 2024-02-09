#include <string>
#include <QApplication>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <onnxruntime_cxx_api.h>

#include <thread>

#include <queue>
#include <mutex>
#include "mainwindow.h"
#include "Yolo.h"

using namespace std;
using namespace cv;
using namespace Ort;


Net_config yolo_nets = { 0.4, 0.1, 0.35, "class.names", "best_lc.onnx" };


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow w(nullptr);
	w.run();
	w.show();
	return a.exec();
}
