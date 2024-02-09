#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <onnxruntime_cxx_api.h>
#include <fstream>
#include <sstream>
#include <iostream>

#include "Yolo.h"

using namespace std;
using namespace cv;
using namespace Ort;




vector<pair<float, float>> YOLO::xywha2vertex(vector<float> box)
{
	pair<float, float> center = { box[0], box[1] };
	float w = box[2];
	float h = box[3];
	float rad = box[4];

	vector<float> verti(2);
	verti[0] = (h / 2) * sin(rad);
	verti[1] = -(h / 2) * cos(rad);

	vector<float> hori(2);
	hori[0] = (w / 2) * cos(rad);
	hori[1] = (w / 2) * sin(rad);

	vector<pair<float, float>> res(4);
	res[0].first = center.first + verti[0] - hori[0];
	res[0].second = center.second + verti[1] - hori[1];

	res[1].first = center.first + verti[0] + hori[0];
	res[1].second = center.second + verti[1] + hori[1];

	res[2].first = center.first - verti[0] + hori[0];
	res[2].second = center.second - verti[1] + hori[1];

	res[3].first = center.first - verti[0] - hori[0];
	res[3].second = center.second - verti[1] - hori[1];

	return res;
}

YOLO::YOLO(Net_config config)
{
	this->confThreshold = config.confThreshold;
	this->nmsThreshold = config.nmsThreshold;
	this->objThreshold = config.objThreshold;
	this->env = Env(ORT_LOGGING_LEVEL_ERROR, "yolov5-angle");
	this->classesFile = config.classesFile;
	this->model_path = config.model_path;
	std::wstring widestr = std::wstring(model_path.begin(), model_path.end());
	//OrtStatus* status = OrtSessionOptionsAppendExecutionProvider_CUDA(sessionOptions, 0);
	sessionOptions.SetGraphOptimizationLevel(ORT_ENABLE_BASIC);
	ort_session = new Session(env, widestr.c_str(), sessionOptions);
	size_t numInputNodes = ort_session->GetInputCount();
	size_t numOutputNodes = ort_session->GetOutputCount();
	AllocatorWithDefaultOptions allocator;
	for (int i = 0; i < numInputNodes; i++)
	{
		input_names.push_back(ort_session->GetInputName(i, allocator));
		Ort::TypeInfo input_type_info = ort_session->GetInputTypeInfo(i);
		auto input_tensor_info = input_type_info.GetTensorTypeAndShapeInfo();
		auto input_dims = input_tensor_info.GetShape();
		input_node_dims.push_back(input_dims);
	}
	for (int i = 0; i < numOutputNodes; i++)
	{
		output_names.push_back(ort_session->GetOutputName(i, allocator));
		Ort::TypeInfo output_type_info = ort_session->GetOutputTypeInfo(i);
		auto output_tensor_info = output_type_info.GetTensorTypeAndShapeInfo();
		auto output_dims = output_tensor_info.GetShape();
		output_node_dims.push_back(output_dims);
	}
	this->inpHeight = input_node_dims[0][2];
	this->inpWidth = input_node_dims[0][3];
	this->nout = output_node_dims[0][2];
	this->num_proposal = output_node_dims[0][1];

	ifstream ifs(classesFile.c_str());
	string line;
	while (getline(ifs, line)) this->class_names.push_back(line);
	this->num_class = class_names.size();
}

Mat YOLO::resize_image(Mat srcimg, int* newh, int* neww, int* top, int* left)
{
	int srch = srcimg.rows, srcw = srcimg.cols;
	*newh = this->inpHeight;
	*neww = this->inpWidth;
	Mat dstimg;
	if (this->keep_ratio && srch != srcw) {
		float hw_scale = (float)srch / srcw;
		if (hw_scale > 1) {
			*newh = this->inpHeight;
			*neww = int(this->inpWidth / hw_scale);
			resize(srcimg, dstimg, Size(*neww, *newh), INTER_AREA);
			*left = int((this->inpWidth - *neww) * 0.5);
			copyMakeBorder(dstimg, dstimg, 0, 0, *left, this->inpWidth - *neww - *left, BORDER_CONSTANT, 114);
		}
		else {
			*newh = (int)this->inpHeight * hw_scale;
			*neww = this->inpWidth;
			resize(srcimg, dstimg, Size(*neww, *newh), INTER_AREA);
			*top = (int)(this->inpHeight - *newh) * 0.5;
			copyMakeBorder(dstimg, dstimg, *top, this->inpHeight - *newh - *top, 0, 0, BORDER_CONSTANT, 114);
		}
	}
	else {
		resize(srcimg, dstimg, Size(*neww, *newh), INTER_AREA);
	}
	return dstimg;
}

void YOLO::normalize_(Mat img)
{
	//    img.convertTo(img, CV_32F);
	int row = img.rows;
	int col = img.cols;
	this->input_image_.resize(row * col * img.channels());
	for (int c = 0; c < 3; c++)
	{
		for (int i = 0; i < row; i++)
		{
			for (int j = 0; j < col; j++)
			{
				float pix = img.ptr<uchar>(i)[j * 3 + 2 - c];
				this->input_image_[c * row * col + i * col + j] = pix / 255.0;

			}
		}
	}
}

