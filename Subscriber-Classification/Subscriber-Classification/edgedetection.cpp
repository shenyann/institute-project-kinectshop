include"zmq.hpp"
include <windows.h>  
include <stdio.h>
include <stdlib.h>
include <iostream>    
include <opencv2/opencv.hpp> 
include "opencv2/highgui/highgui.hpp"
include "opencv2/imgproc/imgproc.hpp"
include "opencv2/video/background_segm.hpp"
include <vector>
include <fstream>
include <math.h>
using namespace std;
using namespace cv;

Mat canny(Mat gray)
{
	Mat detected_edges;

	int edgeThresh = 1;
	int lowThreshold = 250;
	int highThreshold = 750;
	int kernel_size = 5;
	Canny(gray, detected_edges, lowThreshold, highThreshold, kernel_size);

	return detected_edges;}

