#include "DetectRegions.h"

//�趨�ļ���
void DetectRegions::setFilename(string s) {
	filename = s;
}

DetectRegions::DetectRegions() {
	showSteps = false;
	saveRegions = false;

}

//������С
bool DetectRegions::verifySizes(RotatedRect mr) {
	float error = 0.4;//�����ƵĿ�/�߱���40%����
	float aspect = 4.7272;//���������ƵĿ�/�߱ȣ�52*11��
	//�趨�����С���䣬�����ϵı�������
	int min = 15 * aspect * 15;//��С���
	int max = 125 * aspect * 125;//������
	float rmin = aspect*(1 - error);
	float rmax = aspect*(1 + error);

	int area = mr.size.height*mr.size.width;
	float r = (float)mr.size.width / ((float)mr.size.height);
	if (r<1)
		//r = (float)mr.size.height / (float)mr.size.width;//��֤���ʴ���1
		r = (float)mr.size.height / ((float)mr.size.width);
	if ((area < min || area > max) || (r < rmin || r > rmax)) 
		return false;
	else 
		return true;//�����ϵ�����
	
}
Mat DetectRegions::histeq(Mat in) {//���Ȼ�
	Mat out(in.size(), in.type());
	if (in.channels() == 3) {
		Mat hsv;
		vector<Mat>hsvSplit;
		cvtColor(in, hsv, CV_BGR2HSV);//��in�����ʹ�BGRת����HSV��Hɫ��S���Ͷ�V����
		split(hsv, hsvSplit);//��split���齫����ͨ������
		equalizeHist(hsvSplit[2], hsvSplit[2]);//ֱ��ͼ���Ȼ�����ǿ����
		merge(hsvSplit, hsv);//�ϲ�
		cvtColor(hsv, out, CV_HSV2BGR);//ת��ȥ
	}
	else if (in.channels() == 1) {
		equalizeHist(in, out);//�ڰ�ͼ��ֱ�������
	}
	return out;
}
vector<Plate>DetectRegions::segment(Mat input) {
	vector<Plate> output;
	Mat img_gray;
	cvtColor(input, img_gray, CV_BGR2GRAY);
	blur(img_gray, img_gray, Size(5, 5));//5*5��˹ģ��ȥ��
		
	Mat img_sobel;//ʹ��sobel���ӽ��б�Ե���,Ѱ����ֱ��
	Sobel(img_gray, img_sobel,CV_8U, 1, 0, 3, 1, 0, BORDER_DEFAULT);
	if (showSteps) imshow("Sobel", img_sobel);
	
	Mat img_threshold;//��ֵ��ʹ��otsu����򷨣����б����ָ
	threshold(img_gray, img_threshold, 0, 255, CV_THRESH_BINARY + CV_THRESH_OTSU);//�������ͽ���Ժ���threshold�����ĵ���������
	if(showSteps) imshow("OTSU", img_threshold);

	Mat element = getStructuringElement(MORPH_RECT, Size(17, 3));//��17*3�ľ��ν�����̬ѧ�����㣨�����ͺ�ʴ��
	morphologyEx(img_threshold, img_threshold, CV_MOP_CLOSE, element);
	if (showSteps) imshow("Close", img_threshold);

	vector< vector< Point> > contours;
	findContours(img_threshold, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);//��ÿ����Ե�ŵ�contours��
	vector<vector<Point>>::iterator itc = contours.begin();//��ÿ����Ե���м��
	


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
	drawContours(result, contours, -1, Scalar(255, 0, 0), 1);//���ÿ����Ե��һ����ɫ����
	if (showSteps) {
		imshow("Contours", result);
		waitKey();
	}
	for (int i = 0; i< rects.size(); i++) {
		//������������ˮ��䷨����Ϊ���Ʊ����ǰ�ɫ�ģ���ȡ����ȷ�ĳ��ƾ���
		circle(result, rects[i].center, 3, Scalar(0, 255, 0), -1);
		float minSize = (rects[i].size.width < rects[i].size.height) ? rects[i].size.width : rects[i].size.height;
		minSize = minSize - minSize*0.5;
		//����ˮ������Χ�������5���㣨���ӣ�
		srand(time(NULL));
		//��ʼ��floodFill�����ĸ��ֲ���ֵ
		Mat mask;
		mask.create(input.rows + 2, input.cols + 2, CV_8UC1);
		mask = Scalar::all(0);
		int loDiff = 30;//����½��
		int upDiff = 30;//����Ͻ��
		int connectivity = 4;//λ��ֵ
		int newMaskVal = 255;
		int NumSeeds = 10;//���Ӹ���
		Rect ccomp;
		int flags = connectivity + (newMaskVal << 8) + CV_FLOODFILL_FIXED_RANGE + CV_FLOODFILL_MASK_ONLY;
		//CV_FLOODFILL_FIXED_RANGE���õ�ǰ���غ��������صĲ�� CV_FLOODFILL_MASK_ONLYֻ���ڸ�ͼ�����������ͼ����;
		for (int j = 0; j<NumSeeds; j++) {
			Point seed;
			seed.x = rects[i].center.x + rand() % (int)minSize - (minSize / 2);
			seed.y = rects[i].center.y + rand() % (int)minSize - (minSize / 2);
			circle(result, seed, 1, Scalar(0, 255, 255), -1);
			int area = floodFill(input, mask, seed, Scalar(255, 0, 0), &ccomp, Scalar(loDiff, loDiff, loDiff), Scalar(upDiff, upDiff, upDiff), flags);
		}
		if (showSteps)
			imshow("MASK", mask);

		//�����ˮ�����Ƿ����Ҫ��
		//��ÿһ�����룬ʹ��minAreaRect��ȡ����ļ�������
		vector<Point> pointsInterest;
		Mat_<uchar>::iterator itMask = mask.begin<uchar>();
		Mat_<uchar>::iterator end = mask.end<uchar>();
		for (; itMask != end; ++itMask)
			if (*itMask == 255)//����ǰ�ɫ
				pointsInterest.push_back(itMask.pos());

		RotatedRect minRect = minAreaRect(pointsInterest);

		if (verifySizes(minRect)) {
			Point2f rect_points[4]; minRect.points(rect_points);//�ҵ���ת���εĵ�
			for (int j = 0; j < 4; j++)
				line(result, rect_points[j], rect_points[(j + 1) % 4], Scalar(0, 0, 255), 1, 8);//����

			//��ȡ��ת����
			float r = (float)minRect.size.width / (float)minRect.size.height;
			float angle = minRect.angle;
			if (r<1)
				angle = 90 + angle;
			Mat rotmat = getRotationMatrix2D(minRect.center, angle, 1);

			//��תͼ��
			Mat img_rotated;
			warpAffine(input, img_rotated, rotmat, input.size(), CV_INTER_CUBIC);//����任��ȡ��׼ͼ��

			//����
			Size rect_size = minRect.size;
			if (r < 1)
				swap(rect_size.width, rect_size.height);
			Mat img_crop;
			getRectSubPix(img_rotated, rect_size, minRect.center, img_crop);//��ͼ������ȡ���ؾ���

			Mat resultResized;
			resultResized.create(33, 144, CV_8UC3);
			resize(img_crop, resultResized, resultResized.size(), 0, 0, INTER_CUBIC);
			//ʹͼ�������ͬ��С
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

	//�ָ�ͼ��
	vector<Plate> tmp = segment(input);

	//���ؼ������
	return tmp;
}