void YOLO::nms_angle(vector<BoxInfo>& input_boxes)
{
	sort(input_boxes.begin(), input_boxes.end(), [](BoxInfo a, BoxInfo b) { return a.score > b.score; });
	vector<float> vArea(input_boxes.size());
	for (int i = 0; i < int(input_boxes.size()); ++i)
	{
		vArea[i] = input_boxes[i].box.size.area();
	}

	vector<bool> isSuppressed(input_boxes.size(), false);
	for (int i = 0; i < int(input_boxes.size()); ++i)
	{
		if (isSuppressed[i]) { continue; }
		for (int j = i + 1; j < int(input_boxes.size()); ++j)
		{
			if (isSuppressed[j]) { continue; }
			vector<Point2f> intersectingRegion;
			rotatedRectangleIntersection(input_boxes[i].box, input_boxes[j].box, intersectingRegion);
			if (intersectingRegion.empty()) { continue; }
			float inter = contourArea(intersectingRegion);
			float ovr = inter / (vArea[i] + vArea[j] - inter);

			if (ovr >= this->nmsThreshold)
			{
				isSuppressed[j] = true;
			}
		}
	}
	// return post_nms;
	int idx_t = 0;
	input_boxes.erase(remove_if(input_boxes.begin(), input_boxes.end(), [&idx_t, &isSuppressed](const BoxInfo& f) { return isSuppressed[idx_t++]; }), input_boxes.end());
}

inline float sigmoid(float x)
{
	return 1.0 / (1 + expf(-x));
}

void YOLO::detect(Mat& frame, std::vector<BoxInfo>& generate_boxes)
{
	int newh = 0, neww = 0, padh = 0, padw = 0;
	Mat dstimg = this->resize_image(frame, &newh, &neww, &padh, &padw);
	this->normalize_(dstimg);
	array<int64_t, 4> input_shape_{ 1, 3, this->inpHeight, this->inpWidth };

	auto allocator_info = MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
	Value input_tensor_ = Value::CreateTensor<float>(allocator_info, input_image_.data(), input_image_.size(), input_shape_.data(), input_shape_.size());

	//   ʼ    
	vector<Value> ort_outputs = ort_session->Run(RunOptions{ nullptr }, &input_names[0], &input_tensor_, 1, output_names.data(), output_names.size());   //   ʼ    
	/////generate proposals
	//vector<BoxInfo> generate_boxes;
	float ratioh = (float)frame.rows / newh, ratiow = (float)frame.cols / neww;
	int n = 0, q = 0, i = 0, j = 0, row_ind = 0, k = 0; ///xmin,ymin,xamx,ymax, box_score,class_score
	const float* pdata = ort_outputs[0].GetTensorMutableData<float>();
	for (n = 0; n < 3; n++)   ///    ͼ ߶ 
	{
		int num_grid_x = (int)(this->inpWidth / this->stride[n]);
		int num_grid_y = (int)(this->inpHeight / this->stride[n]);
		for (q = 0; q < 3; q++)    ///anchor
		{
			for (i = 0; i < num_grid_y; i++)
			{
				for (j = 0; j < num_grid_x; j++)
				{
					float box_score = sigmoid(pdata[5]);
					if (box_score > this->objThreshold)
					{
						int class_idx = 0;
						float max_class_socre = 0;
						for (k = 0; k < num_class; k++)
						{
							float score = sigmoid(pdata[k + 6]);
							if (score > max_class_socre)
							{
								max_class_socre = score;
								class_idx = k;
							}
						}
						max_class_socre *= box_score;
						if (max_class_socre > this->confThreshold)
						{
							float cx = pdata[0];
							float cy = pdata[1];
							float w = pdata[2];
							float h = pdata[3];  ///h
							cx = (cx - padw) * ratiow;
							cy = (cy - padh) * ratioh;
							w *= ratiow;
							h *= ratioh;
							float angle = pdata[4];

							//RotatedRect box = RotatedRect(Point2f(cx, cy), Size2f(w, h), angle);
							//cx = 0, cy = 0, w = 50, h = 50, angle = -0.78;
							auto box = xywha2vertex({ cx, cy, w, h, angle });
							vector<Point> ployPoints{ Point(box[0].first,box[0].second),Point(box[1].first,box[1].second),
								Point(box[2].first,box[2].second),Point(box[3].first,box[3].second) };
							RotatedRect angle_box = minAreaRect(ployPoints);

							generate_boxes.push_back(BoxInfo{ angle_box, (float)max_class_socre, class_idx });
						}
					}
					row_ind++;
					pdata += nout;
				}
			}
		}
	}

	// Perform non maximum suppression to eliminate redundant overlapping boxes with
	// lower confidences
	//dnn::NMSBoxesRotated(boxes, confidences, this->confThreshold, this->nmsThreshold, indices);
	nms_angle(generate_boxes);
	personNum = generate_boxes.size();
	/*for (size_t i = 0; i < generate_boxes.size(); ++i)
	{
		RotatedRect rectInput = generate_boxes[i].box;
		Point2f* vertices = new cv::Point2f[4];
		rectInput.points(vertices);
		for (int j = 0; j < 4; j++)
		{
			line(frame, vertices[j], vertices[(j + 1) % 4], Scalar(0, 0, 255), 2);
		}

		int xmin = (int)vertices[0].x;
		int ymin = (int)vertices[0].y - 10;
		string label = format("%.2f", generate_boxes[i].score);
		label = this->class_names[generate_boxes[i].label] + ":" + label;
		putText(frame, label, Point(xmin, ymin - 5), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 255, 0), 2);
	}*/
}