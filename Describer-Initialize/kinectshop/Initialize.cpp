#include "zmq.hpp"
#include <windows.h>  
#include <iostream>   
#include <NuiApi.h>  
#include <opencv2/opencv.hpp>  

using namespace std;  
using namespace cv;  

bool tracked[NUI_SKELETON_COUNT]={FALSE};   
CvPoint skeletonPoint[NUI_SKELETON_COUNT][NUI_SKELETON_POSITION_COUNT]={cvPoint(0,0)};   
CvPoint colorPoint[NUI_SKELETON_COUNT][NUI_SKELETON_POSITION_COUNT]={cvPoint(0,0)};   

void getColorImage(HANDLE &colorEvent, HANDLE &colorStreamHandle, Mat &colorImage);   
void getDepthImage(HANDLE &depthEvent, HANDLE &depthStreamHandle, Mat &depthImage);   
void getSkeletonImage(HANDLE &skeletonEvent, Mat &skeletonImage, Mat &colorImage, Mat &depthImage);   
void drawSkeleton(Mat &image, CvPoint pointSet[], int whichone);   

int main(int argc, char *argv[])  
{  
	// Prepare OpenCV structures for image data
	Mat colorImage;  // RGB image
	colorImage.create(480, 640, CV_8UC3);   
	Mat depthImage;  // depth image
	depthImage.create(480, 640, CV_8UC1);   
	Mat skeletonImage;  // image the skeleton will be painted on
	skeletonImage.create(480, 640, CV_8UC3);   

	// Prepare ZMQ process communication
	zmq::context_t context(1);
	zmq::socket_t publisher(context, ZMQ_PUB);
	publisher.bind("tcp://*:5559"); // publish on the localhost, port 5559

	// Prepare Kinect handling
	HANDLE colorEvent = CreateEvent( NULL, TRUE, FALSE, NULL );   
	HANDLE depthEvent = CreateEvent( NULL, TRUE, FALSE, NULL );   
	HANDLE skeletonEvent = CreateEvent( NULL, TRUE, FALSE, NULL );   

	HANDLE colorStreamHandle = NULL;   
	HANDLE depthStreamHandle = NULL;   

	HRESULT hr = NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR | NUI_INITIALIZE_FLAG_USES_DEPTH | NUI_INITIALIZE_FLAG_USES_SKELETON);     
	if( hr != S_OK )     
	{     
		cout<<"NuiInitialize failed"<<endl;     
		return hr;     
	}   

	hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480, NULL, 4, colorEvent, &colorStreamHandle);   
	if( hr != S_OK )     
	{     
		cout<<"Open the color Stream failed"<<endl;   
		NuiShutdown();   
		return hr;     
	}   
	hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH, NUI_IMAGE_RESOLUTION_640x480, NULL, 2, depthEvent, &depthStreamHandle);   
	if( hr != S_OK )     
	{     
		cout<<"Open the depth Stream failed"<<endl;   
		NuiShutdown();   
		return hr;     
	}   
	hr = NuiSkeletonTrackingEnable( skeletonEvent, 0 ); 
	if( hr != S_OK )     
	{     
		cout << "NuiSkeletonTrackingEnable failed" << endl;     
		NuiShutdown();   
		return hr;     
	}   

	// Preview for the image data
	namedWindow("colorImage", CV_WINDOW_AUTOSIZE);  
	namedWindow("depthImage", CV_WINDOW_AUTOSIZE);  
	namedWindow("skeletonImage", CV_WINDOW_AUTOSIZE);  

	// Run programm loop
	// in essence this method fetches images from the kinect and streams them to subscribing appilcations

	while (1)   
	{   
		if(WaitForSingleObject(colorEvent, 0)==0)   
			getColorImage(colorEvent, colorStreamHandle, colorImage);   
		if(WaitForSingleObject(depthEvent, 0)==0)   
			getDepthImage(depthEvent, depthStreamHandle, depthImage);          
		if(WaitForSingleObject(skeletonEvent, INFINITE)==0)  
			getSkeletonImage(skeletonEvent, skeletonImage, colorImage, depthImage);   

		// comment these lines to improve the speed --
		imshow("colorImage", colorImage);   
		imshow("depthImage", depthImage);   
		imshow("skeletonImage", skeletonImage);
		// --up till this line

		int len1 = colorImage.total()*colorImage.channels();
		int len2 = depthImage.total()*depthImage.channels();

		zmq::message_t colormessage(len1);
		zmq::message_t depthmessage(len2);

		memcpy(colormessage.data(),colorImage.data,len1);
		memcpy(depthmessage.data(),depthImage.data,len2);

		bool rc1 = publisher.send (colormessage);
		bool rc2 = publisher.send (depthmessage);

		if(cvWaitKey(1)==27)   // ESC ends the Kinect driver
			break;   
	}   

	NuiShutdown();   
	return 0;   
}  

