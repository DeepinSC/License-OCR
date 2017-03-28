#include <opencv2/opencv.hpp>
#include <ml.h>
#include <string.h>
#include <vector>
#include <highgui.h>
#include <cvaux.h>
#include <cv.h>
#include <fstream>

#include "DetectRegions.h"
#include "OCR.h"


using namespace cv;
using namespace std;
//基于SVM和神经网络的车牌检测
//Tips：该程序基于的车牌数据是外国（西班牙）的车牌，暂不支持中国车牌。

string getFilename(string s) {

	char sep = '/';
	char sepExt = '.';

#ifdef _WIN32
	sep = '\\';
#endif

	size_t i = s.rfind(sep, s.length());
	if (i != string::npos) {
		string fn = (s.substr(i + 1, s.length() - i));
		size_t j = fn.rfind(sepExt, fn.length());
		if (i != string::npos) {
			return fn.substr(0, j);
		}
		else {
			return fn;
		}
	}
	else {
		return "";
	}
}

int main(int argc, char** argv)
{
	cout << "OpenCV 自动车牌检测系统\n";
	char* filename;
	Mat input_image;

	//检查如果用户使用了特定图像
	if (argc >= 2)
	{
		filename = argv[1];
		//使用灰度图像
		input_image = imread(filename, 1);
	}
	else {
		printf("use:\n\t%s image\n", argv[0]);
		printf("no available img found");
		return 0;
	}
	//filename ="ss.jpg";
	//input_image = imread(filename,1);
	//waitKey(30);
	imshow("Original Image",input_image);
	string filename_whithoutExt = getFilename(filename);
	cout << "working with file: " << filename_whithoutExt << "\n";
	//检测可能的车牌边缘
	DetectRegions detectRegions;
	detectRegions.setFilename(filename_whithoutExt);
	detectRegions.saveRegions = false;
	detectRegions.showSteps = false;
	
	
	vector<Plate> posible_regions = detectRegions.run(input_image);

	//SVM
	//读取文件存储信息
	FileStorage fs;
	fs.open("SVM.xml", FileStorage::READ);
	Mat SVM_TrainingData;
	Mat SVM_Classes;
	fs["TrainingData"] >> SVM_TrainingData;
	fs["classes"] >> SVM_Classes;


	//设置SVM参数
	Ptr<SVM> svmClassifier = SVM::create();
	svmClassifier->setType(SVM::C_SVC);
	svmClassifier->setKernel(SVM::LINEAR);
	svmClassifier->setDegree(0);
	svmClassifier->setGamma(1);
	svmClassifier->setCoef0(0);
	svmClassifier->setC(1);
	svmClassifier->setNu(0);
	svmClassifier->setP(0);
	TermCriteria term= TermCriteria(CV_TERMCRIT_ITER,1000,0.01);
	svmClassifier->setTermCriteria(term);
	Ptr<TrainData>tdata = TrainData::create(SVM_TrainingData, ROW_SAMPLE,SVM_Classes);
	
	//训练SVM
	svmClassifier->train(tdata);
	//对于每个可能车牌，SVM判断是否为车牌。
	vector<Plate> plates;
	for (int i = 0; i< posible_regions.size(); i++)
	{
		Mat img = posible_regions[i].plateImg;
		Mat p = img.reshape(1, 1);
		p.convertTo(p, CV_32FC1);

		int response = (int)svmClassifier->predict(p);
		if (response == 1)//如果满足，则入栈
			plates.push_back(posible_regions[i]);
	}

	cout << "Num plates detected: " << plates.size() << "\n";
	
	//对于每个车牌进行OCR处理
	OCR ocr("OCR.xml");
	ocr.saveSegments = true;
	ocr.DEBUG = false;
	ocr.filename = filename_whithoutExt;
	ofstream file;
	file.open("result.txt", ios::out);//结果写入文件
	for (int i = 0; i< plates.size(); i++){
		Plate plate = plates[i];

		string plateNumber = ocr.run(&plate);
		string licensePlate = plate.str();
		cout << "================================================\n";
		cout << "License plate number: " << licensePlate << "\n";
		cout << "================================================\n";
		rectangle(input_image, plate.position, Scalar(0, 0, 200));
		putText(input_image, licensePlate, Point(plate.position.x, plate.position.y), CV_FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 200), 2);
		if (false){
			imshow("Plate Detected seg", plate.plateImg);
			waitKey();
		}
		file << licensePlate << endl;

	}
	file.close();
	imshow("Plate Detected", input_image);
	imwrite("result.jpg", input_image);//保存结果到result.jpg中
	
	
	//waitKey();
	return 0;
}
