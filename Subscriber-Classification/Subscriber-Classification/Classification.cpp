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


int main()
{		

	// Prepare ZMQ context to receive a ROI from localization
	zmq::context_t context(2);
	zmq::socket_t subscriber(context,ZMQ_SUB);
	subscriber.connect("tcp://localhost:5557");
	subscriber.setsockopt(ZMQ_SUBSCRIBE,"",0);

	// Preprare another ZMQ context to publish your classification result
	zmq::context_t(3);
	zmq::socket_t publisher(context,ZMQ_PUB);
	publisher.bind("tcp://*:5556");

	RNG rng(12345); // TODO: I am sure there are better ways to identify a product...

	CvSVM svm; //e.g. using svm for classfication
	svm.load("training_svm_1.xml");   //load svm classifier

	namedWindow("RoiImage",CV_WINDOW_AUTOSIZE);
	Mat roiImage(224,224,CV_8UC3);

	// Program loop
	while(1)
	{
		// fetch data from ZMQ transmission
		zmq::message_t roimessage;
		subscriber.recv(&roimessage);
		memcpy(roiImage.data,roimessage.data(),roimessage.size());
		imshow("RoiImage",roiImage);

		int prediction=waitKey(8); // TODO: find a better signal than pressing a key
		if(prediction=='p')
		{

			//TODO:Implement classification based on your algorithm.			//e.g. a simple example
			Mat img_1d = classification(roiImage);
			int x = int(svm).predict(img_1d); // Please do something more sophisticated than this...
			if(x==1)
			{
				Mat image_mat=imread( "cup.jpg",1);
				imshow("cup",image_mat);
			}
			else if(x==2)
			{
				Mat image_mat=imread( "soda_wasser.jpg",1);
				imshow("soda_wasser",image_mat);
			}
			else if(x==3)
			{
				Mat image_mat=imread( "schocolade.jpg",1);
				imshow("schocolade",image_mat);
			}




			// On identification, publish the result
			int flags=0;
			zmq::message_t msg(sizeof(int));
			memcpy(msg.data(), &prediction, sizeof(int));
			publisher.send(msg,flags);

		}

		if ('q' == waitKey(8)){
			break;
		}

	}

}

Mat classification(Mat roiImage)
{
	Mat channel_2[3];	//define a 3-channel matrix for composition

	split(roiImage,channel_2);           //split image

	Mat greenImage=channel_2[2];

	Mat blueImage=channel_2[1];	//composition the image channel
	Mat redImage=channel_2[0];

	int r_bins=85;                    //split bins for histogram calculation
	int b_bins=85;
	int g_bins=85;

	int histSize_r[]={r_bins};				 //hist size	
	int histSize_b[]={b_bins};
	int histSize_g[]={g_bins};

	float r_ranges[]={0,255};
	float b_ranges[]={0,255};               //color range
	float g_ranges[]={0,255};

	const float* ranges1[]={r_ranges};   
	const float* ranges2[]={b_ranges};
	const float* ranges3[]={g_ranges};

	int channels[]={0};
	MatND hist_base_r;
	calcHist(&roiImage,1,channels,Mat(),hist_base_r,1,histSize_r,ranges1,true,false);
	normalize(hist_base_r,hist_base_r,0,255,NORM_MINMAX,-1,Mat());   //normalize the histogram

	int channels2[]={1};
	MatND hist_base_b;
	calcHist(&roiImage,1,channels2,Mat(),hist_base_b,1,histSize_b,ranges2,true,false);
	normalize(hist_base_b,hist_base_b,0,255,NORM_MINMAX,-1,Mat());

	int channels3[]={2};
	MatND hist_base_g;
	calcHist(&roiImage,1,channels3,Mat(),hist_base_g,1,histSize_g,ranges3,true,false);
	normalize(hist_base_g,hist_base_g,0,255,NORM_MINMAX,-1,Mat());

	Mat tmp;         //define a tmp matrix for concatanate rbg histogram
	vconcat(hist_base_r,hist_base_b,tmp);

	Mat hist_base;
	vconcat(tmp,hist_base_g,hist_base);
	transpose(hist_base,hist_base);   //make a size=(1,255) vector per image

	Mat img_1d=hist_base.reshape(1,1);
	img_1d.convertTo(img_1d,CV_32FC1);
	return img_1d;

}