// converts the Kinect rgb stream into an image
void getColorImage(HANDLE &colorEvent, HANDLE &colorStreamHandle, Mat &colorImage)   
{   
	const NUI_IMAGE_FRAME *colorFrame = NULL;   

	NuiImageStreamGetNextFrame(colorStreamHandle, 0, &colorFrame);   
	INuiFrameTexture *pTexture = colorFrame->pFrameTexture;     

	NUI_LOCKED_RECT LockedRect;   
	pTexture->LockRect(0, &LockedRect, NULL, 0);     

	if( LockedRect.Pitch != 0 )   
	{   
		for (int i=0; i<colorImage.rows; i++)   
		{  
			uchar *ptr = colorImage.ptr<uchar>(i);             

			uchar *pBuffer = (uchar*)(LockedRect.pBits) + i * LockedRect.Pitch;  
			for (int j=0; j<colorImage.cols; j++)   
			{   
				ptr[3*j] = pBuffer[4*j];   
				ptr[3*j+1] = pBuffer[4*j+1];   
				ptr[3*j+2] = pBuffer[4*j+2];   
			}   
		}   
	}   
	else   
	{   
		cout<<"Error in Tracking color Image!"<<endl;   
	}  

	pTexture->UnlockRect(0);   
	NuiImageStreamReleaseFrame(colorStreamHandle, colorFrame );  
}   

// converts the Kinect depth stream into an image
void getDepthImage(HANDLE &depthEvent, HANDLE &depthStreamHandle, Mat &depthImage)   
{   
	const NUI_IMAGE_FRAME *depthFrame = NULL;   

	NuiImageStreamGetNextFrame(depthStreamHandle, 0, &depthFrame);   
	INuiFrameTexture *pTexture = depthFrame->pFrameTexture;     

	NUI_LOCKED_RECT LockedRect;   
	pTexture->LockRect(0, &LockedRect, NULL, 0);     

	//RGBQUAD q;   

	if( LockedRect.Pitch != 0 )   
	{   
		/*for (int i=0; i<depthImage.rows; i++)   
		  {   
		  uchar *ptr = depthImage.ptr<uchar>(i);   
		  uchar *pBuffer = (uchar*)(LockedRect.pBits) + i * LockedRect.Pitch;
		  USHORT *pBufferRun = (USHORT*) pBuffer;   

		  for (int j=0; j<depthImage.cols; j++)   
		  {   
		  ptr[j]=(uchar)((pBufferRun[j]%0x00ff));
		  }   
		  } */ 

		const USHORT* current = (const USHORT*)LockedRect.pBits;
		const USHORT* dataEnd = current + (640 * 480);
		uchar* depthPtr = depthImage.data;
		while (current < dataEnd) {
			USHORT depth = NuiDepthPixelToDepth(*current++);
			*depthPtr++ = (uchar) ((depth < 2048) ? 255*(depth / 8) : 0);
		}
	}   
	else   
	{   
		cout << "Error in Tracking depth Image!" << endl;   
	}   


	pTexture->UnlockRect(0);  
	NuiImageStreamReleaseFrame(depthStreamHandle, depthFrame);    
}   

