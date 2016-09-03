#include "zmq.hpp"
#include <windows.h>  
#include <stdio.h>
#include <stdlib.h>
#include <iostream>    
#include <opencv2/opencv.hpp> 
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/nonfree/features2d.hpp"

using namespace std;  
using namespace cv; 

void localization(Mat &colorImage, Mat &depthImage,Mat &roiImage);

int main()
{   
	// Prepare ZMQ context and subscibe to the Kinect data
	zmq::context_t context(1);
	zmq::socket_t subscriber(context,ZMQ_SUB);
	subscriber.connect("tcp://localhost:5559");
	subscriber.setsockopt(ZMQ_SUBSCRIBE,"",0);

	// Prepare ZMQ context to publish location data
	zmq::context_t(2);
	zmq::socket_t publisher(context,ZMQ_PUB);
	publisher.bind("tcp://*:5557");

	//--Initialize visualization information//
	Mat colorImage(480,640,CV_8UC3);					
	Mat depthImage(480,640,CV_8UC1);	
	Mat roiImage(colorImage,Range(200,350),Range(300, 380));	//Range(rows,cols)

	namedWindow("Sub-ColorImage", CV_WINDOW_AUTOSIZE);   
	namedWindow("ROI Region", CV_WINDOW_AUTOSIZE);

	// Program loop
	while(1)                                             
	{  	

		zmq::message_t colormessage;
		zmq::message_t depthmessage;
		subscriber.recv(&colormessage);
		subscriber.recv(&depthmessage);
		memcpy(colorImage.data,colormessage.data(),colormessage.size());
		memcpy(depthImage.data,depthmessage.data(),depthmessage.size());

		// TODO: fill the localization method with 
		localization(colorImage,depthImage,roiImage);

		imshow("Sub-ColorImage",colorImage);
		imshow("ROI Region",roiImage);

		// publish the resulting region of interest (ROI) to all subscribers
		int len1 = roiImage.total()*roiImage.channels();
		zmq::message_t roimessage(len1);
		memcpy(roimessage.data(),roiImage.data,len1);
		bool rc1 = publisher.send(roimessage);

		if ('q' == waitKey(10)) //press "q" to exit
		{                
			break;
		}
	}
	return 0;
}

localization(Mat &colorImage, Mat &depthImage, Mat &roiImage,int Depthlower,int Depthupper)
{
	Mat imgThresholded;	//define a threshold image
	Mat canny_output;

	inRange(depthImage,Scalar(Depthlower),Scalar(Depthupper),imgThresholded); //restrict depthimage in [depthlower,depthupper]

	erode(imgThresholded,imgThresholded,getStructuringElement(MORPH_ELLIPSE,Size(3,3)));//erode the image by MORPH_ELLIPSE kernel with size(3*3)

	dilate(imgThresholded,imgThresholded,getStructuringElement(MORPH_ELLIPSE,Size(3,3)));//dilate the imgae by MORPH_ELLIPSE kernel with size(3*3)

	imshow("ThresholdedImage",imgThresholded); //display threshold image

	imshow("Sub-DepthImage",depthImage);	//display depth image 

	int largest_area=0;				//initialize the largest area
	int largest_contour_index=0;    //initialize the index

	vector<vector<Point>>contours;	//define a vector container for saving the contour vectors

	vector<Vec4i>hierarchy;			//define a hierarchy output array  
	Canny(imgThresholded,canny_output,Depthlower,Depthupper,3); //call canny function for edge dectection

	findContours(canny_output,contours,hierarchy, CV_RETR_EXTERNAL,CV_CHAIN_APPROX_SIMPLE,Point(0,0));//call findContours function for finding contours


	//approxiamate contours to polygans + get bounding rects
	Rect bounding_rect(0,0,-1,-1);//define bounding box

	vector<vector<Point>>contours_poly(contours.size());//define a polygans container
	vector<Rect>boundRect(contours_poly.size());//define a bounding box container


	//writing a loop for searching the largest contour as the detected contour
	for(int i=0;i<contours.size();i++) 
	{ 
		approxPolyDP(Mat(contours[i]),contours_poly[i],3,true);//approxiate contours to polygans

		double a=contourArea(contours_poly[i],false);//calculate the polygans area
		if(a>largest_area)
		{    
			largest_area=a;
			largest_contour_index=i;    //store the index of largest contour

			bounding_rect=boundingRect(contours[i]); //store the largest bounding rect
		}
	}	

	if (bounding_rect.x > 10 && bounding_rect.y > 10 && bounding_rect.height > 0)
	{
		Scalar color =Scalar(0,255,255);       // define the color scalar for drawing the bounding box

		cv::Point pt1=bounding_rect.tl()-cv::Point(10,10); //define the top left coordinate
		cv::Point pt2=bounding_rect.br()+cv::Point(10,10);  //define the bottom right coordinate

		Mat newimg(colorImage, Rect(pt1, pt2));   //region of interest
		resize(newimg,roiImage,Size(224, 224),0,0,INTER_LANCZOS4);   //resize the roi region
		rectangle(colorImage, pt1,pt2,color,0.3,8,0);   //draw the bouding box in colorImage
	}
	else
	{	

		resize(colorImage,roiImage,Size(224, 224),0,0,INTER_LANCZOS4);   //resize the roi region

	}

}
