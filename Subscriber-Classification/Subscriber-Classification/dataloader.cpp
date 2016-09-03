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

	zmq::context_t context(2);
	zmq::socket_t subscriber(context,ZMQ_SUB);
	subscriber.connect("tcp://localhost:5557");
	subscriber.setsockopt(ZMQ_SUBSCRIBE,"",0);

	zmq::context_t(3);
	zmq::socket_t publisher(context,ZMQ_PUB);
	publisher.bind("tcp://*:5556");

	int file_num=0;     
	char filename[80];

	namedWindow("RoiImage",CV_WINDOW_AUTOSIZE);
	Mat roiImage(224,224,CV_8UC3);

	while(1)
	{
		zmq::message_t roimessage;
		subscriber.recv(&roimessage);
		memcpy(roiImage.data,roimessage.data(),roimessage.size());
		imshow("RoiImage",roiImage);

		int c=waitKey(5);
		if(c=='c')
		{
			sprintf(filename,"image/%i.jpg",file_num+197);
			imwrite(filename,roiImage);
			file_num+=1;
		}

		int k=waitKey(5);
		if(k =='q')
		{
			break;
		}

	}

}











