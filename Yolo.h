#pragma once
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <onnxruntime_cxx_api.h>

using namespace std;
using namespace cv;
using namespace Ort;

typedef struct BoxInfo
{
	RotatedRect box;
	float score;
	int label;
} BoxInfo;


struct Net_config
{
	float confThreshold; // Confidence threshold
	float nmsThreshold;  // Non-maximum suppression threshold
	float objThreshold;  //Object Confidence threshold
	std::string classesFile = "class.names";
	std::string model_path = "best_lc.onnx";
	std::string imgpath;
};

class YOLO
{
public:
	YOLO(Net_config config);
	void detect(Mat& frame, std::vector<BoxInfo>& results);
	vector<pair<float, float>> xywha2vertex(vector<float> box);
	int getPersonNum() const { return personNum;}

	vector<string> class_names;
private:

	int inpWidth = 640;
	int inpHeight = 640;
	int nout;
	int num_proposal;

	int num_class;
	const float stride[3] = { 8.0, 16.0, 32.0 };
	float confThreshold;
	float nmsThreshold;
	float objThreshold;
	string classesFile;
	string model_path;
	const bool keep_ratio = true;
	vector<float> input_image_;
	void normalize_(Mat img);
	Mat resize_image(Mat srcimg, int* newh, int* neww, int* top, int* left);
	void nms_angle(vector<BoxInfo>& input_boxes);

	Env env;
	Ort::Session* ort_session = nullptr;
	SessionOptions sessionOptions = SessionOptions();
	vector<char*> input_names;
	vector<char*> output_names;
	vector<vector<int64_t>> input_node_dims; // >=1 outputs
	vector<vector<int64_t>> output_node_dims; // >=1 outputs

	int personNum;
};