#include"zmq.hpp"
#include <windows.h>  
#include <stdio.h>
#include <stdlib.h>
#include <iostream>    
#include <opencv2/opencv.hpp> 
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/video/background_segm.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2\ml\ml.hpp"
#include "opencv2/opencv_modules.hpp"
#include <vector>
#include <fstream>

using namespace std;
using namespace cv;

Mat canny(Mat src);
Mat classification(Mat roiImage);
int file_num=1;     
char filename[80];
int num_files= 265;

int main()
{		

	CvSVM svm;
	svm.load("training_svm_1.xml");   //load svm classifier



	while(1)
	{
		while(file_num<num_files)
		{		
			sprintf(filename,"image/train (%i).jpg",file_num);
			Mat img=imread(filename,1);
			Mat img_1d=classification(img);
			int x=(int)svm.predict(img_1d);
			printf("Prediction:%i\n",x);
			file_num++; 
		}
		int k=waitKey(10);
		if(k =='q')
		{
			break;
		}

	}

}


Mat classification(Mat roiImage)
{
	Mat channel_2[3];
	split(roiImage,channel_2);
	Mat greenImage=channel_2[2];
	Mat blueImage=channel_2[1];
	Mat redImage=channel_2[0];

	int r_bins=85;
	int b_bins=85;
	int g_bins=85;
	int histSize_r[]={r_bins};
	int histSize_b[]={b_bins};
	int histSize_g[]={g_bins};
	float r_ranges[]={0,255};
	float b_ranges[]={0,255};
	float g_ranges[]={0,255};
	const float* ranges1[]={r_ranges};
	const float* ranges2[]={b_ranges};
	const float* ranges3[]={g_ranges};
	int channels[]={0};
	MatND hist_base_r;
	calcHist(&roiImage,1,channels,Mat(),hist_base_r,1,histSize_r,ranges1,true,false);
	normalize(hist_base_r,hist_base_r,0,255,NORM_MINMAX,-1,Mat());
	int channels2[]={1};
	MatND hist_base_b;
	calcHist(&roiImage,1,channels2,Mat(),hist_base_b,1,histSize_b,ranges2,true,false);
	normalize(hist_base_b,hist_base_b,0,255,NORM_MINMAX,-1,Mat());
	int channels3[]={2};
	MatND hist_base_g;
	calcHist(&roiImage,1,channels3,Mat(),hist_base_g,1,histSize_g,ranges3,true,false);
	normalize(hist_base_g,hist_base_g,0,255,NORM_MINMAX,-1,Mat());
	Mat tmp;
	vconcat(hist_base_r,hist_base_b,tmp);
	Mat hist_base;
	vconcat(tmp,hist_base_g,hist_base);
	transpose(hist_base,hist_base);
	Mat img_1d=hist_base.reshape(1,1);
	img_1d.convertTo(img_1d,CV_32FC1);
	return img_1d;

}



