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
//����SVM��������ĳ��Ƽ��
//Tips���ó�����ڵĳ�����������������������ĳ��ƣ��ݲ�֧���й����ơ�

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
	cout << "OpenCV �Զ����Ƽ��ϵͳ\n";
	char* filename;
	Mat input_image;

	//�������û�ʹ�����ض�ͼ��
	if (argc >= 2)
	{
		filename = argv[1];
		//ʹ�ûҶ�ͼ��
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
	//�����ܵĳ��Ʊ�Ե
	DetectRegions detectRegions;
	detectRegions.setFilename(filename_whithoutExt);
	detectRegions.saveRegions = false;
	detectRegions.showSteps = false;
	
	
	vector<Plate> posible_regions = detectRegions.run(input_image);

	//SVM
	//��ȡ�ļ��洢��Ϣ
	FileStorage fs;
	fs.open("SVM.xml", FileStorage::READ);
	Mat SVM_TrainingData;
	Mat SVM_Classes;
	fs["TrainingData"] >> SVM_TrainingData;
	fs["classes"] >> SVM_Classes;


	//����SVM����
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
	
	//ѵ��SVM
	svmClassifier->train(tdata);
	//����ÿ�����ܳ��ƣ�SVM�ж��Ƿ�Ϊ���ơ�
	vector<Plate> plates;
	for (int i = 0; i< posible_regions.size(); i++)
	{
		Mat img = posible_regions[i].plateImg;
		Mat p = img.reshape(1, 1);
		p.convertTo(p, CV_32FC1);

		int response = (int)svmClassifier->predict(p);
		if (response == 1)//������㣬����ջ
			plates.push_back(posible_regions[i]);
	}

	cout << "Num plates detected: " << plates.size() << "\n";
	
	//����ÿ�����ƽ���OCR����
	OCR ocr("OCR.xml");
	ocr.saveSegments = true;
	ocr.DEBUG = false;
	ocr.filename = filename_whithoutExt;
	ofstream file;
	file.open("result.txt", ios::out);//���д���ļ�
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
	imwrite("result.jpg", input_image);//��������result.jpg��
	
	
	//waitKey();
	return 0;
}