// converts the Kinect skeleton stream into an image
void getSkeletonImage(HANDLE &skeletonEvent, Mat &skeletonImage, Mat &colorImage, Mat &depthImage)   
{   



	NUI_SKELETON_FRAME skeletonFrame = {0};  
	bool bFoundSkeleton = false;    

	if(NuiSkeletonGetNextFrame( 0, &skeletonFrame ) == S_OK )     
	{     
		for( int i = 0 ; i < NUI_SKELETON_COUNT ; i++ )     
		{     
			if( skeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED )   
			{     
				bFoundSkeleton = true;     
				break;   
			}     
		}     
	}   
	else   
	{   
		cout << "not find suitable skeleton frame!" << endl;   
		return;    
	}   

	if( !bFoundSkeleton )     
	{     
		return;    
	}     

	NuiTransformSmooth(&skeletonFrame, NULL);     
	skeletonImage.setTo(0);     

	for( int i = 0 ; i < NUI_SKELETON_COUNT ; i++ )     
	{     
		if( skeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED &&     
				skeletonFrame.SkeletonData[i].eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_SHOULDER_CENTER] != NUI_SKELETON_POSITION_NOT_TRACKED)     
		{     
			float fx, fy;     

			for (int j = 0; j < NUI_SKELETON_POSITION_COUNT; j++)
			{     
				NuiTransformSkeletonToDepthImage(skeletonFrame.SkeletonData[i].SkeletonPositions[j], &fx, &fy );     
				skeletonPoint[i][j].x = (int)fx;     
				skeletonPoint[i][j].y = (int)fy;  

			}     

			for (int j=0; j<NUI_SKELETON_POSITION_COUNT ; j++)     
			{     
				if (skeletonFrame.SkeletonData[i].eSkeletonPositionTrackingState[j] != NUI_SKELETON_POSITION_NOT_TRACKED)
				{     
					LONG colorx, colory;   
					NuiImageGetColorPixelCoordinatesFromDepthPixel(NUI_IMAGE_RESOLUTION_640x480, 0,    
							skeletonPoint[i][j].x, skeletonPoint[i][j].y, 0,&colorx, &colory);   
					colorPoint[i][j].x = int(colorx);  
					colorPoint[i][j].y = int(colory);  
					//     circle(colorImage, colorPoint[i][j], 4, cvScalar(0, 255, 255), 1, 8, 0);   
					circle(skeletonImage, skeletonPoint[i][j], 3, cvScalar(0, 255, 255), 1, 8, 0);   

					tracked[i] = TRUE;   
				}   
			}   

			// drawSkeleton(colorImage, colorPoint[i], i);    
			drawSkeleton(skeletonImage, skeletonPoint[i], i);  





		}   
	}     
}   

