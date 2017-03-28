#ifndef Plate_h
#define Plate_h

#include <opencv2/opencv.hpp>
#include <ml.h>
#include <string.h>
#include <vector>
#include <highgui.h>
#include <cvaux.h>
#include <cv.h>

using namespace std;
using namespace cv;

class Plate {
public:
	Plate();
	Plate(Mat img, Rect pos);
	string str();
	Rect position;
	Mat plateImg;
	vector<char> chars;
	vector<Rect> charsPos;
};


#endif