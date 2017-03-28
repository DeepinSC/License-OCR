#include "DetectRegions.h"

//设定文件名
void DetectRegions::setFilename(string s) {
	filename = s;
}

DetectRegions::DetectRegions() {
	showSteps = false;
	saveRegions = false;

}

//调整大小
bool DetectRegions::verifySizes(RotatedRect mr) {
	float error = 0.4;//允许车牌的宽/高比有40%的误差。
	float aspect = 4.7272;//西班牙车牌的宽/高比（52*11）
	//设定最大最小区间，不符合的被舍弃。
	int min = 15 * aspect * 15;//最小面积
	int max = 125 * aspect * 125;//最大面积
	float rmin = aspect*(1 - error);
	float rmax = aspect*(1 + error);

	int area = mr.size.height*mr.size.width;
	float r = (float)mr.size.width / ((float)mr.size.height);
	if (r<1)
		//r = (float)mr.size.height / (float)mr.size.width;//保证比率大于1
		r = (float)mr.size.height / ((float)mr.size.width);
	if ((area < min || area > max) || (r < rmin || r > rmax)) 
		return false;
	else 
		return true;//不符合的舍弃
	
}
Mat DetectRegions::histeq(Mat in) {//均匀化
	Mat out(in.size(), in.type());
	if (in.channels() == 3) {
		Mat hsv;
		vector<Mat>hsvSplit;
		cvtColor(in, hsv, CV_BGR2HSV);//将in的类型从BGR转换成HSV，H色调S饱和度V明度
		split(hsv, hsvSplit);//用split数组将三个通道分离
		equalizeHist(hsvSplit[2], hsvSplit[2]);//直方图均匀化，增强明度
		merge(hsvSplit, hsv);//合并
		cvtColor(hsv, out, CV_HSV2BGR);//转回去
	}
	else if (in.channels() == 1) {
		equalizeHist(in, out);//黑白图像直接输出了
	}
	return out;
}
vector<Plate>DetectRegions::segment(Mat input) {
	vector<Plate> output;
	Mat img_gray;
	cvtColor(input, img_gray, CV_BGR2GRAY);
	blur(img_gray, img_gray, Size(5, 5));//5*5高斯模糊去噪
		
	Mat img_sobel;//使用sobel算子进行边缘检测,寻找竖直边
	Sobel(img_gray, img_sobel,CV_8U, 1, 0, 3, 1, 0, BORDER_DEFAULT);
	if (showSteps) imshow("Sobel", img_sobel);
	
	Mat img_threshold;//阈值，使用otsu（大津法）进行背景分割。
	threshold(img_gray, img_threshold, 0, 255, CV_THRESH_BINARY + CV_THRESH_OTSU);//两种类型结合以忽视threshold函数的第三个参数
	if(showSteps) imshow("OTSU", img_threshold);

	Mat element = getStructuringElement(MORPH_RECT, Size(17, 3));//用17*3的矩形进行形态学闭运算（先膨胀后腐蚀）
	morphologyEx(img_threshold, img_threshold, CV_MOP_CLOSE, element);
	if (showSteps) imshow("Close", img_threshold);

	vector< vector< Point> > contours;
	findContours(img_threshold, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);//把每个边缘放到contours中
	vector<vector<Point>>::iterator itc = contours.begin();//对每个边缘进行检测
	


	vector<RotatedRect> rects;
	while (itc != contours.end()) {
		
		

		RotatedRect mr = minAreaRect(Mat(*itc));
		if (!verifySizes(mr)){
			itc=contours.erase(itc);

			
		}
		else {
			++itc;//
			rects.push_back(mr);
		}
	}
	Mat result;
	input.copyTo(result);
	drawContours(result, contours, -1, Scalar(255, 0, 0), 1);//针对每个边缘画一条蓝色的线
	if (showSteps) {
		imshow("Contours", result);
		waitKey();
	}
	for (int i = 0; i< rects.size(); i++) {
		//接下来进行漫水填充法（因为车牌背景是白色的）获取更精确的车牌矩形
		circle(result, rects[i].center, 3, Scalar(0, 255, 0), -1);
		float minSize = (rects[i].size.width < rects[i].size.height) ? rects[i].size.width : rects[i].size.height;
		minSize = minSize - minSize*0.5;
		//在漫水中心周围随机生成5个点（种子）
		srand(time(NULL));
		//初始化floodFill函数的各种参数值
		Mat mask;
		mask.create(input.rows + 2, input.cols + 2, CV_8UC1);
		mask = Scalar::all(0);
		int loDiff = 30;//最大下界差
		int upDiff = 30;//最大上界差
		int connectivity = 4;//位移值
		int newMaskVal = 255;
		int NumSeeds = 10;//种子个数
		Rect ccomp;
		int flags = connectivity + (newMaskVal << 8) + CV_FLOODFILL_FIXED_RANGE + CV_FLOODFILL_MASK_ONLY;
		//CV_FLOODFILL_FIXED_RANGE设置当前像素和种子像素的差别， CV_FLOODFILL_MASK_ONLY只会掩盖图像掩码而不是图像本身;
		for (int j = 0; j<NumSeeds; j++) {
			Point seed;
			seed.x = rects[i].center.x + rand() % (int)minSize - (minSize / 2);
			seed.y = rects[i].center.y + rand() % (int)minSize - (minSize / 2);
			circle(result, seed, 1, Scalar(0, 255, 255), -1);
			int area = floodFill(input, mask, seed, Scalar(255, 0, 0), &ccomp, Scalar(loDiff, loDiff, loDiff), Scalar(upDiff, upDiff, upDiff), flags);
		}
		if (showSteps)
			imshow("MASK", mask);

		//检查漫水区域是否符合要求
		//对每一个掩码，使用minAreaRect获取最近的剪裁区域
		vector<Point> pointsInterest;
		Mat_<uchar>::iterator itMask = mask.begin<uchar>();
		Mat_<uchar>::iterator end = mask.end<uchar>();
		for (; itMask != end; ++itMask)
			if (*itMask == 255)//如果是白色
				pointsInterest.push_back(itMask.pos());

		RotatedRect minRect = minAreaRect(pointsInterest);

		if (verifySizes(minRect)) {
			Point2f rect_points[4]; minRect.points(rect_points);//找到旋转矩形的点
			for (int j = 0; j < 4; j++)
				line(result, rect_points[j], rect_points[(j + 1) % 4], Scalar(0, 0, 255), 1, 8);//画线

			//获取旋转矩形
			float r = (float)minRect.size.width / (float)minRect.size.height;
			float angle = minRect.angle;
			if (r<1)
				angle = 90 + angle;
			Mat rotmat = getRotationMatrix2D(minRect.center, angle, 1);

			//旋转图像
			Mat img_rotated;
			warpAffine(input, img_rotated, rotmat, input.size(), CV_INTER_CUBIC);//仿射变换获取标准图像

			//剪裁
			Size rect_size = minRect.size;
			if (r < 1)
				swap(rect_size.width, rect_size.height);
			Mat img_crop;
			getRectSubPix(img_rotated, rect_size, minRect.center, img_crop);//从图像中提取像素矩形

			Mat resultResized;
			resultResized.create(33, 144, CV_8UC3);
			resize(img_crop, resultResized, resultResized.size(), 0, 0, INTER_CUBIC);
			//使图像具有相同大小
			Mat grayResult;
			cvtColor(resultResized, grayResult, CV_BGR2GRAY);
			blur(grayResult, grayResult, Size(3, 3));
			grayResult = histeq(grayResult);
			if (saveRegions) {
				stringstream ss(stringstream::in | stringstream::out);
				ss << "tmp/" << filename << "_" << i << ".jpg";
				imwrite(ss.str(), grayResult);
			}
			output.push_back(Plate(grayResult, minRect.boundingRect()));
		}
	}
	if (showSteps){
		imshow("Contours", result);
		waitKey();
	}
	return output;

}
vector<Plate> DetectRegions::run(Mat input) {

	//分割图像
	vector<Plate> tmp = segment(input);

	//返回检测区域
	return tmp;
}