// helper function to draw the skeleton data
void drawSkeleton(Mat &image, CvPoint pointSet[], int whichone)   
{   
	CvScalar color;   
	switch(whichone)  
	{   
		case 0:   
			color = cvScalar(255);   
			break;   
		case 1:   
			color = cvScalar(0,255);   
			break;   
		case 2:   
			color = cvScalar(0, 0, 255);   
			break;   
		case 3:   
			color = cvScalar(255, 255, 0);   
			break;   
		case 4:   
			color = cvScalar(255, 0, 255);   
			break;   
		case 5:   
			color = cvScalar(0, 255, 255);   
			break;   
	}   

	if((pointSet[NUI_SKELETON_POSITION_HEAD].x!=0 || pointSet[NUI_SKELETON_POSITION_HEAD].y!=0) &&   
			(pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].x!=0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].y!=0))   
		line(image, pointSet[NUI_SKELETON_POSITION_HEAD], pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER], color, 2);   
	if((pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].x!=0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].y!=0) &&   
			(pointSet[NUI_SKELETON_POSITION_SPINE].x!=0 || pointSet[NUI_SKELETON_POSITION_SPINE].y!=0))   
		line(image, pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER], pointSet[NUI_SKELETON_POSITION_SPINE], color, 2);   
	if((pointSet[NUI_SKELETON_POSITION_SPINE].x!=0 || pointSet[NUI_SKELETON_POSITION_SPINE].y!=0) &&   
			(pointSet[NUI_SKELETON_POSITION_HIP_CENTER].x!=0 || pointSet[NUI_SKELETON_POSITION_HIP_CENTER].y!=0))   
		line(image, pointSet[NUI_SKELETON_POSITION_SPINE], pointSet[NUI_SKELETON_POSITION_HIP_CENTER], color, 2);   


	if((pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].x!=0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].y!=0) &&   
			(pointSet[NUI_SKELETON_POSITION_SHOULDER_LEFT].x!=0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_LEFT].y!=0))   
		line(image, pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER], pointSet[NUI_SKELETON_POSITION_SHOULDER_LEFT], color, 2);   
	if((pointSet[NUI_SKELETON_POSITION_SHOULDER_LEFT].x!=0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_LEFT].y!=0) &&   
			(pointSet[NUI_SKELETON_POSITION_ELBOW_LEFT].x!=0 || pointSet[NUI_SKELETON_POSITION_ELBOW_LEFT].y!=0))   
		line(image, pointSet[NUI_SKELETON_POSITION_SHOULDER_LEFT], pointSet[NUI_SKELETON_POSITION_ELBOW_LEFT], color, 2);   
	if((pointSet[NUI_SKELETON_POSITION_ELBOW_LEFT].x!=0 || pointSet[NUI_SKELETON_POSITION_ELBOW_LEFT].y!=0) &&   
			(pointSet[NUI_SKELETON_POSITION_WRIST_LEFT].x!=0 || pointSet[NUI_SKELETON_POSITION_WRIST_LEFT].y!=0))   
		line(image, pointSet[NUI_SKELETON_POSITION_ELBOW_LEFT], pointSet[NUI_SKELETON_POSITION_WRIST_LEFT], color, 2);   
	if((pointSet[NUI_SKELETON_POSITION_WRIST_LEFT].x!=0 || pointSet[NUI_SKELETON_POSITION_WRIST_LEFT].y!=0) &&   
			(pointSet[NUI_SKELETON_POSITION_HAND_LEFT].x!=0 || pointSet[NUI_SKELETON_POSITION_HAND_LEFT].y!=0))   
		line(image, pointSet[NUI_SKELETON_POSITION_WRIST_LEFT], pointSet[NUI_SKELETON_POSITION_HAND_LEFT], color, 2);   


	if((pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].x!=0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].y!=0) &&   
			(pointSet[NUI_SKELETON_POSITION_SHOULDER_RIGHT].x!=0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_RIGHT].y!=0))   
		line(image, pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER], pointSet[NUI_SKELETON_POSITION_SHOULDER_RIGHT], color, 2);   
	if((pointSet[NUI_SKELETON_POSITION_SHOULDER_RIGHT].x!=0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_RIGHT].y!=0) &&   
			(pointSet[NUI_SKELETON_POSITION_ELBOW_RIGHT].x!=0 || pointSet[NUI_SKELETON_POSITION_ELBOW_RIGHT].y!=0))   
		line(image, pointSet[NUI_SKELETON_POSITION_SHOULDER_RIGHT], pointSet[NUI_SKELETON_POSITION_ELBOW_RIGHT], color, 2);   
	if((pointSet[NUI_SKELETON_POSITION_ELBOW_RIGHT].x!=0 || pointSet[NUI_SKELETON_POSITION_ELBOW_RIGHT].y!=0) &&   
			(pointSet[NUI_SKELETON_POSITION_WRIST_RIGHT].x!=0 || pointSet[NUI_SKELETON_POSITION_WRIST_RIGHT].y!=0))   
		line(image, pointSet[NUI_SKELETON_POSITION_ELBOW_RIGHT], pointSet[NUI_SKELETON_POSITION_WRIST_RIGHT], color, 2);   
	if((pointSet[NUI_SKELETON_POSITION_WRIST_RIGHT].x!=0 || pointSet[NUI_SKELETON_POSITION_WRIST_RIGHT].y!=0) &&   
			(pointSet[NUI_SKELETON_POSITION_HAND_RIGHT].x!=0 || pointSet[NUI_SKELETON_POSITION_HAND_RIGHT].y!=0))   
		line(image, pointSet[NUI_SKELETON_POSITION_WRIST_RIGHT], pointSet[NUI_SKELETON_POSITION_HAND_RIGHT], color, 2);   


	if((pointSet[NUI_SKELETON_POSITION_HIP_CENTER].x!=0 || pointSet[NUI_SKELETON_POSITION_HIP_CENTER].y!=0) &&   
			(pointSet[NUI_SKELETON_POSITION_HIP_LEFT].x!=0 || pointSet[NUI_SKELETON_POSITION_HIP_LEFT].y!=0))   
		line(image, pointSet[NUI_SKELETON_POSITION_HIP_CENTER], pointSet[NUI_SKELETON_POSITION_HIP_LEFT], color, 2);   
	if((pointSet[NUI_SKELETON_POSITION_HIP_LEFT].x!=0 || pointSet[NUI_SKELETON_POSITION_HIP_LEFT].y!=0) &&   
			(pointSet[NUI_SKELETON_POSITION_KNEE_LEFT].x!=0 || pointSet[NUI_SKELETON_POSITION_KNEE_LEFT].y!=0))   
		line(image, pointSet[NUI_SKELETON_POSITION_HIP_LEFT], pointSet[NUI_SKELETON_POSITION_KNEE_LEFT], color, 2);   
	if((pointSet[NUI_SKELETON_POSITION_KNEE_LEFT].x!=0 || pointSet[NUI_SKELETON_POSITION_KNEE_LEFT].y!=0) &&   
			(pointSet[NUI_SKELETON_POSITION_ANKLE_LEFT].x!=0 || pointSet[NUI_SKELETON_POSITION_ANKLE_LEFT].y!=0))   
		line(image, pointSet[NUI_SKELETON_POSITION_KNEE_LEFT], pointSet[NUI_SKELETON_POSITION_ANKLE_LEFT], color, 2);   
	if((pointSet[NUI_SKELETON_POSITION_ANKLE_LEFT].x!=0 || pointSet[NUI_SKELETON_POSITION_ANKLE_LEFT].y!=0) &&    
			(pointSet[NUI_SKELETON_POSITION_FOOT_LEFT].x!=0 || pointSet[NUI_SKELETON_POSITION_FOOT_LEFT].y!=0))   
		line(image, pointSet[NUI_SKELETON_POSITION_ANKLE_LEFT], pointSet[NUI_SKELETON_POSITION_FOOT_LEFT], color, 2);   

	if((pointSet[NUI_SKELETON_POSITION_HIP_CENTER].x!=0 || pointSet[NUI_SKELETON_POSITION_HIP_CENTER].y!=0) &&   
			(pointSet[NUI_SKELETON_POSITION_HIP_RIGHT].x!=0 || pointSet[NUI_SKELETON_POSITION_HIP_RIGHT].y!=0))   
		line(image, pointSet[NUI_SKELETON_POSITION_HIP_CENTER], pointSet[NUI_SKELETON_POSITION_HIP_RIGHT], color, 2);   
	if((pointSet[NUI_SKELETON_POSITION_HIP_RIGHT].x!=0 || pointSet[NUI_SKELETON_POSITION_HIP_RIGHT].y!=0) &&   
			(pointSet[NUI_SKELETON_POSITION_KNEE_RIGHT].x!=0 || pointSet[NUI_SKELETON_POSITION_KNEE_RIGHT].y!=0))   
		line(image, pointSet[NUI_SKELETON_POSITION_HIP_RIGHT], pointSet[NUI_SKELETON_POSITION_KNEE_RIGHT],color, 2);   
	if((pointSet[NUI_SKELETON_POSITION_KNEE_RIGHT].x!=0 || pointSet[NUI_SKELETON_POSITION_KNEE_RIGHT].y!=0) &&   
			(pointSet[NUI_SKELETON_POSITION_ANKLE_RIGHT].x!=0 || pointSet[NUI_SKELETON_POSITION_ANKLE_RIGHT].y!=0))   
		line(image, pointSet[NUI_SKELETON_POSITION_KNEE_RIGHT], pointSet[NUI_SKELETON_POSITION_ANKLE_RIGHT], color, 2);   
	if((pointSet[NUI_SKELETON_POSITION_ANKLE_RIGHT].x!=0 || pointSet[NUI_SKELETON_POSITION_ANKLE_RIGHT].y!=0) &&   
			(pointSet[NUI_SKELETON_POSITION_FOOT_RIGHT].x!=0 || pointSet[NUI_SKELETON_POSITION_FOOT_RIGHT].y!=0))   
		line(image, pointSet[NUI_SKELETON_POSITION_ANKLE_RIGHT], pointSet[NUI_SKELETON_POSITION_FOOT_RIGHT], color, 2);   
}
